#include <Arduino.h>

#include "esp32s3/rom/rtc.h"
#include "esp_task_wdt.h"
#include "clips.h"

String inputString = "";
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

void setup()
{
  Serial.begin(11520);
  Serial.setTimeout(2000);
  inputString.reserve(200);

  while (!Serial)
  {
    delay(100);
  }

  Serial.println("\r\nStarting...\r\n");

  if (!psramFound())
  {
    Serial.println(F("PSRAM not available!"));
    delay(2000);
    return;
  }

  // rtc_wdt_protect_off();
  // rtc_wdt_disable();
  // esp_task_wdt_delete(NULL);

  delay(2000);

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

  Watch(mainEnv, ALL); // WatchItem.ALL
  Build(mainEnv, "(defrule hello"
                 "  =>"
                 "  (println \"Hello World!\"))");

  Serial.println(F("Arduino Nano ESP32 + CLIPS ready!"));
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
    inputString = "";
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

  delay(100); // debug
}
