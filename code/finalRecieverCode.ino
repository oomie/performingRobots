/*
   Created by Michael Shiloh, Modified by Toomie & Sara: Fall 2024


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
  //THIS TRANSMITTER CODE IS OUTDATED AND WILL NOT WORK WITH MY RECIEVER CODE.
  // THE UPDATED TRANSMITTER CODE IS WITH SARA.

  // Transmitter pin usage
  const int LCD_RS_PIN = 3, LCD_EN_PIN = 2, LCD_D4_PIN = 4, LCD_D5_PIN = 5, LCD_D6_PIN = 6, LCD_D7_PIN = 7;
  const int SW1_PIN = 8, SW2_PIN = 9, SW3_PIN = 10, SW4_PIN = A3, SW5_PIN = A2;

  // LCD library code
  #include <LiquidCrystal.h>

  // initialize the library with the relevant pins
  LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

  // will change these soon
  const int NUM_OF_STATES = 11;
  char* theStates[] = { "0 start eyes",
                   "1 wave hello",
                   "2 nervous glance",
                   "3 die?! headlight",
                   "4 fascinating!",
                   "5 scene2 line1",   // honeybunch
                   "6 scene2 line2",   // parasite , narrow eyes
                   "7 scene2 line3",   // or commit it! dun dun dunn
                   "8 scene2 line4",   // sterile environment tangent, normal eyes, move arms
                   "9 scene2 line5",   //boddy was wealthy
                   "10 scene2 line6"};  // expose scandalous info


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

// the headlights strip
const int stripPin = 19;
const int NUMPIXELS2 = 144;
const int delayval = 100; // Reduced time to pause between flashes for faster blinking

Adafruit_NeoPixel pixels(NUMPIXELS2, stripPin, NEO_GRB + NEO_KHZ800); //strip

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

Servo LeftArm;
Servo RightArm;

int LeftArmPin = 21;
int RightArmPin = 17;

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

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  randomSeed(analogRead(0)); // Initialize random number generator

}

void setupRF24() {
  setupRF24Common();

  // Set us as a receiver
  radio.openWritingPipe(rcvrAddress);
  radio.openReadingPipe(1, xmtrAddress);
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
  int leftAngle = 0;
  int rightAngle = 120;
  LeftArm.write(leftAngle);
  RightArm.write(rightAngle);
}

// moving arms function to be called when needed
void movingArms() {
  for (int i = 0; i < 2; i++) {  // repeat the sequence twice
    // Move arms to 120 degrees
    LeftArm.write(40);
    RightArm.write(120);
    delay(500);
    // Move arms to 80 degrees
    LeftArm.write(0);
    RightArm.write(80);
    delay(500);

    // Move arms to 120 degrees
    LeftArm.write(40);
    RightArm.write(120);
    delay(500);
    // Move arms to 80 degrees
    LeftArm.write(0);
    RightArm.write(80);
    delay(500);
  }
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

// turn all LEDs off
void clearStrip() {
  for (int i = 0; i < NUMPIXELS; i++) {
    matrix.setPixelColor(i, 0);
  }
  matrix.show();
}

void clearheadStrip() {
  for (int i = 0; i < NUMPIXELS2; i++) {
    pixels.setPixelColor(i, 0);
  }
  pixels.show();
}

// circle pattern for the eyes
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


// horizontal lines pattern to look like squinted eyes
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

// COLORS
uint32_t customPink = matrix.Color(255, 80, 80);
uint32_t RED = matrix.Color(255, 0, 0);
uint32_t WHITE = matrix.Color(255, 0, 0); // Define WHITE color
uint32_t GREEN = matrix.Color(0, 255, 0); // Define GREEN color

// function to draw heart eyes
void drawHeartEyes(uint32_t color) {
  // Heart eyes pattern
  const uint8_t heartPattern[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
  };

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (heartPattern[y][x] == 1) {
        // first grid
        matrix.setPixelColor(pixelMap[y][x], color);
        // 2nd grid
        matrix.setPixelColor(pixelMap2[y][x], color);
      } else {
        // Turn off the pixel if it's OFF in the pattern
        matrix.setPixelColor(pixelMap[y][x], 0);
        matrix.setPixelColor(pixelMap2[y][x], 0);
      }
    }
  }
  matrix.show();
}

// angry eyes
void drawAngryEyes(uint32_t color) {
  // Angry eyes pattern
  const uint8_t angryPattern[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
  };

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (angryPattern[y][x] == 1) {
        // Set pixel color for first grid
        matrix.setPixelColor(pixelMap[y][x], color);
        // Set pixel color for second grid
        matrix.setPixelColor(pixelMap2[y][x], color);
      } else {
        // Turn off the pixel if it's OFF in the pattern
        matrix.setPixelColor(pixelMap[y][x], 0);
        matrix.setPixelColor(pixelMap2[y][x], 0);
      }
    }
  }

  matrix.show();
}

// headlight flicker function
void flickerPixels(uint32_t color) {
  unsigned long startTime = millis();
  unsigned long duration = 3000; // 3 seconds

  while (millis() - startTime < duration) {
    // Randomly set some pixels to the specified color and some to off
    for (int i = 0; i < NUMPIXELS2; i++) {
      if (random(10) < 5) { // Approximately 50% chance to turn on each pixel
        pixels.setPixelColor(i, color);
      } else {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Turn off pixel
      }
    }
    pixels.show();   // Send the updated pixel colors to the hardware.
    delay(100); // Shorter pause for faster blinking
  }
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

    // default arm resting angles
    int leftAngle = 80;
    int rightAngle = 120;

    // cases are line by line, following prof plum's lines/parts in the script

    switch (data.stateNumber) {
      case 0:
        // display starting eyes
        clearStrip();
        drawCircle(3, matrix.Color(240, 240, 240));

        flickerPixels(pixels.Color(240, 240, 200));

        break;
      case 1:
        // hello! line
        Serial.println(F("Playing track 001"));
        musicPlayer.startPlayingFile("/track001.mp3"); // hello! line

        clearStrip();
        drawCircle(3, matrix.Color(240, 240, 240));
        flickerPixels(pixels.Color(255, 255, 255));

        break;
      case 2:
        clearheadStrip();
        clearStrip();

        drawHorizontalLines(3, 4, matrix.Color(240, 240, 240));
        musicPlayer.startPlayingFile("/effect00.mp3"); // dun dun dunnnn!

        break;
      case 3:
        clearStrip();
        drawCircle(3, matrix.Color(240, 240, 240));

        musicPlayer.startPlayingFile("/track002.mp3"); //die?
        Serial.println(F("Playing track 003"));

        flickerPixels(pixels.Color(255, 0, 0));
        break;
      case 4: // dun dun dunn
        musicPlayer.startPlayingFile("/dundundu.mp3"); //dun dun dunn!

        break;
      case 5: // SCREAM
        musicPlayer.startPlayingFile("/scream00.mp3"); // scream
        clearStrip();
        drawCircle(3, matrix.Color(128, 0, 160));
        // purple eyes
        flickerPixels(pixels.Color(128, 0, 160));


        break;
      case 6:
        musicPlayer.startPlayingFile("/track003.mp3"); //fascinating!
        clearStrip();
        drawCircle(3, matrix.Color(255, 255, 255));
        flickerPixels(pixels.Color(255, 255, 255));
        Serial.println(F("Playing track 003"));
        movingArms();
        
        break;

      case 7:
        clearStrip();
        drawCircle(3, matrix.Color(255, 255, 255));

        clearheadStrip();
        musicPlayer.startPlayingFile("/track004.mp3"); //honeybunch scene2Line1
        movingArms();

        break;
      case 8:
        musicPlayer.startPlayingFile("/track005.mp3"); //parasite s2L2
        clearStrip();
        drawHorizontalLines(3, 4, matrix.Color(240, 240, 240));

        movingArms();


        break;
      case 9:
        musicPlayer.startPlayingFile("/track006.mp3"); //or commit it! s2L3

        clearStrip();
        drawHorizontalLines(3, 4, matrix.Color(255, 255, 255));
        delay(4500);
        drawHorizontalLines(3, 4, matrix.Color(255, 0, 0));

        break;
      case 10:
        clearStrip();
        drawCircle(3, matrix.Color(240, 240, 240));
        musicPlayer.startPlayingFile("/track007.mp3"); //tangent s2L4
        flickerPixels(pixels.Color(255, 255, 255));

        movingArms();
        movingArms();
        movingArms();

        break;
      case 11:
        musicPlayer.startPlayingFile("/track008.mp3"); //boddy was wealthy s2L4
        clearStrip();
        clearheadStrip();
        drawHorizontalLines(3, 4, matrix.Color(240, 240, 240));
        movingArms();
        
        break;
      case 12:
        musicPlayer.startPlayingFile("/track011.mp3"); //magazine raise arm s2L7

        clearStrip();
        drawCircle(3, matrix.Color(240, 240, 240));
        LeftArm.write(100);

        clearheadStrip();
        flickerPixels(pixels.Color(200, 200, 0));
        LeftArm.write(0);
        
        break;
      case 13:
        musicPlayer.startPlayingFile("/track012.mp3"); // isnt she a dime?WOW!

        clearStrip();
        drawHeartEyes(matrix.Color(255, 20, 100));

        clearheadStrip();
        flickerPixels(pixels.Color(255, 20, 100));

        break;
      case 14:
        musicPlayer.startPlayingFile("/track013.mp3"); // READING BREAK
        clearStrip();
        drawAngryEyes(matrix.Color(255, 255, 255));
        clearheadStrip();
        movingArms();

        break;

      case 15:
        musicPlayer.startPlayingFile("/track015.mp3"); // boddy was a man who... s2L11
        clearStrip();
        drawHorizontalLines(3, 4, matrix.Color(240, 240, 240));
        movingArms();
        movingArms();

        clearStrip();
        drawHorizontalLines(3, 4, matrix.Color(240, 0, 0));

        flickerPixels(pixels.Color(255, 0, 0));
        clearheadStrip();
        break;

      case 16:
        musicPlayer.startPlayingFile("/track016.mp3"); // what is it? s2L12
        clearStrip();
        drawCircle(3, matrix.Color(240, 240, 240));
        break;

      case 17:
        musicPlayer.startPlayingFile("/track017.mp3"); // emphasis on.. s2L13
        clearStrip();
        drawHorizontalLines(3, 4, matrix.Color(240, 240, 240));
        movingArms();

        break;

      case 18:
        musicPlayer.startPlayingFile("/track018.mp3"); // well maybe s2L14
        clearStrip();
        drawHorizontalLines(3, 4, matrix.Color(240, 240, 240));

        break;
      case 19:
        musicPlayer.startPlayingFile("/track019.mp3"); //org chem tangent s2L15
        clearStrip();
        drawCircle(3, matrix.Color(240, 240, 240));
        flickerPixels(pixels.Color(50, 255, 0));
        movingArms();
        movingArms();
        movingArms();

        clearStrip();
        drawAngryEyes(matrix.Color(255, 0, 0)); // TEST

        clearheadStrip();
        flickerPixels(pixels.Color(255, 0, 0));


        break;
      case 20:
        musicPlayer.startPlayingFile("/track020.mp3"); //wbu? s2L16
        clearStrip();
        drawAngryEyes(matrix.Color(255, 0, 0));
        clearheadStrip();
        flickerPixels(pixels.Color(255, 0, 0));
        clearheadStrip();
        // left up right down, one arm up

        break;
      case 21:
        musicPlayer.startPlayingFile("/effect02.mp3"); //shuffle noise
        clearStrip();
        drawAngryEyes(matrix.Color(255, 255, 255));

        movingArms();

        break;
      case 22:
        musicPlayer.startPlayingFile("/track021.mp3"); //AHA!
        clearStrip();
        drawCircle(3, matrix.Color(240, 240, 240));

        flickerPixels(pixels.Color(255, 255, 200));

        clearStrip();
        drawHorizontalLines(3, 4, matrix.Color(240, 240, 240));

        movingArms();
        movingArms();
        // add star eyes

        break;

      case 23:
        musicPlayer.startPlayingFile("/track022.mp3"); // watching you..
        clearStrip();
        drawHorizontalLines(3, 4, matrix.Color(240, 0, 0)); //angry eyes maybe
        flickerPixels(pixels.Color(255, 0, 0));
        clearheadStrip();
        break;

      case 24:
        musicPlayer.startPlayingFile("/track023.mp3"); // i don't know..
        clearStrip();
        drawCircle(3, matrix.Color(240, 240, 240));
        movingArms();

        break;

      case 25:
        musicPlayer.startPlayingFile("/gasp0001.mp3"); // gasp!
        clearStrip();
        drawCircle(3, matrix.Color(128, 0, 160));
        flickerPixels(pixels.Color(128, 0, 160));

        break;
      case 26:
        musicPlayer.startPlayingFile("/track024.mp3"); // oddly quick...
        clearStrip();
        drawHorizontalLines(3, 4, matrix.Color(240, 240, 240));
        flickerPixels(pixels.Color(255, 255, 200));
        flickerPixels(pixels.Color(255, 255, 200));

        movingArms();
        movingArms();
        movingArms();

        break;

      case 27:
        musicPlayer.startPlayingFile("/gasp0001.mp3"); // gasp!
        clearStrip();
        drawCircle(3, matrix.Color(128, 0, 160));
        flickerPixels(pixels.Color(128, 0, 160));
        clearheadStrip();

        break;
      case 28:
        musicPlayer.startPlayingFile("/track025.mp3"); // arresting...yourself?
        clearStrip();
        drawAngryEyes(matrix.Color(255, 255, 255));

        movingArms();
        break;

      case 29:
        musicPlayer.startPlayingFile("/thankyou.mp3"); // thank u! final bow
        // purple heart eyes
        drawHeartEyes(matrix.Color(128, 0, 160));
        //        drawCircle(3, matrix.Color(240, 240, 240));
        flickerPixels(pixels.Color(128, 0, 160));
        break;

      default:
        Serial.println(F("Invalid option"));
    }

  }
}  // end of loop()
// end of receiver code
// */
// CHANGEHERE
