#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <MAX30105.h>
#include <spo2_algorithm.h>
#include <Adafruit_MLX90614.h>

MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

TwoWire I2C_GY906 = TwoWire(0); // GY-906 trên SDA 9, SCL 10
TwoWire I2C_SENSORS = TwoWire(1); // MAX30102 trên SDA 11, SCL 12

#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2;
int8_t spo2Valid;
int32_t heartRate;
int8_t heartRateValid;

void setupsensors() {
  // Khởi tạo GY-906
  I2C_GY906.begin(9, 10, 100000);
  if (!mlx.begin(0x5A, &I2C_GY906)) {
    Serial.println("Không tìm thấy cảm biến GY-906!");
    while (1);
  }
  Serial.println("GY-906 đã sẵn sàng!");

  // Khởi tạo MAX30102
  I2C_SENSORS.begin(11, 12, 400000);
  if (!particleSensor.begin(I2C_SENSORS)) {
    Serial.println("Không tìm thấy cảm biến MAX30102!");
    while (1);
  }
  Serial.println("MAX30102 đã sẵn sàng!");
  particleSensor.setup(); 
}

void scanSensors() {
  Serial.println("\n===== Đọc cảm biến =====");
  float objectTemp = mlx.readObjectTempC();

  uint32_t irValue = particleSensor.getIR();
  if (irValue < 5000) {
    Serial.println("Vui lòng đặt tay vào cảm biến để đo nhịp tim và SpO2.");
    return;
  }

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

  Serial.print("Nhiệt độ cơ thể: ");
  Serial.print(objectTemp, 1);
  Serial.println(" °C");

  Serial.print("SpO2: ");
  if (spo2Valid) Serial.print(spo2);
  else Serial.print("Không hợp lệ");
  Serial.println(" %");

  Serial.print("Nhịp tim: ");
  if (heartRateValid) Serial.print(heartRate);
  else Serial.print("Không hợp lệ");
  Serial.println(" bpm");
}

void setup() {
  Serial.begin(115200);
  setupsensors();
}

void loop() {
  scanSensors();        
  delay(1000);  
}
