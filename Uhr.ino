
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <LovyanGFX.hpp>
#include "Arduino.h"
#include "CST816D.h"

#define LGFX_USE_V1
#define wifi_ssid "SSID"                                  //wifi ssid
#define wifi_password "PASSWORD"                          //wifi password
#define mqtt_server "IP ADRESS"                           // server name or IP
#define mqtt_user "MQTT USERNAME"                         // username
#define mqtt_password "MQTT PASSWORD"                     // password
#define topic_1 "TOPIC Temperature"   // Wetterstation Lufttemperatur
#define MY_NTP_SERVER "at.pool.ntp.org"
#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"                // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
#define sans20 NotoSansMonoSCB20
#define I2C_SDA 4
#define I2C_SCL 5
#define TP_INT 0
#define TP_RST 1
#define off_pin 35
#define buf_size 120
#define sans20 NotoSansMonoSCB20

int currentSecond;
uint8_t       program_state = 3;
unsigned long lastStatus = 0;
time_t now;                                             // this are the seconds since Epoch (1970) - UTC
tm tm;                                                  // the structure tm holds time information in a more convenient way *
static const uint32_t screenWidth = 240;
static const uint32_t screenHeight = 240;
// Angenommen, Ihr Display hat eine Auflösung von 240x240 Pixel und der Mittelpunkt ist bei (120,120)
const int centerX = 120;
const int centerY = 120;
const int radius = 100; // Radius des Kreises, auf dem der Punkt wandert

WiFiClient espClient_TID;
PubSubClient client(espClient_TID);




class LGFX : public lgfx::LGFX_Device
{

  lgfx::Panel_GC9A01 _panel_instance;

  lgfx::Bus_SPI _bus_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = SPI2_HOST; 
      cfg.spi_mode = 0;                  
      cfg.freq_write = 80000000;         
      cfg.freq_read = 20000000;          
      cfg.spi_3wire = true;              
      cfg.use_lock = true;               
      cfg.dma_channel = SPI_DMA_CH_AUTO; 
      cfg.pin_sclk = 6;  
      cfg.pin_mosi = 7;  
      cfg.pin_miso = -1; 
      cfg.pin_dc = 2;    

      _bus_instance.config(cfg);              
      _panel_instance.setBus(&_bus_instance); 
    }

    {                                      
      auto cfg = _panel_instance.config(); 

      cfg.pin_cs = 10;   
      cfg.pin_rst = -1;  
      cfg.pin_busy = -1; 

      cfg.memory_width = 240;   
      cfg.memory_height = 240;  
      cfg.panel_width = 240;    
      cfg.panel_height = 240;   
      cfg.offset_x = 0;         
      cfg.offset_y = 0;         
      cfg.offset_rotation = 0;  
      cfg.dummy_read_pixel = 8; 
      cfg.dummy_read_bits = 1;  
      cfg.readable = false;     
      cfg.invert = true;        
      cfg.rgb_order = false;    
      cfg.dlen_16bit = false;   
      cfg.bus_shared = false;   

      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance); 
  }
};

LGFX tft;
CST816D touch(I2C_SDA, I2C_SCL, TP_RST, TP_INT);


void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.initDMA();
  tft.startWrite();
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  pinMode(0, INPUT);
  delay(500);
  //tft.unloadFont();
  //tft.loadFont(sans20);
  tft.fillScreen(TFT_BLUE);
  tft.setColorDepth(16);
  tft.setBrightness(10);
  Serial.println("\nNTP TZ DST - bare minimum");
  configTime(0, 0, MY_NTP_SERVER);    // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1);             // Set environment variable with your time zone
  tzset();
  setup_wifi();
  client.setServer(mqtt_server, 1883);    		// Configure MQTT connection, change port if needed.
  if (!client.connected()) {
    reconnect();
  }
  client.setCallback(callback);
  client.subscribe(topic_1);       //muss im reconnect auch geändert werden
  Serial.println("Setup Complete");
  tft.clear();
  showTime(); 
}


void loop() { 
  if (!client.connected()) {
    reconnect();
  }
  unsigned long currentTime = millis();

  
  int currentSecond = (currentTime / 1000) % 60; // Berechnet die Sekunden seit dem Start
  updateSecondHand(currentSecond);
  
  if (millis() - lastStatus > 60000) { 
      showTime(); 
      lastStatus = millis();
  }
  
  
  client.loop();
}


void callback(char* topic, byte* payload, unsigned int length) {
   String msg_1;

  //Callback für Wetterstation Lufttemperatur
  if (strcmp(topic,topic_1)==0) {
  Serial.print("Aktuelle Außentemperatur: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    char tmp = char(payload[i]);
    msg_1 += tmp;
  }
    tft.setTextSize(2);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setCursor(60, 170);
    tft.println(msg_1);
    tft.setCursor(135, 170);
    tft.println("Grad");
    Serial.println();
  }  


}



//Setup connection to wifi
void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("...-...");
  }

 Serial.println("");
 Serial.println("WiFi is UP ");
 Serial.print("=> ESP32 IP Addresse: ");
 Serial.print(WiFi.localIP());
 Serial.println("");
}






//Reconnect to wifi if connection is lost
void reconnect() {
  while (!client.connected()) {
    Serial.print("Verbinde zu MQTT broker ...");
    if (client.connect("ESP32 PID", mqtt_user, mqtt_password)) {
      Serial.println("OK");
      client.subscribe(topic_1);       
      client.publish("Uhr/Status", "Uhr is ONLINE");

    } else {
      Serial.print("[Error] Not connected: ");
      Serial.print(client.state());
      Serial.println("Wait 5 seconds before retry.");
      delay(5000);
    }
  }
}

void showTime() {
  time(&now); // read the current time
  localtime_r(&now, &tm);                     // update the structure tm with the current time
  tft.setTextSize(6);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setCursor(35, 100);
  tft.println(&tm, "%H:%M");
}


void updateSecondHand(int second) {
  static int lastSecond = -1; // Speichert die Sekunde der letzten Aktualisierung

  // Berechnet den Winkel in Radiant für die aktuelle Sekunde
  float angle = second * (2 * PI / 60) - (PI / 2); // -PI/2, da 0 Sekunden oben sein sollen

  // Berechnet die Position des Punktes basierend auf dem Winkel
  int pointX = centerX + radius * cos(angle);
  int pointY = centerY + radius * sin(angle);

  // Löscht den vorherigen Punkt, falls nötig
  if (lastSecond != -1 && lastSecond != second) { // Verhindert das Löschen, wenn es nicht nötig ist
    float lastAngle = lastSecond * (2 * PI / 60) - (PI / 2);
    int lastPointX = centerX + radius * cos(lastAngle);
    int lastPointY = centerY + radius * sin(lastAngle);
    tft.fillCircle(lastPointX, lastPointY, 3, TFT_BLACK); // Ändern Sie die Farbe entsprechend Ihrem Hintergrund
  }

  // Zeichnet den neuen Punkt
  tft.fillCircle(pointX, pointY, 3, TFT_RED); // Größe 3, Farbe Rot

  lastSecond = second; // Aktualisiert die zuletzt gezeichnete Sekunde
}
