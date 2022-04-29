/*
 * STUDENT: Sorawish Kittikhankul
 */

/*
 * PollingLab (c) 2021-22 Christopher A. Bohn
 */

#include "cowpi.h"

#define BUTTON_NO_REPEAT_TIME 500u
#define DEBOUNCE_TIME 20u
#define ILLUMINATION_TIME 500u
#define NUMBER_OF_DIGITS 8

uint8_t getKeyPressed();
void displayData(uint8_t address, uint8_t value);
void testSimpleIO();

cowpi_ioPortRegisters *ioPorts;   // an array of I/O ports
cowpi_spiRegisters *spi;          // a pointer to the single set of SPI registers
unsigned long lastLeftButtonPress = 0;
unsigned long lastRightButtonPress = 0;
unsigned long lastLeftSwitchSlide = 0;
unsigned long lastRightSwitchSlide = 0;
unsigned long lastKeypadPress = 0;


// Layout of Matrix Keypad
//        1 2 3 A
//        4 5 6 B
//        7 8 9 C
//        * 0 # D
// This array holds the values we want each keypad button to correspond to
const uint8_t keys[4][4] = {
  {0x1,0x2,0x3,0xA},
  {0x4,0x5,0x6,0xB},
  {0x7,0x8,0x9,0xC},
  {0xF,0x0,0xE,0xD}
};

// Seven Segment Display mapping between segments and bits
// Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
//  DP   A    B    C    D    E    F    G
// This array holds the bit patterns to display each hexadecimal numeral
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

void negate(int index) {
  // uint8_t rightSwitchCurrentPosition = ioPorts[A0_A5].input & 0b00100000;;
  // if (rightSwitchCurrentPosition == 0) {

    if (numsInDisplay != 0) {
      if (builderArray[index-1] == 255) {
        builderArray[index-1] = 0x00;
        displayData(index, 0x00);
        numsInDisplay--;
        printArray();
      } else {
        if (numsInDisplay <= 7) {
          builderArray[index] = 255;
          displayData(index + 1, 0b00000001);
          numsInDisplay++;
          printArray();
        } else {
          tooBig();
        } 
      }
    }

  // Poor attempt at negating hex
  // } else {
  //   for (int i = 0; i < index; i++) {
  //     builderArray[i] = (~(builderArray[i]) + 0b1);
  //     displayData(i + 1, sevenSegments[builderArray[i]]);
  //   }
  // }

    
}
void printArray() {
  for (int i = 7; i >= 0; i--) {
    // Serial.print("Array at ");
    // Serial.print(i);
    // Serial.print(": ");
    if (builderArray[i] != 0x00) {
      if (builderArray[i] == 255) {
        Serial.print("-");
      } else {
        Serial.print(builderArray[i], HEX);
      }
    } 
  }
  Serial.println();
}

void tooBig() {
  displayData(8,0x00);
  displayData(7,0b00001111);
  displayData(6,0b00011101);
  displayData(5,0b00011101);
  displayData(4,0x00);
  displayData(3,0b00011111);
  displayData(2,0b00000110);
  displayData(1,0b01011110);
}

void clearDisplay() {
  for (int i = 0; i < 8; i++) {
    displayData(i+1, 0x00);
    builderArray[i] = 0x00;
  }
  numsInDisplay = 0;
}

void setup() {
  Serial.begin(9600);
  cowpi_setup(SPI | MAX7219);
  ioPorts = (cowpi_ioPortRegisters *)(cowpi_IObase + 0x03);
  spi = (cowpi_spiRegisters *)(cowpi_IObase + 0x2C);
}

void loop() {
  testSimpleIO();
  if (((ioPorts[A0_A5].input & 0b00001111) != 0b00001111) && (millis() - lastKeypadPress > BUTTON_NO_REPEAT_TIME)) {
    uint8_t keypress = getKeyPressed();
    if (keypress < 0x10) {
      int base10 = ioPorts[A0_A5].input & 0b00100000;
      if ((ioPorts[A0_A5].input & 0b00010000) == 0) { // if left switch is left (demonstration mode)
        unsigned long elasped = millis();
        ioPorts[D8_D13].output |= 0b10000;
        while (millis() - elasped < 500);
        ioPorts[D8_D13].output &= 0b11101111;
        if (base10 == 0) {
          if (base(keypress)) {
            clearDisplay();
            displayData(1, sevenSegments[keypress]);
            builderArray[0] = keypress;
            numsInDisplay++;
          }
        // change digit in first arg to display to which place on 7 seg display
        } else {
          clearDisplay();
          displayData(1, sevenSegments[keypress]);
          builderArray[0] = keypress;
          numsInDisplay++;
        }
      // builder mode
      } else {
        // loop to move elements over
        for (int i = 7; i > 0; i--) {
          if (numsInDisplay < 8) {
            if ((builderArray[i-1] != 0x00)) {
              if (base10 == 0) {
                if (base(keypress)) {
                  uint8_t temp = builderArray[i-1];
                  builderArray[i] = temp;
                  if (builderArray[i] == 255) {
                    displayData((i+1), 0b00000001);
                  } else {
                    displayData((i+1), sevenSegments[builderArray[i]]);
                  }
                }
              } else {
                uint8_t temp = builderArray[i-1];
                builderArray[i] = temp;
                if (builderArray[i] == 255) {
                  displayData((i+1), 0b00000001);
                } else {
                  displayData((i+1), sevenSegments[builderArray[i]]);
                }
              }
            }
          }
          
        }
        if (base10 == 0) {
          if (base(keypress)) {
            if (numsInDisplay < 8) {
              builderArray[0] = keypress;
              displayData(1, sevenSegments[keypress]);
              numsInDisplay++;
            } else {
              tooBig();
            }
          }
        } else {
          if (numsInDisplay < 8) {
            builderArray[0] = keypress;
            displayData(1, sevenSegments[keypress]);
            numsInDisplay++;
          } else {
            tooBig();
          }
        }
        printArray();
      }
      if ((ioPorts[A0_A5].input & 0b00010000) == 0) {
        Serial.print("Key pressed: ");
        Serial.println(keypress, HEX);
      }
      
    } else {
      Serial.println("Error reading keypad.");
    }
  }
}

uint8_t getKeyPressed() {
  uint8_t keyPressed = 0xFF;
  unsigned long now = millis();
  if (now - lastKeypadPress > DEBOUNCE_TIME) {
    lastKeypadPress = now;

    // rows
    for (int i = 0; i < 4; i++) {
      ioPorts[D0_D7].output |= 0b11110000;
      ioPorts[D0_D7].output &= ~(1 << (4+i));
      
      // maybe if statement and put for loop inside
      
      // columns
      for (int j = 0; j < 4; j++) {
        if (!(ioPorts[A0_A5].input & (1 << j))) {
          keyPressed = keys[i][j];
        }
      }
    }
    // resetting the row pins' outputs to 0
    ioPorts[D0_D7].output &= 0b00001111;
  }
  return keyPressed;
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

void testSimpleIO() {
  uint8_t printedThisTime = 0;
  // uint8_t leftSwitchCurrentPosition = digitalRead(A4);
  uint8_t leftSwitchCurrentPosition = ioPorts[A0_A5].input & 0b00010000;
  // uint8_t rightSwitchCurrentPosition = digitalRead(A5);
  uint8_t rightSwitchCurrentPosition = ioPorts[A0_A5].input & 0b00100000;
  // uint8_t leftButtonCurrentPosition = digitalRead(8);
  uint8_t leftButtonCurrentPosition = ioPorts[D8_D13].input & 1;
  
  uint8_t rightButtonCurrentPosition = ioPorts[D8_D13].input & 0b10;

  // cannot use digital Write or read
  // digitalWrite(12, leftSwitchCurrentPosition && rightSwitchCurrentPosition);
  if (leftSwitchCurrentPosition && rightSwitchCurrentPosition) {
    // comment out LED for switches
    // ioPorts[D8_D13].output |= 0b10000;
  } else {
    ioPorts[D8_D13].output &= 0b11101111;
  }

  unsigned long now = millis();
  if ((leftSwitchCurrentPosition != leftSwitchLastPosition) && (now - lastLeftSwitchSlide > DEBOUNCE_TIME)) {
    if (!leftSwitchCurrentPosition) {
      Serial.print(now);
    }
    if (!leftSwitchCurrentPosition) {
      Serial.print("\tLeft switch changed: ");
      Serial.print(leftSwitchCurrentPosition);
    }
    leftSwitchLastPosition = leftSwitchCurrentPosition;
    printedThisTime = 1;
    lastLeftSwitchSlide = now;
  }
  if ((rightSwitchCurrentPosition != rightSwitchLastPosition) && (now - lastRightSwitchSlide > DEBOUNCE_TIME)) {
    if (!printedThisTime) {
      if (!leftSwitchCurrentPosition) {
        Serial.print(now);
      }
    }
    if (!leftSwitchCurrentPosition) {
      Serial.print("\tRight switch changed: ");
      Serial.print(rightSwitchCurrentPosition);
    }
    rightSwitchLastPosition = rightSwitchCurrentPosition;
    printedThisTime = 1;
    lastRightSwitchSlide = now;
  }
  if (!leftButtonCurrentPosition && (now - lastLeftButtonPress > BUTTON_NO_REPEAT_TIME)) {
    if (!printedThisTime) {
      if (!leftSwitchCurrentPosition) {
        Serial.print(now);
      } 
    }
    if (leftSwitchCurrentPosition != 0) {
      negate(numsInDisplay);
    }
    if (!leftSwitchCurrentPosition) {
      Serial.print("\tLeft button pressed");
    }
    printedThisTime = 1;
    lastLeftButtonPress = now;
  }
  if (!rightButtonCurrentPosition && (now - lastRightButtonPress > BUTTON_NO_REPEAT_TIME)) {
    if (!printedThisTime) {
      if (!leftSwitchCurrentPosition) {
        Serial.print(now);
      }
    }
    if (!leftSwitchCurrentPosition) {
      Serial.print("\tRight button pressed");
    }
    printedThisTime = 1;
    lastRightButtonPress = now;
    if (leftSwitchCurrentPosition == 0) {
      clearDisplay();
    } else {
      clearDisplay();
      displayData(1, sevenSegments[0]);
    }
    
  }
  if(printedThisTime) {
    if (!leftSwitchCurrentPosition) {
      Serial.println();
    }
  }
}
