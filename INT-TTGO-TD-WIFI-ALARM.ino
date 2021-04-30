#include <FS.h>
#include <SPIFFS.h>
#include <SPI.h>
#include "WiFi.h"
#include <Wire.h>
#include "esp_adc_cal.h"




#define FORMAT_SPIFFS_IF_FAILED true

#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34





int joystick;

char buff[512];
int vref = 1100;
int btnCick = false;
String output [6];

void setup()
{
    Serial.begin(115200);
    
    espDelay(2000);
    
    display_text("Start",3,800);
    
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
      display_text("SPIFFS Failed",3,800);
      espDelay(1000);
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        display_text("SPIFFS Broken",3,800);            
      } else {
        display_text("SPIFFS now OK",3,800); 
      }
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

  //  read_spiffs_to_TFT(SPIFFS, "/aps.csv");

    
    pinMode(ADC_EN, OUTPUT);
    digitalWrite(ADC_EN, HIGH);


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
  
    output[0] = "ABCDEFGHIJKLMNOPQRS"; 
    output[1] = "ZYXWVUTSRQPONMLKJIH"; 
    output[2] = "abcdefghijklmnopqrs"; 
    output[3] = "zyxwvutsrqponmlkjih"; 
    output[4] = "1234567890!§$%&=?*+"; 
    output[5] = "ABCDEFGHIJKLMNOPQRS"; 
        Serial.println(" ");
                 Serial.println(" ");
}

void loop()
{
    
    //showVoltage();

    wifi_scan();
    delay(29000);  

}

// START read_spiffs_to_TFT()
void read_spiffs_to_TFT(fs::FS &fs, const char * path){
    int i;
    Serial.println("Start");
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
    Serial.println(" ");
} // END read_spiffs_to_sram()

// START  display_text()
void display_text(String text, int tsize, int ttime) {
  Serial.println(text);


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
    }
}




void wifi_scan()
{
    delay(1000);
    Serial.println("Start");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(500);

    int16_t n = WiFi.scanNetworks();
    if (n == 0) {
        Serial.println("keine Netzwerke");
    } else {
        // Serial.printf("gefunden %d net\n", n);
        for (int i = 0; i < n; ++i) {
            String router, shelly;
            router = WiFi.SSID(i).c_str();
        
            
            if (router.length() == 19){
              
              shelly = router.substring(0,13);
              
              
              if(shelly == "shellyplug-s-"){

                Serial.print("Shelly: ");
                shelly = router.substring(13,19);
                Serial.println(shelly);
                }
              }



              
            // Serial.println(router);
            // sprintf(buff,
            //        "[%d]:%s(%d)",
            //        i + 1,
            //        router.c_str(),
            //        WiFi.RSSI(i));
            //Serial.println(buff);
            //router = String(i + 1) + ";" + String(WiFi.SSID(i).c_str()) + "\r\n";     
            //appendFile(SPIFFS, "/aps.csv", router);
        }
    }
    WiFi.mode(WIFI_OFF);
    Serial.println(" ");

    
}



void writeFile(fs::FS &fs, const char * path, const char * message){
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(!file.print(message)){
        Serial.println("- writeFile failed");
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
