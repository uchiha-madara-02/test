#include <GyverOLED.h> 
#include <GyverButton.h> 
#include <Wire.h> 
#include <WiFi.h>
#include "esp_wifi.h"
#include <ArduinoBLE.h>
#include "WORLD_IR_CODES.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "main.h"

// Дисплей
#define OLED_SCK 7 
#define OLED_SDA 6 
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1 
#define SCREEN_ADDRESS 0x3C 

// Кнопки
#define BUTTON_1_PIN 4 // Вверх
#define BUTTON_2_PIN 3 // Вниз
#define BUTTON_3_PIN 10 // ОК

// ИК LED
#define IR_LED_PIN 1 // Пин для ИК светодиода
IRsend irsend(IR_LED_PIN); 

extern const IrCode* const WorldPowerCodes[];
extern uint16_t num_WorldCodes;

uint16_t rawData[300];
uint8_t bitsleft_r = 0;
uint8_t bits_r = 0;
uint16_t code_ptr;
volatile const IrCode* powerCode;

uint8_t read_bits(uint8_t count) {
  uint8_t i, tmp = 0;
  for (i = 0; i < count; i++) {
    if (bitsleft_r == 0) {
      bits_r = powerCode->codes[code_ptr++];
      bitsleft_r = 8;
    }
    bitsleft_r--;
    tmp |= (((bits_r >> (bitsleft_r)) & 1) << (count - 1 - i));
  }
  return tmp;
}

// Инициализация OLED дисплея
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled; 

// Инициализация кнопок
GButton button1(BUTTON_1_PIN);
GButton button2(BUTTON_2_PIN);
GButton button3(BUTTON_3_PIN);

// Меню
const char* menuItems[] = {"WI-FI","Bluetooth","NRF24","IR","MAIN MENU"}; 
const int menuSize = 5; 

// Подпункты для каждого основного пункта
const char* submenuItems[][5] = {
  {"SCAN WI-FI", "CONNECT", "PACKET MONITOR","ATTAK"}, 
  {"BLE SCAN", "GET DEVICE INFO", "", "", ""}, 
  {"JAMMER 2.4 G", "SPECTRUM", "DISCLAIMER",""}, 
  {"IR JAMMER","TV-B-GONE",""}, 
  {"VERSION", "CREATOR", "CHANNEL", "WS2812", ""} 
};

const int submenuSizes[] = {4, 2, 3, 2, 4}; 

// ИК джаммер - универсальные частоты для всех телевизоров
const int universalFreqs[] = {38, 36, 40, 56, 33, 30, 45, 50, 28, 42};
const int numUniversalFreqs = 10;
const char* irFreqItems[] = {"38 kHz", "40 kHz", "56 kHz", "Universal TV"};
const int irFreqSizes = 4;
int currentFreqItem = 0; 
int irFreqValues[] = {38, 40, 56, 38}; 
bool irJamming = false; 
unsigned long jamStartTime = 0;
unsigned long framesSent = 0;
bool statusLedState = false;
unsigned long lastStatusToggle = 0;

// // TV-B-GONE коды
// const unsigned long tvbgoneCodes[] = {
//   0x20DF10EF, 0x20DFC03F, 0x20DF40BF, 0x20DF807F,
//   0x20DF906F, 0x20DFA05F, 0x20DF609F, 0x20DFE01F,
//   0x20DFD02F, 0x20DFF00F, 0x20DF08F7, 0x20DF8877,
//   0x20DF48B7, 0x20DFC837, 0x20DF28D7, 0x20DFA857,
//   0x20DF6897, 0x20DFE817, 0x20DF18E7, 0x20DF9867,
//   0x20DF58A7, 0x20DFD827, 0x20DF38C7, 0x20DFB847,
//   0x20DF7887, 0x20DFF807, 0x20DF04FB, 0x20DF847B,
//   0x20DF44BB, 0x20DFC43B, 0x20DF24DB, 0x20DFA45B,
//   0x20DF649B, 0x20DFE41B, 0x20DF14EB, 0x20DF946B,
//   0x20DF54AB, 0x20DFD42B, 0x20DF34CB, 0x20DFB44B,
//   0x20DF748B, 0x20DFF40B, 0x20DF0CF3, 0x20DF8C73,
//   0x20DF4CB3, 0x20DFCC33, 0x20DF2CD3, 0x20DFAC53,
//   0x20DF6C93, 0x20DFEC13, 0x20DF1CE3, 0x20DF9C63,
//   0x20DF5CA3, 0x20DFDC23, 0x20DF3CC3, 0x20DFBC43,
//   0x20DF7C83, 0x20DFFC03
// };
// const int numTvbgoneCodes = 56;

// // Переменные для кастомной частоты
bool inCustomFreqMode = false;
// int customFreq = 38; 

int currentItem = 0; 
int currentSubItem = 0; 
int menuLevel = 0; // 0 - главное меню, 1 - подменю, 2 - выбор частоты
bool inMenu = false; 
int lastItem = -1; 
int lastSubItem = -1; 
int lastMenuLevel = -1; 
bool lastInMenu = false; 
bool showStartup = true; 
unsigned long startupTimer = 0; 
unsigned long selectTimer = 0; 

// Переменные для сканирования Wi-Fi
bool isScanning = false;
bool scanComplete = false;
int numNetworks = 0;
String wifiNetworks[50]; 
String wifiSSIDs[50]; 
int wifiRSSI[50]; 
bool wifiEncrypted[50]; 
int currentNetwork = 0; 
unsigned long lastLedToggle = 0;
bool ledState = false;

// Переменные для сканирования Bluetooth
bool isBLEScanning = false;
bool bleScanComplete = false;
int numBLEDevices = 0;
String bleDevices[20];
String bleNames[20];
int bleRSSI[20];
String bleAddresses[20];
int currentBLEDevice = 0;

// Переменные для подключения к BLE устройствам
bool isBLEConnecting = false;
bool bleConnectComplete = false;
bool inBLEInfoMode = false;
String bleDeviceInfo = "";
String bleServiceInfo = "";
String bleCharacteristicInfo = "";
int currentInfoPage = 0;
const int MAX_INFO_PAGES = 3;

// Переменные для подключения к Wi-Fi
bool inConnectMode = false;
bool connectScanComplete = false;
bool isConnecting = false;
bool connectionResult = false;
unsigned long connectionStartTime = 0;
const unsigned long CONNECTION_TIMEOUT = 15000; 

// Переменные для Packet Monitor
bool packetMonitorActive = false;
unsigned long packetMonitorStartTime = 0;
unsigned long packetsPerSecond = 0;
unsigned long totalPackets = 0;
unsigned long lastPacketCount = 0;
unsigned long lastSecondCheck = 0;
unsigned long packetTypes[4] = {0, 0, 0, 0}; // Management, Control, Data, Other
String channels = "1,6,11";
int currentChannel = 1;
int channelIndex = 0;
int maxPacketRate = 0;
int packetHistory[128] = {0};
int historyIndex = 0;
unsigned long lastGraphUpdate = 0;
const unsigned long GRAPH_UPDATE_INTERVAL = 100;

// Переменная для отслеживания подключения к Wi-Fi
bool wifiConnected = false;
unsigned long lastWifiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 5000;

// WiFi promiscuous callback function
void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type) {
  if (!packetMonitorActive) return;
  
  totalPackets++;
  packetsPerSecond++;
  
  if (type == WIFI_PKT_MGMT) {
    packetTypes[0]++;
  } else if (type == WIFI_PKT_CTRL) {
    packetTypes[1]++;
  } else if (type == WIFI_PKT_DATA) {
    packetTypes[2]++;
  } else {
    packetTypes[3]++;
  }
}

void setup() {
  Serial.begin(115200);

  oled.init(OLED_SDA, OLED_SCK); 
  oled.clear();
  oled.setScale(1);
  oled.setCursorXY(0, 0);
  oled.print("V:1.0"); 
  oled.setScale(2); 
  oled.setCursorXY(22, 24); 
  oled.print("ESP-GIK"); 
  oled.setScale(1); 
  oled.setCursorXY(24, 42); 
  oled.print("by tranzistor"); 
  oled.update(); 
  startupTimer = millis(); 

  button1.setType(HIGH_PULL);
  button2.setType(HIGH_PULL);
  button3.setType(HIGH_PULL);

  button1.setDebounce(50); 
  button2.setDebounce(50);
  button3.setDebounce(50);
  button1.setTimeout(300); 
  button2.setTimeout(300);
  button3.setTimeout(300);
  button3.setStepTimeout(1000); 
 
  irsend.begin();
  digitalWrite(IR_LED_PIN, LOW); 

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
  }
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void checkWiFiConnection() {
  if (millis() - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
    } else {
      wifiConnected = false;
    }
    lastWifiCheck = millis();
  }
}

void drawWiFiIcon(int x, int y) {
  if (wifiConnected) {
    oled.setCursorXY(x, y);
    oled.print("W");
  }
}

void drawCurrentPosition(int current, int total) {
  oled.setCursorXY(100, 0);
  oled.print(current + 1);
  oled.print("/");
  oled.print(total);
}

// ФУНКЦИЯ TV-B-GONE
// FUNCTION TV-B-GONE (NON-BLOCKING VERSION)
// void tvbGone() {
//   bool tvbRunning = true;
  
//   // Biến cho hiển thị OLED
//   unsigned long lastDisplayUpdate = 0;
//   bool displayState = false;
  
//   // Biến cho logic gửi IR (lấy từ đoạn code mới của bạn)
//   unsigned long lastIrSendTime = 0;
//   unsigned long irDelayTime = 100; // Thay thế cho delay(100) cũ
//   int currentCode = 0;             // Thay thế cho currentIrIndex

//   // Setup màn hình ban đầu
//   oled.clear();
//   oled.setScale(2);
//   oled.setCursorXY(30, 24);
//   oled.print("WORKS");
//   oled.update();
  
//   Serial.println(">>> Bắt đầu TV-B-Gone");

//   while (tvbRunning) {
//     // 1. CẬP NHẬT TRẠNG THÁI NÚT BẤM LIÊN TỤC
//     button1.tick();
//     button2.tick();
//     button3.tick();

//     // 2. XỬ LÝ HIỂN THỊ (Nhấp nháy chữ WORKS)
//     if (millis() - lastDisplayUpdate > 500) {
//       displayState = !displayState;
//       lastDisplayUpdate = millis();
      
//       oled.clear();
//       if (displayState) {
//         oled.setScale(2);
//         oled.setCursorXY(30, 24);
//         oled.print("WORKS");
//         // Hiển thị thêm mã đang phát để dễ theo dõi
//         oled.setScale(1);
//         oled.setCursorXY(40, 50);
//         oled.print(currentCode);
//       }
//       oled.update();
//     }
    
//     // 3. XỬ LÝ GỬI IR (LOGIC MỚI)
//     // Kiểm tra thời gian dùng millis() thay vì delay()
//     if (millis() - lastIrSendTime > irDelayTime) {
      
//       // Kiểm tra xem còn mã trong danh sách không
//       if (currentCode < numTvbgoneCodes) {
        
//         // Gửi mã hiện tại (Sử dụng hàm gửi cũ của bạn)
//         sendIRCode(tvbgoneCodes[currentCode], 38); 
        
//         // Tăng index và cập nhật thời gian
//         currentCode++;
//         lastIrSendTime = millis();
        
//       } else {
//         // Đã bắn hết vòng, quay lại từ đầu
//         currentCode = 0;
//         Serial.println(">>> Đã bắn hết vòng, quay lại từ đầu...");
//       }
//     }
    
//     // 4. KIỂM TRA THOÁT
//     if (button3.isClick()) {
//       tvbRunning = false;
//     }
//   }
  
//   // KẾT THÚC VÀ THOÁT
//   digitalWrite(IR_LED_PIN, LOW);
//   oled.clear();
//   oled.setScale(2);
//   oled.setCursorXY(20, 24);
//   oled.print("STOPPED");
//   drawWiFiIcon(100, 0); // Vẽ lại icon nếu cần
//   oled.update();
//   delay(1000); // Delay nhẹ khi thoát để tránh bấm nhầm
// }

// // Функция отправки IR кода в формате NEC
// void sendIRCode(unsigned long code, int freq) {
//   unsigned long period = 1000000L / freq;
//   unsigned long halfPeriod = period / 2;
  
//   // Send Start Pulse
//   sendIRPulse(9000, 4500, freq);
  
//   // Send Data (32 bits)
//   for (int i = 31; i >= 0; i--) {
//     if (code & (1UL << i)) {
//       sendIRPulse(560, 1690, freq);
//     } else {
//       sendIRPulse(560, 560, freq);
//     }
//   }
  
//   // Send Stop Pulse
//   sendIRPulse(560, 560, freq);
// }

// // Функция отправки IR импульса
// void sendIRPulse(unsigned long pulseHigh, unsigned long pulseLow, int freq) {
//   unsigned long period = 1000000L / freq;
//   unsigned long halfPeriod = period / 2;
//   unsigned long startTime = micros();
  
//   // Send HIGH pulse
//   while (micros() - startTime < pulseHigh) {
//     digitalWrite(IR_LED_PIN, HIGH);
//     delayMicroseconds(halfPeriod);
//     digitalWrite(IR_LED_PIN, LOW);
//     delayMicroseconds(halfPeriod);
//   }
  
//   // Send LOW pulse
//   startTime = micros();
//   while (micros() - startTime < pulseLow) {
//     delayMicroseconds(100);
//   }
// }

// --- HÀM GỬI 1 MÃ IR DUY NHẤT TẠI VỊ TRÍ INDEX ---
void sendOneCode(uint16_t index) {
  if (index >= num_WorldCodes) return; // An toàn: tránh tràn mảng

  powerCode = WorldPowerCodes[index]; // Lấy code tại vị trí index

  const uint8_t freq = powerCode->timer_val;
  const uint8_t numpairs = powerCode->numpairs;
  const uint8_t bitcompression = powerCode->bitcompression;

  Serial.printf("--- Sending Code #%d (Freq: %ukHz) ---\n", index, freq);

  code_ptr = 0;
  for (uint8_t k = 0; k < numpairs; k++) {
    uint16_t ti = (read_bits(bitcompression)) * 2;
    uint16_t ontime = powerCode->times[ti];
    uint16_t offtime = powerCode->times[ti + 1];
    rawData[k * 2] = ontime * 10;
    rawData[(k * 2) + 1] = offtime * 10;
  }

  // Gửi code bằng thư viện IRremoteESP8266 (Chính xác hơn bit-banging thủ công)
  irsend.sendRaw(rawData, (numpairs * 2), freq);
  bitsleft_r = 0; // Reset bộ đếm bit
}

// --- HÀM CHÍNH TV-B-GONE ---
void tvbGone() {
  bool tvbRunning = true;
  
  // Khởi động thư viện IR
  irsend.begin();

  // Biến cho hiển thị OLED
  unsigned long lastDisplayUpdate = 0;
  bool displayState = false;
  
  // Biến quản lý tiến trình bắn IR
  int currentIrIndex = 0;           // Bắt đầu từ mã số 0
  unsigned long lastIrSendTime = 0; 
  const unsigned long irDelayTime = 200; // Khoảng nghỉ giữa các mã (ms)

  // Setup màn hình ban đầu
  oled.clear();
  oled.setScale(2);
  oled.setCursorXY(30, 24);
  oled.print("WORKS");
  oled.update();
  
  Serial.println(">>> Bắt đầu TV-B-Gone (Library Version)");

  while (tvbRunning) {
    // 1. CẬP NHẬT NÚT BẤM (Non-blocking)
    button1.tick();
    button2.tick();
    button3.tick();

    // 2. XỬ LÝ HIỂN THỊ (Nhấp nháy chữ WORKS)
    if (millis() - lastDisplayUpdate > 500) {
      displayState = !displayState;
      lastDisplayUpdate = millis();
      
      oled.clear();
      if (displayState) {
        oled.setScale(2);
        oled.setCursorXY(30, 24);
        oled.print("WORKS");
        
        // Hiển thị mã số đang bắn
        oled.setScale(1);
        oled.setCursorXY(40, 50);
        oled.print("Code: ");
        oled.print(currentIrIndex);
      }
      oled.update();
    }
    
    // 3. XỬ LÝ GỬI IR (Sử dụng sendOneCode mới)
    if (millis() - lastIrSendTime > irDelayTime) {
      
      // Kiểm tra xem còn mã trong danh sách không
      if (currentIrIndex < num_WorldCodes) {
        
        // GỌI HÀM GỬI MỚI
        sendOneCode(currentIrIndex);
        
        // Tăng index và cập nhật thời gian
        currentIrIndex++;
        lastIrSendTime = millis();
        
      } else {
        // Đã bắn hết vòng -> Reset về 0
        currentIrIndex = 0;
        Serial.println(">>> Đã bắn hết vòng, quay lại từ đầu...");
        
        // Có thể thêm delay dài hơn ở đây nếu muốn nghỉ một chút sau 1 vòng
        delay(1000); 
        lastIrSendTime = millis();
      }
    }
    
    // 4. KIỂM TRA THOÁT
    if (button3.isClick()) {
      tvbRunning = false;
    }
  }
  
  // KẾT THÚC
  oled.clear();
  oled.setScale(2);
  oled.setCursorXY(20, 24);
  oled.print("STOPPED");
  drawWiFiIcon(100, 0); 
  oled.update();
  delay(1000);
}

void scanBLE() {
  isBLEScanning = true;
  bleScanComplete = false;
  numBLEDevices = 0;
  currentBLEDevice = 0;

  oled.clear();
  oled.setScale(1);
  oled.setCursorXY(0, 24);
  oled.print("Scanning BLE...");
  drawWiFiIcon(90, 0);
  oled.update();

  for (int i = 0; i < 20; i++) {
    bleDevices[i] = "";
    bleNames[i] = "";
    bleRSSI[i] = 0;
    bleAddresses[i] = "";
  }

  BLE.scan(true);

  unsigned long scanStartTime = millis();
  const unsigned long SCAN_DURATION = 5000;

  while (millis() - scanStartTime < SCAN_DURATION && numBLEDevices < 20) {
    BLEDevice peripheral = BLE.available();
    
    if (peripheral) {
      String deviceName = peripheral.localName();
      String deviceAddress = peripheral.address();
      int rssi = peripheral.rssi();
      
      if (deviceName.length() == 0) {
        deviceName = "Unknown";
      }
      
      bleNames[numBLEDevices] = deviceName;
      bleAddresses[numBLEDevices] = deviceAddress;
      bleRSSI[numBLEDevices] = rssi;
      
      String displayName = deviceName;
      if (displayName.length() > 15) {
        displayName = displayName.substring(0, 15);
      }
      
      bleDevices[numBLEDevices] = displayName + " (" + String(rssi) + "dBm)";
      numBLEDevices++;
      
      oled.clear();
      oled.setScale(1);
      oled.setCursorXY(0, 8);
      oled.print("Scanning BLE...");
      oled.setCursorXY(0, 20);
      oled.print("Found: ");
      oled.print(numBLEDevices);
      oled.print(" devices");
      oled.setCursorXY(0, 32);
      oled.print("Time: ");
      oled.print((SCAN_DURATION - (millis() - scanStartTime)) / 1000);
      oled.print("s");
      drawWiFiIcon(90, 0);
      oled.update();
    }
    
    delay(100);
  }

  BLE.scan(false);
  
  if (numBLEDevices == 0) {
    bleDevices[0] = "No BLE devices found";
    numBLEDevices = 1;
  }
  
  isBLEScanning = false;
  bleScanComplete = true;
}

// НОВАЯ ФУНКЦИЯ: Получение информации о BLE устройстве
void getBLEDeviceInfo() {
  inBLEInfoMode = true;
  isBLEConnecting = true;
  bleConnectComplete = false;
  
  oled.clear();
  oled.setScale(1);
  oled.setCursorXY(0, 24);
  oled.print("Scanning for BLE...");
  drawWiFiIcon(90, 0);
  oled.update();

  // Сканируем устройства
  BLE.scan(true);
  
  unsigned long scanStartTime = millis();
  const unsigned long SCAN_DURATION = 3000;
  BLEDevice targetPeripheral;
  bool deviceFound = false;

  // Сканируем в течение 3 секунд
  while (millis() - scanStartTime < SCAN_DURATION && !deviceFound) {
    BLEDevice peripheral = BLE.available();
    if (peripheral) {
      targetPeripheral = peripheral;
      deviceFound = true;
    }
    delay(100);
  }
  
  BLE.scan(false);

  if (!deviceFound) {
    oled.clear();
    oled.setScale(1);
    oled.setCursorXY(0, 24);
    oled.print("No BLE devices found");
    oled.setCursorXY(0, 40);
    oled.print("Long OK to exit");
    drawWiFiIcon(90, 0);
    oled.update();
    
    while (inBLEInfoMode) {
      button3.tick();
      if (button3.isStep()) {
        inBLEInfoMode = false;
      }
      delay(100);
    }
    return;
  }

  // Подключаемся к найденному устройству
  oled.clear();
  oled.setScale(1);
  oled.setCursorXY(0, 16);
  oled.print("Connecting to:");
  oled.setCursorXY(0, 24);
  
  String deviceName = targetPeripheral.localName();
  if (deviceName.length() == 0) deviceName = "Unknown";
  if (deviceName.length() > 21) deviceName = deviceName.substring(0, 21);
  oled.print(deviceName);
  
  oled.setCursorXY(0, 32);
  oled.print("Address: ");
  String address = targetPeripheral.address();
  if (address.length() > 12) address = address.substring(0, 12);
  oled.print(address);
  
  oled.setCursorXY(0, 40);
  oled.print("Connecting...");
  drawWiFiIcon(90, 0);
  oled.update();

  // Пытаемся подключиться
  if (targetPeripheral.connect()) {
    oled.clear();
    oled.setScale(1);
    oled.setCursorXY(0, 8);
    oled.print("Connected!");
    oled.setCursorXY(0, 16);
    oled.print("Discovering...");
    oled.update();

    // Discover attributes
    targetPeripheral.discoverAttributes();

    // Собираем информацию об устройстве
    bleDeviceInfo = "Device: " + deviceName + "\n";
    bleDeviceInfo += "Addr: " + targetPeripheral.address() + "\n";
    bleDeviceInfo += "RSSI: " + String(targetPeripheral.rssi()) + "dBm\n";
    bleDeviceInfo += "Connected: Yes\n";

    // Собираем информацию о сервисах
    bleServiceInfo = "Services:\n";
    int serviceCount = targetPeripheral.serviceCount();
    bleServiceInfo += "Count: " + String(serviceCount) + "\n";
    
    for (int i = 0; i < serviceCount && i < 3; i++) {
      BLEService service = targetPeripheral.service(i);
      bleServiceInfo += "S" + String(i) + ": " + service.uuid() + "\n";
    }

    // Собираем информацию о характеристиках
    bleCharacteristicInfo = "Characteristics:\n";
    int charCount = 0;
    
    for (int i = 0; i < serviceCount && charCount < 4; i++) {
      BLEService service = targetPeripheral.service(i);
      int characteristicsCount = service.characteristicCount();
      
      for (int j = 0; j < characteristicsCount && charCount < 4; j++) {
        BLECharacteristic characteristic = service.characteristic(j);
        bleCharacteristicInfo += "C" + String(charCount) + ": " + characteristic.uuid() + "\n";
        charCount++;
      }
    }

    // Отображаем информацию
    currentInfoPage = 0;
    displayBLEInfoPage();

    // Ждем действий пользователя
    while (inBLEInfoMode) {
      button1.tick();
      button2.tick();
      button3.tick();
      
      if (button1.isClick()) {
        currentInfoPage = (currentInfoPage - 1 + MAX_INFO_PAGES) % MAX_INFO_PAGES;
        displayBLEInfoPage();
      }
      
      if (button2.isClick()) {
        currentInfoPage = (currentInfoPage + 1) % MAX_INFO_PAGES;
        displayBLEInfoPage();
      }
      
      if (button3.isStep()) {
        inBLEInfoMode = false;
      }
      
      delay(100);
    }

    // Отключаемся от устройства
    targetPeripheral.disconnect();
    
  } else {
    oled.clear();
    oled.setScale(1);
    oled.setCursorXY(0, 24);
    oled.print("Failed to connect!");
    oled.setCursorXY(0, 40);
    oled.print("Long OK to exit");
    drawWiFiIcon(90, 0);
    oled.update();
    
    while (inBLEInfoMode) {
      button3.tick();
      if (button3.isStep()) {
        inBLEInfoMode = false;
      }
      delay(100);
    }
  }
  
  isBLEConnecting = false;
  bleConnectComplete = true;
}

// Функция отображения страницы с информацией о BLE устройстве
void displayBLEInfoPage() {
  oled.clear();
  oled.setScale(1);
  drawWiFiIcon(90, 0);
  
  oled.setCursorXY(0, 0);
  oled.print("BLE Info ");
  oled.print(currentInfoPage + 1);
  oled.print("/");
  oled.print(MAX_INFO_PAGES);
  
  switch(currentInfoPage) {
    case 0:
      // Страница с общей информацией об устройстве
      displayMultilineText(bleDeviceInfo, 8);
      break;
    case 1:
      // Страница с информацией о сервисах
      displayMultilineText(bleServiceInfo, 8);
      break;
    case 2:
      // Страница с информацией о характеристиках
      displayMultilineText(bleCharacteristicInfo, 8);
      break;
  }
  
  oled.setCursorXY(0, 56);
  oled.print("UP/DOWN:Page LONG:Back");
  oled.update();
}

// Вспомогательная функция для отображения многострочного текста
void displayMultilineText(String text, int startY) {
  int lineHeight = 8;
  int currentY = startY;
  int startPos = 0;
  int endPos = 0;
  
  while (startPos < text.length() && currentY < 56) {
    endPos = startPos + 21;
    if (endPos > text.length()) endPos = text.length();
    
    // Находим последний символ новой строки или пробел для переноса
    int lastBreak = text.lastIndexOf('\n', endPos);
    if (lastBreak == -1 || lastBreak < startPos) {
      lastBreak = text.lastIndexOf(' ', endPos);
    }
    
    if (lastBreak != -1 && lastBreak > startPos) {
      endPos = lastBreak + 1;
    }
    
    String line = text.substring(startPos, endPos);
    oled.setCursorXY(0, currentY);
    oled.print(line);
    
    currentY += lineHeight;
    startPos = endPos;
    
    // Пропускаем символы новой строки
    while (startPos < text.length() && (text.charAt(startPos) == '\n' || text.charAt(startPos) == ' ')) {
      startPos++;
    }
  }
}

void startPacketMonitor() {
  packetMonitorActive = true;
  packetMonitorStartTime = millis();
  totalPackets = 0;
  packetsPerSecond = 0;
  lastPacketCount = 0;
  lastSecondCheck = millis();
  maxPacketRate = 0;
  lastGraphUpdate = 0;
  
  for (int i = 0; i < 128; i++) {
    packetHistory[i] = 0;
  }
  historyIndex = 0;
  
  for (int i = 0; i < 4; i++) {
    packetTypes[i] = 0;
  }
  
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  
  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
  
  oled.clear();
  oled.setScale(2);
  oled.setCursorXY(18, 24);
  oled.print("MONITOR");
  oled.update();
  delay(1000);
}

void stopPacketMonitor() {
  packetMonitorActive = false;
  esp_wifi_set_promiscuous(false);
  esp_wifi_stop();
  
  // ПРОСТОЙ ВЫХОД БЕЗ СТАТИСТИКИ 
  oled.clear();
  oled.setScale(2);
  oled.setCursorXY(20, 24);
  oled.print("STOPPED");
  drawWiFiIcon(100, 0);
  oled.update();
  delay(1000);
}

void drawFullScreenPacketGraph() {
  oled.clear();
  
  int graphHeight = 54;
  int graphWidth = 128;
  int graphY = 63;
  
  int maxVal = 1;
  for (int i = 0; i < 128; i++) {
    if (packetHistory[i] > maxVal) maxVal = packetHistory[i];
  }
  
  oled.dot(0, graphY - graphHeight/2);
  oled.dot(127, graphY - graphHeight/2);
  
  for (int i = 0; i < 128; i++) {
    int value = packetHistory[i];
    if (maxVal > 0) {
      int barHeight = map(value, 0, maxVal, 0, graphHeight);
      if (barHeight > 0) {
        oled.fastLineV(i, graphY - barHeight, graphY);
      }
    }
  }
  
  oled.setScale(1);
  oled.setCursorXY(0, 0);
  oled.print("Ch:");
  oled.print(currentChannel);
  oled.print(" PPS:");
  oled.print(packetsPerSecond);
  drawWiFiIcon(100, 0);
  
  static bool activityIndicator = false;
  if (packetsPerSecond > 0) {
    activityIndicator = !activityIndicator;
    if (activityIndicator) {
      oled.setCursorXY(80, 0);
      oled.print("*");
    }
  }
}

void updatePacketMonitor() {
  if (!packetMonitorActive) return;
  
  if (millis() - lastSecondCheck >= 1000) {
    packetHistory[historyIndex] = packetsPerSecond;
    historyIndex = (historyIndex + 1) % 128;
    
    if (packetsPerSecond > maxPacketRate) {
      maxPacketRate = packetsPerSecond;
    }
    
    lastSecondCheck = millis();
    packetsPerSecond = 0;
  }
  
  if (millis() - lastGraphUpdate >= GRAPH_UPDATE_INTERVAL) {
    drawFullScreenPacketGraph();
    oled.update();
    lastGraphUpdate = millis();
  }
}

// ОСНОВНАЯ ФУНКЦИЯ ГЕНЕРАЦИИ ИК СИГНАЛА С МЕРЦАНИЕМ
void generateIRSignal(int frequency) {
  unsigned long period = 1000000L / frequency;
  unsigned long halfPeriod = period / 2;
  
  // Мерцание ИК светодиода с заданной частотой
  for (int i = 0; i < 100; i++) { // Генерируем 100 циклов за один вызов
    digitalWrite(IR_LED_PIN, HIGH);
    delayMicroseconds(halfPeriod);
    digitalWrite(IR_LED_PIN, LOW);
    delayMicroseconds(halfPeriod);
  }
}

// УНИВЕРСАЛЬНЫЙ ТВ ДЖАММЕР
void universalTVJammer() {
  irJamming = true;
  jamStartTime = millis();
  framesSent = 0;
  int currentFreqIndex = 0;
  unsigned long lastFreqChange = 0;
  unsigned long lastBlink = 0;
  bool blinkState = true;
  
  while (irJamming) {
    button1.tick();
    button2.tick();
    button3.tick();
    
    if (millis() - lastFreqChange > 500) {
      currentFreqIndex = (currentFreqIndex + 1) % numUniversalFreqs;
      lastFreqChange = millis();
    }
    
    if (millis() - lastBlink > 300) {
      blinkState = !blinkState;
      lastBlink = millis();
      
      oled.clear();
      if (blinkState) {
        oled.setScale(2);
        oled.setCursorXY(18, 25);
        oled.print("JAMMING");
      }
      oled.update();
    }
    
    // ГЕНЕРАЦИЯ ИК СИГНАЛА С МЕРЦАНИЕМ
    generateIRSignal(universalFreqs[currentFreqIndex] * 1000);
    
    framesSent++;
    
    if (button3.isClick()) {
      irJamming = false;
    }
    
    delay(1);
  }
  
  digitalWrite(IR_LED_PIN, LOW);
  
  // ПРОСТОЙ ВЫХОД БЕЗ СТАТИСТИКИ
  oled.clear();
  oled.setScale(2);
  oled.setCursorXY(20, 24);
  oled.print("STOPPED");
  drawWiFiIcon(100, 0);
  oled.update();
  delay(1000);
}

// ОСНОВНАЯ ФУНКЦИЯ ИК ДЖАММИНГА
void irJammer(int frequency) {
  if (currentFreqItem == 3) {
    universalTVJammer();
  } else {
    irJamming = true;
    jamStartTime = millis();
    framesSent = 0;
    unsigned long lastBlink = 0;
    bool blinkState = true;
    
    while (irJamming) {
      button1.tick();
      button2.tick();
      button3.tick();
      
      if (millis() - lastBlink > 500) {
        blinkState = !blinkState;
        lastBlink = millis();
        
        oled.clear();
        if (blinkState) {
          oled.setScale(2);
          oled.setCursorXY(18, 25);
          oled.print("JAMMING");
        }
        oled.update();
      }
      
      // ГЕНЕРАЦИЯ ИК СИГНАЛА С МЕРЦАНИЕМ
      generateIRSignal(frequency * 1000);
      framesSent++;
      
      if (button3.isClick()) {
        irJamming = false;
      }
      delay(1);
    }
    
    digitalWrite(IR_LED_PIN, LOW);
    
    // ПРОСТОЙ ВЫХОД БЕЗ СТАТИСТИКИ
    oled.clear();
    oled.setScale(2);
    oled.setCursorXY(20, 24);
    oled.print("STOPPED");
    oled.update();
    delay(1000);
  }
}

void scanWiFi() {
  isScanning = true;
  scanComplete = false;
  numNetworks = 0;
  currentNetwork = 0;

  oled.clear();
  oled.setScale(1);
  oled.setCursorXY(0, 24);
  oled.print("Scanning WiFi...");
  drawWiFiIcon(90, 0);
  oled.update();
  
  for (int i = 0; i < 50; i++) {
    wifiNetworks[i] = "";
    wifiSSIDs[i] = "";
    wifiRSSI[i] = 0;
    wifiEncrypted[i] = true;
  }
  
  int n = WiFi.scanNetworks();
  
  if (n == 0) {
    wifiNetworks[0] = "No networks found";
    numNetworks = 1;
  } else {
    numNetworks = min(n, 50);
    for (int i = 0; i < numNetworks; i++) {
      wifiSSIDs[i] = WiFi.SSID(i);
      wifiRSSI[i] = WiFi.RSSI(i);
      wifiEncrypted[i] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
      
      String encryption = (wifiEncrypted[i]) ? "[ENC]" : "[OPEN]";
      wifiNetworks[i] = wifiSSIDs[i] + " " + encryption + " (" + String(wifiRSSI[i]) + "dBm)";
    }
  }
  
  isScanning = false;
  scanComplete = true;
}

void connectToWiFi(int networkIndex) {
  isConnecting = true;
  connectionResult = false;
  connectionStartTime = millis();
  
  String ssid = wifiSSIDs[networkIndex];
  
  oled.clear();
  oled.setScale(1);
  oled.setCursorXY(0, 16);
  oled.print("Connecting to:");
  oled.setCursorXY(0, 24);
  
  if (ssid.length() > 21) {
    oled.print(ssid.substring(0, 21));
  } else {
    oled.print(ssid);
  }
  
  oled.setCursorXY(0, 32);
  
  if (wifiEncrypted[networkIndex]) {
    oled.print("Password required!");
    oled.setCursorXY(0, 40);
    oled.print("Can't connect");
  } else {
    oled.print("Open network");
    oled.setCursorXY(0, 40);
    oled.print("Connecting...");
    
    WiFi.begin(ssid.c_str());
    
    while (millis() - connectionStartTime < CONNECTION_TIMEOUT) {
      if (WiFi.status() == WL_CONNECTED) {
        connectionResult = true;
        wifiConnected = true;
        break;
      }
      delay(500);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
      connectionResult = false;
      wifiConnected = false;
      WiFi.disconnect();
    }
  }
  
  oled.setCursorXY(0, 48);
  if (connectionResult) {
    oled.print("Connected!");
  } else {
    if (wifiEncrypted[networkIndex]) {
      oled.print("Encrypted network");
    } else {
      oled.print("Failed to connect");
    }
  }
  
  drawWiFiIcon(100, 0);
  oled.update();
  delay(3000);
  
  isConnecting = false;
  inConnectMode = false;
  connectScanComplete = false;
}

void executeAction(int mainItem, int subItem) {
  oled.clear();
  oled.setCursorXY(0, 16);
  oled.setScale(1);
  drawWiFiIcon(90, 0);
  
  switch(mainItem) {
    case 0: // WI-FI
      switch(subItem) {
        case 0: // SCAN WI-FI
          scanWiFi();
          break;
        case 1: // CONECT WI-FI
          inConnectMode = true;
          connectScanComplete = false;
          scanWiFi();
          break;
        case 2: // PACET MONITOR
          startPacketMonitor();
          break;
        case 3: // ATTAK
          oled.setScale(2);
          oled.setCursorXY(22, 24);
          oled.print("ATTAK");
          oled.update();
          delay(3000);
          break;
      }
      break;
      
    case 1: // BLE
      switch(subItem) {
        case 0: // SCAN BLE
          scanBLE();
          break;
        case 1: // GET BLE DEVICE INFO
          getBLEDeviceInfo(); 
          break;
      }
      break;
      
    case 2: // NRF24
      switch(subItem) {
        case 0: // JAMMER 2.4 G
          oled.setScale(2);
          oled.setCursorXY(22, 24);
          oled.print("Jammer");
          oled.update();
          delay(3000);
          break;
        case 1: // SPECTRUM
          oled.setScale(2);
          oled.setCursorXY(22, 24);
          oled.print("Spectrum");
          oled.update();
          delay(3000);
          break;
        case 2: // DISCLAIMER
          oled.setScale(1);
          oled.setCursorXY(30, 0);
          oled.println("Disclaimer:");
          oled.println("There functions were");
          oled.println("made to be used in a");
          oled.println("controlled environ");
          oled.print("ment for study only.");    
          oled.update();
          delay(5000);
          break;
      }
      break;

    case 3: // IR 
      switch(subItem) {
        case 0: // IR JAMMER
          menuLevel = 2;
          currentFreqItem = 0;
          lastItem = -1;
          break;
        case 1: // TV-B-GONE
          tvbGone(); 
          break;
      }
      break;

    case 4: // MAIN MENU
      switch(subItem) {
        case 0: // VERSION 
          oled.setScale(2);
          oled.setCursorXY(40, 24);
          oled.print("1.0");
          oled.update();
          delay(3000);
          break;
        case 1: // CREATOR
          oled.setScale(1);
          oled.setCursorXY(22, 24);
          oled.print("by tranzistor");
          oled.update();
          delay(3000);
          break;
        case 2: // CHANNEL
          oled.setScale(1);
          oled.setCursorXY(14, 24);
          oled.print("Tranzistor_misha");
          oled.update();
          delay(3000);
          break;
          case 3: // WS2812
          oled.setScale(2);
          oled.setCursorXY(22, 24);
          oled.print("WS2812");
          oled.update();
          delay(3000);
          break;
      }
      break;
  }
}

void loop() {
  button1.tick();
  button2.tick();
  button3.tick();

  checkWiFiConnection();

  if (showStartup && millis() - startupTimer >= 2000) {
    showStartup = false;
    inMenu = true;
    lastItem = -1;
    lastSubItem = -1;
    lastMenuLevel = -1;
    lastInMenu = false;
  }

  if (packetMonitorActive) {
    updatePacketMonitor();
    
    if (button1.isClick()) {
      int channels[] = {1, 6, 11, 2, 3, 4, 5, 7, 8, 9, 10, 12, 13};
      int numChannels = 13;
      channelIndex = (channelIndex + 1) % numChannels;
      currentChannel = channels[channelIndex];
      esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    }
    
    if (button2.isClick()) {
      int channels[] = {1, 6, 11, 2, 3, 4, 5, 7, 8, 9, 10, 12, 13};
      int numChannels = 13;
      channelIndex = (channelIndex - 1 + numChannels) % numChannels;
      currentChannel = channels[channelIndex];
      esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
    }
    
    if (button3.isClick()) {
      stopPacketMonitor();
    }
    
    return;
  }

  // Обработка режима просмотра информации о BLE устройстве
  if (inBLEInfoMode) {
    // Управление уже обрабатывается внутри функции getBLEDeviceInfo
    return;
  }

  if (menuLevel == 2) {
    if (button1.isClick()) {
      currentFreqItem = (currentFreqItem == 0) ? irFreqSizes - 1 : currentFreqItem - 1;
      lastItem = -1;
    }
    
    if (button2.isClick()) {
      currentFreqItem = (currentFreqItem == irFreqSizes - 1) ? 0 : currentFreqItem + 1;
      lastItem = -1;
    }
    
    if (button3.isClick()) {
      if (currentFreqItem == 3) {
        irJammer(0);
      } else {
        irJammer(irFreqValues[currentFreqItem]);
      }
      lastItem = -1;
    }
    
    if (button3.isStep()) {
      menuLevel = 1;
      lastItem = -1;
    }
  }
  else if (scanComplete && menuLevel == 1 && currentItem == 0 && (currentSubItem == 0 || inConnectMode)) {
    if (button1.isClick()) {
      currentNetwork = (currentNetwork == 0) ? numNetworks - 1 : currentNetwork - 1;
      lastItem = -1;
    }
    
    if (button2.isClick()) {
      currentNetwork = (currentNetwork == numNetworks - 1) ? 0 : currentNetwork + 1;
      lastItem = -1;
    }
    
    if (button3.isClick() && inConnectMode) {
      connectToWiFi(currentNetwork);
      lastItem = -1;
    }
    
    if (button3.isStep()) {
      scanComplete = false;
      inConnectMode = false;
      menuLevel = 1;
      lastItem = -1;
    }
  }
  else if (bleScanComplete && menuLevel == 1 && currentItem == 1 && currentSubItem == 0) {
    if (button1.isClick()) {
      currentBLEDevice = (currentBLEDevice == 0) ? numBLEDevices - 1 : currentBLEDevice - 1;
      lastItem = -1;
    }
    
    if (button2.isClick()) {
      currentBLEDevice = (currentBLEDevice == numBLEDevices - 1) ? 0 : currentBLEDevice + 1;
      lastItem = -1;
    }
    
    if (button3.isStep()) {
      bleScanComplete = false;
      menuLevel = 1;
      lastItem = -1;
    }
  }
  else if (!showStartup && !isScanning && !isConnecting && !irJamming && !inCustomFreqMode && !isBLEScanning && !isBLEConnecting) {
    if (button1.isClick()) {
      if (menuLevel == 0) {
        currentItem = (currentItem == 0) ? menuSize - 1 : currentItem - 1;
      } else if (menuLevel == 1) {
        currentSubItem = (currentSubItem == 0) ? submenuSizes[currentItem] - 1 : currentSubItem - 1;
      }
      lastItem = -1;
    }
    
    if (button2.isClick()) {
      if (menuLevel == 0) {
        currentItem = (currentItem == menuSize - 1) ? 0 : currentItem + 1;
      } else if (menuLevel == 1) {
        currentSubItem = (currentSubItem == submenuSizes[currentItem] - 1) ? 0 : currentSubItem + 1;
      }
      lastItem = -1;
    }
    
    if (button3.isClick()) {
      if (menuLevel == 0) {
        menuLevel = 1;
        currentSubItem = 0;
      } else if (menuLevel == 1) {
        executeAction(currentItem, currentSubItem);
      }
      lastItem = -1;
      
      if (!inMenu) {
        selectTimer = millis();
      }
    }
    
    if (button3.isStep()) {
      if (menuLevel == 2) {
        menuLevel = 1;
      } else if (menuLevel == 1) {
        menuLevel = 0;
      }
      lastItem = -1;
    }
  }

  if (!inMenu && millis() - selectTimer >= 2000) {
    inMenu = true;
    lastItem = -1;
  }

  if (!showStartup && (lastItem != currentItem || lastSubItem != currentSubItem || lastMenuLevel != menuLevel || lastInMenu != inMenu)) {
    oled.clear();
    oled.setScale(1);
    drawWiFiIcon(90, 0);

    if (isScanning) {
      oled.setCursorXY(0, 24);
      oled.setScale(2);
      oled.print("Scanning...");
      oled.setScale(1);
    }
    else if (isBLEScanning) {
      oled.setCursorXY(0, 24);
      oled.setScale(2);
      oled.print("Scanning BLE...");
      oled.setScale(1);
    }
    else if (isBLEConnecting) {
      oled.setCursorXY(0, 24);
      oled.setScale(2);
      oled.print("BLE Connect...");
      oled.setScale(1);
    }
    else if (isConnecting) {
    }
    else if (menuLevel == 2) {
      oled.setCursorXY(0, 16);
      oled.print("Select frequency:");

      for (int i = 0; i < irFreqSizes; i++) {
        oled.setCursorXY(5, 24 + i * 8);
        if (i == currentFreqItem) {
          oled.print(">"); 
        } else {
          oled.print(" ");
        }
        oled.print(irFreqItems[i]);
      }

      oled.setCursorXY(0, 56);
      oled.print("OK-Start  Long-Back");
    }
    else if (scanComplete && menuLevel == 1 && currentItem == 0 && (currentSubItem == 0 || inConnectMode)) {
      oled.setCursorXY(0, 16);
      oled.print(inConnectMode ? "Select network:" : "Wi-Fi Networks:");

      String networkName = wifiNetworks[currentNetwork];
      if (networkName.length() > 21) networkName = networkName.substring(0, 21);

      oled.setCursorXY(5, 32);
      oled.print(">"); 
      oled.print(networkName);

      oled.setCursorXY(0, 48);
      oled.print(inConnectMode ? "OK - Connect" : "Long OK to exit");
      oled.setCursorXY(0, 56);
      oled.print(inConnectMode ? "Long OK - Exit" : "");
    }
    else if (bleScanComplete && menuLevel == 1 && currentItem == 1 && currentSubItem == 0) {
      oled.setCursorXY(0, 16);
      oled.print("BLE Devices:");

      String deviceName = bleDevices[currentBLEDevice];
      if (deviceName.length() > 21) deviceName = deviceName.substring(0, 21);

      oled.setCursorXY(5, 32);
      oled.print(">"); 
      oled.print(deviceName);

      oled.setCursorXY(0, 44);
      oled.print("Addr: ");
      String address = bleAddresses[currentBLEDevice];
      if (address.length() > 12) address = address.substring(0, 12);
      oled.print(address);

      oled.setCursorXY(0, 56);
      oled.print("Long OK to exit");
    }
    else if (menuLevel == 0) {
      drawCurrentPosition(currentItem, menuSize);
      
      for (int i = 0; i < menuSize; i++) {
        oled.setCursorXY(5, 16 + i * 8);
        oled.invertText(i == currentItem);
        oled.print(menuItems[i]);
        oled.invertText(false);
      }
    }
    else {
      oled.setCursorXY(0, 0);
      oled.print(menuItems[currentItem]);
      oled.print(":");

      for (int i = 0; i < submenuSizes[currentItem]; i++) {
        if (submenuItems[currentItem][i][0] == '\0') continue;
        oled.setCursorXY(5, 24 + i * 8);
        if (i == currentSubItem) {
          oled.print(">"); 
        } else {
          oled.print(" ");
        }
        oled.print(submenuItems[currentItem][i]);
      }
    }

    oled.update();
    lastItem = currentItem;
    lastSubItem = currentSubItem;
    lastMenuLevel = menuLevel;
    lastInMenu = inMenu;
  }
  delay(20);
}


