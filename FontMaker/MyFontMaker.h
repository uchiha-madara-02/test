/*
  MyFontMaker.h - Thư viện Font tương thích utf-8 (v2)
  Created by Dao Nguyen - IOT47.com
  Liên hệ: daonguyen20798@gmail.com
  Không sửa file này
*/
#ifndef __MY_FontMaker_h___
#define __MY_FontMaker_h___

#include "Arduino.h"

typedef struct 
{
  const uint8_t  *f_name;
  const uint16_t *f_map;
}MyFont_typedef;

//Khai báo extern các loại font mà bạn sử dụng

extern const MyFont_typedef f_to_vai;
extern const MyFont_typedef f_26;
extern const MyFont_typedef f_22;
extern const MyFont_typedef h_to1;
extern const MyFont_typedef h_to2;
extern const MyFont_typedef h_to3;
extern const MyFont_typedef h_f_20;
extern const MyFont_typedef SimSun1;
extern const MyFont_typedef Tahoma12;
extern const MyFont_typedef Tahoma16;
extern const MyFont_typedef MakeFont_Font1;
extern const MyFont_typedef Microsoft_Sans_Serif_9;
extern const MyFont_typedef Microsoft_Sans_Serif_10;
extern const MyFont_typedef Microsoft_Sans_Serif_11;
extern const MyFont_typedef Microsoft_Sans_Serif_12;
extern const MyFont_typedef Microsoft_Sans_Serif_14;
extern const MyFont_typedef vnnfont16;
extern const MyFont_typedef vnfontdam18;
extern const MyFont_typedef vnfontdam20;
extern const MyFont_typedef vnfontdam22;
extern const MyFont_typedef myfontvn_22;
extern const MyFont_typedef vnfontdam24;
extern const MyFont_typedef vnfontdam16;
extern const MyFont_typedef vnfontdam14;
extern const MyFont_typedef fontdam14_berlin;
extern const MyFont_typedef vnfontdam11;
extern const MyFont_typedef comic_san_ms_dam_18;
extern const MyFont_typedef comic_san_ms_dam_14;
extern const MyFont_typedef comic_san_ms_dam_20;
extern const MyFont_typedef comic_san_ms_dam_22;
extern const MyFont_typedef comic_san_ms_dam_16;
extern const MyFont_typedef comic_san_ms_dam_8;
extern const MyFont_typedef comic_san_ms_dam_9;
//auto add here




#endif //__MY_FontMaker_h___
//---------------------------------------------------------------------Kết thúc file ----------------------------------------------//
