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
unsigned int level = 1; //Current level //TODO: make level work dynamically
unsigned int inGame = 0; //0 = not in game, 1 = in game
unsigned int cursorX = 14; //X position of cursor
unsigned int cursorY = 30; //Y position of cursor
unsigned char movement = 0x00; //New cursor position
unsigned char tower = 0x00; //Selected turret
unsigned char t = 0; //Used to index through towerLEDS[]
unsigned char e = 0; //Used to index through enemyLEDS[]

//------------------------Setup()
void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7));
}

//------------------------Enemies
typedef struct enemyLED{
  unsigned char active;
} enemyLED;
static enemyLED enemyLED1, enemyLED2, enemyLED3, enemyLED4, enemyLED5;
enemyLED *enemyLEDS[] = { &enemyLED1, &enemyLED2, &enemyLED3, &enemyLED4, &enemyLED5 };

//------------------------Towers
typedef struct towerLED{
  unsigned char xPos, yPos, effectRadius, type, active; //Blue: type = 1, Cyan: type = 2, Green: type = 3
} towerLED;
static towerLED towerLED1, towerLED2, towerLED3, towerLED4, towerLED5, towerLED6, towerLED7;
towerLED *towerLEDS[] = { &towerLED1, &towerLED2, &towerLED3, &towerLED4, &towerLED5, &towerLED6, &towerLED7 };
    //NOTE: Storing more than 7 towerLED variables into towerLEDS[] and attempting to iterate through towerLEDS[] causes the LED matrix to bug out.
const unsigned short numTowers = sizeof(towerLEDS)/sizeof(towerLED*);

//------------------------Function Declarations
void moveCursor(); //Moves cursor
void levels(); //Draws level
void drawAllActiveTowers(); //Draws purchased towers
void matrixDisplaySMTick(); //State machine that deals with Cursor and Towers 
void enemySMTick(); //Deals with drawing enemies
void drawEnemyOne();
void drawEnemyTwo();
void drawEnemyThree();
void drawEnemyFour();
void drawEnemyFive();

//------------------------State Machine
enum matrixDisplaySM{matrix_init, notInGameState, moveCursor_Press, moveCursor_Release, enemy_checkUSART, enemy_writeEnemies} state;

//------------------------Loop()
void loop() {
  if(Serial.available() > 0){
    incomingByte = Serial.read();
    if(incomingByte & 0x80 == 0){ inGame = 0; } 
    else if(incomingByte & 0x80 == 1){ inGame = 1; }
  }
  //TODO: Make 2 levels, have the level bit travel via bit 6 of the "inGame byte"
  matrix.fillScreen(0);
  
  matrixDisplaySMTick();
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7)); //Draw cursor position
  drawAllActiveTowers(); //Draws all purchased towers
  if(enemyLEDS[0]->active) { drawEnemyOne(); }
  if(enemyLEDS[1]->active) { drawEnemyTwo(); }
  if(enemyLEDS[2]->active) { drawEnemyThree(); }
  if(enemyLEDS[3]->active) { drawEnemyFour(); }
  if(enemyLEDS[4]->active) { drawEnemyFive(); }
  levels(); //Display current level
  
  matrix.swapBuffers(false); //Update Display
}

//------------------------Functions
void matrixDisplaySMTick(){
  switch(state){ //Transitions
    case matrix_init: state = notInGameState; break;
    case notInGameState:
      if(!inGame){
        if((incomingByte >> 4 != 0)){ //Placing tower
          towerLEDS[t]->xPos = cursorX;
          towerLEDS[t]->yPos = cursorY; //TODO: Check if current X,Y position is taken by any of the other towers
          if(incomingByte >> 4 == 1){ Serial.write(20); towerLEDS[t]->type = 1; } 
          else if(incomingByte >> 4 == 2){ Serial.write(40); towerLEDS[t]->type = 2; } 
          else if(incomingByte >> 4 == 3){ Serial.write(60); towerLEDS[t]->type = 3; }
          towerLEDS[t]->effectRadius = 1;
          towerLEDS[t]->active = 1;
          t++;
          state = notInGameState;
        } else if((incomingByte << 4 != 0)){ state = moveCursor_Press; } // Moving cursor
      }
      else if(inGame){ state = enemy_checkUSART; }
      else { state = notInGameState; }
      break;
    case moveCursor_Press:
      if(!inGame){
        if(incomingByte << 4 != 0){ movement = incomingByte; state = moveCursor_Press; } 
        else if(incomingByte << 4 == 0){ state = moveCursor_Release; }
      }
      break;
    case moveCursor_Release: state = notInGameState; break;
    case enemy_checkUSART:
      if(!inGame){ state = notInGameState; } //Forgot to put this in. 
      else if((incomingByte << 4) != 0 && inGame){ state = enemy_writeEnemies; }
      else if((incomingByte << 4) == 0  && inGame){ state = enemy_checkUSART; }
      break;
    case enemy_writeEnemies: state = enemy_checkUSART; break;
  }
  switch(state){ //Actions
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
    case enemy_checkUSART: break;
    case enemy_writeEnemies:
      //Each enemy "bool" is called 1.5 seconds after the other.
      if(e < 5){ //Changing an entire elemtn in an array is not allowed in C. Can change the aspects of the element;
        enemyLEDS[e]->active = 1;
        e++;
      }
      break;
  }
}

unsigned char ActiveEnemies; //Returns 0 if there are no enemies on map, returns 1 if there are enemies on map
//TODO: Depending on the level, the enemies will "follow" a different path.
unsigned char enemyOneX = 0; 
unsigned char enemyOneY = 0;

void drawEnemyOne(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  //Draw a moving LED
  matrix.drawPixel(0, 0, matrix.Color333(7,7,7));
}
void drawEnemyTwo(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  matrix.drawPixel(0, 1, matrix.Color333(7,7,7));
}
void drawEnemyThree(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  matrix.drawPixel(0, 2, matrix.Color333(7,7,7));
}
void drawEnemyFour(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  matrix.drawPixel(0, 3, matrix.Color333(7,7,7));
}
void drawEnemyFive(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  matrix.drawPixel(0, 4, matrix.Color333(7,7,7));
}

void drawAllActiveTowers(){ //Draw all active towers in towerLEDS[]
  for(int i = 0; i < numTowers; i++){
    if(towerLEDS[i]->active){
      if(towerLEDS[i]->type == 1){
        if(towerLEDS[i]->effectRadius < 3){ //Tower visual effect 
          matrix.drawCircle(towerLEDS[i]->yPos, towerLEDS[i]->xPos, towerLEDS[i]->effectRadius++, matrix.Color333(7, 7, 0));
          delay(30); //This delay effecst the cursor. Cursor is less responsive the higher the delay.
                     //Multiple turrets also decrease cursor responsiveness.
                     //NOTE: This is a game feature. The more power you draw(more turrets purchased), the less responsive your cursor becomes.
        } else if(towerLEDS[i]->effectRadius >= 3){
          towerLEDS[i]->effectRadius = 1;
        }
      } else if(towerLEDS[i]->type == 2){
        if(towerLEDS[i]->effectRadius < 3){
          matrix.drawCircle(towerLEDS[i]->yPos, towerLEDS[i]->xPos, towerLEDS[i]->effectRadius++, matrix.Color333(0, 7, 7));
          delay(30);
        } else if(towerLEDS[i]->effectRadius >= 3){
          towerLEDS[i]->effectRadius = 1;
        }
      } else if(towerLEDS[i]->type == 3){
        if(towerLEDS[i]->effectRadius < 3){
          matrix.drawCircle(towerLEDS[i]->yPos, towerLEDS[i]->xPos, towerLEDS[i]->effectRadius++, matrix.Color333(0, 7, 0));
          delay(30);
        } else if(towerLEDS[i]->effectRadius >= 3){
          towerLEDS[i]->effectRadius = 1;
        }
      }
    }
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

