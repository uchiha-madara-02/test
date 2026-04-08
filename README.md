# 📌 ESP32-S3 SuperMini Wiring Guide

Bảng tổng hợp sơ đồ đấu nối tất cả module với ESP32-S3 SuperMini.

---

## 📊 Wiring Table

| Thiết bị    | Chân module | GPIO ESP32-S3 |
|------------|------------|---------------|
| **SPI BUS** | SCK        | 10            |
|            | MOSI       | 11            |
|            | MISO       | 8             |
|------------|------------|---------------|
| **TFT 1.69** | VCC        | 3.3V          |
|            | GND        | GND           |
|            | SCL        | 10            |
|            | SDA        | 11            |
|            | RES        | 12            |
|            | DC         | 13            |
|            | CS         | 44            |
|            | BLK        | 43            |
|------------|------------|---------------|
| **SD Card** | VCC        | 3.3V          |
|            | GND        | GND           |
|            | SCK        | 10            |
|            | MOSI       | 11            |
|            | MISO       | 8             |
|            | CS         | 7             |
|------------|------------|---------------|
| **NRF24L01** | VCC        | 3.3V ⚠️       |
|            | GND        | GND           |
|            | CE         | 21            |
|            | CSN        | 18            |
|            | SCK        | 10            |
|            | MOSI       | 11            |
|            | MISO       | 8             |
|------------|------------|---------------|
| **CC1101**  | VCC        | 3.3V          |
|            | GND        | GND           |
|            | SCK        | 10            |
|            | MOSI       | 11            |
|            | MISO       | 8             |
|            | CSN        | 17            |
|            | GDO0       | 16            |
|------------|------------|---------------|
| **IR TX**   | SIG        | 9 *(tuỳ chọn)* |
| **IR RX**   | OUT        | 3 *(tuỳ chọn)* |
|------------|------------|---------------|
| **GPS**     | VCC        | 3.3V / 5V     |
|            | GND        | GND           |
|            | TX         | 5 (RX)        |
|            | RX         | 4 (TX)        |
|------------|------------|---------------|
| **BW16**    | VCC        | 3.3V / 5V     |
|            | GND        | GND           |
|            | PB2        | 5 (RX)        |
|            | PB1        | 4 (TX)        |
|------------|------------|---------------|
| **BUTTON**  | SELECT     | 1             |
|            | DOWN       | 2             |
|            | UP         | 3             |
|------------|------------|---------------|
| **RGB LED** | DIN        | 48            |

---

## ⚠️ Notes

- SPI dùng chung → mỗi thiết bị cần CS riêng  
- NRF24L01 không dùng 5V  
- Nên thêm tụ 10–100µF cho NRF24 & CC1101  
- GPIO 4,5 đang dùng chung UART & I2C → tránh xung đột  
