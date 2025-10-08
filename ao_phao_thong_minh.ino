#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <Adafruit_MLX90614.h>

// GPS sử dụng Serial1 (TX=4, RX=5)
TinyGPSPlus gps;

// Sử dụng UART2 cho module SIM: RX=6, TX=7
HardwareSerial sim(2);

// Số điện thoại dùng để gửi cảnh báo (số cố định)
String alertNumber = "0946502863"; // thay số vào đây nha e

// Khai báo các cảm biến
MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Định nghĩa I2C
TwoWire I2C_GY906 = TwoWire(0); // GY-906 trên SDA 9, SCL 10
TwoWire I2C_SENSORS = TwoWire(1); // MPU6050 và MAX30102 trên SDA 11, SCL 12

// Cấu hình buffer cho MAX30102
#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2;
int8_t spo2Valid;
int32_t heartRate;
int8_t heartRateValid;
float measuredTemp;

// Ngưỡng cho phép (đã định nghĩa sẵn tempThreshold cho nhiệt độ cao)
const float tempThreshold = 39.0;      // Ngưỡng nhiệt độ cao bất thường (°C)
const float tempLowThreshold = 35.0;     // Ngưỡng nhiệt độ thấp bất thường (°C)
const int spo2Threshold = 85;            // Ngưỡng SpO2 thấp bất thường (%)
const int heartRateMin = 60;             // Nhịp tim thấp bất thường (bpm)
const int heartRateMax = 100;            // Nhịp tim cao bất thường (bpm)

unsigned long lastWarningTemp = 0;
unsigned long lastWarningSpO2 = 0;
unsigned long lastWarningHR = 0;

String getSenderNumber(String sms) {
  int index = sms.indexOf("+CMGR:");
  if (index < 0) index = sms.indexOf("+CMT:");
  if (index < 0) return "";
  
  int firstQuote = sms.indexOf("\"", index);
  if (firstQuote < 0) return "";
  int secondQuote = sms.indexOf("\"", firstQuote + 1);
  if (secondQuote < 0) return "";
  
  int thirdQuote = sms.indexOf("\"", secondQuote + 1);
  if (thirdQuote < 0) return "";
  int fourthQuote = sms.indexOf("\"", thirdQuote + 1);
  if (fourthQuote < 0) return "";
  
  return sms.substring(thirdQuote + 1, fourthQuote);
}

String convertPhoneNumber(String phoneNumber) {
  if (phoneNumber.startsWith("+84")) {
    return "0" + phoneNumber.substring(3);
  }
  return phoneNumber;
}

String getMessageContent(String sms) {
  int index1 = sms.indexOf("\n");
  if (index1 < 0) return "";
  int index2 = sms.indexOf("\n", index1 + 1);
  if (index2 < 0) return "";
  int index3 = sms.indexOf("\n", index2 + 1);
  if (index3 < 0) return "";
  String content = sms.substring(index2 + 1, index3);
  content.trim();
  return content;
}

// Hàm gửi tin nhắn sử dụng lệnh AT+CMGS
void sendSMS(String phoneNumber, String message) {
  Serial.print("Gửi tin nhắn tới: ");
  Serial.println(phoneNumber);
  
  sim.print("AT+CMGS=\"");
  sim.print(phoneNumber);
  sim.println("\"");
  delay(500);
  
  sim.print(message);
  delay(500);
  
  sim.write(26); // Gửi ký tự Ctrl+Z (ASCII 26) để kết thúc tin nhắn
  delay(3000);
  
  Serial.println("Đã gửi tin nhắn!");
}

void updateGPS() {
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }
}

void setupsensors() {
  bool sensorError = false;
  
  // Khởi tạo GY-906
  I2C_GY906.begin(9, 10, 100000);
  if (!mlx.begin(0x5A, &I2C_GY906)) {
    sensorError = true;
  }
  
  // Khởi tạo MAX30102
  I2C_SENSORS.begin(11, 12, 400000);
  if (!particleSensor.begin(I2C_SENSORS)) {
    sensorError = true;
  }
  
  // Nếu có lỗi, chỉ báo "ERROR"
  if (sensorError) {
    Serial.println("ERROR");
    sendSMS(alertNumber, "ERROR");
    while (1);  // Dừng chương trình
  }
  
  particleSensor.setup();
}

String readSensorData() {
  Serial.println("Bắt đầu đọc cảm biến...");
  measuredTemp = mlx.readObjectTempC();
  uint32_t irValue = particleSensor.getIR();
  String resultMessage;
  
  if (irValue < 5000) {
    resultMessage = "ERROR";
    Serial.println(resultMessage);
    return resultMessage;
  } else {
    for (int i = 0; i < BUFFER_SIZE; i++) {
      while (!particleSensor.check()) {
        delay(1);
      }
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
    }
    
    maxim_heart_rate_and_oxygen_saturation(
      irBuffer, BUFFER_SIZE, redBuffer,
      &spo2, &spo2Valid, &heartRate, &heartRateValid
    );
    
    resultMessage = "Du lieu do duoc:\n";
    resultMessage += "Nhiet do co the: " + String(measuredTemp, 1) + " C\n";
    resultMessage += "SpO2: " + (spo2Valid ? String(spo2) + "%" : "Khong hop le") + "\n";
    resultMessage += "Nhip tim: " + (heartRateValid ? String(heartRate) + " bpm" : "Khong hop le");
    
    Serial.println(resultMessage);
    return resultMessage;
  }
}

void checkThresholds() {
  unsigned long currentMillis = millis();
  String combinedWarning = "";
  
  // Kiểm tra nhiệt độ
  if (measuredTemp > tempThreshold) {
    if (lastWarningTemp == 0 || (currentMillis - lastWarningTemp >= 1800000UL)) {
      combinedWarning += "CANH BAO: Nhiet do co the cao bat thuong (" + String(measuredTemp, 1) + " C)!\n";
      lastWarningTemp = currentMillis;
    }
  } else if (measuredTemp < tempLowThreshold) {
    if (lastWarningTemp == 0 || (currentMillis - lastWarningTemp >= 1800000UL)) {
      //combinedWarning += "CANH BAO: Nhiet do co the thap bat thuong (" + String(measuredTemp, 1) + " C)!\n";
      lastWarningTemp = currentMillis;
    }
  } else {
    lastWarningTemp = 0;
  }
  
  // Kiểm tra SpO2
  if (spo2Valid && spo2 < spo2Threshold) {
    if (lastWarningSpO2 == 0 || (currentMillis - lastWarningSpO2 >= 1800000UL)) {
      combinedWarning += "CANH BAO: SpO2 thap bat thuong (" + String(spo2) + "%)!\n";
      lastWarningSpO2 = currentMillis;
    }
  } else {
    lastWarningSpO2 = 0;
  }
  
  // Kiểm tra nhịp tim
  if (heartRateValid && heartRate > heartRateMax) {
    if (lastWarningHR == 0 || (currentMillis - lastWarningHR >= 1800000UL)) {
      combinedWarning += "CANH BAO: Nhip tim cao bat thuong (" + String(heartRate) + " bpm)!\n";
      lastWarningHR = currentMillis;
    }
  } else if (heartRateValid && heartRate < heartRateMin) {
    if (lastWarningHR == 0 || (currentMillis - lastWarningHR >= 1800000UL)) {
      combinedWarning += "CANH BAO: Nhip tim thap bat thuong (" + String(heartRate) + " bpm)!\n";
      lastWarningHR = currentMillis;
    }
  } else {
    lastWarningHR = 0;
  }
  
  // Nếu có cảnh báo, cập nhật GPS và thêm link nếu tìm thấy, nếu không thì thông báo "Khong tim thay GPS"
  if (combinedWarning != "") {
    updateGPS();
    if (gps.location.isValid()) {
      String gpsLink = "https://www.google.com/maps?q=" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
      combinedWarning += "\nLocation: " + gpsLink;
    } else {
      combinedWarning += "\nKhong tim thay GPS";
    }
    Serial.println(combinedWarning);
    sendSMS(alertNumber, combinedWarning);
  }
}

void checkIncomingSMS(String resultMessage) {
  // Cập nhật dữ liệu GPS
  updateGPS();
  String gpsInfo = "";
  if (gps.location.isValid()) {
    gpsInfo = "https://www.google.com/maps?q=" 
              + String(gps.location.lat(), 6) + "," 
              + String(gps.location.lng(), 6);
  } else {
    gpsInfo = "Khong tim thay GPS";
  }
  
  if (sim.available()) {
    String buffer = sim.readString();
    Serial.println("Dữ liệu nhận được từ SIM:");
    Serial.println(buffer);
    
    if (buffer.indexOf("+CMTI:") >= 0) {
      int commaIndex = buffer.indexOf(",");
      if (commaIndex != -1) {
        String indexStr = buffer.substring(commaIndex + 1);
        indexStr.trim();
        int smsIndex = indexStr.toInt();
        Serial.print("Chỉ số tin nhắn: ");
        Serial.println(smsIndex);
        
        // Gửi lệnh đọc tin nhắn từ SIM
        String readCmd = "AT+CMGR=" + String(smsIndex);
        sim.println(readCmd);
        delay(1000); // Đảm bảo module có thời gian phản hồi
        String fullSMS = sim.readString();
        Serial.println("Nội dung SMS đọc được:");
        Serial.println(fullSMS);
        
        // Trích xuất số điện thoại và nội dung tin nhắn
        String senderNumber = getSenderNumber(fullSMS);
        String convertedNumber = convertPhoneNumber(senderNumber);
        String smsContent = getMessageContent(fullSMS);
        Serial.println("Số điện thoại: " + convertedNumber);
        Serial.println("Tin nhắn: '" + smsContent + "'");
        
        smsContent.trim();
        // Nếu tin nhắn nhận được là "check" (không phân biệt chữ hoa thường)
        if (smsContent.equalsIgnoreCase("check")) {
          String messageToSend = resultMessage;
          messageToSend += "\nLocation: " + gpsInfo;
          sendSMS(convertedNumber, messageToSend);
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, 4, 5);  // Serial1 cho GPS Neo-7M (TX trên pin 4, RX trên pin 5)
  sim.begin(115200, SERIAL_8N1, 6, 7); // Cấu hình UART cho SIM module (RX=6, TX=7)
  setupsensors(); 
  delay(1000);
  Serial.println("Khởi động ESP32 và SIM A7680C...");
  // Cấu hình chế độ SMS dạng văn bản
  sim.println("AT+CMGF=1");
  delay(500);
  // Cấu hình thông báo tin nhắn mới
  sim.println("AT+CNMI=2,1,0,0,0");
  delay(500);
}

void loop() {
  String resultMessage = readSensorData();
  checkThresholds();
  checkIncomingSMS(resultMessage);
  delay(500);
}
