/**
 * TMP117 Test with Custom I2C Pins for ESP32-S3
 * Reads temperature and prints to Serial.
 */

#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>

#define I2C_SDA 21  // Custom SDA pin
#define I2C_SCL 22  // Custom SCL pin

Adafruit_TMP117 tmp117;  // Default constructor uses global Wire

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10); // Wait for Serial Monitor

  Serial.println("Adafruit TMP117 test with custom I2C pins!");

  // Initialize global Wire with custom pins
  Wire.begin(I2C_SDA, I2C_SCL);

  // Try to initialize the sensor
  if (!tmp117.begin()) {
    Serial.println("Failed to find TMP117 chip");
    while (1) { delay(10); }
  }
  Serial.println("TMP117 Found!");
}

void loop() {
  sensors_event_t temp; // Create an empty event to hold data
  tmp117.getEvent(&temp); // Fill the event with current measurements

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degrees C");
  Serial.println("");

  delay(1000);
}