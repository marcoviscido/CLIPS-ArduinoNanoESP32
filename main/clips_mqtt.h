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

#ifndef _H_CLIPS_MQTT_H

#pragma once

#define _H_CLIPS_MQTT_H

#include <string>
#include "clips.h"

struct MqttRouterData
{
    /**
     * The MQTT instance
     */
    Instance *mqttInstance;
    const char *sender;
    const char *msgId;
};

void MqttConnectFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue);
void MqttDisconnectFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue);
void MqttPublishFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue);

void mqtt_on_connected_cb(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void mqtt_on_data_cb(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void WriteMqttReplyCallback(Environment *environment, const char *logicalName, const char *str, void *context);
bool QueryMqttReplyCallback(Environment *environment, const char *logicalName, void *context);

#endif