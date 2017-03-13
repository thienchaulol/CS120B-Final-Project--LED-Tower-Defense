#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

#define CLK 8  // MUST be on PORTB!
#define LAT A3
#define OE  9
#define A   A0
#define B   A1
#define C   A2
RGBmatrixPanel matrix(A, B, C, CLK, LAT, OE, true);

//------------------------Variables
int incomingByte = 0; //USART byte
unsigned int level = 1; //current level //TODO: make level work dynamically
unsigned int inGame = 0; //0 = not in game, 1 = in game
unsigned int cursorX = 14; //X position of cursor
unsigned int cursorY = 30; //Y position of cursor
unsigned char movement = 0x00; //new cursor position
unsigned char tower = 0x00; //selected turret
unsigned int t = 0; //position of last active tower. player can have max of 10 towers

//------------------------Setup()
void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7));
}

//------------------------Towers
typedef struct towerLED{
  unsigned int xPos, yPos, effectRadius, type, active; //blue: type = 1, cyan: type = 2, green: type = 3
} towerLED;
static towerLED towerLED1, towerLED2, towerLED3, towerLED4, towerLED5, towerLED6, towerLED7/*, towerLED8, towerLED9, towerLED10*/;
towerLED *towerLEDS[] = { &towerLED1, &towerLED2, &towerLED3, &towerLED4, &towerLED5, &towerLED6, &towerLED7/*, &towerLED8, &towerLED9, &towerLED10*/ };
    //BUG: Storing more than 7 towerLED variables into towerLEDS[] and attempting to iterate through towerLEDS[] causes the LED matrix to bug out.
const unsigned short numTowers = sizeof(towerLEDS)/sizeof(towerLED*);

//------------------------Enemies
typedef struct enemyLED{
  //Need an array of xPos and yPos that corresponds to current level.
  unsigned int type, active; //pink: type = 1, yellow: type = 2, red?magenta?: type = 3
  int xPos1[], yPos1[]; //Path for enemy to follow for level 1 //TODO: Write paths for enemies for specific levels.
} enemyLED;
static enemyLED enemyLED1, enemyLED2, enemyLED3, enemyLED4, enemyLED5, enemyLED6, enemyLED7/*, enemyLED8, enemyLED9, enemyLED10, enemyLED11, enemyLED12, enemyLED13, enemyLED14, enemyLED15*/;  
enemyLED *enemyLEDS[] = { &enemyLED1, &enemyLED2, &enemyLED3, &enemyLED4, &enemyLED5, &enemyLED6, &enemyLED7/*, &enemyLED8, &enemyLED9, &enemyLED10, &enemyLED11, &enemyLED12, &enemyLED13, &enemyLED14, &enemyLED15*/ };  
const unsigned short numEnemies = sizeof(enemyLEDS)/sizeof(enemyLED*);

//------------------------Function Declarations
void moveCursor();
void levels();
void drawAllActiveTowers();
void drawAllActiveEnemies();
void matrixDisplaySMTick();

//------------------------State Machine
enum matrixDisplaySM{matrix_init, notInGameState, moveCursor_Press, moveCursor_Release, inGameState /*more states*/} state;

//------------------------Loop()
void loop() {
  if(Serial.available() > 0){
    incomingByte = Serial.read();
    if(incomingByte & 0x80 == 0){ inGame = 0; } 
    else if(incomingByte & 0x80 == 1){ inGame = 1; }
  }
  matrix.fillScreen(0);
  matrixDisplaySMTick();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7)); // draw cursor position
  drawAllActiveTowers(); //draws all purchased towers
  levels(); //display current level
  matrix.swapBuffers(false); //Update Display
}

//------------------------Functions
void matrixDisplaySMTick(){
  switch(state){
    case matrix_init: state = notInGameState; break;
    case notInGameState:
      if(!inGame){
        if((incomingByte >> 4 != 0)){ //Placing tower
          towerLEDS[t]->xPos = cursorX;
          towerLEDS[t]->yPos = cursorY;
          //TODO: Check if current X,Y position is taken by any of the other towers
          //      so that the player cannot place multiple towers in one spot.
          //      Make function: checkCurrentTowerLED(), iterates through towerLEDS[] and returns 0 if
          //      there is no tower at current x and y.
          towerLEDS[t]->effectRadius = 1;
          if(incomingByte >> 4 == 1){ //Blue tower
            Serial.write(20); //Send gold cost to ATMega1284 (20 -> 0001 0100 -> 0x14)
            towerLEDS[t]->type = 1;
          } else if(incomingByte >> 4 == 2){ //Cyan tower           
            Serial.write(40); //Send gold cost to ATMega1284 (40 -> 0010 1000 -> 0x28)
            towerLEDS[t]->type = 2;
          } else if(incomingByte >> 4 == 3){ //Green tower            
            Serial.write(60); //Send gold cost to ATMega1284 (60 -> 0011 1100 -> 0x3C)
            towerLEDS[t]->type = 3;
          }
          towerLEDS[t]->active = 1;
          t++;
          state = notInGameState;
        } else if((incomingByte << 4 != 0)){ // Moving cursor
          state = moveCursor_Press;
        }
      }
      else if(inGame){ state = inGameState; }
      else { state = notInGameState; }
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
    case moveCursor_Release: state = notInGameState; break;
    case inGameState: //TODO: Spawn enemies
      if(inGame){ state = inGameState; } 
      else if(!inGame){ state = notInGameState; }
      break;
  }
  switch(state){
    case matrix_init: break;
    case notInGameState: break;
    case moveCursor_Press: break;
    case moveCursor_Release:   
      if((movement & 0x01) && cursorX > 0){ cursorX = cursorX - 1; } //move circle up
      else if((movement & 0x02) && cursorX < 15){ cursorX = cursorX + 1; } //move circle down
      else if((movement & 0x04) && cursorY > 0){cursorY = cursorY - 1; } //move circle left
      else if((movement & 0x08) && cursorY < 31){ cursorY = cursorY + 1; } //move circle right
      else{} //don't move circle
      break;
    case inGameState: break; //TODO: Spawn enemies 
  }
}

void levels(){
  if(level == 1){
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
        if(towerLEDS[i]->effectRadius < 3){
          matrix.drawCircle(towerLEDS[i]->yPos, towerLEDS[i]->xPos, towerLEDS[i]->effectRadius++, matrix.Color333(0, 0, 7));
          delay(20); //This delay effecst the cursor. Cursor is less responsive the higher the delay.
                     //Multiple turrets also decrease cursor responsiveness.
                     //NOTE: This is a game feature. The more power you draw(more turrets purchased), the less responsive your cursor becomes.
        } else if(towerLEDS[i]->effectRadius >= 3){
          towerLEDS[i]->effectRadius = 1;
        }
      } else if(towerLEDS[i]->type == 2){
        matrix.drawPixel(towerLEDS[i]->xPos, towerLEDS[i]->yPos, matrix.Color333(0, 7, 7));
        //tower visual effect 
        if(towerLEDS[i]->effectRadius < 3){
          matrix.drawCircle(towerLEDS[i]->yPos, towerLEDS[i]->xPos, towerLEDS[i]->effectRadius++, matrix.Color333(0, 7, 7));
          delay(20);
        } else if(towerLEDS[i]->effectRadius >= 3){
          towerLEDS[i]->effectRadius = 1;
        }
      } else if(towerLEDS[i]->type == 3){
        matrix.drawPixel(towerLEDS[i]->xPos, towerLEDS[i]->yPos, matrix.Color333(0, 7, 0));
        //tower visual effect
        if(towerLEDS[i]->effectRadius < 3){
          matrix.drawCircle(towerLEDS[i]->yPos, towerLEDS[i]->xPos, towerLEDS[i]->effectRadius++, matrix.Color333(0, 7, 0));
          delay(20);
        } else if(towerLEDS[i]->effectRadius >= 3){
          towerLEDS[i]->effectRadius = 1;
        }
      }
    }
  }
}

void drawAllActiveEnemies(){
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

