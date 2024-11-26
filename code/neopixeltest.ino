#include <Adafruit_NeoPixel.h>

// Define the number of LEDs and the pin they're connected to
#define NUMPIXELS 128 // 64 LEDs for each 8x8 matrix, so 128 in total
#define PIN 18


Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Array defining 8x8 grid layout for both matrices
int pixelMap[8][8] = {
  {0,  1,  2,  3,  4,  5,  6,  7},
  {8,  9,  10, 11, 12, 13, 14, 15},
  {16, 17, 18, 19, 20, 21, 22, 23},
  {24, 25, 26, 27, 28, 29, 30, 31},
  {32, 33, 34, 35, 36, 37, 38, 39},
  {40, 41, 42, 43, 44, 45, 46, 47},
  {48, 49, 50, 51, 52, 53, 54, 55},
  {56, 57, 58, 59, 60, 61, 62, 63}
};

int pixelMap2[8][8] = {
  {64, 65, 66, 67, 68, 69, 70, 71},
  {72, 73, 74, 75, 76, 77, 78, 79},
  {80, 81, 82, 83, 84, 85, 86, 87},
  {88, 89, 90, 91, 92, 93, 94, 95},
  {96, 97, 98, 99, 100, 101, 102, 103},
  {104, 105, 106, 107, 108, 109, 110, 111},
  {112, 113, 114, 115, 116, 117, 118, 119},
  {120, 121, 122, 123, 124, 125, 126, 127}
};

// Function to set all LEDs to off
void clearStrip() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, 0);
  }
  strip.show();
}

// Function to draw a circle pattern
void drawCircle(int r, uint32_t color) {
  float centerX = 3.5, centerY = 3.5; // Correct center for 8x8 grid
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      float distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
      if (distance <= r) {
        // Set for the first grid
        strip.setPixelColor(pixelMap[y][x], color);
        // Set for the second grid
        strip.setPixelColor(pixelMap2[y][x], color);
      }
    }
  }
  strip.show();
}

// Function to draw two horizontal lines
void drawHorizontalLines(int row1, int row2, uint32_t color) {
  for (int x = 0; x < 8; x++) {
    // First grid
    strip.setPixelColor(pixelMap[row1][x], color);
    strip.setPixelColor(pixelMap[row2][x], color);
    // Second grid
    strip.setPixelColor(pixelMap2[row1][x], color);
    strip.setPixelColor(pixelMap2[row2][x], color);
  }
  strip.show();
}

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to off
  clearStrip();
}

void loop() {
  uint32_t red = strip.Color(255, 0, 0);
  strip.setBrightness(20);
  clearStrip();

  // Animation: Fill the circle
  for (int r = 3; r <= 3; r++) { // Expand circle from radius 1 to 3
    clearStrip();
    drawCircle(r, red);
    delay(500);
  }

  delay(1000);

  // Animation: Transform circle into two horizontal lines
  clearStrip();
  drawHorizontalLines(3, 4, red); // Draw lines in rows 3 and 4
  delay(1000);

  // Repeat animation
}
