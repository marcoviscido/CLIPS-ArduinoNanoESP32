/**
 * CLIPS-ArduinoNanoESP32... a porting of CLIPS to Arduino Nano ESP32.
 *
 * The CLIPS Reference Manual states:
 *  "CLIPS programs may be executed in three ways: interactively using a simple
 *   Read-Eval-Print Loop (REPL) interface; interactively using an Integrated
 *   Development Environment (IDE) interface; or as embedded application in
 *   which the user provides a main program and controls execution of the expert
 *   system through the CLIPS Application Programming Interface (API)."
 *
 * This project also consists of mapping the Read-Eval-Print Loop with the
 * standard loop operation of Arduino and integrating "facts" with states/levels
 * of its pins... and much more. This enables endless application possibilities in
 * the field of IoT and automation, as well as benefits such as code portability.
 * COOL coding!
 *
 *********************************************************************************
 *
 * MIT License
 *
 * Copyright (c) 2025 Marco Viscido
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Arduino.h"

#include "clips.h"
#include "clips_utils.h"
#include "clips_digital_io.h"
#include "clips_wifi.h"

#include "main.h"

#include "WiFi.h"         // TODO: https://github.com/espressif/esp-idf/blob/v5.4/examples/protocols/sntp/README.md
#include "PubSubClient.h" // TODO: https://www.emqx.com/en/blog/esp32-connects-to-the-free-public-mqtt-broker

#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include "ArduinoJson-v7.3.0.h"

static WiFiClient espClient;             // WiFiClient
static PubSubClient psClient(espClient); // PubSubClient

static bool stringComplete = false;
static bool stringInEdit = false;
static bool callbackRunning = false;
static Environment *mainEnv;

static bool QueryTraceCallback(
    Environment *environment,
    const char *logicalName,
    void *context)
{
  if ((strcmp(logicalName, STDOUT) == 0) ||
      (strcmp(logicalName, STDIN) == 0) ||
      (strcmp(logicalName, STDERR) == 0) ||
      (strcmp(logicalName, STDWRN) == 0))
  {
    return true;
  }
  return false;
}

static void WriteTraceCallback(
    Environment *environment,
    const char *logicalName,
    const char *str,
    void *context)
{
  if ((strcmp(logicalName, STDOUT) == 0) ||
      (strcmp(logicalName, STDIN) == 0) ||
      (strcmp(logicalName, STDERR) == 0) ||
      (strcmp(logicalName, STDWRN) == 0))
  {
    if (!str)
    {
      ESP_LOGE("WriteTraceCallback", "Error: Null str pointer!");
      return;
    }

    Serial.print(str);
  }
}

void setup()
{
  ESP_LOGI("Setup", "Starting CLIPS-ArduinoNanoESP32...");
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_RED, HIGH);   // reverse logic
  digitalWrite(LED_GREEN, HIGH); // reverse logic
  digitalWrite(LED_BLUE, HIGH);  // reverse logic
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(11520);
  Serial.setTimeout(2000);

  while (!Serial)
  {
    delay(100);
    ESP_LOGI("Setup", "Waiting Serial...");
  }

  if (!psramFound())
  {
    ESP_LOGE("Setup", "PSRAM not available!");
    delay(2000);
    return;
  }

#if DEBUGGING_FUNCTIONS
// Serial.println("CPU0 reset reason:");
// print_reset_reason(rtc_get_reset_reason(0));
// verbose_print_reset_reason(rtc_get_reset_reason(0));

// Serial.println("CPU1 reset reason:");
// print_reset_reason(rtc_get_reset_reason(1));
// verbose_print_reset_reason(rtc_get_reset_reason(1));
#endif

  mainEnv = CreateEnvironment();
  EnablePeriodicFunctions(mainEnv, true);

  // TODO: task wdt CPU1 disabled!!!
  // UtilityData(mainEnv)->YieldTimeFunction = yield; // esp32-hal.h
  // EnableYieldFunction(mainEnv, true);

  AddRouter(mainEnv,
            "trace",            /* Router name */
            40,                 /* Priority */
            QueryTraceCallback, /* Query function */
            WriteTraceCallback, /* Write function */
            NULL,               /* Read function */
            NULL,               /* Unread function */
            NULL,               /* Exit function */
            NULL);              /* Context */

  ActivateRouter(
      mainEnv,
      "trace");

  WriteString(mainEnv, STDOUT, CommandLineData(mainEnv)->BannerString);
  SetHaltExecution(mainEnv, false);
  SetEvaluationError(mainEnv, false);

  CleanCurrentGarbageFrame(mainEnv, NULL);
  CallPeriodicTasks(mainEnv);

  RouterData(mainEnv)->CommandBufferInputCount = 0;
  RouterData(mainEnv)->InputUngets = 0;
  RouterData(mainEnv)->AwaitingInput = true;

  AddStartingFunction(mainEnv, "arduino-init", ArduninoInitFunction, 2010, NULL);

  Reset(mainEnv);
  Writeln(mainEnv, "");
  PrintPrompt(mainEnv);
}

void loop()
{
  while (Serial.available())
  {
    stringInEdit = true;
    char inChar = (char)Serial.read();
    AppendNCommandString(mainEnv, &inChar, 1);

    if (inChar == '\r')
    {
      stringComplete = true;
    }
  }

  if (stringComplete)
  {
    stringComplete = false;

#if DEBUGGING_FUNCTIONS
    Serial.println(GetCommandString(mainEnv)); // debug
#endif

    /*
     * True if a complete command exists, then 1 is returned.
     * False is returned if the command was not complete and without errors or
     * if the command contains an error.
     */
    ExecuteIfCommandComplete(mainEnv);
    stringInEdit = false;
  }

  if (callbackRunning == false)
  {
    psClient.loop();
  }
}

void ArduninoInitFunction(Environment *theEnv, void *context)
{
  AddUDFError addUDFError = AddUDFError::AUE_NO_ERROR;
  addUDFError = AddUDFIfNotExists(theEnv, "digital-read", "y", 1, 1, ";iny", DigitalReadFunction, "DigitalReadFunction", NULL);
  if (addUDFError != AddUDFError::AUE_NO_ERROR)
  {
    return;
  }
  addUDFError = AddUDFIfNotExists(theEnv, "digital-write", "v", 2, 2, ";iny;y", DigitalWriteFunction, "DigitalWriteFunction", NULL);
  if (addUDFError != AddUDFError::AUE_NO_ERROR)
  {
    return;
  }
  addUDFError = AddUDFIfNotExists(theEnv, "pin-mode", "iv", 2, 2, ";y;y", PinModeFunction, "PinModeFunction", NULL);
  if (addUDFError != AddUDFError::AUE_NO_ERROR)
  {
    return;
  }
  addUDFError = AddUDFIfNotExists(theEnv, "pin-reset", "iv", 1, 1, ";iny", PinResetFunction, "PinResetFunction", NULL);
  if (addUDFError != AddUDFError::AUE_NO_ERROR)
  {
    return;
  }
  addUDFError = AddUDFIfNotExists(theEnv, "wifi-begin", "vs", 1, 2, ";ns;s", WifiBeginFunction, "WifiBeginFunction", NULL);
  addUDFError = AddUDFIfNotExists(theEnv, "wifi-status", "v", 0, 0, "*", WifiStatusFunction, "WifiStatusFunction", NULL);
  addUDFError = AddUDFIfNotExists(theEnv, "wifi-disconnect", "v", 0, 0, "*", WifiDisconnectFunction, "WifiDisconnectFunction", NULL);
  addUDFError = AddUDFIfNotExists(theEnv, "wifi-scan", "v", 0, 0, "*", WifiScanNetworksFunction, "WifiScanNetworksFunction", NULL);

  addUDFError = AddUDFIfNotExists(theEnv, "mqtt-connect", "vs", 1, 1, ";n", MqttConnectFunction, "MqttConnectFunction", NULL);
  // addUDFError = AddUDFIfNotExists(theEnv, "mqtt-send", "v", 0, 0, "*", MqttSendFunction, "MqttSendFunction", NULL);

  BuildError buildError = BuildError::BE_NO_ERROR;
  if (FindDefclass(theEnv, "WIFI") == NULL)
  {
    buildError = Build(theEnv, "(defclass WIFI \"A class to store WIFI credentials.\" (is-a USER)"
                               "   (slot ssid (access read-write) (type STRING))"
                               "   (slot pwd (access read-write) (type STRING))"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      ESP_LOGE("ArduninoInitFunction", "Error adding %s - %i", "defclass-WIFI", (int8_t)buildError);
      return;
    }
  }
  if (FindDefclass(theEnv, "MQTT") == NULL)
  {
    buildError = Build(theEnv, "(defclass MQTT \"A class to store MQTT connection's info.\" (is-a USER)"
                               "   (slot broker (access read-write) (type STRING))"
                               "   (slot port (access read-write) (type INTEGER)(default -1))"
                               "   (slot clientid (access read-only) (type STRING))"
                               "   (slot usr (access read-write) (type STRING))"
                               "   (slot pwd (access read-write) (type STRING))"
                               "   (slot topic (access read-write) (type STRING))"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      ESP_LOGE("ArduninoInitFunction", "Error adding %s - %i", "defclass-MQTT", (int8_t)buildError);
      return;
    }
  }

  if (FindDefclass(theEnv, "PIN") == NULL)
  {
    buildError = Build(theEnv, "(defclass PIN \"A generic Arduino GPIO pin.\" (is-a USER) (role concrete) (pattern-match reactive)"
                               "   (slot value (access read-write) (type SYMBOL NUMBER))" // (default nil) ?
                               "   (slot mode (access read-write) (type SYMBOL)(default nil)(allowed-symbols nil INPUT OUTPUT))"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      ESP_LOGE("ArduninoInitFunction", "Error adding %s - %i", "defclass-PIN", (int8_t)buildError);
      return;
    }

    buildError = Build(theEnv, "(defmessage-handler PIN delete before ()"
                               "     (pin-reset (instance-name ?self) )"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      ESP_LOGE("ArduninoInitFunction", "Error adding %s - %i", "defmessage-handler PIN delete before", (int8_t)buildError);
      return;
    }

    buildError = Build(theEnv, "(defmessage-handler PIN print before ()"
                               "     (digital-read (instance-name ?self) )"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      ESP_LOGE("ArduninoInitFunction", "Error adding %s - %i", "defmessage-handler PIN print before", (int8_t)buildError);
      return;
    }

    buildError = Build(theEnv, "(defmessage-handler PIN get-value before ()"
                               "     (digital-read (instance-name ?self) )"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      ESP_LOGE("ArduninoInitFunction", "Error adding %s - %i", "defmessage-handler PIN get-value before", (int8_t)buildError);
      return;
    }

    buildError = Build(theEnv, "(defmessage-handler PIN put-value before (?value)"
                               "  (if (= (str-compare OUTPUT ?self:mode) 0)"
                               "     then"
                               "     (digital-write (instance-name ?self) ?value)"
                               "     else"
                               "     (digital-write (instance-name ?self) ?self:value)"
                               "     (println \"Error: put-value is not a valid accessor for a PIN in INPUT mode.\")"
                               "  )"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      ESP_LOGE("ArduninoInitFunction", "Error adding %s - %i", "defmessage-handler PIN put-value before", (int8_t)buildError);
      return;
    }
  }

  Eval(theEnv, "(pin-mode LED_RED OUTPUT)", NULL);
  Eval(theEnv, "(pin-mode LED_GREEN OUTPUT)", NULL);
  Eval(theEnv, "(pin-mode LED_BLUE OUTPUT)", NULL);
  Eval(theEnv, "(digital-write LED_RED HIGH)", NULL);
  Eval(theEnv, "(digital-write LED_GREEN HIGH)", NULL);
  Eval(theEnv, "(digital-write LED_BLUE HIGH)", NULL);
  // gpio_dump_io_configuration() // gpio.c

  ESP_LOGI("ArduninoInitFunction", "Arduino Nano ESP32 + CLIPS ready!");
  Writeln(theEnv, "Arduino Nano ESP32 + CLIPS ready!");
}

void MqttConnectFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  UDFValue theArg;
  Instance *theInstance;
  const char *mqtt_broker = nullptr, *mqtt_username = nullptr, *mqtt_password = nullptr, *mqtt_topic = nullptr;
  int mqtt_port = -1;

  uint8_t argsCount = UDFArgumentCount(context);
  if (argsCount != 1)
  {
    return;
  }
  if (!UDFNthArgument(context, 1, INSTANCE_NAME_BIT, &theArg))
  {
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    ESP_LOGE("MqttConnectFunction", "WiFi.status() != WL_CONNECTED");
    return;
  }

  if (theArg.lexemeValue->contents == nullptr)
  {
    ESP_LOGE("MqttConnectFunction", "theArg.lexemeValue->contents == nullptr");
    return;
  }

  if (psClient.connected())
  {
    ESP_LOGE("MqttConnectFunction", "already connected");
    return;
  }

  theInstance = FindInstance(theEnv, NULL, theArg.lexemeValue->contents, true);
  if (strcmp(theInstance->cls->header.name->contents, "MQTT") != 0)
  {
    ESP_LOGE("MqttConnectFunction", "The type of the instance %s is not valid", theArg.lexemeValue->contents);
    Writeln(theEnv, "The type of the instance is not valid");
    UDFThrowError(context);
    return;
  }

  // MQTT Broker
  CLIPSValue *brokerCV = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  CLIPSValue *portCV = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  CLIPSValue *usrCV = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  CLIPSValue *pwdCV = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  CLIPSValue *topicCV = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  DirectGetSlot(theInstance, "broker", brokerCV);
  DirectGetSlot(theInstance, "port", portCV);
  DirectGetSlot(theInstance, "usr", usrCV);
  DirectGetSlot(theInstance, "pwd", pwdCV);
  DirectGetSlot(theInstance, "topic", topicCV);
  mqtt_broker = brokerCV->lexemeValue->contents;
  mqtt_port = portCV->integerValue->contents;
  mqtt_username = usrCV->lexemeValue->contents;
  mqtt_password = pwdCV->lexemeValue->contents;
  mqtt_topic = topicCV->lexemeValue->contents;
  genfree(theEnv, brokerCV, sizeof(CLIPSValue));
  genfree(theEnv, portCV, sizeof(CLIPSValue));
  genfree(theEnv, usrCV, sizeof(CLIPSValue));
  genfree(theEnv, pwdCV, sizeof(CLIPSValue));
  genfree(theEnv, topicCV, sizeof(CLIPSValue));

  if (mqtt_topic == nullptr)
  {
    ESP_LOGE("MqttConnectFunction", "mqtt_topic == nullptr");
    return;
  }

  // connecting to a mqtt broker
  psClient.setServer(mqtt_broker, mqtt_port);
  psClient.setCallback(MqttCallbackFunction);
  String client_id = "";
  uint8_t attempt = 10;
  while (!psClient.connected() && attempt != 0)
  {
    client_id.concat("clips-esp32-");
    client_id.concat(WiFi.macAddress());

    if (DirectPutSlotString(theInstance, "clientid", client_id.c_str()) != PutSlotError::PSE_NO_ERROR)
    {
      ESP_LOGE("MqttConnectFunction", "Something goes wrong with direct-put-value clientid");
      Writeln(theEnv, "Something goes wrong with direct-put-value clientid");
      UDFThrowError(context);
      return;
    }

    Write(theEnv, "The client ");
    Write(theEnv, client_id.c_str());
    Writeln(theEnv, " connects to the public MQTT broker");
    if (psClient.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
      Writeln(theEnv, "MQTT broker connected");
      returnValue->lexemeValue = CreateSymbol(theEnv, client_id.c_str());
      break;
    }
    else
    {
      Write(theEnv, "failed with state ");
      WriteInteger(theEnv, STDOUT, psClient.state());
      Writeln(theEnv, "");
      delay(2000);
    }
    attempt--;
  }

  if (client_id.length() == 0)
  {
    ESP_LOGE("MqttConnectFunction", "client_id.length() == 0");
    Writeln(theEnv, "client_id.length() == 0");
    UDFThrowError(context);
    return;
  }

  // Publish and subscribe
  JsonDocument helloDoc;
  helloDoc["src"] = client_id;
  helloDoc["dst"] = "ALL";
  helloDoc["msg"] = "hello!";
  size_t docSize = measureJson(helloDoc) + 1;

  char *output = (char *)genalloc(theEnv, docSize);
  serializeJson(helloDoc, output, docSize);

  psClient.publish(mqtt_topic, output, strlen(output));
  genfree(theEnv, output, docSize);

  if (psClient.subscribe(mqtt_topic))
  {
    ESP_LOGI("MqttConnectFunction", "subscribed to topic %s", mqtt_topic);
  }
  else
  {
    ESP_LOGE("MqttConnectFunction", "NOT subscribed to topic %s", mqtt_topic);
  }

  return;
}

void MqttCallbackFunction(char *topic, byte *payload, unsigned int length)
{
  callbackRunning = true;

  Instance *mqttInstance = FindInstance(mainEnv, NULL, "mqtt", true);
  if (mqttInstance == nullptr)
  {
    ESP_LOGW("MqttCallbackFunction", "An MQTT instance named 'mqtt' not found");
    callbackRunning = false;
    return;
  }

  CLIPSValue *clientIdCV = (CLIPSValue *)genalloc(mainEnv, sizeof(CLIPSValue));
  CLIPSValue *topicCV = (CLIPSValue *)genalloc(mainEnv, sizeof(CLIPSValue));
  DirectGetSlot(mqttInstance, "clientid", clientIdCV);
  DirectGetSlot(mqttInstance, "topic", topicCV);
  const char *mqtt_clientId = clientIdCV->lexemeValue->contents;
  const char *mqtt_topic = topicCV->lexemeValue->contents;
  genfree(mainEnv, clientIdCV, sizeof(CLIPSValue));
  genfree(mainEnv, topicCV, sizeof(CLIPSValue));

  if (length == 0 || payload == nullptr || topic == nullptr || mqtt_clientId == nullptr)
  {
    callbackRunning = false;
    return;
  }

  if (strcmp(topic, mqtt_topic) != 0)
  {
    ESP_LOGW("MqttCallbackFunction", "Message arrived in topic: %s. Will be ignored.", topic);
    callbackRunning = false;
    return;
  }

  String buffer = "";
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    if (!c)
    {
    }
    else
    {
      buffer += c;
    }
  }

  ESP_LOGI("MqttCallbackFunction", "Message arrived in topic: %s", topic);
  ESP_LOGI("MqttCallbackFunction", "Message: %s", buffer.c_str());
  ESP_LOGI("MqttCallbackFunction", "");
  ESP_LOGI("MqttCallbackFunction", "-----------------------");

  // TODO: not safe
  while (stringInEdit)
  {
    ESP_LOGW("MqttCallbackFunction", "stringInEdit = true");
    delay(500);
  }

  JsonDocument doc;
  DeserializationError desError = deserializeJson(doc, buffer);
  if (desError)
  {
    ESP_LOGE("MqttCallbackFunction", "deserializeJson() failed: %s. Will be ignored.", desError.c_str());
    return;
  }

  String src = doc["src"];
  String dst = doc["dst"];
  String msg = doc["msg"];

  ESP_LOGI("MqttCallbackFunction", "Evaluate message from %s -> %s", src.c_str(), dst.c_str());
  ESP_LOGI("MqttCallbackFunction", "%s", msg.c_str());
  ESP_LOGI("MqttCallbackFunction", "");

  if (msg != NULL && msg.length() > 0)
  {
    // TODO: add some security by design best-practice here

    AppendNCommandString(mainEnv, msg.c_str(), msg.length());
    const char lastChar = msg.c_str()[msg.length() - 1];
    if (lastChar != '\r')
    {
      AppendNCommandString(mainEnv, "\r", 1);
    }

    Writeln(mainEnv, "");
    Write(mainEnv, src.c_str());
    Write(mainEnv, "> ");
    Write(mainEnv, GetCommandString(mainEnv));
    Writeln(mainEnv, "");
    PrintPrompt(mainEnv);

    if (!ExecuteIfCommandComplete(mainEnv))
    {
      ESP_LOGE("MqttCallbackFunction", "The command was not complete or the command contains an error.");
    }

    // CLIPSValue *evalResults = (CLIPSValue *)genalloc(mainEnv, sizeof(CLIPSValue));
    // EvalError evalError = Eval(mainEnv, msg.c_str(), evalResults);
    // if (evalError == EvalError::EE_PARSING_ERROR)
    // {
    //   ESP_LOGE("MqttCallbackFunction", "A syntax error was encountered while parsing.");
    // }
    // else if (evalError == EvalError::EE_PROCESSING_ERROR)
    // {
    //   ESP_LOGE("MqttCallbackFunction", "An error occurred while executing the parsed expression.");
    // }

    // WriteCLIPSValue(mainEnv, STDOUT, evalResults);
    // Writeln(mainEnv, "");
    // genfree(mainEnv, evalResults, sizeof(CLIPSValue));
  }
  callbackRunning = false;
}