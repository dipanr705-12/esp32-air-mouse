/*
 * ================================================
 *  ESP32 Air Mouse — Bluetooth HID Gyroscope Mouse
 * ================================================
 * Author  : Dipan Roy
 * Board   : ESP32 FM DevKit
 * Sensor  : MPU6050 (Gyroscope + Accelerometer)
 * Protocol: Bluetooth Classic HID (Mouse)
 *
 * Connections:
 *   MPU6050 VCC  → 3.3V
 *   MPU6050 GND  → GND
 *   MPU6050 SDA  → GPIO 21
 *   MPU6050 SCL  → GPIO 22
 *   Push Button  → GPIO 27 (other pin to GND)
 * ================================================
 */

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <BleMouseESP32.h>  // ESP32 BLE HID Mouse library

// ---- Pin Definitions ----
#define BUTTON_PIN 27

// ---- Objects ----
Adafruit_MPU6050 mpu;
BleMouse bleMouse("ESP32 Air Mouse", "Dipan Roy", 100);

// ---- Sensitivity ----
const float MOUSE_SENSITIVITY = 3.0;
const int   DEADZONE          = 0;   // ignore tiny movements

// ---- Setup ----
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Air Mouse Starting...");

  // Button with internal pull-up (active LOW)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize MPU6050
  Wire.begin(21, 22);  // SDA, SCL
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found! Check wiring.");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU6050 initialized.");

  // Start Bluetooth HID Mouse
  bleMouse.begin();
  Serial.println("Bluetooth HID Mouse started. Pair as 'ESP32 Air Mouse'.");
}

// ---- Main Loop ----
void loop() {
  if (!bleMouse.isConnected()) {
    Serial.println("Waiting for Bluetooth connection...");
    delay(500);
    return;
  }

  // Read sensor data
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Map accelerometer tilt to mouse movement
  float ax = accel.acceleration.x;  // left/right tilt
  float ay = accel.acceleration.y;  // forward/backward tilt

  int moveX = (int)(ax * MOUSE_SENSITIVITY);
  int moveY = (int)(ay * MOUSE_SENSITIVITY);

  // Apply dead zone
  if (abs(moveX) <= DEADZONE) moveX = 0;
  if (abs(moveY) <= DEADZONE) moveY = 0;

  // Send mouse movement
  if (moveX != 0 || moveY != 0) {
    bleMouse.move(moveX, -moveY, 0);  // invert Y for natural feel
  }

  // Left click on button press (active LOW)
  if (digitalRead(BUTTON_PIN) == LOW) {
    bleMouse.click(MOUSE_LEFT);
    Serial.println("Click!");
    delay(200);  // simple debounce
  }

  delay(10);  // ~100 Hz polling rate
}
