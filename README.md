# 📌 ESP32-S3 SuperMini Wiring Guide

## 📊 Wiring Table (Horizontal)

| Thiết bị | VCC | GND | SCK | MOSI | MISO | CS | CE | DC | RST | BL | GDO0 | TX | RX | Khác |
|----------|-----|-----|-----|------|------|----|----|----|-----|----|------|----|----|------|
| **TFT 1.69** | 3.3V | GND | 10 | 11 | - | 44 | - | 13 | 12 | 43 | - | - | - | - |
| **SD Card** | 3.3V | GND | 10 | 11 | 8 | 7 | - | - | - | - | - | - | - | - |
| **NRF24L01** | 3.3V⚠️ | GND | 10 | 11 | 8 | 18 | 21 | - | - | - | - | - | - | - |
| **CC1101** | 3.3V | GND | 10 | 11 | 8 | 17 | - | - | - | - | 16 | - | - | - |
| **IR TX** | - | - | - | - | - | - | - | - | - | - | - | - | - | GPIO 9 |
| **IR RX** | - | - | - | - | - | - | - | - | - | - | - | - | - | GPIO 3 |
| **GPS** | 3.3V/5V | GND | - | - | - | - | - | - | - | - | - | 4 | 5 | UART |
| **BW16** | 3.3V/5V | GND | - | - | - | - | - | - | - | - | - | 4 | 5 | PB1/PB2 |
| **BUTTON** | - | - | - | - | - | - | - | - | - | - | - | - | - | SEL=1, DW=2, UP=3 |
| **RGB LED** | - | GND | - | - | - | - | - | - | - | - | - | - | - | GPIO 48 |

---

## ⚠️ Notes

- SPI dùng chung: SCK=10, MOSI=11, MISO=8  
- Mỗi thiết bị SPI cần CS riêng  
- NRF24L01 chỉ dùng 3.3V  
- Nên thêm tụ 10–100µF cho NRF24 & CC1101  
- GPIO 4,5 đang dùng chung UART → tránh dùng đồng thời GPS và BW16  
