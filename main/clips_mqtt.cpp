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

#include "clips_mqtt.h"

#include "WiFi.h"
#include "PubSubClient.h"
#include "main.h"

void MqttTestFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
    connectMqtt(theEnv);
}

// void MqttConnectFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
// {
//     UDFValue theArg;
//     Instance *theInstance;
//     const char *mqtt_broker = nullptr, *mqtt_topic = nullptr, *mqtt_username = nullptr, *mqtt_password = nullptr, *mqtt_client_id = nullptr;
//     int mqtt_port = -1;

//     // TODO:
//     // void (*myCallbackPointer)(char *, uint8_t *, unsigned int) = (void (*)(char *, uint8_t *, unsigned int))context;
//     // if (myCallbackPointer == nullptr)
//     // {
//     //     return;
//     // }
//     PubSubClient *psClient = (PubSubClient *)context;
//     if (psClient == nullptr)
//     {
//         return;
//     }
//     uint8_t argsCount = UDFArgumentCount(context);
//     if (argsCount != 1)
//     {
//         return;
//     }
//     if (!UDFNthArgument(context, 1, INSTANCE_ADDRESS_BIT, &theArg))
//     {
//         return;
//     }
//     if (WiFi.status() != WL_CONNECTED || psClient->connected())
//     {
//         return;
//     }

//     theInstance = theArg.instanceValue;
//     if (strcmp(theInstance->cls->header.name->contents, "MQTT") != 0)
//     {
//         Writeln(theEnv, "The instance type is not valid");
//         UDFThrowError(context);
//         return;
//     }

//     CLIPSValue *p1 = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
//     CLIPSValue *p2 = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
//     CLIPSValue *p3 = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
//     CLIPSValue *p4 = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
//     CLIPSValue *p5 = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
//     CLIPSValue *p6 = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
//     DirectGetSlot(theInstance, "broker", p1);
//     DirectGetSlot(theInstance, "port", p2);
//     DirectGetSlot(theInstance, "clientid", p3);
//     DirectGetSlot(theInstance, "usr", p4);
//     DirectGetSlot(theInstance, "pwd", p5);
//     DirectGetSlot(theInstance, "topic", p6);
//     mqtt_broker = p1->lexemeValue->contents;
//     mqtt_port = p2->integerValue->contents;
//     mqtt_client_id = p3->lexemeValue->contents;
//     mqtt_username = p4->lexemeValue->contents;
//     mqtt_password = p5->lexemeValue->contents;
//     mqtt_topic = p6->lexemeValue->contents;
//     genfree(theEnv, p1, sizeof(CLIPSValue));
//     genfree(theEnv, p2, sizeof(CLIPSValue));
//     genfree(theEnv, p3, sizeof(CLIPSValue));
//     genfree(theEnv, p4, sizeof(CLIPSValue));
//     genfree(theEnv, p5, sizeof(CLIPSValue));
//     genfree(theEnv, p6, sizeof(CLIPSValue));

//     psClient->setServer(mqtt_broker, mqtt_port);
//     psClient->setCallback(mqttCallback);
//     uint8_t attempt = 10;
//     while (!psClient->connected() && attempt != 0)
//     {
//         Write(theEnv, "The client ");
//         Write(theEnv, mqtt_client_id);
//         Writeln(theEnv, " connects to the public MQTT broker");
//         if (psClient->connect(mqtt_client_id, mqtt_username, mqtt_password))
//         {
//             Writeln(theEnv, "MQTT broker connected");
//             break;
//         }
//         else
//         {
//             Write(theEnv, "failed with state ");
//             WriteInteger(theEnv, STDOUT, psClient->state());
//             Writeln(theEnv, "");
//             delay(2000);
//         }
//         attempt--;
//     }
//     psClient->publish(mqtt_topic, "Hi, I'm ESP32 ^^");
//     psClient->subscribe(mqtt_topic);
// }