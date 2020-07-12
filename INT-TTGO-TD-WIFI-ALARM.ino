#include <FS.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include <Button2.h>
#include "esp_adc_cal.h"
#include "bmp.h"

#define TFT_DISPOFF 0x28
#define TFT_SLPIN   0x10

#define FORMAT_SPIFFS_IF_FAILED true

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23

#define TFT_BL              4   // Display backlight control pin
#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
#define BUTTON_1            35
#define BUTTON_2            0

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

char buff[512];
int vref = 1100;
int btnCick = false;
String output [6];

void setup()
{
    Serial.begin(115200);
    Serial.println("Start");

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    if (TFT_BL > 0) {               
        pinMode(TFT_BL, OUTPUT);                
        digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
    }
    
    tft.setSwapBytes(true);
    tft.pushImage(0, 0,  240, 135, ttgo);
    espDelay(2000);
    
    display_text("Start",3,800);

    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
      display_text("SPIFFS Failed",3,800);
      return;
    } else {
      display_text("SPIFFS OK",3,800);    
    }

    if(!SPIFFS.exists("/ap.csv")){
      writeFile(SPIFFS, "/ap.csv", "ID;NAME\r\n");
      display_text("AP Init",3,800); 
    } else {
      display_text("AP OK",3,800);    
    }

    if(!SPIFFS.exists("/aps.csv")){
      writeFile(SPIFFS, "/aps.csv", "ID;NAME\r\n");
      display_text("Suche Init",3,800); 
    } else {
      display_text("Suche OK",3,800);    
    }

    read_spiffs_to_TFT(SPIFFS, "/aps.csv");

    
    pinMode(ADC_EN, OUTPUT);
    digitalWrite(ADC_EN, HIGH);

    button_init();

    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize((adc_unit_t)ADC_UNIT_1, (adc_atten_t)ADC1_CHANNEL_6, (adc_bits_width_t)ADC_WIDTH_BIT_12, 1100, &adc_chars);
    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
    } else {
        Serial.println("Default Vref: 1100mV");
    }
  
    output[0] = "ANEMOX 0"; 
    output[1] = "ANEMOX 1"; 
    output[2] = "ANEMOX 2"; 
    output[3] = "ANEMOX 3";
    output[4] = "ANEMOX 4"; 
    output[5] = "ANEMOX 5"; 
    do_output();          
 
}

void loop()
{
    if (btnCick) {
        showVoltage();
    }
    button_loop();
}

// START read_spiffs_to_TFT()
void read_spiffs_to_TFT(fs::FS &fs, const char * path){
    int i;
    Serial.printf("Start Printout");
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed");
        return;
    }
    while(file.available()){
    i= file.readBytesUntil(';', buff, sizeof(buff));
      buff[i] = 0;
      display_text(buff,2,400);     
      i= file.readBytesUntil('\n', buff, sizeof(buff));
      buff[i] = 0;
      display_text(buff,2,400);  
    }
} // END read_spiffs_to_sram()

// START  display_text()
void display_text(String text, int tsize, int ttime) {
  tft.fillScreen(TFT_BLACK);
  espDelay(200);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(tsize);
  tft.drawString(text, tft.width() / 2, tft.height() / 2 - 16);
  espDelay(ttime);
  tft.setTextSize(2);
} // END  display_text()

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_light_sleep_start();
}

void showVoltage()
{
    static uint64_t timeStamp = 0;
    if (millis() - timeStamp > 1000) {
        timeStamp = millis();
        uint16_t v = analogRead(ADC_PIN);
        float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
        String voltage = "Voltage :" + String(battery_voltage) + "V";
        Serial.println(voltage);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(voltage,  tft.width() / 2, tft.height() / 2 );
    }
}

void button_init()
{
    btn1.setLongClickHandler([](Button2 & b) {
        btnCick = false;
        int r = digitalRead(TFT_BL);
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Press again to wake up",  tft.width() / 2, tft.height() / 2 );
        espDelay(6000);
        digitalWrite(TFT_BL, !r);

        tft.writecommand(TFT_DISPOFF);
        tft.writecommand(TFT_SLPIN);
        //After using light sleep, you need to disable timer wake, because here use external IO port to wake up
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
        // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
        delay(200);
        esp_deep_sleep_start();
    });
    btn1.setPressedHandler([](Button2 & b) {
        Serial.println("Detect Voltage..");
        btnCick = true;
    });

    btn2.setPressedHandler([](Button2 & b) {
        btnCick = false;
        Serial.println("btn press wifi scan");
        wifi_scan();
    });
}

void button_loop()
{
    btn1.loop();
    btn2.loop();
}

void wifi_scan()
{
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);

    tft.drawString("suche Netzwerke", tft.width() / 2, tft.height() / 2);

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    int16_t n = WiFi.scanNetworks();
    tft.fillScreen(TFT_BLACK);
    if (n == 0) {
        tft.drawString("keine Netzwerke", tft.width() / 2, tft.height() / 2);
    } else {
        deleteFile(SPIFFS, "/aps.csv");
        writeFile(SPIFFS, "/aps.csv", "");
        tft.setTextDatum(TL_DATUM);
        tft.setCursor(0, 0);
        Serial.printf("gefunden %d net\n", n);
        for (int i = 0; i < n; ++i) {
            String router;
            router = WiFi.SSID(i).c_str();
            router = router.substring(0,10);
            Serial.println(router);
            sprintf(buff,
                    "[%d]:%s(%d)",
                    i + 1,
                    router.c_str(),
                    WiFi.RSSI(i));
            tft.println(buff);
            router = String(i + 1) + ";" + String(WiFi.SSID(i).c_str()) + "\r\n";     
            appendFile(SPIFFS, "/aps.csv", router);
        }
    }
    WiFi.mode(WIFI_OFF);


    
}

void do_output()
{
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(output[0], 5, 3);
    tft.drawString(output[1], 5, 25);
    tft.drawString(output[2], 5, 47);
    tft.drawString(output[3], 5, 69);
    tft.drawString(output[4], 5, 91);
    tft.drawString(output[5], 5, 113);
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(!file.print(message)){
        Serial.println("- frite failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return;
    }

    Serial.println("- read from file:");
    while(file.available()){
        Serial.write(file.read());
    }
}

void deleteFile(fs::FS &fs, const char * path){
    if(!fs.remove(path)){
        Serial.println("- delete failed");
    }
}

void appendFile(fs::FS &fs, const char * path, String text){
    const char * message = text.c_str();
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    if(!file.print(message)){
        Serial.println("- append failed");
    }
}
