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
unsigned int level = 2; //current level
unsigned int inGame = 0; //0 = not in game, 1 = in game
unsigned int cursorX = 14; //X position of cursor
unsigned int cursorY = 30; //Y position of cursor
unsigned char movement = 0x00; //new cursor position
unsigned char tower = 0x00; //selected turret
unsigned int t = 0; //position of last active tower. player can have max of 10 towers

void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7));
}

typedef struct towerLED{
  unsigned int xPos, yPos, type, active; //blue: type = 1, cyan: type = 2, green: type = 3
} towerLED;

static towerLED towerLED1, towerLED2, towerLED3, towerLED4, towerLED5, towerLED6, towerLED7, towerLED8, towerLED9, towerLED10;
towerLED *towerLEDS[] = { &towerLED1, &towerLED2, &towerLED3, &towerLED4, &towerLED5, &towerLED6, &towerLED7, &towerLED8, &towerLED9, &towerLED10 };
const unsigned short numTowers = sizeof(towerLEDS)/sizeof(towerLED*); 

void moveCursor();
void levels();
void drawAllActiveTurrets();

enum matrixDisplaySM{matrix_init, notInGameState, moveCursor_Press, moveCursor_Release, inGameState /*more states*/} state;

void matrixDisplaySMTick(){
  switch(state){
    case matrix_init: state = notInGameState; break;
    case notInGameState:
      //TODO: Make transitions to states "inGame" and "!inGame" specific.
      if((incomingByte >> 4 != 0)){ //Placing tower
        //Must do on transition because incomingByte is rewritten too quickly.
        towerLEDS[t]->xPos = cursorX;
        towerLEDS[t]->yPos = cursorY;
        if(incomingByte >> 4 == 1){ //Blue tower
          towerLEDS[t]->type = 1;  
        } else if(incomingByte >> 4 == 2){ //Cyan tower
          towerLEDS[t]->type = 2;
        } else if(incomingByte >> 4 == 3){ //Green tower
          towerLEDS[t]->type = 3;
        }
        towerLEDS[t]->active = 1;
        
        //TODO: Check if current X,Y position is taken by any of the other towers
        //      so that the player cannot place multiple towers in one spot
        //t++; //TODO: FIX! This statement causes the LED matrix to not work.
                     //When fixed, uncomment code in drawAllActiveTowers().
        state = notInGameState;
      } else if((incomingByte << 4 != 0)){ // Moving cursor
        state = moveCursor_Press;
      } else if(!inGame){
        state = notInGameState;
      } else {
        state = notInGameState;
      }
      break;
    case moveCursor_Press:
      if(incomingByte << 4 != 0){
        movement = incomingByte;
        state = moveCursor_Press;
      } else if(incomingByte << 4 == 0){
        state = moveCursor_Release;
      }
      break;
    case moveCursor_Release: 
      state = notInGameState; 
      break;
    case inGameState:
      break;
  }
  switch(state){
    case matrix_init:
      break;
    case notInGameState: 
      break;
    case moveCursor_Press:
      break;
    case moveCursor_Release:   
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
      break;
    case inGameState:
      break;
  }
}

void loop() {
  if(Serial.available() > 0){
    incomingByte = Serial.read();
  }
  matrix.fillScreen(0);
  matrixDisplaySMTick();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7)); // draw current circle
  //drawAllActiveTowers(); //have to draw all turrets since there is "matrix.fillScreen(0);" above
  if(towerLEDS[t]->active != 0){ //Temporary conditional. To be replaced by drawAllActiveTowers()
    if(towerLEDS[t]->type == 1){
      matrix.drawPixel(towerLEDS[t]->yPos, towerLEDS[t]->xPos, matrix.Color333(0, 0, 7));
    } else if(towerLEDS[t]->type == 2){
      matrix.drawPixel(towerLEDS[t]->yPos, towerLEDS[t]->xPos, matrix.Color333(0, 7, 7));
    } else if(towerLEDS[t]->type == 3){
      matrix.drawPixel(towerLEDS[t]->yPos, towerLEDS[t]->xPos, matrix.Color333(0, 7, 0)); 
    }
  }
  levels(); //display current level
  //TODO: Implement enemy functionality
  //TODO: Implement turret functionality

  //Update Display
  matrix.swapBuffers(false);
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

void drawAllActiveTowers(){
  //draw all towers from towerLEDS[]
  /*for(int i = 0; i < numTowers; i++){
    if(towerLEDS[i]->active){
      if(towerLEDS[i]->type == 1){
        matrix.drawPixel(towerLEDS[i]->xPos, towerLEDS[i]->yPos, matrix.Color333(0, 0, 7));
      } else if(towerLEDS[i]->type == 2){
        matrix.drawPixel(towerLEDS[i]->xPos, towerLEDS[i]->yPos, matrix.Color333(0, 7, 7));
      } else if(towerLEDS[i]->type == 3){
        matrix.drawPixel(towerLEDS[i]->xPos, towerLEDS[i]->yPos, matrix.Color333(0, 7, 0));
      }
    }
  }*/
}

