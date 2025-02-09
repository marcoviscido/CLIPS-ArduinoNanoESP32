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

void WifiBeginFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
    UDFValue theSsid, thePwd;
    const char *ssid = nullptr, *pwd = nullptr;

    uint8_t argsCount = UDFArgumentCount(context);
    if (argsCount == 1) // when argsCount == 1 the ssid arg contains the name of a WIFI instance
    {
        if (!UDFFirstArgument(context, INSTANCE_NAME_BIT, &theSsid))
        {
            return;
        }
        if (theSsid.lexemeValue->contents == nullptr)
        {
            SetErrorValue(theEnv, theSsid.header);
            UDFThrowError(context);
            return;
        }
        Instance *theInstance = FindInstance(theEnv, NULL, theSsid.lexemeValue->contents, true);
        if (strcmp(theInstance->cls->header.name->contents, "WIFI") != 0)
        {
            ESP_LOGE("WifiBeginFunction", "The type of the instance %s is not valid", theSsid.lexemeValue->contents);
            Writeln(theEnv, "The type of the instance is not valid");
            UDFThrowError(context);
            return;
        }
        CLIPSValue *ssidCv = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
        CLIPSValue *pwdCv = (CLIPSValue *)genalloc(theEnv, sizeof(CLIPSValue));
        DirectGetSlot(theInstance, "ssid", ssidCv);
        DirectGetSlot(theInstance, "pwd", pwdCv);
        ssid = ssidCv->lexemeValue->contents;
        pwd = pwdCv->lexemeValue->contents;
        genfree(theEnv, ssidCv, sizeof(CLIPSValue));
        genfree(theEnv, pwdCv, sizeof(CLIPSValue));
    }
    else if (argsCount == 2)
    {
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
        ssid = theSsid.lexemeValue->contents;

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
        pwd = thePwd.lexemeValue->contents;
    }

    uint8_t attempt = 10;
    WiFi.begin(ssid, pwd);
    while (WiFi.status() != WL_CONNECTED && attempt != 0)
    {
        delay(2000);
        Write(theEnv, "Connecting to WiFi...");
        Write(theEnv, " (");
        WriteInteger(theEnv, STDOUT, attempt);
        Writeln(theEnv, ")");

        attempt--;
    }

    if (WiFi.status() != WL_CONNECTED && attempt <= 0)
    {
        Write(theEnv, "Unable to connect to SSID: ");
        Writeln(theEnv, ssid);
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

void WifiDisconnectFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
    WiFi.disconnect();
}

void WifiScanNetworksFunction(Environment *theEnv, UDFContext *context, UDFValue *returnValue)
{
    Serial.println("** Scan Networks **");
    int n = WiFi.scanNetworks();
    Serial.println("   done...");
    if (n == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found");
        Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
        for (int i = 0; i < n; ++i)
        {
            // Print SSID and RSSI for each network found
            Serial.printf("%2d", i + 1);
            Serial.print(" | ");
            Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
            Serial.print(" | ");
            Serial.printf("%4ld", WiFi.RSSI(i));
            Serial.print(" | ");
            Serial.printf("%2ld", WiFi.channel(i));
            Serial.print(" | ");
            switch (WiFi.encryptionType(i))
            {
            case WIFI_AUTH_OPEN:
                Serial.print("open");
                break;
            case WIFI_AUTH_WEP:
                Serial.print("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                Serial.print("WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                Serial.print("WPA2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                Serial.print("WPA+WPA2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                Serial.print("WPA2-EAP");
                break;
            case WIFI_AUTH_WPA3_PSK:
                Serial.print("WPA3");
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                Serial.print("WPA2+WPA3");
                break;
            case WIFI_AUTH_WAPI_PSK:
                Serial.print("WAPI");
                break;
            default:
                Serial.print("unknown");
            }
            Serial.println();
            delay(10);
        }
    }
    Serial.println("");

    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
}