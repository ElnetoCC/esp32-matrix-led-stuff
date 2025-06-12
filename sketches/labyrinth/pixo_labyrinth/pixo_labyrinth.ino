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
  
  generateMaze();
}

void loop() {
  // clrscr()
  pixo.fillScreen(0);

  // Draw generated maze, walls (0s), gray
  for (int x = 0; x < MAZE_SIZE; x++) {
    for (int y = 0; y < MAZE_SIZE; y++) {
      if (labyrinth[x][y] == 0){
        pixo.drawPixel(x, y, pixo.Color(200, 200, 200));
      }
    }
  }
  pixo.show();
  delay(1000);
}

void generateMaze() {
  // Fill with walls (0s)
  for (int x = 0; x < MAZE_SIZE; x++) {
    for (int y = 0; y < MAZE_SIZE; y++) {
      labyrinth[x][y] = 0;
    }
  }

  // Start from a cell around the middle of the matrix
  int startX = random(6, 10);
  int startY = random(6, 10);
  labyrinth[startX][startY] = 1;
  Serial.println("Center: ("+String(startX)+", "+String(startY)+")");

  // clrscr() and briefly show the starting cell
  pixo.fillScreen(0);
  pixo.drawPixel(startX, startY, pixo.Color(0, 255, 0));
  pixo.show();
  delay(1000);
}
