#include <Wire.h>   // For I2C communication
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_DotStar.h>
#include <ArduinoJson.h>
#include "I2Cdev.h"
#include "MPU6050.h"

#define DATAPIN       19
#define CLOCKPIN      18
#define MATRIX_WIDTH  16
#define MATRIX_HEIGHT 16
#define BRIGHTNESS    30

Adafruit_DotStarMatrix pixo = Adafruit_DotStarMatrix(
  MATRIX_WIDTH, MATRIX_HEIGHT, DATAPIN, CLOCKPIN,
  DS_MATRIX_TOP + DS_MATRIX_LEFT +
  DS_MATRIX_ROWS + DS_MATRIX_PROGRESSIVE,
  DOTSTAR_BGR
);

MPU6050 IMU;

uint8_t labyrinth[MATRIX_WIDTH][MATRIX_HEIGHT];
// axes movement     UP,RI,DO,LE
int direction_x[] = { 0, 1, 0, -1};
int direction_y[] = {-1, 0, 1,  0};
#define UP    0
#define RIGHT 1
#define DOWN  2
#define LEFT  3

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

  // Draw generated maze, walls(0) in gray
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      if (labyrinth[x][y] == 0){
        pixo.drawPixel(x, y, pixo.Color(200, 200, 200));
      }
    }
  }
  pixo.show();
  delay(1000);
}

void generateMaze() {
  // Fill with walls(0)
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
      labyrinth[x][y] = 0;
    }
  }

  // Start from a pixel around the middle of the matrix
  int startX = random(6, 10);
  int startY = random(6, 10);
  labyrinth[startX][startY] = 1;
  Serial.println("Center: ("+String(startX)+", "+String(startY)+")");

  carveMaze(startX, startY);

  // clrscr() and briefly show the starting pixel
  pixo.fillScreen(0);
  pixo.drawPixel(startX, startY, pixo.Color(0, 255, 0));
  pixo.show();
  delay(1000);
}

// --------------------------------------------------------------------
// Recursive backtracking algorithm for maze generation.
// This particular version jumps two pixels from current as we will be
// using the LEDs of the matrix as walls and paths. PIXEL ON=wall/OFF=path.
// As a side effect, due to the 16x16 matrix, depending on the starting
// pixel, there will be double walls at two edges for each maze generated.
// Source: https://weblog.jamisbuck.org/2010/12/27/maze-generation-recursive-backtracking
// --------------------------------------------------------------------
void carveMaze(int current_x, int current_y) {
  int directions[4] = {UP, DOWN, RIGHT, LEFT};
  shuffleDirections(directions);

  for (int i = 0; i < 4; i++) {
    Serial.print(directions[i]);
    // For each direction, it checks two pixels away
    int next_x = current_x + direction_x[directions[i]] * 2;
    int next_y = current_y + direction_y[directions[i]] * 2;
    Serial.println(": nx: "+String(next_x)+"/ny: "+String(next_y));
    // to see if it is still within bounds and unvisited
    if (isInBounds(next_x, next_y) && labyrinth[next_x][next_y] == 0) {
      labyrinth[next_x][next_y] = 1;
      labyrinth[current_x + direction_x[directions[i]]][current_y + direction_y[directions[i]]] = 1;
      carveMaze(next_x, next_y);
    }
  }
}

void shuffleDirections(int* dirs) {
  for (int i = 0; i < 4; i++) {
    int r = random(4);
    int temp = dirs[i];
    dirs[i] = dirs[r];
    dirs[r] = temp;
  }
}

bool isInBounds(int x, int y) {
  return x > 0 && x < MATRIX_WIDTH - 1 && y > 0 && y < MATRIX_HEIGHT - 1;
}
