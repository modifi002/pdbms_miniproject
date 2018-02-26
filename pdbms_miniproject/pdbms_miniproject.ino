/* i2c Address device
 * OLED Display 0x3C
 * BH1750 0x23
 * ADC 12 Bit 0x48
 */

#include "HX711.h"
#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>
#include "SSD1306.h"

//----------- set wifi--------------------------------
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

BH1750 lightMeter;

// Set OLED Display
SSD1306  display(0x3c, D2, D1);
int16_t Soil_value;

// Scale Settings
const int SCALE_DOUT_PIN = D3;
const int SCALE_SCK_PIN = D4;

HX711 scale(SCALE_DOUT_PIN, SCALE_SCK_PIN);

// Config DHT
#define DHTPIN D5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Config connect WiFi
#define WIFI_SSID "___________________"
#define WIFI_PASSWORD "_____________________"

// Deep Sleep
//#define SECONDS_DS(seconds) ((seconds)*1000000UL)
int httpCode;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  
  
  //scale.set_scale(2280.f);// <- set here calibration factor!!!
  scale.set_scale(752.f);// <- set here calibration factor!!!
  scale.tare();
  
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();

  lightMeter.begin();
  dht.begin();
  wifi();
  display.clear();
  

}

int n = 1;
void loop() {
  display.clear();
  Temp_Humi();
  Weight();
  lighMeter();
  Soil();
  //Send_data();.
  n++;
  display.display();
  if(n == 10) {
    Send_data();
    //if(httpCode == 200) Sleep();
    n = 0;
  }  
  //ESP.deepSleep(SECONDS_DS(5));

}

float h ;
float t ;
void Temp_Humi(){
  delay(50);
  h = dht.readHumidity();
  t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Temp = "); Serial.print(t); Serial.println(" °C");
  Serial.print("Humi = "); Serial.print(h); Serial.println(" %");
  
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, String(t)); display.drawString(40, 0, "°C");
  display.drawString(55, 0, " /"+ String(h)); display.drawString(105, 0, "%");
  
  
}
String weight;
void Weight(){
  delay(100);
  weight = String(scale.get_units(10)/1000, 2);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 15, "Weight : "+ weight);
  display.drawString(105, 15,"kg" );
  
  Serial.print("Weight= ");
  Serial.print(weight);
  Serial.println("  Kg.");
  
}

uint16_t lux;
void lighMeter(){
  
  lux = lightMeter.readLightLevel();
  //lux = 50;
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 30, "Light : "+String(lux));
  display.drawString(105, 30,"lx" );
  delay(100);

}

void Soil(){
  uint32_t start_time = millis();
  while((millis()- start_time) < 500){
    Soil_value = analogRead(A0);
    Soil_value = map(Soil_value,550,0,0,100);
  }
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 45, "Mositure : "+String(Soil_value));
  display.drawString(105, 45,"%" );
  
  Serial.print("Mositure= ");
  Serial.print(Soil_value);
  Serial.println("%");
  Serial.println("-------------------------------------------------------------");
}

void wifi(){
  WiFi.mode(WIFI_STA);
  //WiFiManager wifiManager;
  //wifiManager.setTimeout(300);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(5,25,"Disconnected ");
  display.display();
  delay(1000);
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(20,5,"Connecting");
  display.display();
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    //lcd.print(".");
    delay(500);
  }
  
}

void Send_data(){
  String url = "http://35.197.146.144/pdbms/insert/index.php?temp="+String(t)+String("&humi=")+String(h)+String("&light=")+String(lux)+String("&weight=")+String(weight)+String("&mosi=")+String(Soil_value);
  http.begin(url); //HTTP
  httpCode = http.GET();
  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
    http.end();
}

void Sleep(){
  delay(50);
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(15,5,"Sleep Mode");
  display.display();
  delay(2000);
  display.displayOff();
  //ESP.deepSleep(SECONDS_DS(10));
  ESP.deepSleep(20e6);
  
}


