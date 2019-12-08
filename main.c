/*
 * Snake.c
 *
 * Created: 11/20/2019 11:07:51 AM
 * Author : johnt
 */ 


#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "io.h"
//#include "SNES.c"
#include "glcd.h"
#include "timer.h"
//#include "lcd_8bit_task.h"
#include "fonts/font5x7.h"

// Github library source file link and credit: https://github.com/MCoyne1234

#define SNES_CLOCK  PORTD0
#define SNES_LATCH  PORTD1
#define SNES_DATA   PORTD2
#define SNES_PORT   PORTD
#define SNES_PIN    PIND

#define SNES_NONE        0
#define SNES_R          16
#define SNES_L         	32
#define SNES_X          64
#define SNES_A         128
#define SNES_RIGHT     256
#define SNES_LEFT      512
#define SNES_DOWN     1024
#define SNES_UP       2048
#define SNES_START    4096
#define SNES_SELECT   8192
#define SNES_Y       16384
#define SNES_B       32768

#define OUTPUT_PORT PORTB

void SNES_init(){
	SNES_PORT |= (0x01 << SNES_CLOCK);
	SNES_PORT |= (0x01 << SNES_LATCH);
}

unsigned short SNES_Read(){
	unsigned short snes_pressed = 0x0000;

	SNES_PORT |= (0x01  << SNES_LATCH);
	SNES_PORT |= (0x01 << SNES_CLOCK);
	SNES_PORT &= ~(0x01 << SNES_LATCH);
	snes_pressed = (((~SNES_PIN) & (0x01 << SNES_DATA)) >> SNES_DATA);
	for(int i = 0; i < 16; i++){
		SNES_PORT &= ~(0x01 << SNES_CLOCK);
		snes_pressed <<= 1;
		snes_pressed |= ( ( (~SNES_PIN) & (0x01  << SNES_DATA) ) >> SNES_DATA);
		SNES_PORT |= (0x01 << SNES_CLOCK);
	}
	return snes_pressed;
}

unsigned char wide[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80 };
unsigned char high[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44 };

typedef struct _Snake
{
	int* next; 		// Next block of the snake
	unsigned char xcoor; 		// x coordinate of the snake block
	unsigned char ycoor;		// y coordinate of the snake block
	unsigned char direction;
} *Snake;

typedef struct _Point
{
	unsigned char xcoor;
	unsigned char ycoor;
} *Point;

Snake SnakeInit() {
	Snake S = malloc (sizeof(struct _Snake));
	S->xcoor = 21;
	S->ycoor = 17;
	S->direction = 1;
	S->next = malloc (sizeof(struct _Snake));
	Snake head = S;
	S = S->next;
	S->xcoor = 17;
	S->ycoor = 17;
	S->direction = 1;
	S->next = malloc (sizeof(struct _Snake));
	S = S->next;
	S->xcoor = 13;
	S->ycoor = 17;
	S->direction = 1;
	S->next = malloc (sizeof(struct _Snake));
	S = S->next;
	S->xcoor = 9;
	S->ycoor = 17;
	S->direction = 1;
	S->next = NULL;
	
	return head;
}

Point PointInit(Snake S) {
	srand(time(0));
	Point P = malloc (sizeof(struct _Point));
	P->xcoor = wide[rand()%20]+2;
	P->ycoor = high[rand()%11]+2;
	Snake head = S;
	while (S) {
		if (S->xcoor == P->xcoor-1 && S->ycoor == P->ycoor-1) {
			P->xcoor = wide[rand()%20]+2;
			P->ycoor = high[rand()%11]+2;
			S = head;
		}
		else {
			S = S->next;
		}
	}
	return P;
}

void PrintPoint(Point P) {
	glcd_fill_circle(P->xcoor, P->ycoor, 1, 1);
	return;
}

void PrintSnake(Snake S) {
	while(S) {
		glcd_fill_rect(S->xcoor, S->ycoor, 3, 3, 1);
		S = S->next;
	}
	return;
}

void GameOver(Snake S, Point P) {
	Snake it = S;
	Snake n = S->next;
	while (it) {
		n = it->next;
		free(it);
		it = n;
	}
	S = NULL;
	free(P);
	S = SnakeInit();
	P = PointInit(S);
}

Snake FollowHead (Snake S, unsigned char prevXcoor, unsigned char prevYcoor, unsigned char prevDir) {
	if(S == NULL) {
		return S;
	}
	unsigned char oldX = S->xcoor;
	unsigned char oldY = S->ycoor;
	unsigned char oldDir = S->direction;
	S->xcoor = prevXcoor;
	S->ycoor = prevYcoor;
	S->direction = prevDir;
	S->next = FollowHead(S->next, oldX, oldY, oldDir);
	return S;
}

unsigned char points = 0;
unsigned char HS = 0;

Snake Score(Snake S, Point P) {
	Snake it = S;
	while(it->next) {
		it = it->next;
	}
	Snake prev = it;
	it = it->next;
	it = malloc (sizeof(struct _Snake));
	if (prev->direction == 1) {
		it->xcoor = prev->xcoor-4;
		it->ycoor = prev->ycoor;
		it->direction = prev->direction;
	}
	else if (prev->direction == 2) {
		it->ycoor = prev->ycoor+4;
		it->xcoor = prev->xcoor;
		it->direction = prev->direction;
	}
	else if (prev->direction == 3) {
		it->xcoor = prev->xcoor+4;
		it->ycoor = prev->ycoor;
		it->direction = prev->direction;
	}
	else if (prev->direction == 4) {
		it->ycoor = prev->ycoor-4;
		it->xcoor = prev->xcoor;
		it->direction = prev->direction;
	}
	if (it->xcoor < 1 || it->xcoor > 77) {
		if (prev->ycoor > 1) {
			it->xcoor = prev->xcoor;
			it->ycoor = prev->ycoor - 4;
			it->direction = 4;
		}
		
		else {
			it->xcoor = prev->xcoor;
			it->ycoor = prev->ycoor + 4;
			it->direction = 2;
		}
	}
	else if (it->ycoor < 1 || it->ycoor > 41) {
		if (prev->xcoor < 77) {
			it->xcoor = prev->xcoor + 4;
			it->ycoor = prev->ycoor;
			it->direction = 3;
		}
		else{
			it->xcoor = prev->xcoor - 4;
			it->ycoor = prev->ycoor;
			it->direction = 1;
		}
	}
	it->next = NULL;
	prev->next = it;
	points += 1;
	if (points > HS) {
		HS = points;
	}
	free(P);
	P = PointInit(S);
	return S;
}

unsigned char gg = 0;

void MoveHead(Snake S, Point P) {
	unsigned char oldX = S->xcoor;
	unsigned char oldY = S->ycoor;
	unsigned char oldDir = S->direction;
	if (S->direction == 1) { // right
		S->xcoor = S->xcoor + 4;
	}
	else if (S->direction == 2) { // up
		S->ycoor = S->ycoor - 4;
	}
	else if (S->direction == 3) { // left
		S->xcoor = S->xcoor - 4;
	}
	else if (S->direction == 4) { // down
		S->ycoor = S->ycoor + 4;
	}
	if (S->xcoor == P->xcoor-1 && S->ycoor == P->ycoor-1) {
		S = Score(S, P);
	}
	Snake head = S->next;
	while (head) {
		if ((S->xcoor == head->xcoor && S->ycoor == head->ycoor) || S->xcoor > 77 || S->xcoor < 1 || S->ycoor > 41 || S->ycoor < 1) {
			gg = 1;
			return;
		}
		else {
			head = head->next;
		}
	}
	S->next = FollowHead(S->next, oldX, oldY, oldDir);
}



void BuildGame( Snake S, Point P ) {
	unsigned char i = 0;
	unsigned char j = 0;
	
	glcd_init();
	glcd_set_contrast(200);
	glcd_tiny_set_font(Font5x7, 5, 7, 32, 127);
	//glcd_clear_buffer();
	while(i <= 20) {
		glcd_draw_line(wide[i], 0, wide[i], 44, 1);
		i++;
	}
	i = 0;
	while(j <= 11) {
		glcd_draw_line(0, high[j], 80, high[j], 1);
		j++;
	}
	j = 0;
	//glcd_write();
	//_delay_ms(300000000);
	PrintSnake(S);
	PrintPoint(P);
	glcd_write();
	_delay_ms(1750);
}

unsigned char A0 = 0;
unsigned char A1 = 0;
unsigned char A2 = 0;
unsigned char A3 = 0;
unsigned char A5 = 0;
unsigned char A4 = 0;

enum dirs { neutral, push } states2;

int Tick_Dir(int states, Snake S) {
	switch(states) { // actions & transitions
		case neutral:
			if (A0 || A1 || A2 || A3) {
				states = push;
				if (A0) {
					if (S->direction != 3) {
						S->direction = 1;
					}
				}
				else if (A1) {
					if (S->direction != 4) {
						S->direction = 2;
					}
				}
				else if (A3) {
					if (S->direction != 1) {
						S->direction = 3;
					}
				}
				else if (A2) {
					if (S->direction != 2) {
						S->direction = 4;
					}
				}
			}
			break;
		case push:
			//if (SNES_Read() == SNES_RIGHT || SNES_Read() == SNES_UP || SNES_Read() == SNES_DOWN || SNES_Read() == SNES_LEFT) {
			if (A0 || A1 || A2 || A3) {
				states = push;
			}
			else {
				states = neutral;
			}
			break;
		default:
			states = neutral;
			break;
	}
	return states;
}

enum GameStates { start, play, GO } states1;
unsigned char count = 0;
unsigned char c1 = 0;

int TickAll(int states, Snake S, Point P) {
	unsigned char i = 0;
	unsigned char j = 0;
	char buffer[20];
	switch(states) { // Actions
		case start: 
			LCD_DisplayString(1, "Press Start!");
			glcd_init();
			glcd_set_contrast(200);
			glcd_tiny_set_font(Font5x7, 5, 7, 32, 127);
			glcd_tiny_draw_string(7, 2, "Press Start!");
			glcd_write();
			_delay_ms(3000);
			break;
		case play:
			if (count < 3) {
				count++;
				itoa(points, buffer, 10);
				unsigned char disp[11] = "Score: ";
				strcat(disp, buffer);
				LCD_DisplayString(1, disp);
				states2 = Tick_Dir(states2, S);
				BuildGame(S,P);
			}
			else {
				count = 0;
				itoa(points, buffer, 10);
				unsigned char disp[11] = "Score: ";
				strcat(disp, buffer);
				LCD_DisplayString(1, disp);
				
				states2 = Tick_Dir(states2, S);
				MoveHead(S, P);
				BuildGame(S,P);
				//_delay_ms(100);
			}
			break;
		case GO: 
			GameOver(S, P);
			points = 0;
			LCD_ClearScreen();
			LCD_DisplayString(1, "Press Start to  Replay");
			glcd_init();
			glcd_set_contrast(200);
			glcd_tiny_set_font(Font5x7, 5, 7, 32, 127);
			glcd_tiny_draw_string(9, 2, "GAME OVER");
			glcd_write();
			_delay_ms(5000);
			break;
		default:
			LCD_ClearScreen();
			//glcd_clear_buffer();
			break;
	}
	switch(states) { // Transitions
		SNES_init();
		case start:
			if(A5) {
				states = play;
				//glcd_clear_buffer();
				//glcd_init();
				//BuildGame(S, P);
			}
			break;
		case play:
			if (gg == 1) {
				gg = 0;
				states = GO;
				//glcd_clear_buffer();
			}
			else if (SNES_Read()) {
				states = start;
				//glcd_clear_buffer();
			}
			break;
		case GO:
			if (A5) {
				states = play;
			}
			else if (SNES_Read()) {
				states = start;
				glcd_reset();
				glcd_clear();
			}
			break;
		default:
			states = start;
			break;
	}
	return states;
}

unsigned short out = 0;
unsigned short oneOn = 0x00;


/*
int FilterInput(int state){
	unsigned short input = 0x00;
	out = 0x00;
	input = SNES_Read();
	PORTD = input;
	if(input){
		//if(input & 16) out = 0x01;
		//if(input & 32) out = 0x02;
		//if(input & 64) out = 0x04;
		//if(input & 128) out = 0x08;
		if(input & 256) out |= 0x02; //R
		if(input & 512) out |= 0x04; //L
		//if(input & 1024) out = 0x40;
		//if(input & 2048) out = 0x80;
		if(input & 4096) out |= 0x08; //Start
		if(input & 8192) out |= 0x10; //Select
		//if(input & 16384) out = 0x01;
		if(input & 32768) out |= 0x01; //B
	} else out = 0x00;
	
	return oneOn;
}
*/

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xF0; PORTD = 0x0F;
	glcd_init();
	glcd_set_contrast(20);
	glcd_tiny_set_font(Font5x7, 5, 7, 32, 127);
	Snake S = SnakeInit();
	Point P = PointInit(S);
	LCD_init();
	unsigned long d[20];
	unsigned long f[11];
	unsigned long c = 0;
	//LCD_DisplayString();
	SNES_init();
	TimerSet(10);
	TimerOn();
	
	while(1) {
		A0 = ~PINA & 0x01;
		A1 = ~PINA & 0x02;
		A2 = ~PINA & 0x04;
		A3 = ~PINA & 0x08;
		A4 = ~PINA & 0x10;
		A5 = ~PINA & 0x20;
		//FilterInput(oneOn);
		if (c) {
			//PORTD = 0x0F;
		}
		else {
			//PORTD = 0x00;
		}
		//itoa(c, d, 10);
		//strcat(f,d);
		//LCD_DisplayString(1, f);
		
		states1 = TickAll(states1, S, P);
		glcd_clear_buffer();
		
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 0;
}


