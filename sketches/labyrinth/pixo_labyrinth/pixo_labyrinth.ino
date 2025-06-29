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
// axes movement    {UP,RI,DO, LE}
int direction_x[] = { 0, 1, 0, -1};
int direction_y[] = {-1, 0, 1,  0};
#define UP    0
#define RIGHT 1
#define DOWN  2
#define LEFT  3
int directions[4] = {UP, DOWN, RIGHT, LEFT};
int exitX = -1, exitY = -1;

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
  carveExit();

  // clrscr() and briefly show the starting pixel and exit pixel
  pixo.fillScreen(0);
  pixo.drawPixel(startX, startY, pixo.Color(0, 128, 255));
  pixo.drawPixel(exitX, exitY, pixo.Color(0, 255, 0));
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

// To carve an exit we will just select a random edge pixel in a random
// direction and then verify that this potential exit is actually connected
// to a path, if not we continue trying in another random pixel.
void carveExit() {
  bool exitCarved = false;
  do {
    // Randomizing our four possible directions 
    int exitDirection = random(4);
    switch(exitDirection) {
      case UP:
        exitX = random(1, MATRIX_WIDTH - 1);
        exitY = 0;
        break;
      case DOWN:
        exitX = random(1, MATRIX_WIDTH - 1);
        exitY = MATRIX_HEIGHT - 1;
        break;
      case RIGHT:
        exitX = MATRIX_WIDTH - 1;
        exitY = random(1, MATRIX_HEIGHT - 1);
        break;
      case LEFT:
        exitX = 0;
        exitY = random(1, MATRIX_HEIGHT - 1);
        break;
    }
    // Verify this potential exit is connected to a path
    exitCarved = isConnectedToPath(exitX, exitY, exitDirection);
    Serial.println("Direction: "+String(exitDirection)+", Exit X: "+String(exitX)+", Exit Y: "+String(exitY));
  } while(!exitCarved);
  // If an exit has been placed and it is connected to a path, carve it
  labyrinth[exitX][exitY] = 1;
}

void shuffleDirections(int* dirs) {
  for (int i = 0; i < 4; i++) {
    int r = random(4);
    int temp = dirs[i];
    dirs[i] = dirs[r];
    dirs[r] = temp;
  }
}

bool isConnectedToPath(int x, int y, int direction) {
  // This simple method is checking if there is an adjacent path to the
  // potential exit selected, depending on the direction.
  if (direction == UP && labyrinth[x][y + 1] == 1) {
    return true;
  } else if (direction == DOWN && labyrinth[x][MATRIX_HEIGHT - 2] == 1) {
    return true;
  } else if (direction == RIGHT && labyrinth[MATRIX_WIDTH - 2][y] == 1) {
    return true;
  } else if (direction == LEFT && labyrinth[x + 1][y] == 1) {
    return true;
  }
  return false;
}

bool isInBounds(int x, int y) {
  return x > 0 && x < MATRIX_WIDTH - 1 && y > 0 && y < MATRIX_HEIGHT - 1;
}
