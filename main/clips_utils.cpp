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

#include "Arduino.h"
#include "clips.h"

#include "clips_utils.h"

AddUDFError AddUDFIfNotExists(
    Environment *theEnv,
    const char *clipsFunctionName,
    const char *returnTypes,
    unsigned short minArgs,
    unsigned short maxArgs,
    const char *argumentTypes,
    UserDefinedFunction *cFunctionPointer,
    const char *cFunctionName,
    void *context)
{
    AddUDFError addUDFError = AddUDFError::AUE_NO_ERROR;
    if (FindFunction(theEnv, clipsFunctionName) == NULL)
    {
        addUDFError = AddUDF(theEnv, clipsFunctionName, returnTypes, minArgs, maxArgs, argumentTypes, cFunctionPointer, cFunctionName, context);
        if (addUDFError != AddUDFError::AUE_NO_ERROR)
        {
            ESP_LOGE("ArduninoInitFunction", "Error adding %s - %i", clipsFunctionName, (int8_t)addUDFError);
        }
        else
        {
            ESP_LOGI("ArduninoInitFunction", "Function %s added", clipsFunctionName);
        }
    }
    return addUDFError;
}