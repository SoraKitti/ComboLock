/*
 * STUDENTS: Sorawish Kittikhankul and Corbin Lamplot
 */

/*
 * CombinationLock GroupLab (c) 2022 Christopher A. Bohn
 */

#include <EEPROM.h>
#include "cowpi.h"

#define BUTTON_NO_REPEAT_TIME 500u
#define DEBOUNCE_TIME 20u
#define ILLUMINATION_TIME 500u
#define NUMBER_OF_DIGITS 8

void cursor(int position);
uint8_t getKeyPressed();
void displayData(uint8_t address, uint8_t value);
void buildCombo(uint8_t key, int index);
void testSimpleIO();
void errorDisplay();
void labOpenDisplay();
void badTryDisplay(int attempt);
void alertDisplay();
void enterDisplay();
void reEnterDisplay();
void changedDisplay();
void noChangeDisplay();
void closedDisplay();

cowpi_ioPortRegisters *ioPorts;   // an array of I/O ports
cowpi_spiRegisters *spi;          // a pointer to the single set of SPI registers
unsigned long lastLeftButtonPress = 0;
unsigned long lastRightButtonPress = 0;
unsigned long lastLeftSwitchSlide = 0;
unsigned long lastRightSwitchSlide = 0;
unsigned long lastKeypadPress = 0;
unsigned long cursorHalfSecond = 0;
bool blinkState = false; //Variable for cursor
int position = 3; //Position var for curson
bool key1 = false;
bool key2 = false;
bool key3 = false;
bool key4 = false;
bool key5 = false;
bool key6 = false;

// return 0 for invalid key, return 1 for valid key
int base(uint8_t key) {
  switch (key) {
    case 0xA:
      return 0;
      break;
    case 0xB:
      return 0;
      break;
    case 0xC:
      return 0;
      break;
    case 0xD:
      return 0;
      break;
    case 0xE:
      return 0;
      break;
    case 0xF:
      return 0;
      break;
    default:
      return 1;
   }
}

uint8_t builderArray[8] = {0x00};
int numsInDisplay = 0;


const uint8_t sevenSegments[16] = {
  // {0,1,2,3,4,5,6,7,8,9,"A","b","c","d","E","F"}
  0b01111110, //0
  0b00110000, //1
  0b01101101, //2
  0b01111001, //3
  0b00110011, //4
  0b01011011, //5
  0b01011111, //6
  0b01110000, //7
  0b01111111, //8
  0b01111011, //9
  0b01110111, //A
  0b00011111, //b
  0b00001101, //c
  0b00111101, //d
  0b01001111, //E
  0b01000111, //F
};

const uint8_t keys[4][4] = {
  {0x1,0x2,0x3,0xA},
  {0x4,0x5,0x6,0xB},
  {0x7,0x8,0x9,0xC},
  {0xF,0x0,0xE,0xD}
};

void setup() {
  Serial.begin(9600);
  cowpi_setup(SPI | MAX7219);
  ioPorts = (cowpi_ioPortRegisters *)(cowpi_IObase + 0x03);
  spi = (cowpi_spiRegisters *)(cowpi_IObase + 0x2C);
  builderArray[2] = 255;
   displayData(3, 0b00000001);
   builderArray[5] = 255;
   displayData(6, 0b00000001);
}


void loop() {
  cursor(position);
  if(!digitalRead(9) && (millis() - lastRightButtonPress > 100u)){
    position++;
    lastRightButtonPress = millis();
  }
  
  //testSimpleIO();
  if (((ioPorts[A0_A5].input & 0b00001111) != 0b00001111) && (millis() - lastKeypadPress > BUTTON_NO_REPEAT_TIME)) {
    uint8_t keypress = getKeyPressed();
    if (keypress < 0x10) {

      // get count of nums in combination currently, then call buildCombo with the next index and the key pressed, then update count
      buildCombo(keypress, position);
      
    }

    if ((ioPorts[A0_A5].input & 0b00010000) == 0) {
      Serial.print("Key pressed: ");
      Serial.println(keypress, HEX);
    }
      
  } else {
    Serial.println("Error reading keypad.");
  }
}


// consider calling this once with an index and a value instead of the key
void buildCombo(uint8_t key, int position) {
  switch (position % 3) {
    case(0):
      if(!key1){
        builderArray[7] = sevenSegments[key];
        key1 = true;
        key2 = false;
      } else if(!key2){
        builderArray[6] = sevenSegments[key];
        key2 = true;
        key1 = false;
      }
      break;
    case(1): 
      if(!key3){
        builderArray[4] = sevenSegments[key];
        key3 = true;
        key4 = false;
      } else if(!key4){
        builderArray[3] = sevenSegments[key];
        key4 = true;
        key3 = false;
      }
      break;
    case(2):
      if(!key5){
        builderArray[1] = sevenSegments[key];
        key5 = true;
        key6 = false;
      } else if(!key6){
        builderArray[0] = sevenSegments[key];
        key6 = true;
        key5 = false;
      }
  }
}

//Displays 'Error'
void errorDisplay(){
  displayData(8, 0x00);
  displayData(7, 0x00);
  displayData(6, 0x00);
  displayData(5, 0b01001111); //E
  displayData(4, 0b00000101); //r
  displayData(3, 0b00000101); //r
  displayData(2, 0b00011101); //o
  displayData(1, 0b00000101); //r
}
//Displays 'LAb oPEN'
void labOpenDisplay(){
  displayData(8, 0b00001110); //L
  displayData(7, 0b01110111); //A
  displayData(6, 0b00011111); //b
  displayData(5, 0b00000000); //
  displayData(4, 0b00011101); //o
  displayData(3, 0b01100111); //P
  displayData(2, 0b01001111); //E
  displayData(1, 0b01110110); //N
}
//Displays 'bAdtry #'
void badTryDisplay(int attempt){
  displayData(8, 0b00011111); //b
  displayData(7, 0b01110111); //A
  displayData(6, 0b00111101); //d
  displayData(5, 0b00001111); //t
  displayData(4, 0b00000101); //r
  displayData(3, 0b00111011); //y
  displayData(2, 0b00000000); //

  if(attempt == 1){
    displayData(1, 0b00110000); //1
  } else if(attempt == 2){
    displayData(1, 0b01101101); //2
  } else if(attempt == 3){
    displayData(1, 0b01111001); //3
  } else{
    alertDisplay();
  }
}

//Displays 'ALErt!'
void alertDisplay(){
  displayData(8, 0x00);
  displayData(7, 0x00);
  displayData(6, 0b01110111); //A
  displayData(5, 0b00001110); //L
  displayData(4, 0b01001111); //E
  displayData(3, 0b00000101); //r
  displayData(2, 0b00001111); //t
  displayData(1, 0b10110000); //!
}

//Displays 'ENtEr'
void enterDisplay(){
  displayData(8, 0x00);
  displayData(7, 0x00);
  displayData(6, 0x00);
  displayData(5, 0b01001111); //E
  displayData(4, 0b01110110); //N
  displayData(3, 0b00001111); //t
  displayData(2, 0b01001111); //E
  displayData(1, 0b00000101); //r
}

//Displays 'rE-ENtEr'
void reEnterDisplay(){
  displayData(8, 0b00000101); //r
  displayData(7, 0b01001111); //E
  displayData(6, 0b00000001); //-
  displayData(5, 0b01001111); //E
  displayData(4, 0b01110110); //N
  displayData(3, 0b00001111); //t
  displayData(2, 0b01001111); //E
  displayData(1, 0b00000101); //r
}

//Displays 'cHANGEd'
void changedDisplay(){
  displayData(8, 0x00);
  displayData(7, 0b00001101); //c
  displayData(6, 0b00110111); //H
  displayData(5, 0b01110111); //A
  displayData(4, 0b01110110); //N
  displayData(3, 0b01011110); //G
  displayData(2, 0b01001111); //E
  displayData(1, 0b00111101); //d
}

//Displays 'NocHANGE'
void noChangeDisplay(){
  displayData(8, 0b01110110); //N
  displayData(7, 0b00011101); //o
  displayData(6, 0b00001101); //c
  displayData(5, 0b00110111); //H
  displayData(4, 0b01110111); //A
  displayData(3, 0b01110110); //N
  displayData(2, 0b01011110); //G
  displayData(1, 0b01001111); //E
}

//Displays 'cLoSEd'
void closedDisplay(){
  displayData(8, 0x00);
  displayData(7, 0x00);
  displayData(6, 0b00001101); //c
  displayData(5, 0b00001110); //L
  displayData(4, 0b00011101); //o
  displayData(3, 0b01011011); //S
  displayData(2, 0b01001111); //E
  displayData(1, 0b00111101); //d
}

uint8_t getKeyPressed() {
  uint8_t keyPressed = 0xFF;
  unsigned long now = millis();
  if (now - lastKeypadPress > DEBOUNCE_TIME) {
    lastKeypadPress = now;
    for(int i = 0; i < 4; i++){
      ioPorts[2].output |= 0b11110000;
      ioPorts[2].output &= 0b11101111 << i;
      if((~ioPorts[1].input & 0b00001111)){
        for(int j = 0; j < 4; j++){
          if(~ioPorts[1].input & 0b00000001 << j){
            keyPressed = keys[i][j];
          }
        }
      }
    }
    (ioPorts[2].output &= 0b00000000);
  }
  return keyPressed;
}

//Blinking cursor
void cursor(int position){
  switch(position % 3){
    case 0: 
      displayData(2, builderArray[1] &= 0b01111111); //Clear other decimals with these four.
      displayData(1, builderArray[0] &= 0b01111111);
      displayData(5, builderArray[4] &= 0b01111111);
      displayData(4, builderArray[3] &= 0b01111111);
      // Check if cursor is high or low and wait half second before switching
      if(blinkState){
        if(millis() - cursorHalfSecond >= 500){
          displayData(8, builderArray[7] &= 0b01111111);
          displayData(7, builderArray[6] &= 0b01111111);
          blinkState = false;
          cursorHalfSecond = millis();
        }
      } else{
        if(millis() - cursorHalfSecond >= 500){
          displayData(8, builderArray[7] |= 0b10000000); 
          displayData(7, builderArray[6] |= 0b10000000);
          blinkState = true;
          cursorHalfSecond = millis();
        }
      }
    break;
    //repeat for each position 
    case 1: 
      displayData(8, builderArray[7] &= 0b01111111);
      displayData(7, builderArray[6] &= 0b01111111);
      displayData(2, builderArray[1] &= 0b01111111); 
      displayData(1, builderArray[0] &= 0b01111111);
      if(blinkState){
        if(millis() - cursorHalfSecond >= 500){
          displayData(5, builderArray[4] &= 0b01111111);
          displayData(4, builderArray[3] &= 0b01111111);
          blinkState = false;
          cursorHalfSecond = millis();
        }
      } else{
        if(millis() - cursorHalfSecond >= 500){
          displayData(5, builderArray[4] |= 0b10000000); 
          displayData(4, builderArray[3] |= 0b10000000);
          blinkState = true;
          cursorHalfSecond = millis();
        }
      }
    break;
    case 2:
      displayData(8, builderArray[7] &= 0b01111111);
      displayData(7, builderArray[6] &= 0b01111111);
      displayData(5, builderArray[4] &= 0b01111111);
      displayData(4, builderArray[3] &= 0b01111111);
      if(blinkState){
        if(millis() - cursorHalfSecond >= 500){
          displayData(2, builderArray[1] &= 0b01111111); 
          displayData(1, builderArray[0] &= 0b01111111);
          blinkState = false;
          cursorHalfSecond = millis();
        }
      } else{
        if(millis() - cursorHalfSecond >= 500){
          displayData(2, builderArray[1] |= 0b10000000); 
          displayData(1, builderArray[0] |= 0b10000000);
          blinkState = true;
          cursorHalfSecond = millis();
        }
      }
    break;
    default:
      errorDisplay();
    break;
  }
}

void displayData(uint8_t address, uint8_t value) {
  // address is MAX7219's register address (1-8 for digits; otherwise see MAX7219 datasheet Table 2)
  // value is the bit pattern to place in the register
  cowpi_spiEnable;
  ioPorts[D8_D13].output &= 0b111011;
  spi->data = address;
  while ((spi->status == 0));
  spi->data = value;
  while ((spi->status == 0));
  ioPorts[D8_D13].output |= 0b000100;
  // ...
  cowpi_spiDisable;
}

uint8_t leftSwitchLastPosition = 0;
uint8_t rightSwitchLastPosition = 0;

// void testSimpleIO() {
//   uint8_t printedThisTime = 0;
//   // uint8_t leftSwitchCurrentPosition = digitalRead(A4);
//   uint8_t leftSwitchCurrentPosition = ioPorts[A0_A5].input & 0b00010000;
//   // uint8_t rightSwitchCurrentPosition = digitalRead(A5);
//   uint8_t rightSwitchCurrentPosition = ioPorts[A0_A5].input & 0b00100000;
//   // uint8_t leftButtonCurrentPosition = digitalRead(8);
//   uint8_t leftButtonCurrentPosition = ioPorts[D8_D13].input & 1;
  
//   uint8_t rightButtonCurrentPosition = ioPorts[D8_D13].input & 0b10;

//   // cannot use digital Write or read
//   // digitalWrite(12, leftSwitchCurrentPosition && rightSwitchCurrentPosition);
//   if (leftSwitchCurrentPosition && rightSwitchCurrentPosition) {
//     // comment out LED for switches
//     // ioPorts[D8_D13].output |= 0b10000;
//   } else {
//     ioPorts[D8_D13].output &= 0b11101111;
//   }

//   unsigned long now = millis();
//   if ((leftSwitchCurrentPosition != leftSwitchLastPosition) && (now - lastLeftSwitchSlide > DEBOUNCE_TIME)) {
//     if (!leftSwitchCurrentPosition) {
//       Serial.print(now);
//     }
//     if (!leftSwitchCurrentPosition) {
//       Serial.print("\tLeft switch changed: ");
//       Serial.print(leftSwitchCurrentPosition);
//     }
//     leftSwitchLastPosition = leftSwitchCurrentPosition;
//     printedThisTime = 1;
//     lastLeftSwitchSlide = now;
//   }
//   if ((rightSwitchCurrentPosition != rightSwitchLastPosition) && (now - lastRightSwitchSlide > DEBOUNCE_TIME)) {
//     if (!printedThisTime) {
//       if (!leftSwitchCurrentPosition) {
//         Serial.print(now);
//       }
//     }
//     if (!leftSwitchCurrentPosition) {
//       Serial.print("\tRight switch changed: ");
//       Serial.print(rightSwitchCurrentPosition);
//     }
//     rightSwitchLastPosition = rightSwitchCurrentPosition;
//     printedThisTime = 1;
//     lastRightSwitchSlide = now;
//   }
//   if (!leftButtonCurrentPosition && (now - lastLeftButtonPress > BUTTON_NO_REPEAT_TIME)) {
//     if (!printedThisTime) {
//       if (!leftSwitchCurrentPosition) {
//         Serial.print(now);
//       } 
//     }
//     if (leftSwitchCurrentPosition != 0) {
//       negate(numsInDisplay);
//     }
//     if (!leftSwitchCurrentPosition) {
//       Serial.print("\tLeft button pressed");
//     }
//     printedThisTime = 1;
//     lastLeftButtonPress = now;
//   }
//   if (!rightButtonCurrentPosition && (now - lastRightButtonPress > BUTTON_NO_REPEAT_TIME)) {
//     if (!printedThisTime) {
//       if (!leftSwitchCurrentPosition) {
//         Serial.print(now);
//       }
//     }
//     if (!leftSwitchCurrentPosition) {
//       Serial.print("\tRight button pressed");
//     }
//     printedThisTime = 1;
//     lastRightButtonPress = now;
//     if (leftSwitchCurrentPosition == 0) {
//       clearDisplay();
//     } else {
//       clearDisplay();
//       displayData(1, sevenSegments[0]);
//     }
    
//   }
//   if(printedThisTime) {
//     if (!leftSwitchCurrentPosition) {
//       Serial.println();
//     }
//   }
// }

// Everything below this line was provided by Bohn and I have yet to actually integrate it into current setup

// void alarmUsingDelay();
// void responsiveMessageWithoutInterrupts();
// void displayMessage();
// bool leftButtonIsPressed();
// bool rightButtonIsPressed(){
//   unsigned long now = millis();
//   if(digitalRead(9) && now - lastRightButtonPress > DEBOUNCE_TIME){
//     lastRightButtonPress = now;
//     return true;
//   } else{
//     return false;
//   }
// };
// bool leftSwitchInLeftPosition();
// bool leftSwitchInRightPosition();
// bool rightSwitchInLeftPosition();
// bool rightSwitchInRightPosition();

// unsigned long countdownStart = 0;
// const uint8_t *message = NULL;
// const uint8_t *lastMessage = NULL;

// const uint8_t alertMessage[8] = {...};
// const uint8_t leftMessage[8] = {...};
// const uint8_t rightMessage[8] = {...};
// const uint8_t clearMessage[8] = {...};

// void setup() {
//   Serial.begin(9600);
//   cowpi_setup(SPI | MAX7219);
// }

// void loop() {
//   if (leftSwitchInLeftPosition() && leftButtonIsPressed()) {
//     alarmUsingDelay();
//   } else if (leftSwitchInRightPosition()) {
//     responsiveMessageWithoutInterrupts();
//   }
// }

// void alarmUsingDelay() {
//   displayMessage(alertMessage);
//   while(1) {
//     digitalWrite(12, HIGH);
//     delay(250);
//     digitalWrite(12, LOW);
//     delay(250);
//   }
// }

// void responsiveMessageWithoutInterrupts() {
//   if (leftButtonIsPressed()) {
//     countdownStart = millis();
//     message = leftMessage;
//     lastMessage = clearMessage;
//     displayMessage(message);
//   } else if (rightButtonIsPressed()) {
//     countdownStart = millis();
//     message = rightMessage;
//     lastMessage = clearMessage;
//     displayMessage(message);
//   } else {
//     unsigned long now = millis();
//     if (now - countdownStart > 1000) {
//       countdownStart = now;
//       if (message == clearMessage) {
//         message = lastMessage;
//         lastMessage = clearMessage;
//       } else {
//         lastMessage = message;
//         message = clearMessage;
//       }
//       if (message != NULL) {
//         displayMessage(message);
//       }
//     }
//   }
// }
