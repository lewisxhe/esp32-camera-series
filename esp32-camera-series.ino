#include <WiFi.h>
#include <OneButton.h>
#include "freertos/event_groups.h"
#include <Wire.h>
#include <Adafruit_BME280.h>
#include "esp_camera.h"
#include "esp_wifi.h"
/***************************************
 *  Board select
 **************************************/

//! [T_CAMERA_MIC] With SSD1306 with microphone
// #define TTGO_T_CAMERA_MIC_V16

//! [T_CAMERA_V05 or TTGO_T_CAMERA_V1_7_2] With SSD1306, with BME280 position
//! Different power management, the same pin
// #define TTGO_T_CAMERA_V05

//! [T_CAMERA_PLUS] With 240*240TFT display, SD card slot
// #define TTGO_T_CAMERA_PLUS

//! [T_JORNAL] The most simplified version, without PSRAM
// #define TTGO_T_JORNAL

/***************************************
 *  Modules
 **************************************/
#define ENABLE_SSD1306
#define SOFTAP_MODE       //The comment will be connected to the specified ssid
// #define ENABLE_BME280
#define ENABLE_SLEEP
// #define ENABLE_IP5306
#define ENABLE_AS312
#define ENABLE_BUTTON


/***************************************
 *  WiFi
 **************************************/
#define WIFI_SSID   "your wifi ssid"
#define WIFI_PASSWD "you wifi password"

/***************************************
 *  PinOUT
 **************************************/
#if defined(TTGO_T_CAMERA_MIC_V16)
#define PWDN_GPIO_NUM       -1
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM       4
#define SIOD_GPIO_NUM       18
#define SIOC_GPIO_NUM       23

#define Y9_GPIO_NUM         36
#define Y8_GPIO_NUM         15
#define Y7_GPIO_NUM         12
#define Y6_GPIO_NUM         39
#define Y5_GPIO_NUM         35
#define Y4_GPIO_NUM         14
#define Y3_GPIO_NUM         13
#define Y2_GPIO_NUM         34
#define VSYNC_GPIO_NUM      5
#define HREF_GPIO_NUM       27
#define PCLK_GPIO_NUM       25
#define AS312_PIN           19
#define BUTTON_1            0

#define I2C_SDA             21
#define I2C_SCL             22

#undef ENABLE_BME280
#undef ENABLE_IP5306
#define SSD130_MODLE_TYPE   GEOMETRY_128_64
#elif defined(TTGO_T_CAMERA_V05) || defined(TTGO_T_CAMERA_V1_7_2)
#define PWDN_GPIO_NUM       26
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM       32
#define SIOD_GPIO_NUM       13
#define SIOC_GPIO_NUM       12

#define Y9_GPIO_NUM         39
#define Y8_GPIO_NUM         36
#define Y7_GPIO_NUM         23
#define Y6_GPIO_NUM         18
#define Y5_GPIO_NUM         15
#define Y4_GPIO_NUM         4
#define Y3_GPIO_NUM         14
#define Y2_GPIO_NUM         5
#define VSYNC_GPIO_NUM      27
#define HREF_GPIO_NUM       25
#define PCLK_GPIO_NUM       19
#define AS312_PIN           33
#define BUTTON_1            34

#define I2C_SDA             21
#define I2C_SCL             22

#define SSD130_MODLE_TYPE   GEOMETRY_128_64

#elif defined(TTGO_T_JORNAL)

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    15
#define XCLK_GPIO_NUM     27
#define SIOD_GPIO_NUM     25
#define SIOC_GPIO_NUM     23

#define Y9_GPIO_NUM       19
#define Y8_GPIO_NUM       36
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       39
#define Y5_GPIO_NUM       5
#define Y4_GPIO_NUM       34
#define Y3_GPIO_NUM       35
#define Y2_GPIO_NUM       17
#define VSYNC_GPIO_NUM    22
#define HREF_GPIO_NUM     26
#define PCLK_GPIO_NUM     21

#define I2C_SDA           14
#define I2C_SCL           13

#define BUTTON_1          32

#define SSD130_MODLE_TYPE   GEOMETRY_128_32

#undef ENABLE_BME280
#undef ENABLE_AS312

#elif defined(TTGO_T_CAMERA_PLUS)
#define PWDN_GPIO_NUM       -1
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM       4
#define SIOD_GPIO_NUM       18
#define SIOC_GPIO_NUM       23

#define Y9_GPIO_NUM         36
#define Y8_GPIO_NUM         37
#define Y7_GPIO_NUM         38
#define Y6_GPIO_NUM         39
#define Y5_GPIO_NUM         35
#define Y4_GPIO_NUM         26
#define Y3_GPIO_NUM         13
#define Y2_GPIO_NUM         34
#define VSYNC_GPIO_NUM      5
#define HREF_GPIO_NUM       27
#define PCLK_GPIO_NUM       25

#define I2C_SDA             -1
#define I2C_SCL             -1

#undef ENABLE_SSD1306
#undef ENABLE_BME280
#undef ENABLE_SLEEP
#undef ENABLE_IP5306
#undef ENABLE_AS312
#undef ENABLE_BUTTON
#else
#error "Please select board type"
#endif



#ifdef ENABLE_SSD1306
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#define SSD1306_ADDRESS 0x3c
SSD1306 oled(SSD1306_ADDRESS, I2C_SDA, I2C_SCL, SSD130_MODLE_TYPE);
OLEDDisplayUi ui(&oled);
#endif

String ip;
EventGroupHandle_t evGroup;

#ifdef ENABLE_BUTTON
OneButton button1(BUTTON_1, true);
#endif

#ifdef ENABLE_BME280
#define BEM280_ADDRESS 0X77
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
#endif


#define IP5306_ADDR 0X75
#define IP5306_REG_SYS_CTL0 0x00


void startCameraServer();
char buff[128];

#ifdef ENABLE_IP5306
bool setPowerBoostKeepOn(int en)
{
    Wire.beginTransmission(IP5306_ADDR);
    Wire.write(IP5306_REG_SYS_CTL0);
    if (en)
        Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
    else
        Wire.write(0x35); // 0x37 is default reg value
    return Wire.endTransmission() == 0;
}
#endif

void buttonClick()
{
    static bool en = false;
    xEventGroupClearBits(evGroup, 1);
    sensor_t *s = esp_camera_sensor_get();
    en = en ? 0 : 1;
    s->set_vflip(s, en);
    delay(200);
    xEventGroupSetBits(evGroup, 1);
}

void buttonLongPress()
{
#ifdef ENABLE_SSD1306
    int x = oled.getWidth() / 2;
    int y = oled.getHeight() / 2;
    ui.disableAutoTransition();
    oled.setTextAlignment(TEXT_ALIGN_CENTER);
    oled.setFont(ArialMT_Plain_10);
    oled.clear();
#endif
#ifdef ENABLE_SLEEP
    if (PWDN_GPIO_NUM > 0) {
        pinMode(PWDN_GPIO_NUM, PULLUP);
        digitalWrite(PWDN_GPIO_NUM, HIGH);
    }
    oled.drawString(x, y, "Press again to wake up");
    oled.display();

    delay(6000);

    oled.clear();
    oled.display();
    oled.displayOff();

    // esp_sleep_enable_ext0_wakeup((gpio_num_t )BUTTON_1, LOW);
    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
#endif
}


#ifdef ENABLE_SSD1306
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    display->setTextAlignment(TEXT_ALIGN_CENTER);

    int y1 = 25;

    if (oled.getHeight() == 32) {
        y1 = 10;
    }

#ifdef SOFTAP_MODE
    display->setFont(ArialMT_Plain_10);
    display->drawString(64 + x, y1 + y, buff);
#else
    display->setFont(ArialMT_Plain_16);
    display->drawString(64 + x, y1 + y, ip);
#endif

#ifdef ENABLE_AS312
    if (digitalRead(AS312_PIN)) {
        display->drawString(64 + x, 5 + y, "AS312 Trigger");
    }
#endif
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
#ifdef ENABLE_BME280
    static String temp, pressure, altitude, humidity;
    static uint64_t lastMs;

    if (millis() - lastMs > 2000) {
        lastMs   = millis();
        temp     = "Temp:" + String(bme.readTemperature()) + " *C";
        pressure = "Press:" + String(bme.readPressure() / 100.0F) + " hPa";
        altitude = "Altitude:" + String(bme.readAltitude(SEALEVELPRESSURE_HPA)) + " m";
        humidity = "Humidity:" + String(bme.readHumidity()) + " %";
    }
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0 + x, 0 + y, temp);
    display->drawString(0 + x, 16 + y, pressure);
    display->drawString(0 + x, 32 + y, altitude);
    display->drawString(0 + x, 48 + y, humidity);
#else
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->setFont(ArialMT_Plain_10);

    if (oled.getHeight() == 32) {
        display->drawString( 64 + x, 0 + y, "Camera Ready! Use");
        display->drawString(64 + x, 10 + y, "http://" + ip );
        display->drawString(64 + x, 16 + y, "to connect");
    } else {
        display->drawString( 64 + x, 5 + y, "Camera Ready! Use");
        display->drawString(64 + x, 25 + y, "http://" + ip );
        display->drawString(64 + x, 45 + y, "to connect");
    }
#endif
}

FrameCallback frames[] = {drawFrame1, drawFrame2};
#define FRAMES_SIZE (sizeof(frames) / sizeof(frames[0]))
#endif

void setup()
{

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

#ifdef ENABLE_AS312
    pinMode(AS312_PIN, INPUT);
#endif
    if (I2C_SDA > 0)
        Wire.begin(I2C_SDA, I2C_SCL);

#ifdef ENABLE_IP5306
    bool   isOk = setPowerBoostKeepOn(1);
    String info = "IP5306 KeepOn " + String((isOk ? "PASS" : "FAIL"));
#endif

#ifdef ENABLE_SSD1306
    int x = oled.getWidth() / 2;
    int y = oled.getHeight() / 2;
    oled.init();
    // Wire.setClock(100000);  //! Reduce the speed and prevent the speed from being too high, causing the screen
    oled.setFont(ArialMT_Plain_16);
    oled.setTextAlignment(TEXT_ALIGN_CENTER);
    delay(50);
    oled.drawString(x, y - 10, "TTGO Camera");
    oled.display();
#endif

#ifdef ENABLE_IP5306
    delay(1000);
    oled.setFont(ArialMT_Plain_10);
    oled.clear();
    oled.drawString(x, y - 10, info);
    oled.display();
    oled.setFont(ArialMT_Plain_16);
    delay(1000);
#endif

#ifdef ENABLE_BME280
    if (!bme.begin(BEM280_ADDRESS)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
#endif

    if (!(evGroup = xEventGroupCreate())) {
        Serial.println("evGroup Fail");
        while (1);
    }
    xEventGroupSetBits(evGroup, 1);

#ifdef TTGO_T_CAMERA_MIC_V16
    /* IO13, IO14 is designed for JTAG by default,
    * to use it as generalized input,
    * firstly declair it as pullup input */
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    //init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init Fail");
#ifdef ENABLE_SSD1306
        oled.clear();
        oled.drawString(oled.getWidth() / 2, oled.getHeight() / 2, "Camera init Fail");
        oled.display();
#endif
        while (1);
    }

    //drop down frame size for higher initial frame rate
    sensor_t *s = esp_camera_sensor_get();
    s->set_framesize(s, FRAMESIZE_QVGA);

#ifdef ENABLE_BUTTON
    button1.attachLongPressStart(buttonLongPress);
    button1.attachClick(buttonClick);
#endif

#ifdef SOFTAP_MODE
    Serial.println("Configuring access point...");
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    sprintf(buff, "TTGO-CAMERA-%02X:%02X", mac[4], mac[5]);
    WiFi.softAP(buff);
    ip = WiFi.softAPIP().toString();
#else
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    ip = WiFi.localIP().toString();
#endif

    startCameraServer();

    delay(50);

#ifdef ENABLE_SSD1306
    ui.setTargetFPS(30);
    ui.setIndicatorPosition(BOTTOM);
    ui.setIndicatorDirection(LEFT_RIGHT);
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setFrames(frames, FRAMES_SIZE);
    ui.setTimePerFrame(6000);
#endif

    Serial.print("Camera Ready! Use 'http://");
    Serial.print(ip);
    Serial.println("' to connect");
}

void loop()
{
#ifdef ENABLE_SSD1306
    if (ui.update()) {
#endif
#ifdef ENABLE_BUTTON
        button1.tick();
#else
        delay(500);
#endif
#ifdef ENABLE_SSD1306
    }
#endif
}
