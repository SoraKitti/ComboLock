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

void keyPadInput();
void clearDisplay();
void comboDisplay();
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
bool leftButtonIsPressed();
bool rightButtonIsPressed();
void confirmCombo();
void clearBuilder();
void clearKeys();

cowpi_ioPortRegisters *ioPorts;   // an array of I/O ports
cowpi_spiRegisters *spi;          // a pointer to the single set of SPI registers
unsigned long lastLeftButtonPress = 0;
unsigned long lastRightButtonPress = 0;
unsigned long lastKeypadPress = 0;
unsigned long cursorHalfSecond = 0;
bool blinkState = false; //Variable for cursor
int position = 3; //Position var for curson
int attempt = 1; //Bad try tracker
bool sixEntered = false; //Check if 6 numbers were entered
bool key1 = false;
bool key2 = false;
bool key3 = false;
bool key4 = false;
bool key5 = false;
bool key6 = false;
bool correctCombo = true;
bool locked = true;
bool changing = false;
bool enter = true;
bool reEnter = false;

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
uint8_t keyArray[6] = {0x00};
uint8_t keyArrayCopy[6] = {0x00};


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
  comboDisplay();
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
}


void loop() {
  keyPadInput();
  if(locked){
    cursor(position);
    if(rightButtonIsPressed()){
      position++;
    }
    //Checks Combo and displays attempts
    if(leftButtonIsPressed()){
      for(int i = 0; i < 6; i++){
        int value = 0;
        EEPROM.get(i, value);
        Serial.println(keyArray[i]);
        Serial.println(value);
      }
      confirmCombo();
      if(!correctCombo){
        badTryDisplay(attempt);
        delay(1000);
        comboDisplay();
        attempt++;
      }
      if(attempt == 4){
        while(0 < 1){
          alertDisplay();
          digitalWrite(12, HIGH);
          delay(250);
          clearDisplay();
          digitalWrite(12, LOW);
          delay(250);
        }
      }
      if(correctCombo && sixEntered){
        labOpenDisplay();
        delay(1000);
        locked = false;
      }
    }
  }
  if(!locked){
    if(rightButtonIsPressed() && !digitalRead(A4) && !digitalRead(A5) && !changing){
      unsigned long now = millis();
      while(millis() - now < 500){
        if(rightButtonIsPressed()){
          closedDisplay();
          delay(1000);
          clearDisplay();
          clearBuilder();
          clearKeys();
          comboDisplay();
          position = 3;
          locked = true;
        }
      }
    }
    if(digitalRead(A4) && digitalRead(A5) && leftButtonIsPressed() && !changing){
      changing = true;
      enter = true;
      enterDisplay();
      delay(1000);
    }
    if(changing){
      clearDisplay();
      clearBuilder();
      clearKeys();
      comboDisplay();
      position = 3;
      while(enter){
        cursor(position);
        if(rightButtonIsPressed()){
          position++;
        }
        keyPadInput();
        if(!digitalRead(A4) && leftButtonIsPressed()){
          if(builderArray[0] == 0x00 || builderArray[1] == 0x00 || builderArray[3] == 0x00 || 
             builderArray[4] == 0x00 || builderArray[6] == 0x00 || builderArray[7] == 0x00){
            errorDisplay();
            delay(1000);
            comboDisplay();
            sixEntered = false;
          } else{
            reEnterDisplay();
            delay(1000);
            for(int i = 0; i < 6; i++){
              keyArrayCopy[i] = keyArray[i];
            }
            reEnter = true;
            enter = false;
          } 
        }
      }
      clearDisplay();
      clearBuilder();
      clearKeys();
      comboDisplay();
      position = 3;
      while(reEnter){
        cursor(position);
        if(rightButtonIsPressed()){
          position++;
        }
        keyPadInput();
        correctCombo = false;
        if(!digitalRead(A5) && leftButtonIsPressed()){
          if(builderArray[0] == 0x00 || builderArray[1] == 0x00 || builderArray[3] == 0x00 || 
             builderArray[4] == 0x00 || builderArray[6] == 0x00 || builderArray[7] == 0x00){
            errorDisplay();
            delay(1000);
            comboDisplay();
            sixEntered = false;
          } else{
              for(int i = 0; i < sizeof(keyArray); i++){
              correctCombo = true;            
              if(keyArrayCopy[i] != keyArray[i]){
                correctCombo = false;
              }      
            }
            if(correctCombo){
              changedDisplay();
              delay(1000);
              for(int i = 0; i < 6; i++){
                EEPROM.put(i, keyArray[i]);
              }
              labOpenDisplay();
              changing = false;
              reEnter = false;          
            }
            if(!correctCombo){
              noChangeDisplay();
              delay(1000);
              labOpenDisplay();
              changing = false;
              reEnter = false;
            }
          }
        }
      }
    }
  }
}

void keyPadInput(){
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
    //Serial.println("Error reading keypad.");
  }
}

void clearBuilder(){
  for(int i = 0; i < 8; i++){
    builderArray[i] = 0x00;
  }
}

void clearKeys(){
  for(int i = 0; i < 6; i++){
    keyArray[i] = 0x00;
  }
}

void clearDisplay() {
  for (int i = 0; i < 8; i++) {
    displayData(i+1, 0x00);
  }
}

void comboDisplay(){
  builderArray[2] = 255;
  displayData(3, 0b00000001);
  builderArray[5] = 255;
  displayData(6, 0b00000001);
}

void confirmCombo(){
  if(builderArray[0] == 0x00 || builderArray[1] == 0x00 || builderArray[3] == 0x00 || 
     builderArray[4] == 0x00 || builderArray[6] == 0x00 || builderArray[7] == 0x00){
    errorDisplay();
    delay(1000);
    comboDisplay();
    sixEntered = false;
  }
  else{
    for(int i = 0; i < sizeof(keyArray); i++){
      correctCombo = true;
      int input = 0;
      EEPROM.get(i, input);
      if(input != keyArray[i]){
        correctCombo = false;
      } else{
        sixEntered = true;
      }
    }
  }  
}


// consider calling this once with an index and a value instead of the key
void buildCombo(uint8_t key, int position) {
  switch (position % 3) {
    case(0):
      if(!key1){
        builderArray[7] = sevenSegments[key];
        keyArray[5] = key;
        key1 = true;
        key2 = false;
      } else if(!key2){
        builderArray[6] = sevenSegments[key];
        keyArray[4] = key;
        key2 = true;
        key1 = false;
      }
      break;
    case(1): 
      if(!key3){
        builderArray[4] = sevenSegments[key];
        keyArray[3] = key;
        key3 = true;
        key4 = false;
      } else if(!key4){
        builderArray[3] = sevenSegments[key];
        keyArray[2] = key;
        key4 = true;
        key3 = false;
      }
      break;
    case(2):
      if(!key5){
        builderArray[1] = sevenSegments[key];
        keyArray[1] = key;
        key5 = true;
        key6 = false;
      } else if(!key6){
        builderArray[0] = sevenSegments[key];
        keyArray[0] = key;
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
    //Repeat for each position 
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

bool leftButtonIsPressed(){
  if(!digitalRead(8)){
    while(!digitalRead(8));
    delay(20);
    return true;
  } else{
    return false;
  }
}
bool rightButtonIsPressed(){
  if(!digitalRead(9)){
    while(!digitalRead(9));
    delay(20);
    return true;
  } else{
    return false;
  }
}
