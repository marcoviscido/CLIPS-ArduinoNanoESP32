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
#include <WiFi.h>

#include "clips_wifi.h"

void WifiConnectFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
    UDFValue theSsid, thePwd;

    if (!UDFFirstArgument(context, STRING_BIT, &theSsid))
    {
        return;
    }
    if (theSsid.lexemeValue->contents == nullptr)
    {
        SetErrorValue(theEnv, theSsid.header);
        UDFThrowError(context);
        return;
    }

    if (!UDFNthArgument(context, 2, STRING_BIT, &thePwd))
    {
        return;
    }
    if (thePwd.lexemeValue->contents == nullptr)
    {
        SetErrorValue(theEnv, thePwd.header);
        UDFThrowError(context);
        return;
    }

    int attempt = 10;
    WiFi.begin(theSsid.lexemeValue->contents, thePwd.lexemeValue->contents);
    while (WiFi.status() != WL_CONNECTED && attempt != 0)
    {
        delay(2000);
        Writeln(theEnv, "Connecting to WiFi..");
        attempt--;
    }

    if (WiFi.status() != WL_CONNECTED && attempt <= 0)
    {
        Write(theEnv, "Unable to connect to SSID: ");
        Writeln(theEnv, theSsid.lexemeValue->contents);
        return;
    }

    String ip = "";
    ip += WiFi.localIP().toString();

    returnValue->lexemeValue = CreateString(theEnv, ip.c_str());

    String outMsg = "Connected to the Wi-Fi network with IP ";
    outMsg += ip;
    Writeln(theEnv, outMsg.c_str());
}

/*  by Tom Igoe */
void printWifiData()
{

    // print your WiFi shield's IP address:

    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    Serial.println(ip);

    // print your MAC address:

    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");
    Serial.print(mac[5], HEX);
    Serial.print(":");
    Serial.print(mac[4], HEX);
    Serial.print(":");
    Serial.print(mac[3], HEX);
    Serial.print(":");
    Serial.print(mac[2], HEX);
    Serial.print(":");
    Serial.print(mac[1], HEX);
    Serial.print(":");
    Serial.println(mac[0], HEX);
}

/*  by Tom Igoe */
void printCurrentNet()
{

    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print the MAC address of the router you're attached to:
    byte bssid[6];
    WiFi.BSSID(bssid);
    Serial.print("BSSID: ");
    Serial.print(bssid[5], HEX);
    Serial.print(":");
    Serial.print(bssid[4], HEX);
    Serial.print(":");
    Serial.print(bssid[3], HEX);
    Serial.print(":");
    Serial.print(bssid[2], HEX);
    Serial.print(":");
    Serial.print(bssid[1], HEX);
    Serial.print(":");
    Serial.println(bssid[0], HEX);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.println(rssi);

    // // TODO: print the encryption type:
    // byte encryption = WiFi.encryptionType();
    // Serial.print("Encryption Type:");
    // Serial.println(encryption, HEX);
    // Serial.println();
}

void WifiStatusFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
    printCurrentNet();
    printWifiData();
}