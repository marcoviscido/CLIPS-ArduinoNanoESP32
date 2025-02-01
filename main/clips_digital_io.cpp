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
  const char *pinArg = nullptr;
  Instance *pinInstanceInInput = nullptr;

  if (!UDFFirstArgument(context, INSTANCE_BITS | SYMBOL_BIT, &theArg))
  {
    return;
  }

  if (CVIsType(&theArg, SYMBOL_BIT | INSTANCE_NAME_BIT))
  {
    pinArg = theArg.lexemeValue->contents;
  }
  else if (CVIsType(&theArg, INSTANCE_ADDRESS_BIT))
  {
    pinInstanceInInput = theArg.instanceValue;
    pinArg = theArg.instanceValue->name->contents;
  }

  if (pinArg == nullptr)
  {
    SetErrorValue(theEnv, theArg.header);
    UDFThrowError(context);
    return;
  }

  int pin = getPinFromName(pinArg);
  if (pin < 0)
  {
    SetErrorValue(theEnv, theArg.header);
    UDFInvalidArgumentMessage(context, "symbol of a valid pin name");
    UDFThrowError(context);
    return;
  }

  CLIPSValue *insdata = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  if (pinInstanceInInput != nullptr)
  {
    insdata->instanceValue = pinInstanceInInput;
  }
  else
  {
    insdata->instanceValue = FindInstance(theEnv, NULL, pinArg, true);
  }

  if (insdata->instanceValue == nullptr)
  {
    SetErrorValue(theEnv, theArg.header);
    UDFInvalidArgumentMessage(context, "symbol with the name of an already registered pin");
    UDFThrowError(context);
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    return;
  }

  InstanceSlot *modeSlot = FindInstanceSlot(theEnv, insdata->instanceValue, CreateSymbol(theEnv, "mode"));
  if (modeSlot == nullptr)
  {
    SetErrorValue(theEnv, theArg.header);
    UDFInvalidArgumentMessage(context, "instance name of PIN class");
    UDFThrowError(context);
  }
  else
  {
    if (digitalRead(pin) != 0)
    {
      returnValue->lexemeValue = CreateSymbol(theEnv, "HIGH");
    }
    else
    {
      returnValue->lexemeValue = CreateSymbol(theEnv, "LOW");
    }

    CLIPSValue *newVal = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
    newVal->lexemeValue = returnValue->lexemeValue;
    PutSlotError putNewValErr = DirectPutSlot(insdata->instanceValue, "value", newVal);
    genfree(theEnv, newVal, sizeof(CLIPSValue));
    if (putNewValErr != PutSlotError::PSE_NO_ERROR)
    {
      Writeln(theEnv, "Something goes wrong with direct-put-value in digital-read");
      UDFThrowError(context);
      return;
    }
  }
  genfree(theEnv, insdata, sizeof(CLIPSValue));
}

void DigitalWriteFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  UDFValue nextPossible;
  const char *pinArg = nullptr;
  const char *stateArg;
  Instance *pinInstanceInInput = nullptr;

  if (!UDFNthArgument(context, 1, INSTANCE_BITS | SYMBOL_BIT, &nextPossible))
  {
    return;
  }

  if (CVIsType(&nextPossible, SYMBOL_BIT | INSTANCE_NAME_BIT))
  {
    pinArg = nextPossible.lexemeValue->contents;
  }
  else if (CVIsType(&nextPossible, INSTANCE_ADDRESS_BIT))
  {
    pinInstanceInInput = nextPossible.instanceValue;
    pinArg = nextPossible.instanceValue->name->contents;
  }

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
  stateArg = nextPossible.lexemeValue->contents;
  if (stateArg == nullptr)
  {
    SetErrorValue(theEnv, nextPossible.header);
    UDFThrowError(context);
    return;
  }

  int pin = getPinFromName(pinArg);
  if (pin < 0)
  {
    UDFInvalidArgumentMessage(context, "symbol of a valid pin name");
    UDFThrowError(context);
    return;
  }

  CLIPSValue *insdata = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  if (pinInstanceInInput != nullptr)
  {
    insdata->instanceValue = pinInstanceInInput;
  }
  else
  {
    insdata->instanceValue = FindInstance(theEnv, NULL, pinArg, true);
  }

  if (insdata->instanceValue == nullptr)
  {
    UDFInvalidArgumentMessage(context, "symbol with the name of an already registered pin");
    UDFThrowError(context);
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    return;
  }

  InstanceSlot *modeSlot = FindInstanceSlot(theEnv, insdata->instanceValue, CreateSymbol(theEnv, "mode"));
  if (modeSlot == nullptr)
  {
    UDFInvalidArgumentMessage(context, "instance name of PIN class");
    UDFThrowError(context);
  }
  else
  {
    if (strcmp(modeSlot->lexemeValue->contents, "OUTPUT") != 0)
    {
      UDFInvalidArgumentMessage(context, "pin with OUTPUT mode");
      UDFThrowError(context);
      genfree(theEnv, insdata, sizeof(CLIPSValue));
      return;
    }

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
      UDFInvalidArgumentMessage(context, "symbol with value LOW or HIGH");
      UDFThrowError(context);
    }
  }
  genfree(theEnv, insdata, sizeof(CLIPSValue));
}

/**
 * Ex.: (pin-mode D5 OUTPUT)
 */
void PinModeFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  UDFValue nextPossible;
  const char *pinArg;
  const char *modeArg;

  // CONTROLLO ARGOMENTI
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

  // CONTROLLO ARGOMENTI
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

  // VALIDAZIONE ARGOMENTO PIN
  int pin = getPinFromName(pinArg);
  if (pin < 0)
  {
    UDFInvalidArgumentMessage(context, "symbol of a valid pin name");
    UDFThrowError(context);
    return;
  }

  // SE NON ESISTE ISTANZA PER IL PIN INDICATO ALLORA LA CREO E RITORNO
  CLIPSValue *insdata = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  returnValue->instanceValue = FindInstance(theEnv, NULL, pinArg, true);

  if (returnValue->instanceValue == nullptr)
  {
    genfree(theEnv, insdata, sizeof(CLIPSValue));
    String makeInstCmd = "(";
    makeInstCmd += pinArg;
    makeInstCmd += " of PIN ";
    makeInstCmd += "(mode ";
    // todo: add default value (PULLUP->HIGH, etc..)

    gpio_num_t gpioNum = static_cast<gpio_num_t>(pin);
    if (gpioNum == GPIO_NUM_NC)
    {
      UDFInvalidArgumentMessage(context, "symbol of a valid pin name");
      UDFThrowError(context);
      return;
    }

    esp_err_t resetErr = gpio_reset_pin(gpioNum);
    if (resetErr != ESP_OK)
    {
      Writeln(theEnv, "Something goes wrong with the IOMUX for this pin and the GPIO function.");
      UDFThrowError(context);
      return;
    }

    if (modeArg != nullptr && strcmp(modeArg, "INPUT") == 0)
    {
      pinMode(pin, INPUT);
      makeInstCmd += "INPUT";
    }
    else if (modeArg != nullptr && strcmp(modeArg, "OUTPUT") == 0)
    {
      pinMode(pin, OUTPUT);
      makeInstCmd += "OUTPUT";
    }
    else if (modeArg != nullptr && strcmp(modeArg, "PULLUP") == 0)
    {
      pinMode(pin, PULLUP);
      makeInstCmd += "OUTPUT";
    }
    else if (modeArg != nullptr && strcmp(modeArg, "INPUT_PULLUP") == 0)
    {
      pinMode(pin, INPUT_PULLUP);
      makeInstCmd += "INPUT";
    }
    else if (modeArg != nullptr && strcmp(modeArg, "PULLDOWN") == 0)
    {
      pinMode(pin, PULLDOWN);
      makeInstCmd += "OUTPUT";
    }
    else if (modeArg != nullptr && strcmp(modeArg, "INPUT_PULLDOWN") == 0)
    {
      pinMode(pin, INPUT_PULLDOWN);
      makeInstCmd += "INPUT";
    }
    else if (modeArg != nullptr && strcmp(modeArg, "OPEN_DRAIN") == 0)
    {
      pinMode(pin, OPEN_DRAIN);
      makeInstCmd += "INPUT";
    }
    else if (modeArg != nullptr && strcmp(modeArg, "OUTPUT_OPEN_DRAIN") == 0)
    {
      pinMode(pin, OUTPUT_OPEN_DRAIN);
      makeInstCmd += "OUTPUT";
    }
    // else if (modeArg != nullptr && strcmp(modeArg, "ANALOG") == 0)
    else
    {
      UDFInvalidArgumentMessage(context, "symbol with value INPUT, OUTPUT, PULLUP, INPUT_PULLUP, PULLDOWN, INPUT_PULLDOWN, OPEN_DRAIN, OUTPUT_OPEN_DRAIN");
      UDFThrowError(context);
      return;
    }

    makeInstCmd += "))";
    returnValue->instanceValue = MakeInstance(theEnv, makeInstCmd.c_str()); // todo: Definstances?
    // todo: gpio_dump_io_configuration(buffer, pin); parse(buffer); ...
    return;
  }

  // ALTRIMENTI LA MODIFICO
  insdata->instanceValue = returnValue->instanceValue;

  // VERIFICO SE L'ISTANZA POSSIEDE LO SLOT MODE
  InstanceSlot *modeSlot = FindInstanceSlot(theEnv, insdata->instanceValue, CreateSymbol(theEnv, "mode"));
  if (modeSlot == nullptr)
  {
    UDFInvalidArgumentMessage(context, "instance name of PIN class");
    UDFThrowError(context);
  }
  else
  {
    // PROCEDO CON LA MODIFICA
    if (strcmp(modeArg, "INPUT") == 0)
    {
      pinMode(pin, INPUT);
      Send(theEnv, insdata, "put-mode", "INPUT", NULL);
      returnValue->instanceValue = insdata->instanceValue;
    }
    else if (strcmp(modeArg, "OUTPUT") == 0)
    {
      pinMode(pin, OUTPUT);
      Send(theEnv, insdata, "put-mode", "OUTPUT", NULL);
      returnValue->instanceValue = insdata->instanceValue;
    }
    else if (strcmp(modeArg, "PULLUP") == 0)
    {
      pinMode(pin, PULLUP);
      Send(theEnv, insdata, "put-mode", "OUTPUT", NULL);
      returnValue->instanceValue = insdata->instanceValue;
    }
    else if (strcmp(modeArg, "INPUT_PULLUP") == 0)
    {
      pinMode(pin, INPUT_PULLUP);
      Send(theEnv, insdata, "put-mode", "INPUT", NULL);
      returnValue->instanceValue = insdata->instanceValue;
    }
    else if (strcmp(modeArg, "PULLDOWN") == 0)
    {
      pinMode(pin, PULLDOWN);
      Send(theEnv, insdata, "put-mode", "OUTPUT", NULL);
      returnValue->instanceValue = insdata->instanceValue;
    }
    else if (strcmp(modeArg, "INPUT_PULLDOWN") == 0)
    {
      pinMode(pin, INPUT_PULLDOWN);
      Send(theEnv, insdata, "put-mode", "INPUT", NULL);
      returnValue->instanceValue = insdata->instanceValue;
    }
    else if (strcmp(modeArg, "OPEN_DRAIN") == 0)
    {
      pinMode(pin, OPEN_DRAIN);
      Send(theEnv, insdata, "put-mode", "INPUT", NULL);
      returnValue->instanceValue = insdata->instanceValue;
    }
    else if (strcmp(modeArg, "OUTPUT_OPEN_DRAIN") == 0)
    {
      pinMode(pin, OUTPUT_OPEN_DRAIN);
      Send(theEnv, insdata, "put-mode", "OUTPUT", NULL);
      returnValue->instanceValue = insdata->instanceValue;
    }
    // else if (strcmp(modeArg, "ANALOG") == 0)
    else
    {
      UDFInvalidArgumentMessage(context, "symbol with value INPUT, OUTPUT, PULLUP, INPUT_PULLUP, PULLDOWN, INPUT_PULLDOWN, OPEN_DRAIN, OUTPUT_OPEN_DRAIN");
      UDFThrowError(context);
      return;
    }
  }
  genfree(theEnv, insdata, sizeof(CLIPSValue));
}

void PinResetFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  UDFValue theArg;
  const char *pinArg = nullptr;
  Instance *pinInstanceInInput = nullptr;

  if (!UDFFirstArgument(context, INSTANCE_BITS | SYMBOL_BIT, &theArg))
  {
    return;
  }

  if (CVIsType(&theArg, SYMBOL_BIT | INSTANCE_NAME_BIT))
  {
    pinArg = theArg.lexemeValue->contents;
  }
  else if (CVIsType(&theArg, INSTANCE_ADDRESS_BIT))
  {
    pinInstanceInInput = theArg.instanceValue;
    pinArg = theArg.instanceValue->name->contents;
  }

  if (pinArg == nullptr)
  {
    SetErrorValue(theEnv, theArg.header);
    UDFThrowError(context);
    return;
  }

  int pin = getPinFromName(pinArg);
  if (pin < 0)
  {
    SetErrorValue(theEnv, theArg.header);
    UDFInvalidArgumentMessage(context, "symbol of a valid pin name");
    UDFThrowError(context);
    return;
  }

  CLIPSValue *insdata = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
  if (pinInstanceInInput != nullptr)
  {
    insdata->instanceValue = pinInstanceInInput;
  }
  else
  {
    insdata->instanceValue = FindInstance(theEnv, NULL, pinArg, true);
  }

  if (insdata->instanceValue != nullptr)
  {
    returnValue->instanceValue = insdata->instanceValue;
  }

  gpio_num_t gpioNum = static_cast<gpio_num_t>(pin);
  if (gpioNum == GPIO_NUM_NC)
  {
    UDFInvalidArgumentMessage(context, "symbol of a valid pin name");
    UDFThrowError(context);
    return;
  }

  esp_err_t resetErr = gpio_reset_pin(gpioNum);
  if (resetErr != ESP_OK)
  {
    Writeln(theEnv, "Something goes wrong with the IOMUX for this pin and the GPIO function.");
    UDFThrowError(context);
    return;
  }

  genfree(theEnv, insdata, sizeof(CLIPSValue));
}
