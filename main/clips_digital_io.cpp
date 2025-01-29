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

#include "pins_arduino.h"
#include "Arduino.h"
#include "clips.h"
#include "clips_digital_io.h"

int getPinFromName(const char *key)
{
  for (int i = 0; i < pinsLookupTableSize; ++i)
  {
    if (strcmp(pinsLookupTable[i].key, key) == 0)
    {
      return pinsLookupTable[i].value;
    }
  }
  return -1;
}

void DigitalReadFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  UDFValue theArg;
  const char *pinArg;

  if (!UDFFirstArgument(context, SYMBOL_BIT, &theArg))
  {
    return;
  }

  pinArg = theArg.lexemeValue->contents;
  if (pinArg == nullptr)
  {
    UDFThrowError(context);
    return;
  }

  int pin = getPinFromName(pinArg);
  if (pin < 0)
  {
    Writeln(theEnv, "Pin name is not valid.");
    UDFThrowError(context);
    return;
  }

  CLIPSValue *insdata = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  insdata->instanceValue = FindInstance(theEnv, NULL, pinArg, true);
  if (insdata->instanceValue == nullptr)
  {
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    Write(theEnv, pinArg);
    Writeln(theEnv, " pin has not yet been registered.");
    UDFThrowError(context);
    return;
  }

  CLIPSValue *modeSlotVal = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  if (DirectGetSlot(insdata->instanceValue, "mode", modeSlotVal) != GSE_NO_ERROR)
  {
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    genfree(theEnv, modeSlotVal, sizeof(CLIPSValue));
    // se non riesco a leggere l'attributo mode della classe PIN
    Writeln(theEnv, "Pin instance is not valid.");
    UDFThrowError(context);
    return;
  }

  if (strcmp(modeSlotVal->lexemeValue->contents, "INPUT") != 0)
  {
    genfree(theEnv, modeSlotVal, sizeof(CLIPSValue));
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    Write(theEnv, "Pin ");
    Write(theEnv, pinArg);
    Writeln(theEnv, " is not in INPUT mode.");
    UDFThrowError(context);
    return;
  }
  genfree(theEnv, modeSlotVal, sizeof(CLIPSValue));

  if (digitalRead(pin) == 0)
  {
    returnValue->lexemeValue = CreateSymbol(theEnv, "LOW");
  }
  else
  {
    returnValue->lexemeValue = CreateSymbol(theEnv, "HIGH");
  }
  Send(theEnv, insdata, "put-value", returnValue->lexemeValue->contents, NULL);
  genfree(theEnv, insdata, sizeof(CLIPSValue));
}

void DigitalWriteFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  UDFValue nextPossible;
  const char *pinArg;
  const char *stateArg;

  if (!UDFNthArgument(context, 1, SYMBOL_BIT, &nextPossible))
  {
    return;
  }
  pinArg = nextPossible.lexemeValue->contents;
  if (pinArg == nullptr)
  {
    UDFThrowError(context);
    return;
  }

  if (!UDFNthArgument(context, 2, SYMBOL_BIT, &nextPossible))
  {
    return;
  }
  stateArg = nextPossible.lexemeValue->contents;
  if (stateArg == nullptr)
  {
    UDFThrowError(context);
    return;
  }

  int pin = getPinFromName(pinArg);
  if (pin < 0)
  {
    Writeln(theEnv, "Pin name is not valid.");
    UDFThrowError(context);
    return;
  }

  CLIPSValue *insdata = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  insdata->instanceValue = FindInstance(theEnv, NULL, pinArg, true);
  if (insdata->instanceValue == nullptr)
  {
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    Write(theEnv, pinArg);
    Writeln(theEnv, " pin has not yet been registered.");
    UDFThrowError(context);
    return;
  }

  CLIPSValue *modeSlotVal = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  if (DirectGetSlot(insdata->instanceValue, "mode", modeSlotVal) != GSE_NO_ERROR)
  {
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    genfree(theEnv, modeSlotVal, sizeof(CLIPSValue));
    // se non riesco a leggere l'attributo mode della classe PIN
    Writeln(theEnv, "Pin instance is not valid.");
    UDFThrowError(context);
    return;
  }

  if (strcmp(modeSlotVal->lexemeValue->contents, "OUTPUT") != 0)
  {
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    genfree(theEnv, modeSlotVal, sizeof(CLIPSValue));
    Write(theEnv, "Pin ");
    Write(theEnv, pinArg);
    Writeln(theEnv, " is not in OUTPUT mode.");
    UDFThrowError(context);
    return;
  }
  genfree(theEnv, modeSlotVal, sizeof(CLIPSValue));

  if (stateArg != nullptr && strcmp(stateArg, "LOW") == 0)
  {
    digitalWrite(pin, LOW);
    Send(theEnv, insdata, "put-value", CreateSymbol(theEnv, "LOW")->contents, NULL);
    genfree(theEnv, insdata, sizeof(CLIPSValue));
  }
  else if (stateArg != nullptr && strcmp(stateArg, "HIGH") == 0)
  {
    digitalWrite(pin, HIGH);
    Send(theEnv, insdata, "put-value", CreateSymbol(theEnv, "HIGH")->contents, NULL);
    genfree(theEnv, insdata, sizeof(CLIPSValue));
  }
  else
  {
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    UDFThrowError(context);
  }
}

/**
 * Ex.: (pin-mode D5 OUTPUT)
 */
void PinModeFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  UDFValue nextPossible;
  const char *pinArg;
  const char *modeArg;

  if (!UDFNthArgument(context, 1, SYMBOL_BIT, &nextPossible))
  {
    return;
  }
  pinArg = nextPossible.lexemeValue->contents;
  if (pinArg == nullptr)
  {
    SetErrorValue(theEnv, nextPossible.header);
    UDFThrowError(context);
    return;
  }

  if (!UDFNthArgument(context, 2, SYMBOL_BIT, &nextPossible))
  {
    return;
  }
  modeArg = nextPossible.lexemeValue->contents;
  if (modeArg == nullptr)
  {
    SetErrorValue(theEnv, nextPossible.header);
    UDFThrowError(context);
    return;
  }

  int pin = getPinFromName(pinArg);
  if (pin < 0)
  {
    Writeln(theEnv, "Pin name is not valid.");
    UDFThrowError(context);
    return;
  }

  returnValue->instanceValue = FindInstance(theEnv, NULL, pinArg, true);
  if (returnValue->instanceValue != NULL)
  {
    Writeln(theEnv, "Pin is already defined. No action performed.");
    return;
  }

  String makeInstCmd = "(";
  makeInstCmd += pinArg;
  makeInstCmd += " of PIN ";
  makeInstCmd += "(mode \"";

  if (modeArg != nullptr && strcmp(modeArg, "INPUT") == 0)
  {
    pinMode(pin, INPUT);
    makeInstCmd += "INPUT";
    makeInstCmd += "\"))";
    returnValue->instanceValue = MakeInstance(theEnv, makeInstCmd.c_str());
  }
  else if (modeArg != nullptr && strcmp(modeArg, "OUTPUT") == 0)
  {
    pinMode(pin, OUTPUT);
    makeInstCmd += "OUTPUT";
    makeInstCmd += "\"))";
    returnValue->instanceValue = MakeInstance(theEnv, makeInstCmd.c_str());
  }
  else if (modeArg != nullptr && strcmp(modeArg, "PULLUP") == 0)
  {
    pinMode(pin, PULLUP);
    makeInstCmd += "OUTPUT";
    makeInstCmd += "\"))";
    returnValue->instanceValue = MakeInstance(theEnv, makeInstCmd.c_str());
  }
  else if (modeArg != nullptr && strcmp(modeArg, "INPUT_PULLUP") == 0)
  {
    pinMode(pin, INPUT_PULLUP);
    makeInstCmd += "INPUT";
    makeInstCmd += "\"))";
    returnValue->instanceValue = MakeInstance(theEnv, makeInstCmd.c_str());
  }
  else if (modeArg != nullptr && strcmp(modeArg, "PULLDOWN") == 0)
  {
    pinMode(pin, PULLDOWN);
    makeInstCmd += "OUTPUT";
    makeInstCmd += "\"))";
    returnValue->instanceValue = MakeInstance(theEnv, makeInstCmd.c_str());
  }
  else if (modeArg != nullptr && strcmp(modeArg, "INPUT_PULLDOWN") == 0)
  {
    pinMode(pin, INPUT_PULLDOWN);
    makeInstCmd += "INPUT";
    makeInstCmd += "\"))";
    returnValue->instanceValue = MakeInstance(theEnv, makeInstCmd.c_str());
  }
  else if (modeArg != nullptr && strcmp(modeArg, "OPEN_DRAIN") == 0)
  {
    pinMode(pin, OPEN_DRAIN);
    makeInstCmd += "INPUT";
    makeInstCmd += "\"))";
    returnValue->instanceValue = MakeInstance(theEnv, makeInstCmd.c_str());
  }
  else if (modeArg != nullptr && strcmp(modeArg, "OUTPUT_OPEN_DRAIN") == 0)
  {
    pinMode(pin, OUTPUT_OPEN_DRAIN);
    makeInstCmd += "OUTPUT";
    makeInstCmd += "\"))";
    returnValue->instanceValue = MakeInstance(theEnv, makeInstCmd.c_str());
  }
  else if (modeArg != nullptr && strcmp(modeArg, "ANALOG") == 0)
  {
    // pinMode(pin, ANALOG);
    // makeInstCmd += modeArg;
    // makeInstCmd += "\"))";
    // MakeInstance(theEnv, makeInstCmd.c_str());
    returnValue->instanceValue = nullptr;
    UDFThrowError(context);
  }
  else
  {
    returnValue->instanceValue = nullptr;
    UDFThrowError(context);
  }
}
