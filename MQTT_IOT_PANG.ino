#include <ESP8266WiFi.h>
#include <uMQTTBroker.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

#define WORKING_LED D0
#define AUTO_MODE D1
#define MANUAL_MODE D2
#define WIFI_AP_NAME "ESP8266 AP mode"
#define WIFI_AP_PASS NULL

class myMQTTBroker: public uMQTTBroker
{
  public:
    virtual bool onConnect(IPAddress addr, uint16_t client_count) {
      Serial.println(addr.toString() + " connected");
      return true;
    }

    virtual bool onAuth(String username, String password) {
      Serial.println("Username/Password: " + username + "/" + password);
      return true;
    }

    virtual void onData(String topic, const char *data, uint32_t length) {
      String message;
      char data_str[length + 1];
      os_memcpy(data_str, data, length);
      data_str[length] = '\0';

      Serial.println("received topic '" + topic + "' with data '" + (String)data_str + "'");
      message = (String)data_str;
      message.trim();

      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, message);

      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
      }

      String mode = doc["mode"];
      if (mode == "settings") {
        settingWIFI(doc);
      } else if (mode == "control") {
        controlLED(doc);
      }
    }
    virtual void WORKING_AUTO_MODE(){
      Serial.println("---------- AUTO MODE ----------");
      digitalWrite(AUTO_MODE, HIGH);
      digitalWrite(MANUAL_MODE, LOW);
    }
    virtual void WORKING_MANUAL_MODE(){
      Serial.println("---------- MANUAL MODE ----------");
      digitalWrite(AUTO_MODE, LOW);
      digitalWrite(MANUAL_MODE, HIGH);
    }
    virtual void controlLED(StaticJsonDocument<200> root) {
//      digitalWrite(root["led"], (root["cmd"] == 0) ? 0 : 1);
      String autoORmanual = root["working"];
      if(autoORmanual=="auto"){
         WORKING_AUTO_MODE();
      }else if (autoORmanual=="manual"){
        WORKING_MANUAL_MODE();
      }
    }
    virtual void settingWIFI(StaticJsonDocument<200> root) {
      String qsid = root["ssid"];
      String qpass = root["pwd"];
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 32; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i) {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i) {
          EEPROM.write(16 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        EEPROM.commit();
        Serial.println("Please press RESET BUTTON and waitting for connecting again...");
      }
    }
};

myMQTTBroker myBroker;

/*
   Your WiFi config here
*/
char ssid[] = "*********";      // Replace with your network SSID (name)
char pass[] = "*********"; // Replace with your network password
long lastMsg = 0;
String _tssid = "", _tpass = "";
/*
   WiFi init stuff
*/
bool startWiFiClient(char _ssid[], char _pass[])
{
  Serial.println("Waiting for Wifi to connecting to " + (String)ssid);
  WiFi.begin(_ssid, _pass);
  int c = 0;
  while (c < 50) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    Serial.print(".");
    c++;
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void startWiFiAP()
{
  Serial.println("AP started");
  Serial.println("Please connect to IP address below for configurate WIFI");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_NAME, WIFI_AP_PASS);

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  myBroker.subscribe("iot101/settings");
}

void CHECK_HAVE_WIFI_EEPROM() {
  Serial.println("Reading EEPROM ssid");
  for (int i = 0; i < 16; ++i)
  {
    _tssid += char(EEPROM.read(i));
  }
  Serial.println("Reading EEPROM pass");
  String pass = "";
  for (int i = 16; i < 32; ++i)
  {
    _tpass += char(EEPROM.read(i));
  }
  _tssid.trim();
  _tpass.trim();
  Serial.println("---------- CONFIGURATE SSID ----------");
  Serial.println(_tssid);
  Serial.println(_tpass);
  Serial.println("-------------- END SSID --------------");
}

void setup()
{
  Serial.begin(9600);
  EEPROM.begin(512);
  delay(10);
  Serial.println();
  Serial.println();
  pinMode(WORKING_LED, OUTPUT);
  pinMode(AUTO_MODE, OUTPUT);
  pinMode(MANUAL_MODE, OUTPUT);
  digitalWrite(WORKING_LED, HIGH);
  digitalWrite(AUTO_MODE, LOW);
  digitalWrite(MANUAL_MODE, LOW);
  // Connect to a WiFi network
  WiFi.mode(WIFI_AP_STA);
  CHECK_HAVE_WIFI_EEPROM();
  char s[_tssid.length()];
  char p[_tpass.length()];
  _tssid.toCharArray(s, _tssid.length() + 1);
  _tpass.toCharArray(p, _tpass.length() + 1);

  if (startWiFiClient(s, p)) {
    Serial.println();
    Serial.println("WiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());
    Serial.println("Starting MQTT broker");
    myBroker.init();
    myBroker.subscribe("iot/control");
    myBroker.subscribe("iot/settings");
    return;
  }
  WiFi.disconnect();
  delay(500);
  startWiFiAP();
  myBroker.init();
  myBroker.subscribe("iot/settings");
}

void loop() {}
