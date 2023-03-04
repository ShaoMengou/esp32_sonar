// 基于Arduino平台的ESP32可视化超声波测距方案
#include <Arduino.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <SPI.h>              // Arduino SPI library

#define TFT_MOSI 23  // SDA Pin on ESP32
#define TFT_SCLK 18  // SCL Pin on ESP32
#define TFT_CS 15    // Chip select control pin
#define TFT_DC 2     // Data Command control pin
#define TFT_RST 4    // Reset pin (could connect to RST pin)
#define TFT_W 240
#define TFT_H 240

#define TRIG 39
#define ECHO 36
#define DIST_MAX 400.0

#define SAMPLE 2

#define PLOT_HEIGHT_BASE 70
#define PLOT_HEIGHT_MAX 128

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup(void) {
    tft.init(240, 240, SPI_MODE2);
    tft.setRotation(2);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextWrap(false);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(4);

    pinMode(ECHO, INPUT);
    pinMode(TRIG, OUTPUT);
}

unsigned long time_echo_us = 0;
unsigned long avg_time_echo_us = 0;
unsigned long len_mm_x100 = 0;
unsigned long len_int = 0;
unsigned int len_fraction = 0;

int plot_x = 0;
bool plot_clr_invert = false;

void loop() {

    int it = 0;
    avg_time_echo_us = 0;

    while (it < SAMPLE) { //采样五次，取平均值以消除抖动
        digitalWrite(TRIG, HIGH);
        delay(12);
        digitalWrite(TRIG, LOW);
        time_echo_us = pulseIn(ECHO, HIGH);

        avg_time_echo_us += time_echo_us;
        it++;
    }

    avg_time_echo_us /= SAMPLE;

    if ((time_echo_us < 60000) && (time_echo_us > 1)) {
        len_mm_x100 = (avg_time_echo_us * 34) / 2;
        len_int = len_mm_x100 / 100;
        len_fraction = len_mm_x100 % 100;
    }

    tft.setCursor(32, 24);
    tft.fillRect(32, 24, 208, 32, ST77XX_BLACK);

    String str = String(len_int);
    str += ".";
    if (len_fraction < 10)
        str += "0";
    str += String(len_fraction);
    str += "mm";

    tft.println(str);

    float pct_h = (len_int + len_fraction) / DIST_MAX;  // 有效量程约为 400 mm
    int h = pct_h * PLOT_HEIGHT_MAX;                    //直方图最大高度为 128 pixel(但可以显示的不止于此)

    if(plot_x++>TFT_W){ // 采用双色覆盖之前的采样结果
        plot_x = 0;
        plot_clr_invert =!plot_clr_invert;
    }

    tft.drawLine(plot_x, PLOT_HEIGHT_BASE, plot_x,PLOT_HEIGHT_BASE+h,plot_clr_invert? ST77XX_BLUE:ST77XX_GREEN); //绘制直方图的每一条竖线
    tft.drawPixel(plot_x, PLOT_HEIGHT_BASE+PLOT_HEIGHT_MAX,ST77XX_RED); //使用单点补充底部标注有效最大量程的横线
}
