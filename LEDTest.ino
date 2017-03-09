#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

#define CLK 8  // MUST be on PORTB!
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

int incomingByte = 0; //USART byte
int level = 2; //current level
int currentTurret = 0; //0 = blue, 1 = cyan, 2 = green
int inGame = 0; //0 = not in game, 1 = in game
int cursorX = 14;
int cursorY = 30;
unsigned char movement = 0x00;

void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7));
}

void moveCursor();
void levels();

enum moveCursorSM{moveCursor_init, moveCursor_wait, moveCursor_Press, moveCursor_Release} moveCursor_states;

void moveCursorTick(){
  switch(moveCursor_states){
    case moveCursor_init: 
      moveCursor_states = moveCursor_wait; 
      break;
    case moveCursor_wait:
      if(incomingByte << 4 != 0){
        moveCursor_states = moveCursor_Press;
      } else if(incomingByte << 4 == 0){
        moveCursor_states = moveCursor_wait;
      }
      break;
    case moveCursor_Press:
      if(incomingByte << 4 != 0){
        moveCursor_states = moveCursor_Press;
        movement = incomingByte;
      } else if(incomingByte << 4 == 0){
        moveCursor_states = moveCursor_Release;
      }
      break;
    case moveCursor_Release:
      moveCursor_states = moveCursor_wait; 
      break;
  }
  switch(moveCursor_states){
    case moveCursor_init:
      break;
    case moveCursor_wait:
      break;
    case moveCursor_Press:
      break;
    case moveCursor_Release: 
      moveCursor(); 
      break;
  }
}

void loop() {
  if(Serial.available() > 0){
    incomingByte = Serial.read();
  }
  matrix.fillScreen(0);
  moveCursorTick();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7)); // draw new circle
  levels();
  //Place Turret
  /*
  if(incomingByte & 0x40){
    matrix.drawPixel(cursorY, cursorX, matrix.Color333(0, 0, 7));
  }
  */
  //TODO: Implement UART functionality

  //TODO: Implement enemy functionality

  //TODO: Implement turret functionality

  //Update Display
  matrix.swapBuffers(false);
}

void moveCursor(){ //Moving Cursor
  if((movement & 0x01) && cursorX > 0){ //move circle up
    cursorX = cursorX - 1;
  } else if((movement & 0x02) && cursorX < 15){ //move circle down
    cursorX = cursorX + 1;
  } else if((movement & 0x04) && cursorY > 0){ //move circle left
    cursorY = cursorY - 1;
  } else if((movement & 0x08) && cursorY < 31){ //move circle right
    cursorY = cursorY + 1;
  } else{
    //don't move circle
  }
}

void levels(){
  if(level == 1){
    //Level 1
    matrix.drawLine(0, 8, 9, 8, matrix.Color333(7, 0, 0));
    matrix.drawLine(0, 6, 11, 6, matrix.Color333(7, 0, 0));
  
    matrix.drawLine(9, 8, 9, 15, matrix.Color333(7, 0, 0));
    matrix.drawLine(11, 6, 11, 13, matrix.Color333(7, 0, 0));
    
    matrix.drawLine(9, 15, 22, 15, matrix.Color333(7, 0, 0));
    matrix.drawLine(11, 13, 20, 13, matrix.Color333(7, 0, 0));
    
    matrix.drawLine(20, 13, 20, 5, matrix.Color333(7, 0, 0));
    matrix.drawLine(22, 15, 22, 7, matrix.Color333(7, 0, 0));
    
    matrix.drawLine(22, 7, 32, 7, matrix.Color333(7, 0, 0));
    matrix.drawLine(20, 5, 32, 5, matrix.Color333(7, 0, 0));
  } else if(level == 2){
    //Level 2
    matrix.drawLine(2, 15, 2, 5, matrix.Color333(7, 0, 0));
    matrix.drawLine(4, 15, 4, 7, matrix.Color333(7, 0, 0));
    
    matrix.drawLine(4, 7, 10, 7, matrix.Color333(7, 0, 0));
    matrix.drawLine(2, 5, 12, 5, matrix.Color333(7, 0, 0));
    
    matrix.drawLine(10, 7, 10, 11, matrix.Color333(7, 0, 0));
    matrix.drawLine(12, 5, 12, 9, matrix.Color333(7, 0, 0));
    
    matrix.drawLine(10, 11, 18, 11, matrix.Color333(7, 0, 0));
    matrix.drawLine(12, 9, 16, 9, matrix.Color333(7, 0, 0));
    
    matrix.drawLine(16, 9, 16, 2, matrix.Color333(7, 0, 0));
    matrix.drawLine(18, 11, 18, 4, matrix.Color333(7, 0, 0));
    
    matrix.drawLine(18, 4, 25, 4, matrix.Color333(7, 0, 0));
    matrix.drawLine(16, 2, 27, 2, matrix.Color333(7, 0, 0));
    
    matrix.drawLine(25, 15, 25, 4, matrix.Color333(7, 0, 0));
    matrix.drawLine(27, 15, 27, 2, matrix.Color333(7, 0, 0));
  } else if(level == 3){
    //Level 3
    matrix.drawLine(0, 9, 32, 9, matrix.Color333(7, 0, 0));
    matrix.drawLine(0, 7, 32, 7, matrix.Color333(7, 0, 0));
  }
}

