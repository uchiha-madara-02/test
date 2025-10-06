#include <TFT_eSPI.h>
#include <SPI.h>
#include "FontMaker.h"
#include <JPEGDecoder.h> 

TFT_eSPI tft = TFT_eSPI();  

#include "jpeg2.h"

#define minimum(a,b)     (((a) < (b)) ? (a) : (b))

void setpx(int16_t x,int16_t y,uint16_t color)
{
  tft.drawPixel(x,y,color); 
}
MakeFont myfont(&setpx);

void setup() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_WHITE);
  //drawArrayJpeg(nen, sizeof(nen), 0, 0);

  // myfont.set_font(vnfontdam22);
  // tft.fillRoundRect(42, 6, 150, 38, 10, 0x24BE);
  // myfont.print(48, 8, "Thiết Bị Đo", TFT_BLACK, 0x24BE);
  // tft.fillRoundRect(5, 62, 230, 165, 10, 0x55E);

  // myfont.set_font(vnfontdam18);
  // myfont.print(10, 85, "Nhịp Tim : ", TFT_BLACK, 0x55E);
  // myfont.set_font(vnfontdam24);
  // //tft.fillRect(118, 67, 55, 40, 0xE73C);
  // myfont.print(122, 82, "150", TFT_RED, 0x55E);
  // myfont.set_font(vnfontdam18);
  // myfont.print(180, 85, "BPM", TFT_WHITE, 0x55E);

  // myfont.print(10, 120, "Oxy Trong", TFT_BLACK , 0x55E); 
  // myfont.print(40, 146, "Máu :", TFT_BLACK , 0x55E); 
  // myfont.set_font(vnfontdam24);
  // //tft.fillRect(140, 125, 55, 40, 0xE73C);
  // myfont.print(144, 125, "100", TFT_RED, 0x55E); 
  // myfont.set_font(vnfontdam18);
  // myfont.print(201, 130, " %", TFT_WHITE, 0x55E); 
  
  // myfont.set_font(vnfontdam18);
  // myfont.print(10, 177, "Nhiệt Độ :", TFT_BLACK , 0x55E);
  // myfont.set_font(vnfontdam24);
  // //tft.fillRect(123, 188, 63, 40, 0xE73C);
  // myfont.print(127, 175, "40,9", TFT_RED, 0x55E);
  // myfont.set_font(vnfontdam18);
  // myfont.print(198, 179, "C", TFT_WHITE, 0x55E);

/////////////////////////////////////////////////////////////////////////////////
  // tft.fillRoundRect(62, 4, 115, 38, 7, 0xBDF7);
  // myfont.set_font(vnfontdam22);
  // myfont.print(72, 6, "Cài Đặt", TFT_BLACK, 0xBDF7); 

  // tft.fillRoundRect(13, 67, 215, 150, 10, 0xBDF7);
  // myfont.set_font(vnnfont16);
  // myfont.print(22, 95, "Tên Wifi: Cài đặt", TFT_BLACK, 0xBDF7);
  // myfont.print(22, 125, "Mật khẩu: 12345678", TFT_BLACK, 0xBDF7);
  // myfont.print(22, 155, "Địa chỉ: 192.168.4.1", TFT_BLACK, 0xBDF7);


  tft.fillRoundRect(5, 38, 230, 165, 15, 0xEAA9);
}

void loop() {

}
void drawArrayJpeg(const uint8_t arrayname[], uint32_t array_size, int xpos, int ypos) {
  int x = xpos;
  int y = ypos;

  JpegDec.decodeArray(arrayname, array_size);
  renderJPEG(x, y);
  Serial.println("#########################");
}

void renderJPEG(int xpos, int ypos) {
  uint16_t *pImg;
  uint16_t mcu_w = JpegDec.MCUWidth;
  uint16_t mcu_h = JpegDec.MCUHeight;
  uint32_t max_x = JpegDec.width;
  uint32_t max_y = JpegDec.height;

  uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
  uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

  uint32_t win_w = mcu_w;
  uint32_t win_h = mcu_h;

  uint32_t drawTime = millis();

  max_x += xpos;
  max_y += ypos;

  while (JpegDec.readSwappedBytes()) {
    pImg = JpegDec.pImage;
    int mcu_x = JpegDec.MCUx * mcu_w + xpos;
    int mcu_y = JpegDec.MCUy * mcu_h + ypos;

    if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
    else win_w = min_w;

    if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
    else win_h = min_h;

    if (win_w != mcu_w) {
      uint16_t *cImg;
      int p = 0;
      cImg = pImg + win_w;
      for (int h = 1; h < win_h; h++) {
        p += mcu_w;
        for (int w = 0; w < win_w; w++) {
          *cImg = *(pImg + w + p);
          cImg++;
        }
      }
    }

    if ((mcu_x + win_w) <= tft.width() && (mcu_y + win_h) <= tft.height()) {
      tft.pushRect(mcu_x, mcu_y, win_w, win_h, pImg);
    } else if ((mcu_y + win_h) >= tft.height()) {
      JpegDec.abort();
    }
  }

  drawTime = millis() - drawTime;
  Serial.print(F(" JPEG drawn in "));
  Serial.print(drawTime);
  Serial.println(F(" ms"));
}
