// servos are in this code however it was not working. reciever code only. 

#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Servo.h>

// Define the number of LEDs and the pin they're connected to
#define NUMPIXELS 128  // 64 LEDs for each 8x8 matrix, so 128 in total
#define PIN 18

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Array defining 8x8 grid layout for both matrices
int pixelMap[8][8] = {
  { 0, 1, 2, 3, 4, 5, 6, 7 },
  { 8, 9, 10, 11, 12, 13, 14, 15 },
  { 16, 17, 18, 19, 20, 21, 22, 23 },
  { 24, 25, 26, 27, 28, 29, 30, 31 },
  { 32, 33, 34, 35, 36, 37, 38, 39 },
  { 40, 41, 42, 43, 44, 45, 46, 47 },
  { 48, 49, 50, 51, 52, 53, 54, 55 },
  { 56, 57, 58, 59, 60, 61, 62, 63 }
};

int pixelMap2[8][8] = {
  { 64, 65, 66, 67, 68, 69, 70, 71 },
  { 72, 73, 74, 75, 76, 77, 78, 79 },
  { 80, 81, 82, 83, 84, 85, 86, 87 },
  { 88, 89, 90, 91, 92, 93, 94, 95 },
  { 96, 97, 98, 99, 100, 101, 102, 103 },
  { 104, 105, 106, 107, 108, 109, 110, 111 },
  { 112, 113, 114, 115, 116, 117, 118, 119 },
  { 120, 121, 122, 123, 124, 125, 126, 127 }
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
  float centerX = 3.5, centerY = 3.5;  // Correct center for 8x8 grid
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

// Common code for RF24
const int NRF_CE_PIN = A11, NRF_CSN_PIN = A15;
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
const byte CUSTOM_ADDRESS_BYTE = 0x73;
const int CUSTOM_CHANNEL_NUMBER = 40;
const byte xmtrAddress[] = { CUSTOM_ADDRESS_BYTE, CUSTOM_ADDRESS_BYTE, 0xC7, 0xE6, 0xCC };
const byte rcvrAddress[] = { CUSTOM_ADDRESS_BYTE, CUSTOM_ADDRESS_BYTE, 0xC7, 0xE6, 0x66 };
const int RF24_POWER_LEVEL = RF24_PA_LOW;
uint8_t pipeNum;
unsigned int totalTransmitFailures = 0;
struct DataStruct {
  uint8_t stateNumber;
} data;

void setupRF24Common() {
  if (!radio.begin()) {
    Serial.println(F("radio initialization failed"));
    while (1);
  } else {
    Serial.println(F("radio successfully initialized"));
  }
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(CUSTOM_CHANNEL_NUMBER);
  radio.setPALevel(RF24_POWER_LEVEL);
}

// Additional libraries for music maker shield
#include <Adafruit_VS1053.h>
#include <SD.h>

// Servo library
#include <Servo.h>

// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

// Servo motors
const int NOSE_SERVO_PIN = 20;

Servo nose;  // change names to describe what's moving
Servo antenna;
Servo tail;
Servo grabber;
Servo disk;

Servo LeftArm;
Servo RightArm;

int LeftArmPin = 21;
int RightArmPin = 17;

// change as per your robot
const int NOSE_WRINKLE = 45;
const int NOSE_TWEAK = 90;
const int TAIL_ANGRY = 0;
const int TAIL_HAPPY = 180;
const int GRABBER_RELAX = 0;
const int GRABBER_GRAB = 180;

void setup() {
  Serial.begin(9600);

  // Set up all the attached hardware
  setupMusicMakerShield();
  setupServoMotors();
  setupNeoPixels();
  setupRF24();

  // Brief flash to show we're done with setup()
  flashNeoPixels();
}

void setupRF24() {
  setupRF24Common();

  // Set us as a receiver
  radio.openWritingPipe(rcvrAddress);
  radio.openReadingPipe(1, xmtrAddress);

  // radio.printPrettyDetails();
  Serial.println(F("I am a receiver"));
}

void setupMusicMakerShield() {
  if (!musicPlayer.begin()) {  // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1);
  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD card failed or not present"));
    while (1);  // don't do anything more
  }

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20, 20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
}


void setupServoMotors() {
  // Set up the arms
  LeftArm.attach(LeftArmPin);
  RightArm.attach(RightArmPin);

  // resting position
  LeftArm.write(80);
  RightArm.write(120);
}

void setupNeoPixels() {
  strip.begin();
  strip.setBrightness(40);
  clearStrip();
}

void flashNeoPixels() {
  strip.fill(strip.Color(0, 255, 0), 0, NUMPIXELS);
  strip.show();
  delay(500);
  clearStrip();
}

void loop() {
  radio.startListening();
  if (radio.available(&pipeNum)) {
    radio.read(&data, sizeof(data));
    Serial.print(F("message received Data = "));
    Serial.println(data.stateNumber);

    switch (data.stateNumber) {
      case 0:
        clearStrip();
        drawCircle(3, strip.Color(255, 0, 0));  // Draw a red circle with radius 3
        break;
      case 1:
        clearStrip();
        drawHorizontalLines(3, 4, strip.Color(0, 255, 0));  // Draw green horizontal lines
        break;
      case 2:

      break;
      case 3:
        Serial.println(F("arms moving up and down"));
        LeftArm.write(80);
        RightArm.write (80);
        delay(1000);
        LeftArm.write(120);
        RightArm.write(120);
        delay(1000);

        break;
      case 4:

        break;
        
      default:
        clearStrip();
        break;
    }
  }
}
