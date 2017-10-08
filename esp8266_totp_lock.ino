#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESP8266WiFi.h>
#include "ESP8266TOTP.h"
#include <Keypad.h>

const byte n_rows = 4;
const byte n_cols = 4;


char keys[n_rows][n_cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte colPins[n_rows] = {D3, D2, D1, D0};
byte rowPins[n_cols] = {D7, D6, D5, D4};
String userInput = "";
 //Initialize keypad object
  Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, n_rows, n_cols);
#ifndef WIFI_CONFIG_H
#define YOUR_WIFI_SSID "linksys"
#define YOUR_WIFI_PASSWD ""
#endif // !WIFI_CONFIG_H

#define ONBOARDLED 2 // Built in LED on ESP-12/ESP-07

// Start NTP only after IP network is connected
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {
  Serial.printf("Got IP: %s\r\n", ipInfo.ip.toString().c_str());
  NTP.begin("pool.ntp.org", 1, true);
  NTP.setInterval(63);
  digitalWrite(ONBOARDLED, LOW); // Turn on LED
}

// Manage network disconnection
void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {
  Serial.printf("Disconnected from SSID: %s\n", event_info.ssid.c_str());
  Serial.printf("Reason: %d\n", event_info.reason);
  digitalWrite(ONBOARDLED, HIGH); // Turn off LED
  //NTP.stop(); // NTP sync can be disabled to avoid sync errors
}

void processSyncEvent(NTPSyncEvent_t ntpEvent) {
  if (ntpEvent) {
    Serial.print("Time Sync error: ");
    if (ntpEvent == noResponse)
      Serial.println("NTP server not reachable");
    else if (ntpEvent == invalidAddress)
      Serial.println("Invalid NTP server address");
  }
  else {
    Serial.print("Got NTP time: ");
    Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
  }
}

bool totpCheck(String usercode) {
  

}

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event

void setup()
{
  static WiFiEventHandler e1, e2;

  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);

  pinMode(ONBOARDLED, OUTPUT); // Onboard LED
  digitalWrite(ONBOARDLED, HIGH); // Switch off LED

  NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
    ntpEvent = event;
    syncEventTriggered = true;
  });

  e1 = WiFi.onStationModeGotIP(onSTAGotIP);// As soon WiFi is connected, start NTP Client
  e2 = WiFi.onStationModeDisconnected(onSTADisconnected);

}

void loop()
{

  static int i = 0;
  static int last = 0;

  if (syncEventTriggered) {
    processSyncEvent(ntpEvent);
    syncEventTriggered = false;
  }

   if ((millis() - last) > 5100) {
    //Serial.println(millis() - last);
    last = millis();
      Serial.print(NTP.getTimeDateString());
      
   }


  
  //Read the current key
  char myKey = myKeypad.getKey();

  if (myKey != NULL) {
    Serial.print("Key pressed: ");
    
    userInput = userInput + myKey;

    Serial.println(userInput);
  } 

  if (userInput.length() == 6) {
    
        totpData data;
    uint64_t STATIC_EPOCH = now();
    uint8_t statickey[] = {0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61};

    unsigned char data32[BASE_32_ENCODE_LENGTH];
    if (ESP8266TOTP::GetBase32Key(statickey, data32)) {

      Serial.println(reinterpret_cast<char*>(&data32));
      Serial.println(ESP8266TOTP::GetQrCodeImageUri(statickey, "Some host", "Some issuer"));


      int otp4 = ESP8266TOTP::GetTOTPToken(STATIC_EPOCH - 7200, statickey);
      Serial.println("Current Auth Pass:");
      Serial.println(otp4);
      ;
      if (otp4 == userInput.toInt()) {
        Serial.println("OTP is correct");
        pinMode(D8, OUTPUT);
        digitalWrite(D8, HIGH);
        delay(5000);
        digitalWrite(D8, LOW);  
        userInput = "";
        } else {
          Serial.println("Otp is incorrect");
          userInput = "";
          }
    } else {
      Serial.println("ESP8266TOTP::GetBase32Key failed");
    }
    } 

//////////////////////////////////////////////////////////


}
