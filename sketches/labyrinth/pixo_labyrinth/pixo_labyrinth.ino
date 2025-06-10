#include <Wire.h>   // For I2C communication
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_DotStar.h>
#include <ArduinoJson.h>
#include "I2Cdev.h"
#include "MPU6050.h"

#define DATAPIN    19
#define CLOCKPIN   18
#define BRIGHTNESS 30
#define MAZE_SIZE 16

Adafruit_DotStarMatrix pixo = Adafruit_DotStarMatrix(
  16, 16, DATAPIN, CLOCKPIN,
  DS_MATRIX_TOP    + DS_MATRIX_LEFT +
  DS_MATRIX_ROWS + DS_MATRIX_PROGRESSIVE,
  DOTSTAR_BGR
);

uint8_t labyrinth[MAZE_SIZE][MAZE_SIZE];
MPU6050 IMU;

void setup() {
  Serial.begin(115200);

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  Serial.println("Initializing I2C devices...");
  IMU.initialize();
  Serial.println("Testing device connections...");
  Serial.println(IMU.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
  pixo.begin();
  pixo.setBrightness(BRIGHTNESS);
}

void loop() {
  int16_t x_axis = IMU.getRotationX();
  int16_t y_axis = IMU.getRotationY();
  int16_t z_axis = IMU.getRotationZ();
  Serial.println("X Axis: " + String(x_axis) + "/Y Axis: " + String(y_axis) + "/Z Axis: " + String(z_axis));
}
