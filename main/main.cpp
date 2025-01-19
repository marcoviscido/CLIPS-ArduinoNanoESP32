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
#include "clips_gpio.h"

bool stringComplete = false;
bool systemReady = false;
static Environment *mainEnv;

// variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;
unsigned long last_button_time = 0;

Fact *pinStateD2 = nullptr;

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

void ARDUINO_ISR_ATTR isr()
{
  if (!systemReady)
  {
    return;
  }

  button_time = millis();
  if (button_time - last_button_time > 500)
  {
    if (!digitalRead(D2))
    {
      if (pinStateD2 != nullptr)
      {
        Retract(pinStateD2);
        ReleaseFact(pinStateD2);
      }
      pinStateD2 = AssertString(mainEnv, "(PIN-D2 INPUT HIGH)");
    }
    else
    {
      if (pinStateD2 != nullptr)
      {
        Retract(pinStateD2);
        ReleaseFact(pinStateD2);
      }

      pinStateD2 = AssertString(mainEnv, "(PIN-D2 INPUT LOW)");
    }
    last_button_time = button_time;
  }
}

// void LedOnFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
// {
//   digitalWrite(D5, HIGH);
//   Serial.println("D5 high");
// }

// void LedOffFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
// {
//   digitalWrite(D5, LOW);
//   Serial.println("D5 low");
// }

// void PinModeFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
// {
//   UDFValue nextPossible;
//   const char *arg1;
//   const char *arg2;

//   if (!UDFNthArgument(context, 1, LEXEME_BITS, &nextPossible))
//   {
//     return;
//   }
//   else
//   {
//     arg1 = nextPossible.lexemeValue->contents;
//   }

//   if (!UDFNthArgument(context, 2, LEXEME_BITS, &nextPossible))
//   {
//     return;
//   }
//   else
//   {
//     arg2 = nextPossible.lexemeValue->contents;
//   }

//   Serial.print(arg1);
//   Serial.print("  ");
//   Serial.print(arg2);
// }

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

  pinMode(D2, INPUT_PULLUP);
  attachInterrupt(D2, isr, CHANGE);

  pinMode(D5, OUTPUT);
  digitalWrite(D5, HIGH);

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
    digitalWrite(LED_RED, HIGH);
    delay(2000);
    return;
  }

  // rtc_wdt_protect_off();
  // rtc_wdt_disable();
  // esp_task_wdt_delete(NULL);

  // Serial.println("CPU0 reset reason:");
  // print_reset_reason(rtc_get_reset_reason(0));
  // verbose_print_reset_reason(rtc_get_reset_reason(0));

  // Serial.println("CPU1 reset reason:");
  // print_reset_reason(rtc_get_reset_reason(1));
  // verbose_print_reset_reason(rtc_get_reset_reason(1));

  mainEnv = CreateEnvironment();

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

  PrintPrompt(mainEnv);
  RouterData(mainEnv)->CommandBufferInputCount = 0;
  RouterData(mainEnv)->InputUngets = 0;
  RouterData(mainEnv)->AwaitingInput = true;

  AddUDF(mainEnv, "pin-mode", "v", 2, 2, "y", PinModeFunction, "PinModeFunction", NULL);
  AddUDF(mainEnv, "led-on", "v", 0, 0, NULL, LedOnFunction, "LedOnFunction", NULL);
  AddUDF(mainEnv, "led-off", "v", 0, 0, NULL, LedOffFunction, "LedOffFunction", NULL);

  Watch(mainEnv, ALL); // debug
  // Build(mainEnv, "(defrule hello"
  //                "  =>"
  //                "  (println \"Hello World!\"))");

  Build(mainEnv, "(defrule led-off-se-scollegato"
                 "  (PIN-D2 INPUT LOW)"
                 "  =>"
                 "  (led-off))");

  Build(mainEnv, "(defrule led-on-se-collegato"
                 "  (PIN-D2 INPUT HIGH)"
                 "  =>"
                 "  (led-on))");

  Writeln(mainEnv, "Arduino Nano ESP32 + CLIPS ready!");
  PrintPrompt(mainEnv);
  systemReady = true;
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  while (Serial.available())
  {
    char inChar = (char)Serial.read();
    AppendNCommandString(mainEnv, &inChar, 1);

    if (inChar == '\r')
    {
      stringComplete = true;
      // Serial.print(F("debug: stringComplete true")); // debug
    }

    // // debug
    // Serial.print(F("debug: "));
    // Serial.println(GetCommandString(mainEnv));
    // Serial.println("");
  }

  if (stringComplete)
  {
    Serial.println(GetCommandString(mainEnv));
    stringComplete = false;
    // Serial.print(F("debug: stringComplete false")); // debug

    /*
     * If a complete command exists, then 1 is returned.
     * 0 is returned if the command was not complete and without errors.
     * -1 is returned if the command contains an error.
     */
    bool exec = ExecuteIfCommandComplete(mainEnv);
    // // debug
    // Serial.print(F("debug: exec "));
    // Serial.print(exec);
    // Serial.println("");
  }

  delay(100);                     // debug
  digitalWrite(LED_BUILTIN, LOW); // debug
  delay(400);
}
