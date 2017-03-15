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
unsigned char numActiveTowers = 0;

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
enemyLED enemyLED1, enemyLED2, enemyLED3, enemyLED4, enemyLED5;
enemyLED *enemyLEDS[] = { &enemyLED1, &enemyLED2, &enemyLED3, &enemyLED4, &enemyLED5 };

//------------------------Towers
typedef struct towerLED{
  unsigned char xPos, yPos, effectRadius, type, active; //Blue: type = 1, Cyan: type = 2, Green: type = 3
} towerLED;
towerLED towerLED1, towerLED2, towerLED3, towerLED4, towerLED5, towerLED6, towerLED7;
towerLED *towerLEDS[] = { &towerLED1, &towerLED2, &towerLED3, &towerLED4, &towerLED5, &towerLED6, &towerLED7 };
    //NOTE: Storing more than 7 towerLED variables into towerLEDS[] and attempting to iterate through towerLEDS[] causes the LED matrix to bug out.
const unsigned short numTowers = sizeof(towerLEDS)/sizeof(towerLED*);

//------------------------Function Declarations
void moveCursor(); //Moves cursor
void levels(); //Draws level
void drawAllActiveTowers(); //Draws purchased towers
void checkCursor();
void checkTowers();
void drawEnemies();
void drawEnemyOne();
void drawEnemyTwo();
void drawEnemyThree();
void drawEnemyFour();
void drawEnemyFive();

//------------------------State Machine
enum matrixDisplaySM{matrix_init, notInGameState, moveCursor_Press, moveCursor_Release} state;

//------------------------Loop()
void loop() {
  if(Serial.available() > 0){
    incomingByte = Serial.read();
  }
  matrix.fillScreen(0);
  drawEnemies(); //Checks user input and draws enemies
  checkCursor(); //Checks user input and moves cursor
  checkTowers(); //Checks user input and draws towers
  levels(); //Display current level
  matrix.swapBuffers(false); //Update Display
  delay(60 - (numActiveTowers*20)); //TODO: Find more exact equation for cursor delay.
}

//------------------------Functions
void drawEnemies(){
  if(incomingByte == 0x81){ enemyLEDS[0]->active = 1; } //Activate enemy LED
  if(incomingByte == 0x82) { enemyLEDS[1]->active = 1; }
  if(incomingByte == 0x83) { enemyLEDS[2]->active = 1; }
  if(incomingByte == 0x84) { enemyLEDS[3]->active = 1; }
  if(incomingByte == 0x85) {enemyLEDS[4]->active = 1; }
  if(enemyLEDS[0]->active == 1) { drawEnemyOne(); } //Draw enemy LED
  if(enemyLEDS[1]->active == 1) { drawEnemyTwo(); }
  if(enemyLEDS[2]->active == 1) { drawEnemyThree(); }
  if(enemyLEDS[3]->active == 1) { drawEnemyFour(); }
  if(enemyLEDS[4]->active == 1) { drawEnemyFive(); }
}

void checkCursor(){
  if(incomingByte == 0x08 && cursorY < 31){ cursorY++;} //Move right 
  else if(incomingByte == 0x04 && cursorY > 0){ cursorY--;}  //Move left
  else if(incomingByte == 0x01 && cursorX > 0){ cursorX--;} //Move up
  else if(incomingByte == 0x02 && cursorX < 15){ cursorX++;} //Move down
  else if(incomingByte == 0x00){ } //Don't move
  matrix.drawCircle(cursorY, cursorX, 1, matrix.Color333(7, 0, 7)); //Draws cursor's position
}

void checkTowers(){
 if(incomingByte == 0x10){ //Yellow tower
    towerLEDS[t]->xPos = cursorX; //Draw tower at cursor position
    towerLEDS[t]->yPos = cursorY;
    Serial.write(20); //Write cost to ATmega1284
    Serial.flush(); 
    towerLEDS[t]->type = 1; //Save tower type
    towerLEDS[t]->effectRadius = 1; //Set tower visual effect radius
    towerLEDS[t]->active = 1; //Activate tower
    t++; //Increment t
 } else if(incomingByte == 0x20){ //Cyan tower
    towerLEDS[t]->xPos = cursorX;
    towerLEDS[t]->yPos = cursorY;
    Serial.write(40); 
    Serial.flush(); 
    towerLEDS[t]->type = 2; 
    towerLEDS[t]->effectRadius = 1;
    towerLEDS[t]->active = 1;
    t++;
 } else if(incomingByte == 0x30){ //Green tower
    towerLEDS[t]->xPos = cursorX;
    towerLEDS[t]->yPos = cursorY;
    Serial.write(60); 
    Serial.flush(); 
    towerLEDS[t]->type = 3; 
    towerLEDS[t]->effectRadius = 1;
    towerLEDS[t]->active = 1;
    t++;
 }
 drawAllActiveTowers(); //Draw all purchased towers
}

void drawEnemyOne(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  matrix.fillCircle(8, 2, 1, matrix.Color333(7, 0, 0)); 
}
void drawEnemyTwo(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  matrix.fillCircle(12, 2, 1, matrix.Color333(0, 7, 0));
}
void drawEnemyThree(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  matrix.fillCircle(16, 2, 1, matrix.Color333(0, 0, 7));
}
void drawEnemyFour(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  matrix.fillCircle(20, 2, 1, matrix.Color333(7, 0, 0));
}
void drawEnemyFive(){
  //- Each enemy LED will have a function that will draw the pixel on it's path for each level
  matrix.fillCircle(24, 2, 1, matrix.Color333(0, 7, 0));
}

void drawAllActiveTowers(){ //Draw all active towers in towerLEDS[]
  for(int i = 0; i < numTowers; i++){
    if(towerLEDS[i]->active){
      if(towerLEDS[i]->type == 1){
        if(towerLEDS[i]->effectRadius < 3){ //Tower visual effect 
          matrix.drawCircle(towerLEDS[i]->yPos, towerLEDS[i]->xPos, towerLEDS[i]->effectRadius++, matrix.Color333(7, 7, 0));
          delay(30);
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
      if(numActiveTowers < 3) numActiveTowers++;
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

