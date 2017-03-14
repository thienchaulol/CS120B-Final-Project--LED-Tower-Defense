/*
 * LED Tower Defense.c
 *
 * Created: 3/1/2017 10:26:01 AM
 * Author : Thien Chau
 */

#include <avr/io.h>
#include "io.c" //contains LCD functionality
#include "dataStructs.c" //contains player, enemy, tower structs
#include "timer.h" //contains timer functionality
#include "scheduler.h" //contains scheduler functionality and task struct
#include "usart_ATmega1284.h" //found online

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//------------------------------------Shared Variables
const unsigned char* playerInfo = "Gold:20  Stage:1Health:10"; //LCD Display variable
unsigned char outgoingByte = 0x00; //USART0 outgoing byte
unsigned char outgoingByte1 = 0x00; //USART1 outgoing byte
unsigned char receivedByte = 0x00; //USART0 received byte
int gold = 240; //Player's starting gold
int health = 10; //Player's starting health
unsigned char A2; //Used to test if LCD Display works.
int level = 0x01; //Player's current level
unsigned short x; //variable to record ADC value from 2-axis joystick
unsigned char inGame; //true/false value that checks if user is "in game". TODO: UART 
					  //when user is "in game", user cannot input any values; user will watch until level finish or loss
					  //highest bit(bit 7) of outgoing byte to Arduino Uno
unsigned int currentTurret; //Current turret selected to be placed. 2 = best, 1 = second best, 0 = third best
unsigned char C0; //Start/Pause button
unsigned char C1; //Place turret button
unsigned char C2; //Select "blue" turret
unsigned char C3; //Select "purple" turret
unsigned char C4; //Select "green" turret
unsigned char C5; //Reset button
int x2, y2; //Coordinates for 2-axis joystick
char a[20], b[20]; //Used to test if 2-axis joystick works.

	
//Array of towers
//Allocate array of towers at beginning. Activate towers and edit variables when needed.
int t = 0; //tracks used turrets
static tower tower1, tower2, tower3, tower4, tower5, tower6, tower7, tower8, tower9, tower10, tower11, tower12, tower13, tower14, tower15;
tower *towers[] = { &tower1, &tower2, &tower3, &tower4, &tower5, &tower6, &tower7, &tower8, &tower9, &tower10, &tower11, &tower12, &tower13, &tower14, &tower15};
	
//Array of enemies on current level
int e = 0;
static enemy enemy1;
enemy *enemies[] = {};

//------------------------------------End Shared Variables

//------------------------------------Functions

//Takes gold, level, and health variables and returns
//string to display with inputs
char* updatePlayerInfo(int newGold, int newLevel, int newHealth){
	char updatedInfo[33] = "Gold:";
	char temp[20];
	char buffer[20];
	
	//Gold
	//updatedInfo = temp; //Can't assign to char*
	itoa(newGold, buffer, 10); //convert int(newGold) to string(buffer)
	strcat(updatedInfo, buffer); //concatenate string(buffer) onto updatedInfo
								 //Can concatenate onto char*?..

	//Stage
	if(newGold < 10){ //gold will be 1 digit
		char temp2[20] = "   Stage:"; 
		strcpy(temp, temp2);
	} else if(newGold >= 10 && newGold < 100){ //gold will be 2 digits
		char temp2[20] = "  Stage:";
		strcpy(temp, temp2);
	} else if(newGold >= 100){ //gold will be 3 digits
		char temp2[20] = " Stage:";
		strcpy(temp, temp2);
	}
	strcat(updatedInfo, temp);
	itoa(newLevel, buffer, 10);
	strcat(updatedInfo, buffer);

	//Health
	char temp3[] = "Health:";
	strcat(updatedInfo, temp3);
	itoa(newHealth, buffer, 10);
	strcat(updatedInfo, buffer);

	return updatedInfo;
}

void InitADC(void)
{
	ADMUX|=(1<<REFS0);
	ADCSRA|=(1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); //ENABLE ADC, PRESCALER 128
}

int readadc(int ch)
{
	ch&=0b00000111;         //ANDing to limit input to 7
	ADMUX = (ADMUX & 0xf8)|ch;  //Clear last 3 bits of ADMUX, OR with ch
	ADCSRA|=(1<<ADSC);        //START CONVERSION
	while((ADCSRA)&(1<<ADSC));    //WAIT UNTIL CONVERSION IS COMPLETE
	return(ADC);        //RETURN ADC VALUE
}
//------------------------------------End functions

//------------------------------------FSMs
//Maybe have only one type of turret due to time constraints.
enum selectTurret_States{selTur_init, selTur_wait, selTur_bluePress, selTur_blueRelease, selTur_purpPress, selTur_purpRelease, selTur_greenPress, selTur_greenRelease};

//"selTur" -> select turret

int selTurTick(int state){
	switch(state){
		case selTur_init: state = selTur_wait; break;
		case selTur_wait:
			if(C2 && !inGame){ state = selTur_bluePress; }
			else if(C3 && !inGame){ state = selTur_purpPress; }
			else if(C4 && !inGame){ state = selTur_greenPress; }
			else {state = selTur_wait; }
			break;
		case selTur_bluePress:
			if(C2 && !inGame){ state = selTur_bluePress; }
			else if(!C2 && !inGame){ state = selTur_blueRelease; }
			break;
		case selTur_blueRelease: state = selTur_wait; break;
		case selTur_purpPress:
			if(C3 && !inGame){ state = selTur_purpPress; }
			else if(!C3 && !inGame){ state = selTur_purpRelease; }
			break;
		case selTur_purpRelease: state = selTur_wait; break;
		case selTur_greenPress:
			if(C4 && !inGame){ state = selTur_greenPress; }
			else if(!C4 && !inGame) { state = selTur_greenRelease; }
			break;
		case selTur_greenRelease: state = selTur_wait; break;
	}
	switch(state){
		case selTur_init: break;
		case selTur_wait: break;
		case selTur_bluePress: break;
		case selTur_blueRelease:
			//LCD_DisplayString(25, "C2");
			towers[t]->cost = 20;
			if(gold >= towers[t]->cost){
				towers[t]->purchased = 1;
				outgoingByte = outgoingByte | 0x10;
				gold -= towers[t]->cost;
				t++;
			}
			break;
		case selTur_purpPress: break;
		case selTur_purpRelease:
			//LCD_DisplayString(25, "C3");
			towers[t]->cost = 40;
			if(gold >= towers[t]->cost){
				towers[t]->purchased = 1;
				outgoingByte = outgoingByte | 0x20;
				gold -= towers[t]->cost;
				t++;
			}
			break;
		case selTur_greenPress: break;
		case selTur_greenRelease: 
			//add and place green
			//LCD_DisplayString(25, "C4");
			towers[t]->cost = 60;
			if(gold >= towers[t]->cost){
				towers[t]->purchased = 1;
				outgoingByte = outgoingByte | 0x30;
				gold -= towers[t]->cost;
				t++;
			}
			break;
	}
	return state;
}

enum ADC_States{ADC_initialize, ADC_display};

int ADCTick(int state){
	switch(state){
		case ADC_initialize: state = ADC_display; break;
		case ADC_display: state = ADC_display; break;
	}
	switch(state){
		case ADC_initialize: break;
		case ADC_display:
			//LCD_Cursor(1);
			x2 = readadc(0);
			y2 = readadc(1);
			x2 = x2 - 512;
			y2 = y2 - 512;
			//LCD_DisplayString(1, itoa(x2, a, 10)); //Must disabled ClearScreen() in LCD_DisplayString() in io.c to see coordinates.
			//LCD_DisplayString(17, itoa(y2, b, 10));
			if(!inGame){ //NOTE: Cannot move cursor while in game
				if(y2 > 150){ outgoingByte = outgoingByte | 0x08; } //Right
				else if(y2 < -150){ outgoingByte = outgoingByte | 0x04; } //Left 
				else if(x2 < -150){ outgoingByte = outgoingByte | 0x01; } //Up
				else if(x2 > 150){ outgoingByte = outgoingByte | 0x02; } //Down
				else{ outgoingByte = outgoingByte | 0x00; } //No Input
			}
			break;
	}
	return state;
}

enum LCD_States{LCD_initialize, LCD_info, LCD_win, LCD_loss};

int LCDTick(int state){
	switch(state){
		case LCD_initialize: state = LCD_info; break;
		case LCD_info:
			if(level == 3){ state = LCD_win; } 
			else if(health == 0){ state = LCD_loss; } 
			else { state = LCD_info; }
			break;
		case LCD_win: break;
		case LCD_loss: break;
	}
	switch(state){
		case LCD_initialize: break;
		case LCD_info:
			if(USART_HasReceived(0)){ //update info
				receivedByte = USART_Receive(0); //check USART0
				if(receivedByte << 2 == 20){ 
					gold -= 20;
					updatePlayerInfo(gold, level, health);
				} else if(receivedByte << 2 == 40){ 
					gold -= 40;
					updatePlayerInfo(gold, level, health);
				} else if(receivedByte << 2 == 60){ 
					gold -= 60;
					updatePlayerInfo(gold, level, health);
				} else {
					LCD_DisplayString(1, updatePlayerInfo(gold, level, health));
				}
			}
			break;
		case LCD_win: break;
		case LCD_loss: break;
	}
	return state;
}

enum usartSM_States{usartSM_init, usartSM_check0};

int usartSMTick(int state){
	switch(state){
		case usartSM_init: state = usartSM_check0; break;
		case usartSM_check0: state = usartSM_check0; break;
	}
	switch(state){
		case usartSM_init: break;
		case usartSM_check0:
			if(USART_IsSendReady(0)){ //if the USART is ready
				USART_Send(outgoingByte, 0); //send USART 0
				outgoingByte &= 0x80; //Reset bits 0-6 after being sent. Keep track of 7th bit to see if inGame
			}
			break;
	}
	return state;
}

unsigned char spawnedEnemies, enemyCount, timeCount;

//Deals with enemies and current level
enum enemySM{enemy_init, enemy_wait, enemy_C0Press, enemy_spawn, enemy_spawnWait, enemy_levelComplete};

int enemySMTick(int state){
	switch(state){ //Transitions
		case enemy_init:
			spawnedEnemies = 0; //Initialize values in "transitions"
			enemyCount = 5; //"timeCount" won't be set if initialized in "actions"
			timeCount = 0;
			state = enemy_wait; 
			break;
		case enemy_wait: //TODO: Win Message
			if(C0){
				inGame = 1;
				outgoingByte |= 0x81; // 1000 0001. (outgoingByte << 4 != 0)
				outgoingByte &= 0x81;
				state = enemy_C0Press; 
			} 
			else{ state = enemy_wait; }
			break;
		case enemy_C0Press:
			if(C0){ state = enemy_C0Press; }
			else if(!C0){ inGame = 1; state = enemy_spawn; }
			break;
		case enemy_spawn:
			if(spawnedEnemies < enemyCount){ state = enemy_spawnWait; }
			else if(spawnedEnemies >= enemyCount){ state = enemy_levelComplete; }
			break;
		case enemy_spawnWait:
			if(timeCount >= 15){ 
				outgoingByte |= 0x81; // 1000 0001. Send signal to spawn enemy
				outgoingByte &= 0x81;
				timeCount = 0; 
				state = enemy_spawn; 
			} 
			else if(timeCount < 15){
				outgoingByte |= 0x80; // 1000 0000. No signal
				outgoingByte &= 0x80;
				timeCount++;
				state = enemy_spawnWait; 
			}
			break;
		case enemy_levelComplete: state = enemy_init; break;
	}
	switch(state){ //Actions
		case enemy_init:
			break;
		case enemy_wait: break;
		case enemy_C0Press: break;
		case enemy_spawn: //Send info to USART about enemies. //TODO: Check if player's health reaches 0
			spawnedEnemies++; 
			break;
		case enemy_spawnWait: break;
		case enemy_levelComplete:
			outgoingByte &= 0x7F; // "inGame bit" to 0
			if(level == 1){ 
				gold += 25;
				level++;
				updatePlayerInfo(gold, level, health); 
			}
			else if(level == 2){
				gold += 50;
				level++;
				updatePlayerInfo(gold, level, health); 
			}
			else if(level == 3){ 
				gold += 75;
				level++;
				updatePlayerInfo(gold, level, health);
			}
			LCD_DisplayString(1, updatePlayerInfo(gold, level, health));
			inGame = 0;
			break;
	}
	return state;
}

//------------------------------------End FSMs

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; //Inputs. Using A0 and A1 as inputs for 2-axis joystick
	DDRB = 0xFF; PORTB = 0x00; //LCD Display, output to PORTB
	DDRC = 0x00; PORTC = 0xFF; //Inputs. Using C0-C5 as user input
	DDRD = 0x30; PORTD = 0xCF; //D4 and D5 are outputs(control bus). D0 - D3 are RXD0, TXD0, RXD1, and TXD1 (UART Functionality)
	
	// Period for the tasks
	unsigned long int LCDTick_calc = 500;
	unsigned long int ADCTick_calc = 200;
	unsigned long int selTurTick_calc = 200;
	unsigned long int usartSMTick_calc = 100;
	unsigned long int enemySMTick_calc = 100;
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(LCDTick_calc, ADCTick_calc);
	tmpGCD = findGCD(tmpGCD, selTurTick_calc);
	tmpGCD = findGCD(tmpGCD, usartSMTick_calc);
	tmpGCD = findGCD(tmpGCD, enemySMTick_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int LCDTick_period = LCDTick_calc/GCD;
	unsigned long int ADCTick_period = ADCTick_calc/GCD;
	unsigned long int selTurTick_period = selTurTick_calc/GCD;
	unsigned long int usartSMTick_period = usartSMTick_calc/GCD;
	unsigned long int enemySMTick_period = enemySMTick_calc/GCD;

	//Declare an array of tasks
	static task task1, task2, task5, task6, task7;
	task *tasks[] = { &task1, &task2, &task5, &task6, &task7};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	// Task 1
	task1.state = LCD_initialize;//Task initial state.
	task1.period = LCDTick_period;//Task Period.
	task1.elapsedTime = LCDTick_period;//Task current elapsed time.
	task1.TickFct = &LCDTick;//Function pointer for the tick.

	// Task 2
	task2.state = ADC_initialize;
	task2.period = ADCTick_period;
	task2.elapsedTime = ADCTick_period;
	task2.TickFct = &ADCTick;
	
	// Task 5
	task5.state = selTur_init;
	task5.period = selTurTick_period;
	task5.elapsedTime = selTurTick_period;
	task5.TickFct = &selTurTick;
	
	//Task 6
	task6.state = usartSM_init;
	task6.period = usartSMTick_period;
	task6.elapsedTime = usartSMTick_period;
	task6.TickFct = &usartSMTick;
	
	//Task 7
	task7.state = enemy_init;
	task7.period = enemySMTick_period;
	task7.elapsedTime = enemySMTick_period;
	task7.TickFct = &enemySMTick;
	
	TimerSet(GCD);
	TimerOn();
	LCD_init();
	LCD_DisplayString(1, updatePlayerInfo(gold, level, health));
	InitADC();
	initUSART(0);
	USART_Flush(0);
	
	unsigned short i; // Scheduler for-loop iterator
	while(1) {
		C0 = ~PINC & 0x01; //Start. TODO: Holding down C0 for 5 seconds resets the game
		C2 = ~PINC & 0x04; //Select "blue" turret //Best turret
		C3 = ~PINC & 0x08; //Select "purple" turret //Second best turret
		C4 = ~PINC & 0x10; //Select "green" turret //Third best turret
		C5 = ~PINC & 0x20; //Reset
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}

	// Error: Program should not exit!
	return 0;
}

