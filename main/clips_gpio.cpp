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
#include "clips_gpio.h"

void LedOnFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  digitalWrite(D5, HIGH);
  Serial.println("D5 high");
}

void LedOffFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  digitalWrite(D5, LOW);
  Serial.println("D5 low");
}

void PinModeFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
  UDFValue nextPossible;
  const char *arg1;
  const char *arg2;

  if (!UDFNthArgument(context, 1, LEXEME_BITS, &nextPossible))
  {
    return;
  }
  else
  {
    arg1 = nextPossible.lexemeValue->contents;
  }

  if (!UDFNthArgument(context, 2, LEXEME_BITS, &nextPossible))
  {
    return;
  }
  else
  {
    arg2 = nextPossible.lexemeValue->contents;
  }

  Serial.print(arg1);
  Serial.print("  ");
  Serial.print(arg2);
}