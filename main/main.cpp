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
#include "clips_digital_io.h"

bool stringComplete = false;
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
      Serial.println("Error: Null str pointer!");
      return;
    }

    Serial.print(str);
  }
}

void ArduninoInitFunction(Environment *theEnv, void *context)
{
  AddUDFError addUDFError = AddUDFError::AUE_NO_ERROR;
  if (FindFunction(theEnv, "digital-read") == NULL)
  {
    addUDFError = AddUDF(theEnv, "digital-read", "y", 1, 1, ";iny", DigitalReadFunction, "DigitalReadFunction", NULL);
    if (addUDFError != AddUDFError::AUE_NO_ERROR)
    {
      Write(theEnv, "ArduninoInitFunction digital-read: ");
      WriteInteger(theEnv, STDOUT, addUDFError);
      Writeln(theEnv, "");
      return;
    }
  }
  if (FindFunction(theEnv, "digital-write") == NULL)
  {
    addUDFError = AddUDF(theEnv, "digital-write", "v", 2, 2, ";iny;y", DigitalWriteFunction, "DigitalWriteFunction", NULL);
    if (addUDFError != AddUDFError::AUE_NO_ERROR)
    {
      Write(theEnv, "ArduninoInitFunction digital-write: ");
      WriteInteger(theEnv, STDOUT, addUDFError);
      Writeln(theEnv, "");
      return;
    }
  }
  if (FindFunction(theEnv, "pin-mode") == NULL)
  {
    addUDFError = AddUDF(theEnv, "pin-mode", "iv", 2, 2, ";y;y", PinModeFunction, "PinModeFunction", NULL);
    if (addUDFError != AddUDFError::AUE_NO_ERROR)
    {
      Write(theEnv, "ArduninoInitFunction pin-mode: ");
      WriteInteger(theEnv, STDOUT, addUDFError);
      Writeln(theEnv, "");
      return;
    }
  }

  BuildError buildError = BuildError::BE_NO_ERROR;
  if (FindDefclass(theEnv, "PIN") == NULL)
  {
    buildError = Build(theEnv, "(defclass PIN \"A generic Arduino GPIO pin.\" (is-a USER) (role concrete) (pattern-match reactive)"
                               "   (slot value (access read-write) (type SYMBOL NUMBER))"
                               "   (slot mode (access read-write) (type SYMBOL)(default nil)(allowed-symbols nil INPUT OUTPUT))"
                               "   (message-handler get-value primary)"
                               "   (message-handler set-value primary)"
                               "   (message-handler get-mode primary)"
                               "   (message-handler set-mode primary)"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      Write(theEnv, "ArduninoInitFunction defclass-PIN: ");
      WriteInteger(theEnv, STDOUT, buildError);
      Writeln(theEnv, "");
      return;
    }

    buildError = Build(theEnv, "(defmessage-handler PIN get-value before ()"
                               "  (if (= (str-compare INPUT ?self:mode) 0)"
                               "     then"
                               "     (digital-read (instance-name ?self) )"
                               "  )"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      Write(theEnv, "ArduninoInitFunction defclass-PIN: ");
      WriteInteger(theEnv, STDOUT, buildError);
      Writeln(theEnv, "");
      return;
    }

    buildError = Build(theEnv, "(defmessage-handler PIN set-value after (?value)"
                               "  (if (= (str-compare OUTPUT ?self:mode) 0)"
                               "     then"
                               "     (digital-write (instance-name ?self) ?value)"
                               "  )"
                               ")");
    if (buildError != BuildError::BE_NO_ERROR)
    {
      Write(theEnv, "ArduninoInitFunction defclass-PIN: ");
      WriteInteger(theEnv, STDOUT, buildError);
      Writeln(theEnv, "");
      return;
    }
  }

  Eval(theEnv, "(pin-mode LED_RED OUTPUT)", NULL);
  Eval(theEnv, "(pin-mode LED_GREEN OUTPUT)", NULL);
  Eval(theEnv, "(pin-mode LED_BLUE OUTPUT)", NULL);
  Eval(theEnv, "(digital-write LED_RED HIGH)", NULL);
  Eval(theEnv, "(digital-write LED_GREEN HIGH)", NULL);
  Eval(theEnv, "(digital-write LED_BLUE HIGH)", NULL);
  // TODO: reset all pin related to existing instances
  // gpio_dump_io_configuration() // gpio.c
  // gpio_reset_pin // gpio.c

  Writeln(theEnv, "Arduino Nano ESP32 + CLIPS ready!");
}

void setup()
{
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
  }
  Serial.println("\r\nStarting...\r\n");

  if (!psramFound())
  {
    Serial.println(F("PSRAM not available!"));
    digitalWrite(LED_RED, LOW); // reverse logic
    delay(2000);
    return;
  }

  // TODO: https://github.com/espressif/esp-idf/blob/v5.4/examples/protocols/sntp/README.md

#if DEBUGGING_FUNCTIONS
// TODO: wdt disabled!!!
// rtc_wdt_protect_off();
// rtc_wdt_disable();
// esp_task_wdt_delete(NULL);

// Serial.println("CPU0 reset reason:");
// print_reset_reason(rtc_get_reset_reason(0));
// verbose_print_reset_reason(rtc_get_reset_reason(0));

// Serial.println("CPU1 reset reason:");
// print_reset_reason(rtc_get_reset_reason(1));
// verbose_print_reset_reason(rtc_get_reset_reason(1));
#endif

  mainEnv = CreateEnvironment();
  UtilityData(mainEnv)->YieldTimeFunction = yield; // esp32-hal.h
  EnableYieldFunction(mainEnv, true);

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

  AddStartingFunction(mainEnv, "arduino-init", ArduninoInitFunction, 2001, NULL);

  Reset(mainEnv);
  Writeln(mainEnv, "");
  PrintPrompt(mainEnv);
}

void loop()
{
  while (Serial.available())
  {
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
  }
}
