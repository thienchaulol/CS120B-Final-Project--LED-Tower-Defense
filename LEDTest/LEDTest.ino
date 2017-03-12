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
unsigned int level = 1; //current level //TODO: make level work dynamically
unsigned int inGame = 0; //0 = not in game, 1 = in game
unsigned int cursorX = 14; //X position of cursor
unsigned int cursorY = 30; //Y position of cursor
unsigned char movement = 0x00; //new cursor position
unsigned char tower = 0x00; //selected turret
unsigned int t = 0; //position of last active tower. player can have max of 10 towers
unsigned int x = 9;

void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7));
}

typedef struct towerLED{
  unsigned int xPos, yPos, effectRadius, type, active; //blue: type = 1, cyan: type = 2, green: type = 3
} towerLED;

static towerLED towerLED1, towerLED2, towerLED3, towerLED4, towerLED5, towerLED6, towerLED7, towerLED8, towerLED9, towerLED10;
towerLED *towerLEDS[] = { &towerLED1, &towerLED2, &towerLED3, &towerLED4, &towerLED5, &towerLED6, &towerLED7, &towerLED8, &towerLED9, &towerLED10 };
const unsigned short numTowers = sizeof(towerLEDS)/sizeof(towerLED*);

typedef struct enemyLED{
  //Need an array of xPos and yPos that corresponds to current level.
  unsigned int type, active; //pink: type = 1, yellow: type = 2, red?magenta?: type = 3
  //TODO: Write paths for enemies for specific levels.
  int xPos1[], yPos1[]; //Path for enemy to follow for level 1
  int xPos2[], yPos2[]; //Path for enemy to follow for level 2
  int xPos3[], yPos3[]; //Path for enemy to follow for level 3
} enemyLED;

static enemyLED enemyLED1, enemyLED2, enemyLED3, enemyLED4, enemyLED5, enemyLED6, enemyLED7, enemyLED8, enemyLED9, enemyLED10, enemyLED11, enemyLED12, enemyLED13, enemyLED14, enemyLED15;  
enemyLED *enemyLEDS[] = { &enemyLED1, &enemyLED2, &enemyLED3, &enemyLED4, &enemyLED5, &enemyLED6, &enemyLED7, &enemyLED8, &enemyLED9, &enemyLED10, &enemyLED11, &enemyLED12, &enemyLED13, &enemyLED14, &enemyLED15 };  
const unsigned short numEnemies = sizeof(enemyLEDS)/sizeof(enemyLED*);

void moveCursor();
void levels();
void drawAllActiveTurrets();
void drawAllActiveEnemies();

enum matrixDisplaySM{matrix_init, notInGameState, moveCursor_Press, moveCursor_Release, inGameState /*more states*/} state;

void matrixDisplaySMTick(){
  switch(state){
    case matrix_init: state = notInGameState; break;
    case notInGameState:
      if(!inGame){
        //TODO: Make any state deal with current level display.
        //TODO: Make transitions to states "inGame" and "!inGame" specific.
        if((incomingByte >> 4 != 0)){ //Placing tower
          //Must do on transition(Mealy action) because "incomingByte" is rewritten too quickly.
          towerLEDS[t]->xPos = cursorX;
          towerLEDS[t]->yPos = cursorY;
          towerLEDS[t]->effectRadius = 1;
          if(incomingByte >> 4 == 1){ //Blue tower
            //Send gold cost to ATMega1284 (20 -> 0001 0100 -> 0x14)
            Serial.write(20);
            towerLEDS[t]->type = 1;  
          } else if(incomingByte >> 4 == 2){ //Cyan tower
            //Send gold cost to ATMega1284 (40 -> 0010 1000 -> 0x28)
            Serial.write(40);
            towerLEDS[t]->type = 2;
          } else if(incomingByte >> 4 == 3){ //Green tower
            //Send gold cost to ATMega1284 (60 -> 0011 1100 -> 0x3C)
            Serial.write(60);
            towerLEDS[t]->type = 3;
          }
          towerLEDS[t]->active = 1;
          //TODO: Check if current X,Y position is taken by any of the other towers
          //      so that the player cannot place multiple towers in one spot
          //t++; //TODO: FIX! This statement causes the LED matrix to not work.
                       //When fixed, uncomment code in drawAllActiveTowers().
                       //3-11-17 2:50 PM. i++ works in drawAllActiveTowers() and i is used to access towerLEDS[]!!
          x = x + 1;
          //t++;
          state = notInGameState;
        } else if((incomingByte << 4 != 0)){ // Moving cursor
          state = moveCursor_Press;
        } else if(!inGame){
          state = notInGameState;
        } else {
          state = notInGameState;
        }
      }
      break;
    case moveCursor_Press:
      if(!inGame){
        if(incomingByte << 4 != 0){
          movement = incomingByte;
          state = moveCursor_Press;
        } else if(incomingByte << 4 == 0){
          state = moveCursor_Release;
        }
      }
      break;
    case moveCursor_Release: 
      state = notInGameState; 
      break;
    case inGameState:
      //TODO: Spawn enemies
      if(inGame){
        state = inGameState;
      } else if(!inGame){
        state = notInGameState;
      }
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
      //TODO: Spawn enemies
        //Make array of enemies and draw all active enemies and their current positions
        //using "drawAllActiveEnemies()"
      //This state's action is empty for now.
      //Maybe use this state to change the positions of the enemyLEDs?
      //For now, will have enemyLEDs lit up by "drawAllActiveEnemies()"
      break;
  }
}

void loop() {
  if(Serial.available() > 0){
    incomingByte = Serial.read();
    if(incomingByte & 0x80 == 0){
      inGame = 0;
    } else if(incomingByte & 0x80 == 1){
      inGame = 1;
    }
  }
  matrix.fillScreen(0);
  matrixDisplaySMTick();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7)); // draw current circle
  //drawAllActiveTowers(); //have to draw all turrets since there is "matrix.fillScreen(0);" above
                           //draws all active towers and their corresponding effects
  drawAllActiveEnemies();
  if(towerLEDS[t]->active != 0){ //Temporary conditional. To be replaced by drawAllActiveTowers()
    if(towerLEDS[t]->type == 1){
      matrix.drawPixel(towerLEDS[t]->yPos, towerLEDS[t]->xPos, matrix.Color333(0, 0, 7));
      if(towerLEDS[t]->effectRadius < 3){
        matrix.drawCircle(towerLEDS[t]->yPos, towerLEDS[t]->xPos, towerLEDS[t]->effectRadius++, matrix.Color333(0, 0, 7));
      } else if(towerLEDS[t]->effectRadius >= 3){
        towerLEDS[t]->effectRadius = 1;
      }
    } else if(towerLEDS[t]->type == 2){
      matrix.drawPixel(towerLEDS[t]->yPos, towerLEDS[t]->xPos, matrix.Color333(0, 7, 7));
      if(towerLEDS[t]->effectRadius < 3){
        matrix.drawCircle(towerLEDS[t]->yPos, towerLEDS[t]->xPos, towerLEDS[t]->effectRadius++, matrix.Color333(0, 7, 7));
      } else if(towerLEDS[t]->effectRadius >= 3){
        towerLEDS[t]->effectRadius = 1;
      }
    } else if(towerLEDS[t]->type == 3){
      matrix.drawPixel(towerLEDS[t]->yPos, towerLEDS[t]->xPos, matrix.Color333(0, 7, 0)); 
      if(towerLEDS[t]->effectRadius < 3){
        matrix.drawCircle(towerLEDS[t]->yPos, towerLEDS[t]->xPos, towerLEDS[t]->effectRadius++, matrix.Color333(0, 7, 0));
      } else if(towerLEDS[t]->effectRadius >= 3){
        towerLEDS[t]->effectRadius = 1;
      }
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
  for(int i = 0; i < numTowers; i++){
    if(towerLEDS[i]->active){
      if(towerLEDS[i]->type == 1){
        matrix.drawPixel(towerLEDS[i]->xPos, towerLEDS[i]->yPos, matrix.Color333(0, 0, 7));
        //tower visual effect
      } else if(towerLEDS[i]->type == 2){
        matrix.drawPixel(towerLEDS[i]->xPos, towerLEDS[i]->yPos, matrix.Color333(0, 7, 7));
        //tower visual effect
      } else if(towerLEDS[i]->type == 3){
        matrix.drawPixel(towerLEDS[i]->xPos, towerLEDS[i]->yPos, matrix.Color333(0, 7, 0));
        //tower visual effect
      }
    }
  }
}

void drawAllActiveEnemies(){
  //draw all active enemies from enemyLEDS[]
  if((incomingByte << 4 == 5) && inGame){ //Level 1 Enemies
    //Spawn pink enemy from enemyLEDS[]
    matrix.drawPixel(0, 0, matrix.Color333(5, 0, 5));
  } else if((incomingByte << 4 == 10) && inGame){ //Level 2 Enemies
    //Spawn yellow enemy
    matrix.drawPixel(0, 0, matrix.Color333(0, 5, 5));
  } else if((incomingByte << 4 == 15) && inGame){ //Level 3 Enemies
    //Spawn red?magenta? enemy
    matrix.drawPixel(0, 0, matrix.Color333(5, 0, 0));
  }
}

