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

bool pinInstanceExists(Environment *theEnv, const char *pinName)
{
  UDFValue iterate;
  Instance *theInstance;
  Defclass *theClass;

  if (pinName == NULL || pinName == nullptr)
  {
    return false;
  }

  theClass = FindDefclass(theEnv, "PIN");
  for (theInstance = GetNextInstanceInClassAndSubclasses(&theClass, NULL, &iterate);
       theInstance != NULL;
       theInstance = GetNextInstanceInClassAndSubclasses(&theClass,
                                                         theInstance, &iterate))
  {
    if (strcmp(InstanceName(theInstance), pinName) == 0)
    {
      return true;
    }
  }
  return false;
}

Instance *getInstanceByName(Environment *theEnv, const char *pinName)
{
  Instance *c3 = nullptr;
  c3 = FindInstance(theEnv, NULL, pinName, true);
  return c3;
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
    Writeln(theEnv, "Pin index cannot be negative.");
    UDFThrowError(context);
    return;
  }

  // (any-instancep ((?pin PIN)) (= (str-compare ?pin:id "D2") 0))
  if (!pinInstanceExists(theEnv, pinArg))
  {
    Write(theEnv, pinArg);
    Writeln(theEnv, " pin has not yet been registered.");
    UDFThrowError(context);
    return;
  }

  if (digitalRead(pin) == 0)
  {
    returnValue->lexemeValue = CreateSymbol(theEnv, "LOW");
  }
  else
  {
    returnValue->lexemeValue = CreateSymbol(theEnv, "HIGH");
  }
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
    Writeln(theEnv, "Pin index cannot be negative.");
    UDFThrowError(context);
    return;
  }

  // (any-instancep ((?pin PIN)) (= (str-compare ?pin:id "D2") 0))
  if (!pinInstanceExists(theEnv, pinArg))
  {
    Write(theEnv, pinArg);
    Writeln(theEnv, " pin has not yet been registered.");
    UDFThrowError(context);
    return;
  }

  // send [instance] get-mode == INPUT / OUTPUT?

  if (stateArg != nullptr && strcmp(stateArg, "LOW") == 0)
  {
    digitalWrite(pin, LOW);
  }
  else if (stateArg != nullptr && strcmp(stateArg, "HIGH") == 0)
  {
    digitalWrite(pin, HIGH);
  }
  else
  {
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
    Writeln(theEnv, "Pin index cannot be negative.");
    UDFThrowError(context);
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
    UDFThrowError(context);
  }
  else
  {
    UDFThrowError(context);
  }
}
