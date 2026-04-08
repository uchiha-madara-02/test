# 📌 ESP32-S3 SuperMini Wiring Guide

Tài liệu này mô tả sơ đồ đấu nối giữa **ESP32-S3 SuperMini** với các module:

- TFT 1.69" (ST7789)
- SD Card
- NRF24L01
- CC1101
- IR TX / IR RX
- GPS

---

## ⚡ SPI Bus (dùng chung)

Các thiết bị SPI dùng chung bus:

| Tín hiệu | GPIO |
|----------|------|
| SCK      | 10   |
| MOSI     | 11   |
| MISO     | 8    |

---

## 🖥️ TFT 1.69" (ST7789)

| TFT Pin | ESP32-S3 |
|--------|----------|
| VCC    | 3.3V     |
| GND    | GND      |
| SCL    | GPIO 10  |
| SDA    | GPIO 11  |
| RES    | GPIO 12  |
| DC     | GPIO 13  |
| CS     | GPIO 44  |
| BLK    | GPIO 43  |

---

## 💾 SD Card

| SD Pin | ESP32-S3 |
|--------|----------|
| VCC    | 3.3V     |
| GND    | GND      |
| SCK    | GPIO 10  |
| MOSI   | GPIO 11  |
| MISO   | GPIO 8   |
| CS     | GPIO 7   |

---

## 📡 NRF24L01

| NRF24 Pin | ESP32-S3 |
|-----------|----------|
| VCC       | 3.3V ⚠️ |
| GND       | GND      |
| CE        | GPIO 21  |
| CSN       | GPIO 18  |
| SCK       | GPIO 10  |
| MOSI      | GPIO 11  |
| MISO      | GPIO 8   |

---

## 📶 CC1101

| CC1101 Pin | ESP32-S3 |
|------------|----------|
| VCC        | 3.3V     |
| GND        | GND      |
| SCK        | GPIO 10  |
| MOSI       | GPIO 11  |
| MISO       | GPIO 8   |
| CSN        | GPIO 17  |
| GDO0       | GPIO 16  |

---

## 🔴 IR Module

### IR TX

| Pin | ESP32-S3 |
|-----|----------|
| SIG | GPIO 9 *(tuỳ chọn)* |

### IR RX

| Pin | ESP32-S3 |
|-----|----------|
| OUT | GPIO 3 *(tuỳ chọn)* |

---

## 📍 GPS (UART)

| GPS Pin | ESP32-S3 |
|---------|----------|
| VCC     | 3.3V / 5V |
| GND     | GND      |
| TX      | GPIO 5 (RX) |
| RX      | GPIO 4 (TX) |

---

## 📍 BW16 (UART)

| BW16 Pin | ESP32-S3 |
|---------|----------|
| VCC     | 3.3V / 5V |
| GND     | GND      |
| PB2     | GPIO 5 (RX) |
| PB1     | GPIO 4 (TX) |

---

## 🔘 Buttons

| Button | GPIO |
|--------|------|
| SELECT | 1    |
| DOWN   | 2    |
| UP     | 3    |

- Active: LOW

---

## 📊 Sơ đồ tổng quan
