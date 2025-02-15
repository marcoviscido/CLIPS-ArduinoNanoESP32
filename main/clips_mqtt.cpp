/*
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

#include <atomic>
#include "Arduino.h"
#include "clips.h"

#include "WiFi.h"
#include "mqtt_client.h"
#include "main.h"

#include "clips_mqtt.h"

ESP_EVENT_DEFINE_BASE(MQTT_EVENTS);

#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include "ArduinoJson-v7.3.0.h"

static esp_mqtt_client_handle_t mqtt_client_handle = nullptr;
static esp_mqtt_client_config_t mqtt_config = {};
static const char *mqtt_topic = nullptr;
static String mqtt_client_id = "";
static std::atomic<bool> mqttCbRunning(false);

void MqttConnectFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
    UDFValue theArg;
    Instance *theInstance;
    const char *mqtt_broker = nullptr, *mqtt_username = nullptr, *mqtt_password = nullptr;
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

    theInstance = FindInstance(theEnv, NULL, theArg.lexemeValue->contents, true);
    if (strcmp(theInstance->cls->header.name->contents, "MQTT") != 0)
    {
        ESP_LOGE("MqttConnectFunction", "The type of the instance %s is not valid", theArg.lexemeValue->contents);
        Writeln(theEnv, "The type of the instance is not valid");
        UDFThrowError(context);
        return;
    }

    CLIPSValue *isConnected = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
    DirectGetSlot(theInstance, "connected", isConnected);
    if (isConnected->lexemeValue == theEnv->TrueSymbol)
    {
        genfree(theEnv, isConnected, sizeof(CLIPSValue));
        ESP_LOGE("MqttConnectFunction", "already connected");
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
    genfree(theEnv, isConnected, sizeof(CLIPSValue));

    if (mqtt_topic == nullptr)
    {
        ESP_LOGE("MqttConnectFunction", "mqtt_topic == nullptr");
        return;
    }

    if (mqtt_client_id.length() == 0)
    {
        mqtt_client_id.concat("clips-esp32-");
        mqtt_client_id.concat(WiFi.macAddress());
    }

    mqtt_config.broker = {};
    mqtt_config.broker.address = {};
    mqtt_config.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
    mqtt_config.broker.address.hostname = mqtt_broker;
    mqtt_config.broker.address.port = mqtt_port;
    // mqtt_config.broker.address.transport
    mqtt_config.credentials = {};
    mqtt_config.credentials.username = mqtt_username;
    mqtt_config.credentials.client_id = mqtt_client_id.c_str();
    mqtt_config.credentials.authentication = {};
    mqtt_config.credentials.authentication.password = mqtt_password;

    if (mqtt_client_handle == nullptr)
    {
        mqtt_client_handle = esp_mqtt_client_init(&mqtt_config);

        if (mqtt_client_handle == NULL)
        {
            ESP_LOGE("MqttConnectFunction", "esp_mqtt_client_init error");
            return;
        }

        ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client_handle, MQTT_EVENT_CONNECTED, mqtt_on_connected_cb, theInstance));
        ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client_handle, MQTT_EVENT_DATA, mqtt_on_data_cb, theInstance));
        ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client_handle));
    }
    else
    {
        ESP_LOGW("MqttConnectFunction", "mqtt already connected");
    }

    return;
}

void mqtt_on_connected_cb(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == MQTT_EVENTS)
    {
        if (event_id == MQTT_EVENT_ERROR)
        {
            ESP_LOGE("mqtt_on_connected_cb", "MQTT Event Error");
        }
        else if (event_id == MQTT_EVENT_CONNECTED)
        {
            Instance *theInstance = static_cast<Instance *>(event_handler_arg); // The MQTT instance
            if (theInstance == nullptr)
            {
                ESP_LOGE("mqtt_on_connected_cb", "MQTT Event Error - mqtt instance not found");
                return;
            }

            int msg_id = esp_mqtt_client_subscribe(mqtt_client_handle, mqtt_topic, 0);
            if (msg_id == -1)
            {
                ESP_LOGE("mqtt_on_connected_cb", "MQTT Event Error - esp_mqtt_client_subscribe -1");
                return;
            }

            DirectPutSlotSymbol(theInstance, "connected", "TRUE");
            DirectPutSlotCLIPSExternalAddress(theInstance, "config-handle", CreateCExternalAddress(mainEnv, &mqtt_config));
            DirectPutSlotCLIPSExternalAddress(theInstance, "client-handle", CreateCExternalAddress(mainEnv, &mqtt_client_handle));

            Write(mainEnv, "MQTT Connected - clientId: ");
            Writeln(mainEnv, mqtt_client_id.c_str());
            ESP_LOGI("mqtt_on_connected_cb", "Subscribed to topic '%s', msg_id=%d", mqtt_topic, msg_id);

            Eval(mainEnv, "(mqtt-publish \"ALL\" \"hello!\")", NULL);
        }
        return;
    }
    else
    {
        ESP_LOGW("mqtt_on_connected_cb", "event_base != MQTT_EVENTS");
    }
}

void mqtt_on_data_cb(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == MQTT_EVENTS)
    {
        esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

        if (event_id == MQTT_EVENT_ERROR)
        {
            ESP_LOGE("mqtt_on_data_cb", "MQTT Event Error");
        }
        else if (event_id == MQTT_EVENT_DATA)
        {
            ESP_LOGI("mqtt_on_data_cb", "Received data:");
            ESP_LOGI("mqtt_on_data_cb", "\tTopic: %.*s\n", event->topic_len, event->topic);
            ESP_LOGI("mqtt_on_data_cb", "\tData: %.*s\n", event->data_len, event->data);
            if (mqttCbRunning.load() == true)
            {
                ESP_LOGE("mqtt_on_data_cb", "mqttCbRunning, the message will be discarded", );
                return;
            }
            if (EngineData(mainEnv)->AlreadyRunning)
            {
                ESP_LOGE("mqtt_on_data_cb", "AlreadyRunning, the message will be discarded", );
                return;
            }
            if (stringInEdit.load() == true)
            {
                ESP_LOGE("mqtt_on_data_cb", "stringInEdit on Serial, the message will be discarded", );
                return;
            }

            mqttCbRunning.store(true);

            Instance *theInstance = static_cast<Instance *>(event_handler_arg); // The MQTT instance
            if (theInstance == nullptr)
            {
                ESP_LOGE("mqtt_on_connected_cb", "MQTT Event Error - mqtt instance not found");
                mqttCbRunning.store(false);
                return;
            }

            JsonDocument filter;
            filter["src"] = true;
            filter["dst"] = true;
            filter["msg"] = true;
            filter["msg_id"] = true;
            filter["reply_me"] = true;

            JsonDocument doc;
            DeserializationError desError = ArduinoJson::deserializeJson(doc, event->data, event->data_len, DeserializationOption::Filter(filter));
            if (desError)
            {
                ESP_LOGE("MqttCallbackFunction", "deserializeJson() failed: %s. Will be ignored.", desError.c_str());
                mqttCbRunning.store(false);
                return;
            }

            String src = doc["src"];
            String dst = doc["dst"];
            String msg = doc["msg"];
            String msgId = doc["msg_id"];
            String replyMeString = doc["reply_me"] | "false";

            ESP_LOGV("MqttCallbackFunction", "Evaluate message from %s -> %s", src.c_str(), dst.c_str());
            ESP_LOGV("MqttCallbackFunction", "msg_id: %s", msgId.c_str());
            ESP_LOGV("MqttCallbackFunction", "reply_me: %s", replyMeString.c_str());
            ESP_LOGV("MqttCallbackFunction", "");
            ESP_LOGV("MqttCallbackFunction", "%s", msg.c_str());
            ESP_LOGV("MqttCallbackFunction", "");

            if (src.length() == 0 || dst.length() == 0 || msg.length() == 0 || msgId.length() == 0)
            {
                ESP_LOGE("MqttCallbackFunction", "THE MESSAGE IS NOT VALID");
                mqttCbRunning.store(false);
                return;
            }

            // I sent this message, return
            if (strcmp(mqtt_client_id.c_str(), src.c_str()) == 0)
            {
                mqttCbRunning.store(false);
                return;
            }

            if (!(strcmp(mqtt_client_id.c_str(), dst.c_str()) == 0 || strcmp("ALL", dst.c_str()) == 0))
            {
                ESP_LOGW("MqttCallbackFunction", "THE MESSAGE IS NOT FOR ME");
                mqttCbRunning.store(false);
                return;
            }

            if (msg != NULL && msg.length() > 0)
            {
                if (!msg.endsWith("\r"))
                {
                    msg.concat('\r');
                }

                if (!EngineData(mainEnv)->AlreadyRunning)
                {
                    // TODO: add some security by design best-practice here
                    AppendNCommandString(mainEnv, msg.c_str(), strlen(msg.c_str()));
                }

                if (!(strcmp(mqtt_client_id.c_str(), src.c_str()) == 0 && strcmp("ALL", dst.c_str()) == 0))
                {
                    Writeln(mainEnv, "");
                    Write(mainEnv, src.c_str());
                    Write(mainEnv, "> ");
                    Writeln(mainEnv, GetCommandString(mainEnv));
                }

                MqttRouterData *mqttRouterData = nullptr;
                if (strcmp("true", replyMeString.c_str()) == 0 || EngineData(mainEnv)->AlreadyRunning)
                {
                    mqttRouterData = (MqttRouterData *)genalloc(mainEnv, sizeof(MqttRouterData));
                    mqttRouterData->mqttInstance = theInstance;
                    mqttRouterData->msgId = msgId.c_str();
                    mqttRouterData->sender = src.c_str();

                    AddRouter(mainEnv,
                              "mqtt",                 /* Router name */
                              20,                     /* Priority */
                              QueryMqttReplyCallback, /* Query function */
                              WriteMqttReplyCallback, /* Write function */
                              NULL,                   /* Read function */
                              NULL,                   /* Unread function */
                              NULL,                   /* Exit function */
                              mqttRouterData);        /* Context */
                    ActivateRouter(mainEnv, "mqtt");  // sends a reply to src of the message
                }

                if (EngineData(mainEnv)->AlreadyRunning)
                {
                    ESP_LOGW("MqttCallbackFunction", "Clips is busy!");
                    Writeln(mainEnv, "BUSY");
                }
                else
                {
                    if (!ExecuteIfCommandComplete(mainEnv))
                    {
                        ESP_LOGE("MqttCallbackFunction", "The received command was not complete or the command contains an error:");
                        ESP_LOGE("MqttCallbackFunction", "%s", msg.c_str());
                    }
                }

                if (strcmp("true", replyMeString.c_str()) == 0 || EngineData(mainEnv)->AlreadyRunning)
                {
                    DeactivateRouter(mainEnv, "mqtt");
                    DeleteRouter(mainEnv, "mqtt");
                    genfree(mainEnv, mqttRouterData, sizeof(MqttRouterData));
                }
            }
            else
            {
                ESP_LOGW("MqttCallbackFunction", "msg.length() == 0");
            }
            mqttCbRunning.store(false);
        }
        return;
    }
    else
    {
        ESP_LOGW("mqtt_on_data_cb", "event_base != MQTT_EVENTS");
    }
}

void MqttDisconnectFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
    Instance *theInstance;

    theInstance = FindInstance(theEnv, NULL, "mqtt", true);
    if (strcmp(theInstance->cls->header.name->contents, "MQTT") != 0)
    {
        ESP_LOGE("MqttConnectFunction", "The type of the instance 'mqtt' is not valid");
        Writeln(theEnv, "The type of the instance is not valid");
        UDFThrowError(context);
        return;
    }

    CLIPSValue *isConnected = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
    DirectGetSlot(theInstance, "connected", isConnected);
    if (isConnected->lexemeValue == theEnv->FalseSymbol)
    {
        genfree(theEnv, isConnected, sizeof(CLIPSValue));
        return;
    }

    esp_mqtt_client_unsubscribe(mqtt_client_handle, mqtt_topic);
    ESP_ERROR_CHECK(esp_mqtt_client_disconnect(mqtt_client_handle));
    ESP_ERROR_CHECK(esp_mqtt_client_stop(mqtt_client_handle));
    DirectPutSlotSymbol(theInstance, "connected", "FALSE");

    Writeln(theEnv, "MQTT is not connected");
    genfree(theEnv, isConnected, sizeof(CLIPSValue));
    return;
}

void MqttPublishFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
    UDFValue theArgDest;
    UDFValue theArgMsg;
    Instance *theInstance;

    if (!UDFFirstArgument(context, STRING_BIT, &theArgDest))
    {
        return;
    }

    if (!UDFNthArgument(context, 2, STRING_BIT, &theArgMsg))
    {
        return;
    }

    theInstance = FindInstance(theEnv, NULL, "mqtt", true);
    if (strcmp(theInstance->cls->header.name->contents, "MQTT") != 0)
    {
        ESP_LOGE("MqttPublishFunction", "The type of the instance 'mqtt' is not valid");
        Writeln(theEnv, "The type of the instance is not valid");
        UDFThrowError(context);
        return;
    }

    CLIPSValue *isConnected = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
    DirectGetSlot(theInstance, "connected", isConnected);
    if (isConnected->lexemeValue == theEnv->FalseSymbol)
    {
        ESP_LOGE("MqttPublishFunction", "MQTT is not connected");
        Writeln(theEnv, "MQTT is not connected");
        UDFThrowError(context);
        genfree(theEnv, isConnected, sizeof(CLIPSValue));
        return;
    }

    uuid.generate();
    JsonDocument helloDoc;
    helloDoc["src"] = mqtt_client_id;
    helloDoc["dst"] = theArgDest.lexemeValue->contents;
    helloDoc["msg_id"] = uuid.toCharArray();
    helloDoc["msg"] = theArgMsg.lexemeValue->contents;
    helloDoc["reply_me"] = "false";
    size_t docSize = measureJson(helloDoc) + 1;

    char *output = (char *)genalloc(theEnv, docSize);
    ArduinoJson::serializeJson(helloDoc, output, docSize);

    esp_mqtt_client_publish(mqtt_client_handle, mqtt_topic, output, strlen(output), 2, 0);
    genfree(theEnv, output, docSize);
    return;
}

bool QueryMqttReplyCallback(Environment *theEnv, const char *logicalName, void *context)
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

void WriteMqttReplyCallback(Environment *theEnv, const char *logicalName, const char *str, void *context)
{
    if ((strcmp(logicalName, STDOUT) == 0))
    {
        if (!str || !context || context == nullptr)
        {
            ESP_LOGW("WriteMqttReplyCallback", "Null str pointer!");
            return;
        }

        if (strcmp("\r", str) == 0 || strcmp("\n", str) == 0 || strcmp("CLIPS> ", str) == 0)
        {
            ESP_LOGW("WriteMqttReplyCallback", "empty string");
            return;
        }

        const MqttRouterData *mqttRouterData = (MqttRouterData *)context;
        ESP_LOGI("WriteMqttReplyCallback", "reply message to %s: %s", mqttRouterData->sender, str);

        // sto rispondendo al sender che chiede replyMe quindi non devo essere io
        if (strcmp(mqtt_client_id.c_str(), mqttRouterData->sender) == 0)
        {
            return;
        }

        if (strlen(mqtt_client_id.c_str()) == 0 || strlen(mqttRouterData->sender) == 0 || strlen(mqttRouterData->msgId) == 0 || strlen(str) == 0)
        {
            return;
        }

        DeactivateRouter(theEnv, "trace");

        JsonDocument helloDoc;
        helloDoc["src"] = mqtt_client_id.c_str();
        helloDoc["dst"] = mqttRouterData->sender;
        helloDoc["msg_id"] = mqttRouterData->msgId; // the same of the first one
        helloDoc["msg"] = str;
        helloDoc["reply_me"] = "false";
        size_t docSize = measureJson(helloDoc) + 1;

        char *output = (char *)genalloc(theEnv, docSize);
        ArduinoJson::serializeJson(helloDoc, output, docSize);
        if (esp_mqtt_client_publish(mqtt_client_handle, mqtt_topic, output, strlen(output), 2, 0) == -1)
        {
            ESP_LOGE("WriteMqttReplyCallback", "esp_mqtt_client_publish error");
        }
        genfree(theEnv, output, docSize);

        ActivateRouter(theEnv, "trace");
    }
}