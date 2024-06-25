#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include "hempb.h"

// Pinbelegungen von https://github.com/Xinyuan-LilyGO/TTGO-T-Display
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23
#define TFT_BL 4

// Konstruktor für das Datenobjekt tft
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

#include "hemp.h"
#include <BME280I2C.h>
#include <Wire.h>

/* * * * * WIFI  * * * * */
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
WiFiMulti wifiMulti;

String ssid = "";
char* password = "";
int onoff = 0;

/* * * * * SHELLY  * * * * */
String shelly_sn = "", shelly_id;
int shelly_RSSI;
int shelly_on = 0;
unsigned int shellys = 0, trigger_radon, trigger_iaq;
unsigned int shelly_pow, shelly_meter, shelly_return;

BME280I2C bme;         // Default : forced mode, standby time = 1000 ms
float BMEtemp, BMEhum; // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,

// macadr -> sn
unsigned long mac_adr;
String sn = "";

void setup(void) {
  delay(3000);
  Serial.begin(115200);
  delay(1000);
  pinMode(TFT_BL, OUTPUT);      // TTGO T-Display Backlight Pin 4 aktivieren

  Serial.println(F("Nufsitop V0.2"));

  delay(1000);

  sn = generateMacSerial();
  Serial.println("Flash:" + showDateGerman() + "SN: " + sn);

  delay(1000);

  initBME280();
  printBME280();

  delay(1000);

  wifi_scan();

  delay(1000);

  digitalWrite(TFT_BL, HIGH);   // T-Display Backlight einschalten
  tft.init(135, 240);           // ST7789 initialisieren 240x135
  tft.setRotation(3);           // Optional: Display-Rotation einstellen
  Serial.println(F("showLogo"));
  showLogo();

  wifiMulti.addAP("Studio2", "Wolf1212");
}

void loop() {
  printBME280();
  sendDataWifi();
  delay(300000);

//   if (shelly_on == 0) {   
//     shelly_wifi(1);
//   } else {
//     shelly_wifi(0);
//   }
}

String showDateGerman() {
  const char* months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  const char* monthNumbers[12] = {"01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12"};
  String originalDate = __DATE__;
  String monthString = originalDate.substring(0, 3);
  String day = originalDate.substring(4, 6);
  String year = originalDate.substring(9, 11);
  String month = "00";
  for (int i = 0; i < 12; i++) {
    if (monthString == months[i]) {
      month = monthNumbers[i];
      break;
    }
  }
  return day + "." + month + "." + year;
}

String convertToArbitraryBase(unsigned long value, int base) {
  static char baseChars[] = "ZWAFBCDEFGHIJKLMNZPFRSTUV71234WXYZABCDEF7123456789GHIJKLM6789NZPHRSTUVWXYZPB";
  String result = "";
  do {
    result = String(baseChars[value % base]) + result; // Add on the left
    value /= base;
  } while (value != 0);
  return result; 
} // Spezielle convertToArbitraryBase Funktion

String generateMacSerial() {
  WiFi.begin();  // Start WiFi to get MAC address
  String macStr = WiFi.macAddress();  // Buffer for the MAC address
  uint8_t baseMac[6];  // Parse MAC address
  sscanf(macStr.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
         &baseMac[0], &baseMac[1], &baseMac[2], &baseMac[3], &baseMac[4], &baseMac[5]);
  unsigned long mac_adr = (baseMac[2]) + (baseMac[3] * 213) + (baseMac[4] * 231 * 253) + (baseMac[5] * 237 * 219 * 251);
  mac_adr = mac_adr + 1321754191; // Calculate mac_adr
  String sn = convertToArbitraryBase(mac_adr, 68); // Convert to arbitrary base
  if (sn.length() > 5) { // Ensure sn is not longer than 5 characters
    sn = sn.substring(0, 5);
  }
  return sn;
}


int sendDataWifi() {
  if ((wifiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    //                            Sensor;GSMTRY;BQM;STATUS;TMP;HUM;HPA;IAQ;STA_IAQ;POW_1;METER;$EPOCH

    http.begin("http://con.radocon.de/r.php?a="+sn+";1;0;1;"+String(BMEtemp*10, 0)+";"+String(BMEhum*10, 0)+";990;0;0;0;0;1");  //HTTP
    Serial.print("[HTTP] GET...\n");
    int httpCode = http.GET();
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
  return 0; // Füge einen Rückgabewert hinzu, um sicherzustellen, dass die Funktion korrekt endet
}

int shelly_wifi(int shelly_task) {
    long timeout = millis();                // Speichert den aktuellen Zeitpunkt in Millisekunden
    unsigned int shelly_return = 9999;      // Initialwert für den Rückgabewert
    ssid = "ShellyPlusPlugS-" + shelly_sn;  // Setzt die SSID für das Shelly-Gerät
    
    WiFi.mode(WIFI_STA);                    // Setzt den WiFi-Modus auf Station
    WiFi.begin(ssid.c_str(), password);     // Verbindet mit dem WiFi-Netzwerk
    
    while (WiFi.status() != WL_CONNECTED) { // Wartet, bis die WiFi-Verbindung hergestellt ist
      if ((millis() - timeout) > 15000) {   // Bricht nach 15 Sekunden ab
        break;
      }
    }

    shelly_id = "1 ID:" + WiFi.SSID() + "S:" + (WiFi.RSSI() * -1); // Setzt die shelly_id mit SSID und Signalstärke
    delay(100);                             // Wartet 100 Millisekunden
    const uint16_t port = 80;               // Setzt den Zielport
    const char * host = "192.168.33.1";     // Setzt die Ziel-IP-Adresse
    WiFiClient client;                      // Erstellt einen WiFiClient

    if (!client.connect(host, port)) {      // Verbindet zum Host, falls nicht erfolgreich
        shelly_return = 8888;               // Setzt den Rückgabewert auf 8888
        return shelly_return;               // Gibt den Rückgabewert zurück
    }

    switch (shelly_task) {                  // Führt eine Aufgabe basierend auf shelly_task aus
      case 0:
        client.print("GET /relay/0?turn=off HTTP/1.1\n\n");   // Schaltet das Relais aus
        shelly_return = 0;
        shelly_on = 0;
        break;
      case 1:
        client.print("GET /relay/0?turn=on HTTP/1.1\n\n");    // Schaltet das Relais ein
        shelly_return = 1;
        shelly_on = 1;
        break;
      default:
        client.print("GET /meter/0 HTTP/1.1\n\n");            // Liest den Zählerwert
        shelly_return = 0;
    }

    int maxloops = 0;                        // Initialisiert eine Schleifenvariable
    while (!client.available() && maxloops < 1000) {          // Wartet, bis Daten verfügbar sind
        maxloops++;
        delay(1);                            // Wartet 1 Millisekunde
    }

    String watt;  
    if (client.available() > 0) {             // Wenn Daten verfügbar sind
        String line = client.readStringUntil('{'); // Liest bis zum ersten '{'
        line = client.readStringUntil(':');   // Liest bis zum ersten ':'
        line = client.readStringUntil(',');   // Liest bis zum ersten ','
        shelly_id = line + " " + shelly_id;   // Aktualisiert die shelly_id
        watt = line;                          // Setzt watt auf den gelesenen Wert
    } else {
        Serial.println("client.available() timed out "); // Meldet Timeout
    }

    client.stop();                            // Beendet die Verbindung zum Server
    WiFi.disconnect();                        // Trennt die WiFi-Verbindung
    shelly_id = shelly_id + " T:" + (millis() - timeout) + "ms"; // Aktualisiert shelly_id mit der verstrichenen Zeit

    if (shelly_task > 1) {
        shelly_return = watt.toInt();         // Setzt den Rückgabewert auf den gelesenen watt-Wert
    } 

    return shelly_return;                     // Gibt den Rückgabewert zurück
}


void wifi_scan()
{
    WiFi.mode(WIFI_STA);
    disconnectWifi();
    int routerRSSI;
    String router, shelly;
    int16_t n = WiFi.scanNetworks();
    shellys = 0;
    if (n == 0) {
      Serial.println("> keine Netzwerke");
    } else {
         Serial.printf("gefunden %d net\n", n);
        for (int i = 0; i < n; ++i) {
            router = WiFi.SSID(i).c_str();  
            routerRSSI = 101 + WiFi.RSSI(i);     
            //Serial.println(router + " " + String(routerRSSI));          
            if (router.length() == 28){
              if (router.substring(0,16) == "ShellyPlusPlugS-"){
                Serial.print("> Shelly SN: ");
                shelly_RSSI = routerRSSI;                  
                shelly_sn = router.substring(16,28);;
                Serial.println(shelly_sn + " - RSSI: " + String(shelly_RSSI));         
                shellys = 1;       
              }
            }
        }
    }
    disconnectWifi();
    WiFi.mode(WIFI_OFF);
    delay(100);
}

void disconnectWifi() {
    if (WiFi.status() == WL_CONNECTED) {    
        WiFi.disconnect(true);
        delay(100);
    }
 }

void initBME280() {
  Wire.begin();
  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }
  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");
  }
  delay(100);
  BME280Data();
  delay(100);  
  BME280Data();
  delay(100);  
}

void printBME280()
{
  BME280Data();
  delay(100);  
  BME280Data();
  Serial.println("Tmp: " + String(BMEtemp,0) + "C -  Hum:" + String(BMEhum,0) + "%");
}

void BME280Data()
{
  float temp(NAN), hum(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  bme.read(pres, temp, hum, tempUnit, presUnit);
  BMEtemp = temp;
  BMEhum = hum;
}


