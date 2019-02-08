#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

#define DHTTYPE DHT22
#define DHTPIN D4
#define BlynkLcd V2
#define BlynkTemp V0
#define BlynkHumid V1
#define BlynkLed V3
#define OnboardLED D0
char auth[]="cf2751dec23246959b7a3e9b7410890b";
char ssid[]="gga";
char pwd[]="1111111111";

DHT dht(DHTPIN, DHTTYPE, 15);
BlynkTimer timer;
WidgetLCD lcd(BlynkLcd);
WidgetLED led(BlynkLed);

void sendData_Tempo_Humid(){
 float temp = dht.readTemperature();
 float humid = dht.readHumidity();
 Blynk.virtualWrite(BlynkTemp,temp);
 Blynk.virtualWrite(BlynkHumid,humid);
 Serial.print("Temp: ");
 Serial.print(temp);
 Serial.print(",Humid: ");
 Serial.println(humid);
 if(temp > 27.5){
  lcd.clear();
  lcd.print(0,0,"Tempo UP");
  led.on();
  digitalWrite(OnboardLED,HIGH);
  //Blynk.notify("Hey Now The Temperature is going up !!!");
 }else{
  lcd.clear();
  lcd.print(0,0,"Regularly Value");
  led.off();
  digitalWrite(OnboardLED,LOW);
 }
 delay(1000);
}

void setup(){
  pinMode(OnboardLED,OUTPUT);
  digitalWrite(OnboardLED,LOW);
  dht.begin();
  Serial.begin(9600);
  Blynk.begin(auth,ssid,pwd);
  lcd.clear();
  lcd.print(0,0,"STARTED!");
  delay(2000);
  timer.setInterval(1000L,sendData_Tempo_Humid);
 }
 
void loop(){
  
  Blynk.run();
  timer.run();  
  
}
