/*
   edited nov 26 - neopixels are not turning on at all. even when trying regular code that is not part of the reciever
   Created by Michael Shiloh, Modified by Toomie & Sara: Fall 2024
   With code from Pranav and Ahsen that was modified.

   Using the nRF24L01 radio module to communicate
   between two Arduinos with much increased reliability following
   various tutorials, conversations, and studying the nRF24L01 datasheet
   and the library reference.

   Transmitter is
   https://github.com/michaelshiloh/resourcesForClasses/tree/master/kicad/Arduino_Shield_RC_Controller

  Receiver is
  https://github.com/michaelshiloh/resourcesForClasses/blob/master/kicad/nRF_servo_Mega

   This file contains code for both transmitter and receiver.
   Transmitter at the top, receiver at the bottom.
   One of them is commented out, so you need to comment in or out
   the correct section. You don't need to make changes to this
   part of the code, just to comment in or out depending on
   whether you are programming your transmitter or receiver

   You need to set the correct address for your robot.

   Search for the phrase CHANGEHERE to see where to
   comment or uncomment or make changes.

   These sketches require the RF24 library by TMRh20
   Documentation here: https://nrf24.github.io/RF24/index.html

   change log

   11 Oct 2023 - ms - initial entry based on
                  rf24PerformingRobotsTemplate
   26 Oct 2023 - ms - revised for new board: nRF_Servo_Mega rev 2
   28 Oct 2023 - ms - add demo of NeoMatrix, servo, and Music Maker Shield
   20 Nov 2023 - as - fixed the bug which allowed counting beyond the limits
   22 Nov 2023 - ms - display radio custom address byte and channel
   12 Nov 2024 - ms - changed names for channel and address allocation for Fall 2024
                      https://github.com/michaelshiloh/resourcesForClasses/blob/master/kicad/nRF_servo_Mega
                      https://github.com/michaelshiloh/resourcesForClasses/blob/master/kicad/nRFControlPanel
*/

// Common code
//

// Common pin usage
// Note there are additional pins unique to transmitter or receiver
//

// nRF24L01 uses SPI which is fixed
// on pins 11, 12, and 13 on the Uno
// and on pins 50, 51, and 52 on the Mega

// It also requires two other signals
// (CE = Chip Enable, CSN = Chip Select Not)
// Which can be any pins:

// CHANGEHERE
// For the transmitter
//const int NRF_CE_PIN = A4, NRF_CSN_PIN = A5;

// CHANGEHERE
// for the receiver
const int NRF_CE_PIN = A11, NRF_CSN_PIN = A15;

// nRF 24L01 pin   name
//          1      GND
//          2      3.3V
//          3      CE
//          4      CSN
//          5      SCLK
//          6      MOSI/COPI
//          7      MISO/CIPO

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);  // CE, CSN

//#include <printf.h>  // for debugging

// See note in rf24Handshaking about address selection
//

// Channel and address allocation:
// Rama and Hind Y: Channel 30, addr = 0x76
// Ahsen and Pranav: Channel 40, addr = 0x73
// Sara & Toomie:  Channel 50, addr = 0x7C
// Avinash and Vahagn: Channel 60, addr = 0xC6
// Hind A & Javeria:  Channel 70, addr = 0xC3
// Mbebo and Aaron: Channel 80, addr = 0xCC
// Linh and Luke: Channel 90, addr = 0x33

// CHANGEHERE
const byte CUSTOM_ADDRESS_BYTE = 0x7C;  // change as per the above assignment
const int CUSTOM_CHANNEL_NUMBER = 50;   // change as per the above assignment

// Do not make changes here
const byte xmtrAddress[] = { CUSTOM_ADDRESS_BYTE, CUSTOM_ADDRESS_BYTE, 0xC7, 0xE6, 0xCC };
const byte rcvrAddress[] = { CUSTOM_ADDRESS_BYTE, CUSTOM_ADDRESS_BYTE, 0xC7, 0xE6, 0x66 };

const int RF24_POWER_LEVEL = RF24_PA_LOW;

// global variables
uint8_t pipeNum;
unsigned int totalTransmitFailures = 0;

struct DataStruct {
  uint8_t stateNumber;
};
DataStruct data;

void setupRF24Common() {

  // RF24 setup
  if (!radio.begin()) {
    Serial.println(F("radio  initialization failed"));
    while (1)
      ;
  } else {
    Serial.println(F("radio successfully initialized"));
  }

  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(CUSTOM_CHANNEL_NUMBER);
  radio.setPALevel(RF24_POWER_LEVEL);
}

// CHANGEHERE
/*
  // Transmitter code

  // Transmitter pin usage
  const int LCD_RS_PIN = 3, LCD_EN_PIN = 2, LCD_D4_PIN = 4, LCD_D5_PIN = 5, LCD_D6_PIN = 6, LCD_D7_PIN = 7;
  const int SW1_PIN = 8, SW2_PIN = 9, SW3_PIN = 10, SW4_PIN = A3, SW5_PIN = A2;

  // LCD library code
  #include <LiquidCrystal.h>

  // initialize the library with the relevant pins
  LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

  // will change these soon
  const int NUM_OF_STATES = 6;
  char* theStates[] = { "0 robot wakes", // will change these soon
                     "1 robot hello",
                     "2 robot explains",
                     "3 robot pleads",
                     "4 robot weeps",
                     "5 robot goodbye" };

  void updateLCD() {

  lcd.clear();
  lcd.print(theStates[data.stateNumber]);
  lcd.setCursor(0, 1);  // column, line (from 0)
  lcd.print("not transmitted yet");
  }

  void countDown() {
  data.stateNumber = (data.stateNumber > 0) ? (data.stateNumber - 1) : 0;
  updateLCD();
  }

  void countUp() {
  if (++data.stateNumber >= NUM_OF_STATES) {
   data.stateNumber = NUM_OF_STATES - 1;
  }
  updateLCD();
  }


  void spare1() {}
  void spare2() {}

  void rf24SendData() {

  radio.stopListening();  // go into transmit mode
  // The write() function will block
  // until the message is successfully acknowledged by the receiver
  // or the timeout/retransmit maxima are reached.
  int retval = radio.write(&data, sizeof(data));

  lcd.clear();
  lcd.setCursor(0, 0);  // column, line (from 0)
  lcd.print("transmitting");
  lcd.setCursor(14, 0);  // column, line (from 0)
  lcd.print(data.stateNumber);

  Serial.print(F(" ... "));
  if (retval) {
   Serial.println(F("success"));
   lcd.setCursor(0, 1);  // column, line (from 0)
   lcd.print("success");
  } else {
   totalTransmitFailures++;
   Serial.print(F("failure, total failures = "));
   Serial.println(totalTransmitFailures);

   lcd.setCursor(0, 1);  // column, line (from 0)
   lcd.print("error, total=");
   lcd.setCursor(13, 1);  // column, line (from 0)
   lcd.print(totalTransmitFailures);
  }
  }

  class Button {
  int pinNumber;
  bool previousState;
  void (*buttonFunction)();
  public:

  // Constructor
  Button(int pn, void* bf) {
   pinNumber = pn;
   buttonFunction = bf;
   previousState = 1;
  }

  // update the button
  void update() {
   bool currentState = digitalRead(pinNumber);
   if (currentState == LOW && previousState == HIGH) {
     Serial.print("button on pin ");
     Serial.print(pinNumber);
     Serial.println();
     buttonFunction();
   }
   previousState = currentState;
  }
  };

  const int NUMBUTTONS = 5;
  Button theButtons[] = {
  Button(SW1_PIN, countDown),
  Button(SW2_PIN, rf24SendData),
  Button(SW3_PIN, countUp),
  Button(SW4_PIN, spare1),
  Button(SW5_PIN, spare2),
  };

  void setupRF24() {

  setupRF24Common();

  // Set us as a transmitter
  radio.openWritingPipe(xmtrAddress);
  radio.openReadingPipe(1, rcvrAddress);

  // radio.printPrettyDetails();
  Serial.println(F("I am a transmitter"));

  data.stateNumber = 0;
  }

  void setup() {
  Serial.begin(9600);
  Serial.println(F("Setting up LCD"));

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();
  // Print a message to the LCD.
  lcd.print("Radio setup");

  // Display the address in hex
  lcd.setCursor(0, 1);
  lcd.print("addr 0x");
  lcd.setCursor(7, 1);
  char s[5];
  sprintf(s, "%02x", CUSTOM_ADDRESS_BYTE);
  lcd.print(s);

  // Display the channel number
  lcd.setCursor(10, 1);
  lcd.print("ch");
  lcd.setCursor(13, 1);
  lcd.print(CUSTOM_CHANNEL_NUMBER);

  Serial.println(F("Setting up radio"));
  setupRF24();

  // If setupRF24 returned then the radio is set up
  lcd.setCursor(0, 0);
  lcd.print("Radio OK state=");
  lcd.print(theStates[data.stateNumber]);

  // Initialize the switches
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW3_PIN, INPUT_PULLUP);
  pinMode(SW4_PIN, INPUT_PULLUP);
  pinMode(SW5_PIN, INPUT_PULLUP);
  }



  void loop() {
  for (int i = 0; i < NUMBUTTONS; i++) {
   theButtons[i].update();
  }
  delay(50);  // for testing
  }


  void clearData() {
  // set all fields to 0
  data.stateNumber = 0;
  }

  // End of transmitter code
  // CHANGEHERE
*/
// /*
// Receiver Code
// CHANGEHERE

// Additional libraries for music maker shield
#include <Adafruit_VS1053.h>
#include <SD.h>

// Servo library
#include <Servo.h>

// Additional libraries for graphics on the Neo Pixel Matrix
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif


// Define the number of LEDs and the pin they're connected to
#define NUMPIXELS 128  // 64 LEDs for each 8x8 matrix, so 128 in total
#define PIN 18

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

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

// Additional pin usage for receiver

// Adafruit music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)
#define CARDCS 4         // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin
Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

// Connectors for NeoPixels and Servo Motors are labeled
// M1 - M6 which is not very useful. Here are the pin
// assignments:
// M1 = 19
// M2 = 21 //left servo
// M3 = 20
// M4 = 16
// M5 = 18 // neopixel
// M6 = 17 //right servo

// Servo motors
const int NOSE_SERVO_PIN = 20;


// Neopixel
//const int NEOPIXELPIN = 18;
////const int NUMPIXELS = 64;
////#define NEOPIXELPIN 18
////#define NUMPIXELS 64  // change to fit
////Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXELPIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, NEOPIXELPIN,
//                            NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
//                            NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
//                            NEO_GRB            + NEO_KHZ800);



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
  // printf_begin();

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
    while (1)
      ;
  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD card failed or not present"));
    while (1)
      ;  // don't do anything more
  }

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20, 20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
}

void setupServoMotors() {
  // Set up the arms
  LeftArm.attach(LeftArmPin);
  RightArm.attach(RightArmPin);

  // resting position
  int leftAngle = 80;
  int rightAngle = 120;
  LeftArm.write(leftAngle);
  RightArm.write(rightAngle);
}

void setupNeoPixels() {
  //  pixels.begin();
  //  pixels.clear();
  //  pixels.show();
  matrix.begin();
  // matrix.setTextWrap(false);
  matrix.setBrightness(40);
  //matrix.setTextColor(matrix.Color(200, 30, 40));
}

void flashNeoPixels() {

  // Using the Matrix library
  //  matrix.fillScreen(matrix.Color(0, 255, 0));
  matrix.show();
  delay(500);
  // matrix.fillScreen(0);
  matrix.show();

  //  // all on
  //  for (int i = 0; i < NUMPIXELS; i++) {  // For each pixel...
  //    pixels.setPixelColor(i, pixels.Color(0, 100, 0));
  //  }
  //  pixels.show();
  //  delay(500);
  //
  //  // all off
  //  pixels.clear();
  //  pixels.show();
}

// Function to set all LEDs to off
void clearStrip() {
  for (int i = 0; i < NUMPIXELS; i++) {
    matrix.setPixelColor(i, 0);
  }
  matrix.show();
}

// Function to draw a circle pattern
void drawCircle(int r, uint32_t color) {
  float centerX = 3.5, centerY = 3.5;  // Correct center for 8x8 grid
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      float distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
      if (distance <= r) {
        // Set for the first grid
        matrix.setPixelColor(pixelMap[y][x], color);
        // Set for the second grid
        matrix.setPixelColor(pixelMap2[y][x], color);
      }
    }
  }
  matrix.show();
}


// Function to draw two horizontal lines
void drawHorizontalLines(int row1, int row2, uint32_t color) {
  for (int x = 0; x < 8; x++) {
    // First grid
    matrix.setPixelColor(pixelMap[row1][x], color);
    matrix.setPixelColor(pixelMap[row2][x], color);
    // Second grid
    matrix.setPixelColor(pixelMap2[row1][x], color);
    matrix.setPixelColor(pixelMap2[row2][x], color);
  }
  matrix.show();
}


void loop() {
  // If there is data, read it,
  // and do the needfull
  // Become a receiver
  radio.startListening();
  if (radio.available(&pipeNum)) {
    radio.read(&data, sizeof(data));
    Serial.print(F("message received Data = "));
    Serial.print(data.stateNumber);
    Serial.println();

    // Resting angles
    int leftAngle = 80;
    int rightAngle = 120;

    switch (data.stateNumber) {
      case 0:
        // tail.write(TAIL_ANGRY);
        // play track 0
        // display something on LEDs
        clearStrip();
        drawCircle(3, matrix.Color(255, 0, 0));  // Draw a red circle with radius 3
        Serial.print(F("Received 0"));
        break;
      case 1:

        Serial.println(F("Playing track 001"));
        musicPlayer.startPlayingFile("/track001.mp3");

        break;
      case 2:
        // Serial.println(F("moving nose to 30"));
        // nose.write(30);

        //         matrix.drawRect(2, 2, 5, 5, matrix.Color(0, 200, 30));

        matrix.show();
        clearStrip();
        drawCircle(3, matrix.Color(255, 0, 0));  // Draw a red circle with radius 3
        // Serial.println(F("Playing track 001"));
        // musicPlayer.startPlayingFile("/track001.mp3");
        Serial.println(F("Playing suspicious sound effect"));
        musicPlayer.startPlayingFile("/effect01.wav");
        break;
      case 3:
        // Edited - Testing servo arm movements
        LeftArm.write(80);
        RightArm.write (80);
        //delay(1000);

        //delay(1000);

        break;
      case 4:
        LeftArm.write(120);
        RightArm.write(120);
        Serial.println(F("Playing shuffling sounds"));
        musicPlayer.startPlayingFile("/effect02.mp3");
        break;

      default:
        Serial.println(F("Invalid option"));
    }

  }
}  // end of loop()
// end of receiver code
// */
// CHANGEHERE
