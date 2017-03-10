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


//Shared Variables
const unsigned char* playerInfo = "Gold:20  Stage:1Health:10"; //LCD Display variable
unsigned char outgoingByte = 0x00; //USART0 outgoing byte
unsigned char outgoingByte1 = 0x00; //USART1 outgoing byte
int gold = 20; //Player's starting gold
int health = 10; //Player's starting health
unsigned char A2; //Used to test if LCD Display works.
unsigned char level = 0x00; //Player's current level
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
unsigned char joystickMovement; //lower 4 bits of outgoing byte to Arduino Uno

	
//Array of towers
//CANNOT ADD TO ARRAY DURING RUNTIME.
//Allocate array of towers at beginning. Activate towers and edit variables when needed.
int t = 0; //tracks used turrets
static tower tower1, tower2, tower3, tower4, tower5, tower6, tower7, tower8, tower9, tower10, tower11, tower12, tower13, tower14, tower15;
tower *towers[] = { &tower1, &tower2, &tower3, &tower4, &tower5, &tower6, &tower7, &tower8, &tower9, &tower10, &tower11, &tower12, &tower13, &tower14, &tower15};
	
//Array of enemies on current level
int e = 0;
static enemy enemy1;
enemy *enemies[] = {};

//End Shared Variables

//Functions

//Takes gold, level, and health variables and returns
//string to display with inputs
char* updatePlayerInfo(int newGold, int newLevel, int newHealth){
	//Writing to char* is ILLEGAL
	//http://stackoverflow.com/questions/1704407/what-is-the-difference-between-char-s-and-char-s-in-c
	//Going to use char[] instead of char* even though LCD_DisplayString() takes in a char*
		//char[] is essentially pointer...anyways..
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
		char temp2[20] = "    Stage:"; 
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
//End functions

//FSMs

enum startAndPause_States{sNp_init, sNp_wait, sNp_startPress, sNp_startRelease, sNp_pausePress, sNp_pauseRelease};

int sNpTick(int state){
	switch(state){
		case sNp_init: state = sNp_wait; break;
		case sNp_wait:
			if(C0 && !inGame){ state = sNp_startPress; } 
			else if(C0 && inGame){ state = sNp_pausePress; } 
			else { state = sNp_wait; }
			break;
		case sNp_startPress:
			if(C0){ state = sNp_startPress; } 
			else if(!C0){ state = sNp_startRelease; }
			break;
		case sNp_startRelease: state = sNp_wait; break;
		case sNp_pausePress:
			if(C0){ state = sNp_pausePress; } 
			else if(!C0){ state = sNp_pauseRelease; }
			break;
		case sNp_pauseRelease: state = sNp_wait; break;
	}
	switch(state){
		case sNp_init: break;
		case sNp_wait: break;
		case sNp_startPress: break;
		case sNp_startRelease:
			//TODO: Start game
			break;
		case sNp_pausePress: break;
		case sNp_pauseRelease:
			//TODO: Pause game
			break;
	}
	return state;
}
/*
enum placeTurret_States{placeTurret_init, placeTurret_wait, placeTurret_Press, placeTurret_Release};

int placeTurretTick(int state){
	switch(state){
		case placeTurret_init: state = placeTurret_wait; break;
		case placeTurret_wait:
			if(C1 && !inGame){ state = placeTurret_Press; } 
			else { state = placeTurret_wait; }
			break;
		case placeTurret_Press:
			if(C1){ state = placeTurret_Press; } 
			else if(!C1){ state = placeTurret_Release; }
			break;
		case placeTurret_Release: state = placeTurret_wait; break;
	}
	switch(state){
		case placeTurret_init: break;
		case placeTurret_wait: 
			//outgoingByte = outgoingByte & 0x7F; //Turn 7th bit low
			break;
		case placeTurret_Press: break;
		case placeTurret_Release:
			//outgoingByte = outgoingByte | 0x80; //Turn 7th bit high
			break;
	}
	return state;
}
*/
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
			if(C2){ state = selTur_bluePress; }
			else if(!C2){ state = selTur_blueRelease; }
			break;
		case selTur_blueRelease: state = selTur_wait; break;
		case selTur_purpPress:
			if(C3){ state = selTur_purpPress; }
			else if(!C3){ state = selTur_purpRelease; }
			break;
		case selTur_purpRelease: state = selTur_wait; break;
		case selTur_greenPress:
			if(C4){ state = selTur_greenPress; }
			else if(!C4) { state = selTur_greenRelease; }
			break;
		case selTur_greenRelease: state = selTur_wait; break;
	}
	switch(state){
		case selTur_init: break;
		case selTur_wait: break;
		case selTur_bluePress:
			//TODO: Place turret. Need to check "currentTurret", player's gold, and current state of game(in game or not)
			//add and place blue
			towers[t]->cost = 20;
			if(gold >= towers[t]->cost){
				towers[t]->attackSpeed = 1;
				towers[t]->damage = 1;
				towers[t]->purchased = 1;
				outgoingByte = outgoingByte | 0x10;
				gold -= towers[t]->cost;
				t++;
			}
			break;
		case selTur_blueRelease: break;
		case selTur_purpPress: 
			//add and place purple
			towers[t]->cost = 40;
			if(gold >= towers[t]->cost){
				towers[t]->attackSpeed = 1;
				towers[t]->damage = 2;
				towers[t]->purchased = 1;
				outgoingByte = outgoingByte | 0x20;
				gold -= towers[t]->cost;
				t++;
			}
			break;
		case selTur_purpRelease: break;
		case selTur_greenPress: 
			//add and place green
			towers[t]->cost = 60;
			if(gold >= towers[t]->cost){
				towers[t]->attackSpeed = 2;
				towers[t]->damage = 1;
				towers[t]->purchased = 1;
				outgoingByte = outgoingByte | 0x30;
				gold -= towers[t]->cost;
				t++;
			}
			break;
		case selTur_greenRelease: break;
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
			LCD_Cursor(1);
			x2 = readadc(0);
			y2 = readadc(1);
			x2 = x2 - 512;
			y2 = y2 - 512;
			//LCD_DisplayString(1, itoa(x2, a, 10)); //Must disabled ClearScreen() in LCD_DisplayString() in io.c to see coordinates.
			//LCD_DisplayString(17, itoa(y2, b, 10));
			if(y2 > 150){
				LCD_DisplayString(1, "right");
				joystickMovement = 0x08;
				outgoingByte = outgoingByte | joystickMovement;
			} else if(y2 < -150){
				LCD_DisplayString(1, "left");
				joystickMovement = 0x04;
				outgoingByte = outgoingByte | joystickMovement;
			} else if(x2 < -150){
				LCD_DisplayString(1, "up");
				joystickMovement = 0x01;
				outgoingByte = outgoingByte | joystickMovement;
			} else if(x2 > 150){
				LCD_DisplayString(1, "down");
				joystickMovement = 0x02;
				outgoingByte = outgoingByte | joystickMovement;
			} else{
				LCD_DisplayString(1, "no input");
				joystickMovement = 0x00;
				outgoingByte = outgoingByte | joystickMovement;
			}
			break;
	}
	return state;
}

enum LCD_States{LCD_initialize, LCD_info, LCD_updatePress, LCD_updateRelease, LCD_win, LCD_loss};

int LCDTick(int state){
	switch(state){
		case LCD_initialize: state = LCD_info; break;
		case LCD_info:
			if(A2){ state = LCD_updatePress; } 
			else if(level == 3){ state = LCD_win; } 
			else if(health == 0){ state = LCD_loss; } 
			else { state = LCD_info; }
			break;
		case LCD_updatePress:
			if(A2){ state = LCD_updatePress; } 
			else if(!A2){ state = LCD_updateRelease; }
			break;
		case LCD_updateRelease: state = LCD_info; break;
		case LCD_win: break;
		case LCD_loss: break;
	}
	switch(state){
		case LCD_initialize: break;
		case LCD_info: break;
		case LCD_updatePress: break;
		case LCD_updateRelease:
			++gold;
			++level;
			++health;
			//LCD_DisplayString(1, updatePlayerInfo(gold, level, health));
				//is it possible for the task scheduler to move on to the next state
				//before the data has finished updating?
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
				outgoingByte = 0x00; //reset outgoingByte after being sent
			}
			break;
	}
	return state;
}

//End FSMs

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; //Inputs. Using A0 and A1 as inputs for 2-axis joystick
	DDRB = 0xFF; PORTB = 0x00; //LCD Display, output to PORTB
	DDRC = 0x00; PORTC = 0xFF; //Inputs. Using C0-C5 as user input
	//DDRD = 0x03; PORTD = 0xFC; //Using D0 and D1 as outputs(control bus) and D2-D7 as inputs
	DDRD = 0x30; PORTD = 0xCF; //D4 and D5 are outputs(control bus). D0 - D3 are RXD0, TXD0, RXD1, and TXD1 (UART Functionality)
	
	// Period for the tasks
	unsigned long int LCDTick_calc = 500;
	unsigned long int ADCTick_calc = 200;
	unsigned long int sNpTick_calc = 200;
	//unsigned long int placeTurretTick_calc = 200;
	unsigned long int selTurTick_calc = 200;
	unsigned long int usartSMTick_calc = 100;
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(LCDTick_calc, ADCTick_calc);
	tmpGCD = findGCD(tmpGCD, sNpTick_calc);
	//tmpGCD = findGCD(tmpGCD, placeTurretTick_calc);
	tmpGCD = findGCD(tmpGCD, selTurTick_calc);
	tmpGCD = findGCD(tmpGCD, usartSMTick_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int LCDTick_period = LCDTick_calc/GCD;
	unsigned long int ADCTick_period = ADCTick_calc/GCD;
	unsigned long int sNpTick_period = sNpTick_calc/GCD;
	//unsigned long int placeTurretTick_period = placeTurretTick_calc/GCD;
	unsigned long int selTurTick_period = selTurTick_calc/GCD;
	unsigned long int usartSMTick_period = usartSMTick_calc/GCD;

	//Declare an array of tasks
	static task task1, task2, task3, /*task4,*/ task5, task6;
	task *tasks[] = { &task1, &task2 ,&task3, /*&task4,*/ &task5, &task6};
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
	
	// Task 3
	task3.state = sNp_init;
	task3.period = sNpTick_period;
	task3.elapsedTime = sNpTick_period;
	task3.TickFct = &sNpTick;
	/*
	// Task 4
	task4.state = placeTurret_init;
	task4.period = placeTurretTick_period;
	task4.elapsedTime = placeTurretTick_period;
	task4.TickFct = &placeTurretTick;
	*/
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
	
	TimerSet(GCD);
	TimerOn();
	LCD_init();
	//LCD_DisplayString(1, playerInfo);
	InitADC();
	initUSART(0);
	USART_Flush(0);
	
	unsigned short i; // Scheduler for-loop iterator
	while(1) {
		A2 = ~PINA & 0x04;
		C0 = ~PINC & 0x01; //Start/Pause button
		C1 = ~PINC & 0x02; //Place turret button
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

