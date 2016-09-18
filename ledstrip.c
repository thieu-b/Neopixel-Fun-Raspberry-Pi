/*
 * Ledstrip.c
 * 
 * Copyright 2016  <pi@raspberrypi>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


/*            Geany Settings
 * Compile	:gcc -Wall -c "%f"
 * Build	:gcc -Wall -pthread -o "%e" "%f" -lpigpio -lrt   
 * Run		:sudo "./%e"
*/

/*
 * thieu b 
*/

// PROGRAMMA ALTIJD STOPPEN MET CTRL-C ANDERS PROBLEMEN MET pigpio

#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define RAM_0 16														// controle RAM
#define RAM_1 21														// data RAM's
#define RAM_2 24
#define RAM_3 23
#define RAM_4 18
#define RAM_5 26
#define RAM_6 19
#define RAM_7 13
#define RAM_8 25
#define OSC 20															// enable Oscillator output (1)
#define SET 22														    // set 74HC74
#define RESET 6															// reset 74HC74
#define OUT 12	                                                        // Stop ? (1 = einde transfer > leds bereikt) komt van 74HC74

#define aantalLeds 72


int LCV512;
char LCV512_sequentieel[] = {0x01, 0x40};								// waarde voor instellen sequentiele mode
char LCV512_write[] = {0x02, 0x00, 0x00, 0x00};							// start write vanaf adres 0x00 0x00 + 1 extra 0x00 byte (1ste led anders fout)
char LCV512_read[] = {0x03, 0x00, 0x00};								// start read vanaf adres 0x00 0x00
char LCV512_RAM0_stopbyte = {0xff};										// laatste byte RAM0 0xff naar 74HC74

char alfabetArray[128][9][5] = {{},
							   {{5, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 1, 1, 1, 1}},
                               {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {0, 1, 1, 1, 1}, {0, 0, 0, 0, 0}},
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 0, 0, 0, 0}},
							   {{4, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {0, 0, 0, 0, 0}},
							   {{5, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {0, 0, 0, 0, 0}},
							   {{5, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0}, {0, 0, 0, 0, 0}},
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0}, {0, 0, 0, 0, 0}},
							   {{1, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
								{}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, 
	                           {{3, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}}, 		// <SPACE>
	                            {}, {}, {}, {}, {}, {}, {}, {}, {},
	                           {{5, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 0, 1, 0, 1}, {0, 1, 1, 1, 0}, {1, 1, 1, 1, 1}, {0, 1, 1, 1, 0}, {1, 0, 1, 0, 1}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},		// *
	                           {{5, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {1, 1, 1, 1, 1}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},		// +
	                            {},
	                           {{5, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 1, 1, 1, 1}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}}, 		// -
	                           {{1, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},		// .
	                           {{5, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 1}, {0, 0, 0, 1, 0}, {0, 0, 1, 0, 0}, {0, 1, 0, 0, 0}, {1, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},		// /
	                           {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 1, 1},	{1, 0, 1, 0, 1}, {1, 1, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// 0
							   {{3, 0, 0, 0, 0}, {0, 1, 1, 0, 0}, {1, 0, 1, 0, 0}, {0, 0, 1, 0, 0},	{0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0},	{0, 0, 0, 0, 0}},		// 1
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {0, 0, 0, 0, 1},	{0, 0, 0, 1, 0}, {0, 0, 1, 0, 0}, {0, 1, 0, 0, 0}, {1, 1, 1, 1, 1},	{0, 0, 0, 0, 0}},		// 2
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {0, 0, 0, 0, 1},	{0, 1, 1, 1, 0}, {0, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// 3
							   {{5, 0, 0, 0, 0}, {0, 0, 0, 0, 1}, {0, 0, 0, 1, 1}, {0, 0, 1, 0, 1},	{0, 1, 0, 0, 1}, {1, 1, 1, 1, 1}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 1},	{0, 0, 0, 0, 0}},		// 4
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0},	{0, 1, 1, 1, 0}, {0, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// 5
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 0},	{1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// 6
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 1}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 1},	{0, 0, 0, 1, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0},	{0, 0, 0, 0, 0}},		// 7	
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// 8
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{0, 1, 1, 1, 1}, {0, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// 9	  	
							   {{3, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 1, 0, 0, 0},	{0, 0, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 0, 0, 0, 0},	{0, 0, 0, 0, 0}},		// :
								{}, {}, 
							   {{5, 0, 0, 0, 0},
								{0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0},
								{1, 1, 1, 1, 1},
								{0, 0, 0, 0, 0},
								{1, 1, 1, 1, 1},
								{0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0},
								{0, 0, 0, 0, 0}},
								{}, {}, {},
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{0, 0, 0, 0, 0}},		// A
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// B
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 0},	{1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// C
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// D
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0},	{1, 1, 1, 1, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 1},	{0, 0, 0, 0, 0}},		// E
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 1}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0},	{1, 1, 1, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0},	{0, 0, 0, 0, 0}},		// F
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 0, 0, 0, 0}, {1, 0, 1, 1, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// G
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 1, 1, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{0, 0, 0, 0, 0}},		// H
							   {{3, 0, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 1, 0, 0, 0},	{0, 1, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 1, 0, 0, 0},	{0, 0, 0, 0, 0}},		// I
							   {{3, 0, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {1, 0, 1, 0, 0}, {0, 1, 0, 0, 0}, {0, 0, 0, 0, 0}},		// J
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 1, 0}, {1, 0, 1, 0, 0},	{1, 1, 0, 0, 0}, {1, 0, 1, 0, 0}, {1, 0, 0, 1, 0}, {1, 0, 0, 0, 1},	{0, 0, 0, 0, 0}},		// K
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0},	{1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 1},	{0, 0, 0, 0, 0}},		// L
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 1, 0, 1, 1}, {1, 0, 1, 0, 1},	{1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{0, 0, 0, 0, 0}},		// M
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 1, 0, 0, 1},	{1, 0, 1, 0, 1}, {1, 0, 0, 1, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{0, 0, 0, 0, 0}},		// N
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// O
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 1, 1, 1, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 0, 0, 0, 0},	{0, 0, 0, 0, 0}},		// P
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 1, 0, 1}, {1, 0, 0, 1, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// Q
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 1, 1, 1, 0}, {1, 0, 1, 0, 0}, {1, 0, 0, 1, 0}, {1, 0, 0, 0, 1},	{0, 0, 0, 0, 0}},		// R
							   {{5, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 0},	{0, 1, 1, 1, 0}, {0, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// S
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 1}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0},	{0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0},	{0, 0, 0, 0, 0}},		// T
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 1, 1, 0},	{0, 0, 0, 0, 0}},		// U
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 0, 1, 0}, {0, 0, 1, 0, 0},	{0, 0, 0, 0, 0}},		// V
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 1, 0, 1}, {0, 1, 0, 1, 0},	{0, 0, 0, 0, 0}},		// W
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {0, 1, 0, 1, 0},	{0, 0, 1, 0, 0}, {0, 1, 0, 1, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{0, 0, 0, 0, 0}},		// X
							   {{5, 0, 0, 0, 0}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1}, {1, 0, 0, 0, 1},	{0, 1, 0, 1, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 1, 0, 0},	{0, 0, 0, 0, 0}},		// Y
							   {{5, 0, 0, 0, 0}, {1, 1, 1, 1, 1}, {0, 0, 0, 0, 1}, {0, 0, 0, 1, 0},	{0, 0, 1, 0, 0}, {0, 1, 0, 0, 0}, {1, 0, 0, 0, 0}, {1, 1, 1, 1, 1},	{0, 0, 0, 0, 0}},		// Z
							   	{},
							   {{5, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {1, 0, 0, 0, 0}, {0, 1, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 0, 0, 1, 0}, {0, 0, 0, 0, 1}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}}		// < BACKSLASH>	
							};
							
char byteHoog[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
				   0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88,
				   0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e,
				   0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e,
				   0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8,
				   0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8,
				   0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
				   0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee};
char byteMiddenHoog[] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e,
						 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e,
						 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e,
						 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e, 0x8e,
						 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee};
char byteMiddenLaag[] = {0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee,
						 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee, 0x88, 0x88, 0x88, 0x88, 0x8e, 0x8e, 0x8e, 0x8e, 0xe8, 0xe8, 0xe8, 0xe8, 0xee, 0xee, 0xee, 0xee};
char byteLaag[] = {0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee,
				   0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee,
				   0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee,
				   0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee,
				   0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee,
				   0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee,
				   0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee,
				   0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee, 0x88, 0x8e, 0xe8, 0xee};

char pixelBuf1[2100] = "";
char pixelBuf2[2100] = "";
char pixelBuf3[2100] = "";
char pixelBuf4[2100] = "";
char pixelBuf5[2100] = "";
char pixelBuf6[2100] = "";
char pixelBuf7[2100] = "";
char pixelBuf8[2100] = "";

char pixelBufTmp[2100] = "";

char pixelTmp[7][2100];

char rood1[1000];
char groen1[1000];
char blauw1[1000];
char rood2[1000];
char groen2[1000];
char blauw2[1000];
char rood3[1000];
char groen3[1000];
char blauw3[1000];
char rood4[1000];
char groen4[1000];
char blauw4[1000];
char rood5[1000];
char groen5[1000];
char blauw5[1000];
char rood6[1000];
char groen6[1000];
char blauw6[1000];
char rood7[1000];
char groen7[1000];
char blauw7[1000];
char rood8[1000];
char groen8[1000];
char blauw8[1000];


char strip1Red[2100] = "";
char strip1Green[2100] = "";
char strip1Blue[2100] = "";
char strip2Red[2100] = "";
char strip2Green[2100] = "";
char strip2Blue[2100] = "";
char strip3Red[2100] = "";
char strip3Green[2100] = "";
char strip3Blue[2100] = "";
char strip4Red[2100] = "";
char strip4Green[2100] = "";
char strip4Blue[2100] = "";
char strip5Red[2100] = "";
char strip5Green[2100] = "";
char strip5Blue[2100] = "";
char strip6Red[2100] = "";
char strip6Green[2100] = "";
char strip6Blue[2100] = "";
char strip7Red[2100] = "";
char strip7Green[2100] = "";
char strip7Blue[2100] = "";
char strip8Red[2100] = "";
char strip8Green[2100] = "";
char strip8Blue[2100] = "";


char strip1Buffer[25250] = "";											// 1000 leds * 3 kleuren * 4 bytes per kleur + 1 "0" byte + 12 bytes voor shift = 25250
char strip2Buffer[25250] = "";
char strip3Buffer[25250] = "";
char strip4Buffer[25250] = "";
char strip5Buffer[25250] = "";
char strip6Buffer[25250] = "";
char strip7Buffer[25250] = "";
char strip8Buffer[25250] = "";
char RAM0Buffer[25250] = "";




float kleurMatrix1[8][3][3] = {{{255, 0, 1}, {0, 0, 1}, {0, 255, 1}},	//ledstrip 1 {{R beginwaarde, R eindewaarde, aantal verdelingen}, {G beginwaarde, G eindwaarde, aantal verdelingen}, {B beginwaarde, B eindwaarde, aantal verdelingen}},
							   {{255, 0, 1}, {0, 0, 1}, {0, 255, 1}},
							   {{255, 0, 1}, {0, 0, 1}, {0, 255, 1}},
							   {{255, 0, 1}, {0, 0, 1}, {0, 255, 1}},
							   {{255, 0, 1}, {0, 0, 1}, {0, 255, 1}},
							   {{255, 0, 1}, {0, 0, 1}, {0, 255, 1}},
							   {{255, 0, 1}, {0, 0, 1}, {0, 255, 1}},
							   {{255, 0, 1}, {0, 0, 1}, {0, 255, 1}}};
							   
float kleurMatrix2[8][3][3] = {{{0, 0, 1}, {255, 0, 1}, {0, 255, 1}},			
							   {{0, 0, 1}, {255, 0, 1}, {0, 255, 1}},
							   {{0, 0, 1}, {255, 0, 1}, {0, 255, 1}},
							   {{0, 0, 1}, {255, 0, 1}, {0, 255, 1}},
							   {{0, 0, 1}, {255, 0, 1}, {0, 255, 1}},
							   {{0, 0, 1}, {255, 0, 1}, {0, 255, 1}},
							   {{0, 0, 1}, {255, 0, 1}, {0, 255, 1}},
							   {{0, 0, 1}, {255, 0, 1}, {0, 255, 1}}};
							   
float kleurMatrix3[8][3][3] = {{{255, 0, 1}, {0, 255, 1}, {0, 0, 1}},			
							   {{255, 0, 1}, {0, 255, 1}, {0, 0, 1}},
							   {{255, 0, 1}, {0, 255, 1}, {0, 0, 1}},
							   {{255, 0, 1}, {0, 255, 1}, {0, 0, 1}},
							   {{255, 0, 1}, {0, 255, 1}, {0, 0, 1}},
							   {{255, 0, 1}, {0, 255, 1}, {0, 0, 1}},
							   {{255, 0, 1}, {0, 255, 1}, {0, 0, 1}},
							   {{225, 0, 1}, {0, 255, 1}, {0, 0, 1}}};
							   
float kleurMatrix4[8][3][3] = {{{255, 0, 1}, {255, 0, 2}, {0, 255, 1}},			
							   {{255, 0, 1}, {255, 0, 2}, {0, 255, 1}},
							   {{255, 0, 1}, {255, 0, 2}, {0, 255, 1}},
							   {{255, 0, 1}, {255, 0, 2}, {0, 255, 1}},
							   {{255, 0, 1}, {255, 0, 2}, {0, 255, 1}},
							   {{255, 0, 1}, {255, 0, 2}, {0, 255, 1}},
							   {{255, 0, 1}, {255, 0, 2}, {0, 255, 1}},
							   {{225, 0, 1}, {255, 0, 2}, {0, 255, 1}}};
						 							   
						 
float kleurMatrixRood[8][3][3] = {{{255, 255, 1}, {0, 0, 1}, {0, 0, 1}},			
							      {{255, 255, 1}, {0, 0, 1}, {0, 0, 1}},
							      {{255, 255, 1}, {0, 0, 1}, {0, 0, 1}},
							      {{255, 255, 1}, {0, 0, 1}, {0, 0, 1}},
							      {{255, 255, 1}, {0, 0, 1}, {0, 0, 1}},
							      {{255, 255, 1}, {0, 0, 1}, {0, 0, 1}},
							      {{225, 255, 1}, {0, 0, 1}, {0, 0, 1}},
							      {{255, 255, 1}, {0, 0, 1}, {0, 0, 1}}};						
						 
float kleurMatrixGroen[8][3][3] = {{{0, 0, 1}, {255, 255, 1}, {0, 0, 1}},			
							       {{0, 0, 1}, {255, 255, 1}, {0, 0, 1}},
							       {{0, 0, 1}, {255, 255, 1}, {0, 0, 1}},
							       {{0, 0, 1}, {255, 255, 1}, {0, 0, 1}},
							       {{0, 0, 1}, {255, 255, 1}, {0, 0, 1}},
							       {{0, 0, 1}, {255, 255, 1}, {0, 0, 1}},
							       {{0, 0, 1}, {255, 255, 1}, {0, 0, 1}},
							       {{0, 0, 1}, {255, 255, 1}, {0, 0, 1}}};

float kleurMatrixBlauw[8][3][3] = {{{0, 0, 1}, {0, 0, 1}, {255, 255, 1}},			
							       {{0, 0, 1}, {0, 0, 1}, {255, 255, 1}},
							       {{0, 0, 1}, {0, 0, 1}, {255, 255, 1}},
							       {{0, 0, 1}, {0, 0, 1}, {255, 255, 1}},
							       {{0, 0, 1}, {0, 0, 1}, {255, 255, 1}},
							       {{0, 0, 1}, {0, 0, 1}, {255, 255, 1}},
							       {{0, 0, 1}, {0, 0, 1}, {255, 255, 1}},
							       {{0, 0, 1}, {0, 0, 1}, {255, 255, 1}}};
							       
float kleurMatrixWit[8][3][3] = {{{255, 255, 1}, {255, 255, 1}, {255, 255, 1}},			
							    {{255, 255, 1}, {255, 255, 1}, {255, 255, 1}},
							    {{255, 255, 1}, {255, 255, 1}, {255, 255, 1}},
							    {{255, 255, 1}, {255, 255, 1}, {255, 255, 1}},
							    {{255, 255, 1}, {255, 255, 1}, {255, 255, 1}},
							    {{255, 255, 1}, {255, 255, 1}, {255, 255, 1}},
							    {{255, 255, 1}, {255, 255, 1}, {255, 255, 1}},
							    {{225, 255, 1}, {255, 255, 1}, {255, 255, 1}}};

int sterrenKleur1[3] = {255, 255, 255};
int sterrenKleur2[3] = {255, 0, 0};
int sterrenKleur3[3] = {0, 255, 0};
int sterrenKleur4[3] = {0, 0, 255};

						 							       
// **********************

// Begin programma functies
void setupInOut();
void setupLTC6904();
void setupLCV512();
void bufferInit();
void data2Ram0();
void strip(char buffer[], char red[], char green[], char blue[] );
void strip2Buffer8();
void stopData2Leds();
void data2Ram();
void ram2Leds();
void stopNetjes();
// Einde programma functies

// Begin tekst naar pixels functies
void print7(char tekst[]);
void print8(char tekst[]);
void printBasis(char tekst[]);
void print7Midden(char tekst[]);
void print8Midden(char tekst[]);
void printBasisMidden(char tekst[]);
void pixel2Tmp7(char tekst[]);
void pixel2Tmp8(char tekst[]);
void cls7();
void cls8();
void clsBasis();
void cls1ste();
void cls7de();
void cls8ste();
// Einde tekst naar pixels functies

// Begin tekst manipulatie functies
void shiftRechts(char buffer[]);
void shiftRechts7();
void shiftRechts8();
void shiftRechtsBasis();
void shiftLinks(char buffer[]);
void shiftLinks7();
void shiftLinks8();
void shiftLinksBasis();
void shiftRechtsLinks7();
void shiftRechtsLinks8();
void shiftRechtsLinksBasis();
void stringCopy(char doel[], char bron[], int aantal);
void omhoog7();
void omhoog8();
void zevenOmhoog(char tekst[]);
void achtOmhoog(char tekst[]);
void omlaag7();
void omlaag8();
void zevenOmlaag(char tekst[]);
void achtOmlaag(char tekst[]);
// Einde tekst manipulatie functies

// Begin kleur functies
void kleurOpmaak(float kleurMatrix[8][3][3], char rood[], char groen[], char blauw[], int rij);
void kleurOpmaak7(float matrix[8][3][3]);								
void kleurOpmaak8(float matrix[8][3][3]);
void kleurOpmaakBasis(float matrix[8][3][3]);	
void shiftKleurLinks(char rbuf[], char gbuf[], char bbuf[]);							
void shiftKleurLinks7();
void shiftKleurLinks8();
void shiftKleurLinksBasis();
void pixelBuf2Kleur7();
void pixelBuf2Kleur8();
void pixelBuf2KleurBasis();
// Einde kleur functies

// Begin animatie functies
void sterTekst7(int kleur[], int aantal);
void sterTekst8(int kleur[], int aantal);
void sterTekstBasis(int kleur[], int aantal);
void ster7(int kleur[], int aantal);
void ster8(int kleur[], int aantal);
void sterBasis(int kleur[], int aantal);
void klok();
void fade7Uit();
void fade8Uit();
void fadeBasisUit();
void fade7In();
void fade8In();
void fadeBasisIn();
// Einde manipulatie functies


void pauze(int seconden);
// CTRL-C functies
void INThandler(int);

// Programma variabelen
int aantal = aantalLeds *12;			
int run = 1;															// "1" normaal cyclus verloop bij"0" naar stop programma met Terminate pigpio (ZEER BELANGRIJK)

int frameRate = 25;														// Aantal loops per seconde												
int maxHelderheid = 255;												// Maximum helderheid
int fade7 = 255;														// Totale helderheid bovenste 7 strips = maxHelderheid * fade7 bij functie fade7() en pixelBuf2Kleur7()
int fade8 = 255;														// Totale helderheid alle strips = maxHelderheid * fade8 bij functie fade8() en pixelBuf2Kleur8()
int fadeBasis = 255;													// Totale helderheid onderste strip = maxHelderheid * fadeBasis en printBasis()	
								


int v = 0;																// Wordt in sommige functies gebruikt om eerste cyclus te detecteren
int volgendeStap;														// Wordt 1 als functie eindpunt bereikt heeft kan gebruikt worden om naar volgende stap te gaan
int stapTeller = 0;	

char tekst1[] = {0x02, 0x08, 0x03, 0x08, 0x04, 0x08, 0x05, 0x08, 0x06, 0x08, 0x08, 0x08, 0x07};														
char tekst2[] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
char tekst3[] = {0x01};
int aantalPixels;
int aantalTmpPixels;
int aantalPixelsBasis;
int kleurShiftTeller = 0;
int rechtsTeller = 0;
int linksTeller = 0;
int rijTeller;
	
					
clock_t now, then, nu, vorig, now1, then1;								// Initialiseren clock voor cyclustijd meten

int main(int argc, char **argv)
{
	if (gpioInitialise() < 0)											// Initialiseren pigpio
	{
		printf ("gpioInitialise NOK \n");
	}
	else
	{
		printf ("gpioInitialise OK \n");
	}
	
	setupInOut();														// Setup input output
	setupLTC6904();														// Instellen externe oscillator (3.2Mhz)
	setupLCV512();														// Instellen seriele ram
	bufferInit();														// Buffers naar leds met 0x88 vullen (dit zijn voor de leds allemaal "0")
	data2Ram0();														// Schrijven data naar ram0 voor de totale lengte van aantalLeds met "0" vullen en op de volgende plaats "0xff"
																		// voor het triggerenvan 74HC74
	signal(SIGINT, INThandler);											// Initialiseren CTRL-C											
	srand(time(NULL));													// Initialiseren random generator
	now = then = then1 =clock();										// Tijd gelijk zetten
	
	int cyclusTijd = 1000000 / frameRate;								// Instellen minimum cyclustijd (1 ms) alleen nuttig bij heel weinig leds
	if (cyclusTijd < 1000){
		cyclusTijd = 1000;
	}
	
	while(run){
		
// Stel hier je show samen				
		switch (stapTeller){
			case 0:
				maxHelderheid = 77;
				frameRate = 25;
				kleurOpmaak7(kleurMatrixRood);
				print7Midden("NEOPIXEL");
				stapTeller++;
				break;
			case 1:
				pauze(2);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 2:
				print7("FUN");
				stapTeller++;
				break;
			case 3:
				shiftRechtsLinks7();
				pauze(5);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 4:
				print7Midden("RASPBERRY");
				stapTeller++;
				break;
			case 5:
				pauze(2);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 6:
				print7Midden("PI");
				pixelBuf2Kleur7();
				stapTeller++;
				break;
			case 7:
				pauze(2);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 8:
				kleurOpmaak7(kleurMatrixBlauw);
				print7("CYCLUSTIJD +/- 5 MS PER 8 * 100 LEDS   MAX = 8 * 1000 LEDS           ");
				stapTeller++;
				break;
			case 9:
				pauze(1);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 10:
				frameRate = 25;
				shiftLinks7();
				pauze(25);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 11:
				cls7();
				kleurOpmaakBasis(kleurMatrixRood);
				maxHelderheid = 255;
				printBasis(tekst3);
				frameRate = 150;
				stapTeller++;
				break;
			case 12:
				shiftRechtsLinksBasis();
				pauze(10);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 13:
				kleurOpmaakBasis(kleurMatrix3);
				frameRate = 200;
				stapTeller++;
				break;
			case 14:
				shiftRechtsLinksBasis();
				pauze(10);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 15:
				maxHelderheid = 77;
				kleurOpmaakBasis(kleurMatrix2);
				printBasis(tekst2);
				stapTeller++;
				break;
			case 16:
				frameRate = 35;
				shiftKleurLinksBasis();													
				pauze(5);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 17:
				kleurOpmaakBasis(kleurMatrix4);
				stapTeller++;
				break;
			case 18:
				shiftKleurLinksBasis();												
				pauze(10);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 19:
				clsBasis();
				kleurOpmaak7(kleurMatrixBlauw);
				stapTeller++;
				break;
			case 20:
				klok();
				sterTekst7(sterrenKleur1, 5);
				pauze(10);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 21:
				klok();
				fade7Uit();
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 22:
				kleurOpmaak7(kleurMatrix1);
				stapTeller++;
				break;
			case 23:
				klok();
				shiftKleurLinks7();
				fade7In();
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 24:
				klok();
				shiftKleurLinks7();
				sterTekst7(sterrenKleur1, 5);
				pauze(10);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 25:
				cls8();
				stapTeller++;
				break;
			case 26:
				cls8();
				ster8(sterrenKleur1, 5);
				pauze(5);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 27:
				cls8();
				ster8(sterrenKleur2, 10);
				pauze(5);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 28:
				cls8();
				ster8(sterrenKleur3, 15);
				pauze(5);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 29:
				cls8();
				ster8(sterrenKleur4, 20);
				pauze(5);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;	
			case 30:
				cls8();
				kleurOpmaak7(kleurMatrixBlauw);
				frameRate = 5;
				stapTeller++;
				break;
			case 31:
				zevenOmhoog("DAT WAS HET");
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 32:
				pauze(2);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 33:
				zevenOmhoog("GROETEN");
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 34:
				pauze(2);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 35:
				zevenOmlaag(tekst1);
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 36:
				pauze(2);
				frameRate = 50;
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 37:
				fade7Uit();
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller++;
				}
				break;
			case 38:
				pauze(2);
				fade7 = 255;
				if(volgendeStap == 1){
					volgendeStap = 0;
					stapTeller = 0;
				}
				break;
				
		}

		
// begin klaarzetten en overdracht data naar RAM en dan naar Leds
		cyclusTijd = 1000000 / frameRate;
		strip2Buffer8();												// Schrijf alle data als timing nibles naar de stripbuffers
		
		while (!gpioRead(OUT)){											// wachten op eind data overdracht naar leds
			gpioDelay(1);
		}
		
		stopData2Leds();												// Stop data overdracht van ram naar led
		data2Ram();														// Schrijf de strip.buffers naar de verschillende rams
				
		now = clock();													//wachten met begin nieuwe cyclus ingesteld door frame
		while (now - then < cyclusTijd){
			now = clock();
		}
		now = then = clock();
		
		ram2Leds();														// Start data overdracht van ram naar leds onder controle van externe oscillator (LTC6904)
	}
	stopNetjes();														// afsluiten programma bij Ctrl-C en run = 0
	
	return 0;
}

// Begin functies hoofdprogramma

void setupInOut()														// instellen van de verschillend GPIO's
{
	gpioSetMode (RAM_0, PI_OUTPUT);
	gpioSetPullUpDown(RAM_0, PI_PUD_UP);
	gpioWrite(RAM_0, 1);
	
	gpioSetMode (RAM_1, PI_OUTPUT);
	gpioSetPullUpDown(RAM_1, PI_PUD_UP);
	gpioWrite(RAM_1, 1);
	
	gpioSetMode (RAM_2, PI_OUTPUT);
	gpioSetPullUpDown(RAM_2, PI_PUD_UP);
	gpioWrite(RAM_2, 1);
	
	gpioSetMode (RAM_3, PI_OUTPUT);
	gpioSetPullUpDown(RAM_3, PI_PUD_UP);
	gpioWrite(RAM_3, 1);
	
	gpioSetMode (RAM_4, PI_OUTPUT);
	gpioSetPullUpDown(RAM_4, PI_PUD_UP);
	gpioWrite(RAM_4, 1);
	
	gpioSetMode (RAM_5, PI_OUTPUT);
	gpioSetPullUpDown(RAM_5, PI_PUD_UP);
	gpioWrite(RAM_5, 1);
	
	gpioSetMode (RAM_6, PI_OUTPUT);
	gpioSetPullUpDown(RAM_6, PI_PUD_UP);
	gpioWrite(RAM_6, 1);
	
	gpioSetMode (RAM_7, PI_OUTPUT);
	gpioSetPullUpDown(RAM_7, PI_PUD_UP);
	gpioWrite(RAM_7, 1);
	
	gpioSetMode (RAM_8, PI_OUTPUT);
	gpioSetPullUpDown(RAM_8, PI_PUD_UP);
	gpioWrite(RAM_8, 1);
	
	gpioSetMode (OSC, PI_OUTPUT);
	gpioSetPullUpDown(OSC, PI_PUD_DOWN);
	gpioWrite(OSC, 0);
	
	gpioSetMode (SET, PI_OUTPUT);
	gpioSetPullUpDown(SET, PI_PUD_UP);
	gpioWrite(SET, 0);
	
	gpioSetMode (RESET, PI_OUTPUT);
	gpioSetPullUpDown(RESET, PI_PUD_UP);
	gpioWrite(RESET, 0);
	
	gpioSetMode (OUT, PI_INPUT);
	gpioSetPullUpDown(OUT, PI_PUD_UP);
}


void setupLTC6904()
{
	char LTC6904_setup[] = {0xba, 0xc2};								// 0xba 0xc2 frequentie 3.2Mhz alleen clock out
	
	int LTC6904 = i2cOpen(1, 0x17, 0);										
	int controle = i2cWriteDevice(LTC6904, (char*)&LTC6904_setup, 2);	
	i2cClose(LTC6904);
	if (controle < 0){
		printf("LTC6904 communicatie fout \n");
	}
	else{
		printf("LTC6904 communicatie OK \n");
	}
}


void setupLCV512()
{
	LCV512 = spiOpen(0,25000000, 0xe0);									// instellen SPI poort op 25 Mhz vlgs spec's is 20Mhz maximum 
																		// maar 25 Mhz werkt ook
	gpioWrite(RAM_0, 0);
	gpioWrite(RAM_1, 0);
	gpioWrite(RAM_2, 0);
	gpioWrite(RAM_3, 0);
	gpioWrite(RAM_4, 0);
	gpioWrite(RAM_5, 0);
	gpioWrite(RAM_6, 0);
	gpioWrite(RAM_7, 0);
	gpioWrite(RAM_8, 0);
	
	spiWrite(LCV512, (char*)&LCV512_sequentieel, 2);					
	
	gpioWrite(RAM_0, 1);
	gpioWrite(RAM_1, 1);
	gpioWrite(RAM_2, 1);
	gpioWrite(RAM_3, 1);
	gpioWrite(RAM_4, 1);
	gpioWrite(RAM_5, 1);
	gpioWrite(RAM_6, 1);
	gpioWrite(RAM_7, 1);
	gpioWrite(RAM_8, 1);
}

void bufferInit()														// buffers die naar led's gestuurd worden instellen op 0 waardes
{																		// voor de 3 kleuren
	int x = 0;
	while (x < 12002){
		strip1Buffer[x] = 0x88;
		strip2Buffer[x] = 0x88;
		strip3Buffer[x] = 0x88;
		strip4Buffer[x] = 0x88;
		strip5Buffer[x] = 0x88;
		strip6Buffer[x] = 0x88;
		strip7Buffer[x] = 0x88;
		strip8Buffer[x] = 0x88;
		x++;
	}
}

void data2Ram0()
{
	gpioWrite(RAM_0, 0);												// initialiseren RAM_0 
	spiWrite(LCV512, (char*)&LCV512_write, 4);
	spiWrite(LCV512, (char*)&RAM0Buffer, aantal);
	spiWrite(LCV512, (char*)&LCV512_RAM0_stopbyte, 1);
	gpioWrite(RAM_0, 1);
}

void strip(char buffer[], char red[], char green[], char blue[])		// data uit de verschillende kleurbuffers samenbrengen in 1 buffer in de volgord groen rood blauw 
{																		// in eeen voor de ledstrips verstaanbare timing nibbles			
	int byte;															// zet "1" en "0" om naar timing nibles met behulp van de byteHoog byteMiddenHoog byteMiddenLaag
	int teller = 0;														// en byteLaag look-up tabellen
	int karakterTeller = 0;
	int einde = aantalLeds;
	if(aantalPixels > aantalLeds){
		einde = aantalPixels;
	}
	while (karakterTeller < einde){
		byte = green[karakterTeller];
		buffer[teller] = byteHoog[byte];
		teller ++;
		buffer[teller] = byteMiddenHoog[byte];
		teller ++;
		buffer[teller] = byteMiddenLaag[byte];
		teller ++;
		buffer[teller] = byteLaag[byte];
		teller ++;
		byte = red[karakterTeller];
		buffer[teller] = byteHoog[byte];
		teller ++;
		buffer[teller] = byteMiddenHoog[byte];
		teller ++;
		buffer[teller] = byteMiddenLaag[byte];
		teller ++;
		buffer[teller] = byteLaag[byte];
		teller ++;
		byte = blue[karakterTeller];
		buffer[teller] = byteHoog[byte];
		teller ++;
		buffer[teller] = byteMiddenHoog[byte];
		teller ++;
		buffer[teller] = byteMiddenLaag[byte];
		teller ++;
		buffer[teller] = byteLaag[byte];
		teller ++;
		karakterTeller ++;
	}
}

void strip2Buffer8()
{
	strip(strip1Buffer, strip1Red, strip1Green, strip1Blue);
	strip(strip2Buffer, strip2Red, strip2Green, strip2Blue);
	strip(strip3Buffer, strip3Red, strip3Green, strip3Blue);
	strip(strip4Buffer, strip4Red, strip4Green, strip4Blue);
	strip(strip5Buffer, strip5Red, strip5Green, strip5Blue);
	strip(strip6Buffer, strip6Red, strip6Green, strip6Blue);
	strip(strip7Buffer, strip7Red, strip7Green, strip7Blue);
	strip(strip8Buffer, strip8Red, strip8Green, strip8Blue);
}

void stopData2Leds()													// Alles naar beginstand zetten
{
	gpioWrite(SET, 0);												
	gpioWrite(RESET, 0);
	gpioWrite(OSC, 0);
	gpioWrite(RAM_0, 1);
	gpioWrite(RAM_1, 1);
	gpioWrite(RAM_2, 1);
	gpioWrite(RAM_3, 1);
	gpioWrite(RAM_4, 1);
	gpioWrite(RAM_5, 1);
	gpioWrite(RAM_6, 1);
	gpioWrite(RAM_7, 1);
	gpioWrite(RAM_8, 1);
}

void data2Ram()															// Data voor de strips in RAM schrijven
{
	gpioWrite(RAM_1, 0);											
	spiWrite(LCV512, (char*)&LCV512_write, 4);
	spiWrite(LCV512, (char*)&strip1Buffer, aantal);
	gpioWrite(RAM_1, 1);
	
	gpioWrite(RAM_2, 0);
	spiWrite(LCV512, (char*)&LCV512_write, 4);
	spiWrite(LCV512, (char*)&strip2Buffer, aantal);
	gpioWrite(RAM_2, 1);
	
	gpioWrite(RAM_3, 0);
	spiWrite(LCV512, (char*)&LCV512_write, 4);
	spiWrite(LCV512, (char*)&strip3Buffer, aantal);
	gpioWrite(RAM_3, 1);
	
	gpioWrite(RAM_4, 0);
	spiWrite(LCV512, (char*)&LCV512_write, 4);
	spiWrite(LCV512, (char*)&strip4Buffer, aantal);
	gpioWrite(RAM_4, 1);
	
	gpioWrite(RAM_5, 0);
	spiWrite(LCV512, (char*)&LCV512_write, 4);
	spiWrite(LCV512, (char*)&strip5Buffer, aantal);
	gpioWrite(RAM_5, 1);
	
	gpioWrite(RAM_6, 0);
	spiWrite(LCV512, (char*)&LCV512_write, 4);
	spiWrite(LCV512, (char*)&strip6Buffer, aantal);
	gpioWrite(RAM_6, 1);
	
	gpioWrite(RAM_7, 0);
	spiWrite(LCV512, (char*)&LCV512_write, 4);
	spiWrite(LCV512, (char*)&strip7Buffer, aantal);
	gpioWrite(RAM_7, 1);
	
	gpioWrite(RAM_8, 0);
	spiWrite(LCV512, (char*)&LCV512_write, 4);
	spiWrite(LCV512, (char*)&strip8Buffer, aantal);
	gpioWrite(RAM_8, 1);
}

void ram2Leds()															// RAM's initialiseren voor data overdracht naar strips
{
	gpioWrite(RAM_0, 0);											
	gpioWrite(RAM_1, 0);
	gpioWrite(RAM_2, 0);
	gpioWrite(RAM_3, 0);
	gpioWrite(RAM_4, 0);
	gpioWrite(RAM_5, 0);
	gpioWrite(RAM_6, 0);
	gpioWrite(RAM_7, 0);
	gpioWrite(RAM_8, 0);
	spiWrite(LCV512, (char*)&LCV512_read, 3);
	gpioWrite(SET, 1);													// initialiseren 74HC74 Q output wordt 0
	gpioWrite(RESET, 1);												// data van RAM's kan via 74HCT245 naar strips bij Q=1 sluit de 74HCT245
	gpioWrite(OSC, 1);													// start Oscillator overdracht naar strips begint nu
}

void stopNetjes()														// indien run = 0 via Ctrl-C of Error worden alle outputs juist gezet
{																		// en pigpio wordt afgesloten (zeer belangrijk)
	gpioWrite(SET, 0);													
	gpioWrite(RESET, 0);												
	gpioWrite(OSC, 0);
	gpioWrite(RAM_0, 1);
	gpioWrite(RAM_1, 1);
	gpioWrite(RAM_2, 1);
	gpioWrite(RAM_3, 1);
	gpioWrite(RAM_4, 1);
	gpioWrite(RAM_5, 1);
	gpioWrite(RAM_6, 1);
	gpioWrite(RAM_7, 1);
	gpioWrite(RAM_8, 1);
	gpioTerminate();
}
// einde functies hoofdprogramma

// begin functies tekst naar pixels

// fucties eindigend op 7 zijn voor de bovenste 7 strips (tekst)
// functies eindigend op 8 zijn voor alle strips
// functies eindigend op Basis zijn voor de 8ste strip

void print7(char tekst[])												// Zet de tekst om naar pixels met behulp van alfabetArray look-up tabel
{
	cls7();
	aantalPixels = 0;
	int lengte = strlen(tekst);
	if(lengte > 300){
		printf("Aantal karakters > 300 %d \n", lengte);
		run = 0;
	}
	int pixels;
	int byte;
	int i = 0;
	int j;
	while(i < lengte){
		byte = tekst[i];
		pixels = alfabetArray[byte][0][0];
		j = 0;
		while(j < pixels){
			pixelBuf1[aantalPixels + j] = alfabetArray[byte][1][j];
			pixelBuf2[aantalPixels + j] = alfabetArray[byte][2][j];
			pixelBuf3[aantalPixels + j] = alfabetArray[byte][3][j];
			pixelBuf4[aantalPixels + j] = alfabetArray[byte][4][j];
			pixelBuf5[aantalPixels + j] = alfabetArray[byte][5][j];
			pixelBuf6[aantalPixels + j] = alfabetArray[byte][6][j];
			pixelBuf7[aantalPixels + j] = alfabetArray[byte][7][j];
			j++;
		}
		if(byte > 31){
			aantalPixels = aantalPixels + pixels;
			pixelBuf1[aantalPixels] = 0;
			pixelBuf2[aantalPixels] = 0;
			pixelBuf3[aantalPixels] = 0;
			pixelBuf4[aantalPixels] = 0;
			pixelBuf5[aantalPixels] = 0;
			pixelBuf6[aantalPixels] = 0;
			pixelBuf7[aantalPixels] = 0;
			aantalPixels = aantalPixels + 1;
			pixelBuf1[aantalPixels] = 0;
			pixelBuf2[aantalPixels] = 0;
			pixelBuf3[aantalPixels] = 0;
			pixelBuf4[aantalPixels] = 0;
			pixelBuf5[aantalPixels] = 0;
			pixelBuf6[aantalPixels] = 0;
			pixelBuf7[aantalPixels] = 0;
		}
		else{
			aantalPixels = aantalPixels + pixels;
		}
		i++;
	}
	pixelBuf2Kleur7();
}

void print8(char tekst[])
{
	cls8();
	aantalPixels = 0;
	int lengte = strlen(tekst);
	if(lengte > 300){
		printf("Aantal karakters > 300 %d \n", lengte);
		run = 0;
	}
	int pixels;
	int byte;
	int i = 0;
	int j;
	while(i < lengte){
		byte = tekst[i];
		pixels = alfabetArray[byte][0][0];
		j = 0;
		while(j < pixels){
			pixelBuf1[aantalPixels + j] = alfabetArray[byte][1][j];
			pixelBuf2[aantalPixels + j] = alfabetArray[byte][2][j];
			pixelBuf3[aantalPixels + j] = alfabetArray[byte][3][j];
			pixelBuf4[aantalPixels + j] = alfabetArray[byte][4][j];
			pixelBuf5[aantalPixels + j] = alfabetArray[byte][5][j];
			pixelBuf6[aantalPixels + j] = alfabetArray[byte][6][j];
			pixelBuf7[aantalPixels + j] = alfabetArray[byte][7][j];
			pixelBuf8[aantalPixels + j] = alfabetArray[byte][8][j];
			j++;
		}
		if(byte > 31){
			aantalPixels = aantalPixels + pixels;
			pixelBuf1[aantalPixels] = 0;
			pixelBuf2[aantalPixels] = 0;
			pixelBuf3[aantalPixels] = 0;
			pixelBuf4[aantalPixels] = 0;
			pixelBuf5[aantalPixels] = 0;
			pixelBuf6[aantalPixels] = 0;
			pixelBuf7[aantalPixels] = 0;
			pixelBuf8[aantalPixels] = 0;
			aantalPixels = aantalPixels + 1;
			pixelBuf1[aantalPixels] = 0;
			pixelBuf2[aantalPixels] = 0;
			pixelBuf3[aantalPixels] = 0;
			pixelBuf4[aantalPixels] = 0;
			pixelBuf5[aantalPixels] = 0;
			pixelBuf6[aantalPixels] = 0;
			pixelBuf7[aantalPixels] = 0;
			pixelBuf8[aantalPixels] = 0;
		}
		else{
			aantalPixels = aantalPixels + pixels;
		}
		i++;
	}
	pixelBuf2Kleur8 ();
}

void printBasis(char tekst[])
{
	clsBasis();
	aantalPixelsBasis = 0;
	int lengte = strlen(tekst);
	if(lengte > 300){
		printf("Aantal karakters > 300 %d \n", lengte);
		run = 0;
	}
	int pixels;
	int byte;
	int i = 0;
	int j;
	while(i < lengte){
		byte = tekst[i];
		pixels = alfabetArray[byte][0][0];
		j = 0;
		while(j < pixels){
			pixelBuf8[aantalPixelsBasis + j] = alfabetArray[byte][8][j];
			j++;
		}
		if(byte > 31){
			aantalPixelsBasis = aantalPixelsBasis + pixels;
			pixelBuf8[aantalPixelsBasis] = 0;
			aantalPixelsBasis++;
			pixelBuf8[aantalPixelsBasis] = 0;
		}
		else{
			aantalPixelsBasis = aantalPixelsBasis + pixels;
		}
		i++;
	}
	pixelBuf2KleurBasis();
}

void print7Midden(char tekst[])											// Zet tekst in het midden van de display aantal pixels mag echter niet groter zijn dan het aantal leds
{
	cls7();
	aantalPixels = 0;
	int lengte = strlen(tekst);
	int pixels;
	int byte;
	int i = 0;
	int j;
	while(i < lengte){
		byte = tekst[i];
		pixels = alfabetArray[byte][0][0];
		j = 0;
		while(j < pixels){
			pixelBuf1[aantalPixels + j] = alfabetArray[byte][1][j];
			pixelBuf2[aantalPixels + j] = alfabetArray[byte][2][j];
			pixelBuf3[aantalPixels + j] = alfabetArray[byte][3][j];
			pixelBuf4[aantalPixels + j] = alfabetArray[byte][4][j];
			pixelBuf5[aantalPixels + j] = alfabetArray[byte][5][j];
			pixelBuf6[aantalPixels + j] = alfabetArray[byte][6][j];
			pixelBuf7[aantalPixels + j] = alfabetArray[byte][7][j];
			j++;
		}
		aantalPixels = aantalPixels + pixels + 1;
		pixelBuf1[aantalPixels] = 0;
		pixelBuf2[aantalPixels] = 0;
		pixelBuf3[aantalPixels] = 0;
		pixelBuf4[aantalPixels] = 0;
		pixelBuf5[aantalPixels] = 0;
		pixelBuf6[aantalPixels] = 0;
		pixelBuf7[aantalPixels] = 0;
		i++;
	}
	if(aantalPixels > aantalLeds){
		run = 0;
		printf("aantal pixels > aantal leds %d  %d \n", aantalPixels, aantalLeds);
	}
	int aantal = (aantalLeds - aantalPixels) / 2;
	int teller = 0;
	while(teller < aantal){
		shiftRechts7();
		teller ++;
	}
	pixelBuf2Kleur7();
}

void print8Midden(char tekst[])											
{
	cls8();
	aantalPixels = 0;
	int lengte = strlen(tekst);
	int pixels;
	int byte;
	int i = 0;
	int j;
	while(i < lengte){
		byte = tekst[i];
		pixels = alfabetArray[byte][0][0];
		j = 0;
		while(j < pixels){
			pixelBuf1[aantalPixels + j] = alfabetArray[byte][1][j];
			pixelBuf2[aantalPixels + j] = alfabetArray[byte][2][j];
			pixelBuf3[aantalPixels + j] = alfabetArray[byte][3][j];
			pixelBuf4[aantalPixels + j] = alfabetArray[byte][4][j];
			pixelBuf5[aantalPixels + j] = alfabetArray[byte][5][j];
			pixelBuf6[aantalPixels + j] = alfabetArray[byte][6][j];
			pixelBuf7[aantalPixels + j] = alfabetArray[byte][7][j];
			pixelBuf8[aantalPixels + j] = alfabetArray[byte][8][j];
			j++;
		}
		aantalPixels = aantalPixels + pixels + 1;
		pixelBuf1[aantalPixels] = 0;
		pixelBuf2[aantalPixels] = 0;
		pixelBuf3[aantalPixels] = 0;
		pixelBuf4[aantalPixels] = 0;
		pixelBuf5[aantalPixels] = 0;
		pixelBuf6[aantalPixels] = 0;
		pixelBuf7[aantalPixels] = 0;
		pixelBuf8[aantalPixels] = 0;
		i++;
	}
	if(aantalPixels > aantalLeds){
		run = 0;
		printf("aantal pixels > aantal leds %d  %d \n", aantalPixels, aantalLeds);
	}
	int aantal = (aantalLeds - aantalPixels) / 2;
	int teller = 0;
	while(teller < aantal){
		shiftRechts7();
		teller ++;
	}
	pixelBuf2Kleur8();
}

void printBasisMidden(char tekst[])										
{
	clsBasis();
	aantalPixelsBasis = 0;
	int lengte = strlen(tekst);
	int pixels;
	int byte;
	int i = 0;
	int j;
	while(i < lengte){
		byte = tekst[i];
		pixels = alfabetArray[byte][0][0];
		j = 0;
		while(j < pixels){
			pixelBuf8[aantalPixelsBasis + j] = alfabetArray[byte][8][j];
			j++;
		}
		aantalPixelsBasis = aantalPixelsBasis + pixels + 1;
		pixelBuf8[aantalPixels] = 0;
		i++;
	}
	if(aantalPixelsBasis > aantalLeds){
		run = 0;
		printf("aantal pixels > aantal leds %d  %d \n", aantalPixelsBasis, aantalLeds);
	}
	int aantal = (aantalLeds - aantalPixelsBasis) / 2;
	int teller = 0;
	while(teller < aantal){
		shiftRechts7();
		teller ++;
	}
	pixelBuf2KleurBasis();
}


void pixel2Tmp7(char tekst[])											// Zet tekst om naar pixels wordt gebruikt in omhoog en omlaag draaien
{
	int lengte = strlen(tekst);
	if(lengte > 300){
		printf("Aantal karakters > 300 %d \n", lengte);
		run = 0;
	}
	int pixels = 0;
	int byte;
	int i = 0;
	while(i < lengte){
		byte = tekst[i];
		pixels = pixels + alfabetArray[byte][0][0];
		if(byte > 31){
			pixels++;
		}
		i++;
	}
	pixels--;
	if(pixels > aantalLeds){
		printf("Aantal pixels > aantal leds %d  %d", pixels, aantalLeds);
		run = 0;
	}
	pixels = (aantalLeds - pixels) / 2;
	i = 0;
	while(i < pixels){
		pixelTmp[0][i] = 0;
		pixelTmp[1][i] = 0;
		pixelTmp[2][i] = 0;
		pixelTmp[3][i] = 0;
		pixelTmp[4][i] = 0;
		pixelTmp[5][i] = 0;
		pixelTmp[6][i] = 0;
		i++;
	}
	aantalTmpPixels = pixels;
	i = 0;
	int j;
	while(i < lengte){
		byte = tekst[i];
		pixels = alfabetArray[byte][0][0];
		j = 0;
		while(j < pixels){
			pixelTmp[0][aantalTmpPixels + j] = alfabetArray[byte][1][j];
			pixelTmp[1][aantalTmpPixels + j] = alfabetArray[byte][2][j];
			pixelTmp[2][aantalTmpPixels + j] = alfabetArray[byte][3][j];
			pixelTmp[3][aantalTmpPixels + j] = alfabetArray[byte][4][j];
			pixelTmp[4][aantalTmpPixels + j] = alfabetArray[byte][5][j];
			pixelTmp[5][aantalTmpPixels + j] = alfabetArray[byte][6][j];
			pixelTmp[6][aantalTmpPixels + j] = alfabetArray[byte][7][j];
			j++;
		}
		if(byte > 31){
			aantalTmpPixels = aantalTmpPixels + pixels;
			pixelTmp[0][aantalTmpPixels] = 0;
			pixelTmp[1][aantalTmpPixels] = 0;
			pixelTmp[2][aantalTmpPixels] = 0;
			pixelTmp[3][aantalTmpPixels] = 0;
			pixelTmp[4][aantalTmpPixels] = 0;
			pixelTmp[5][aantalTmpPixels] = 0;
			pixelTmp[6][aantalTmpPixels] = 0;
			aantalTmpPixels = aantalTmpPixels + 1;
			pixelTmp[0][aantalTmpPixels] = 0;
			pixelTmp[1][aantalTmpPixels] = 0;
			pixelTmp[2][aantalTmpPixels] = 0;
			pixelTmp[3][aantalTmpPixels] = 0;
			pixelTmp[4][aantalTmpPixels] = 0;
			pixelTmp[5][aantalTmpPixels] = 0;
			pixelTmp[6][aantalTmpPixels] = 0;
		}
		else{
			aantalTmpPixels = aantalTmpPixels + pixels;
		}
		i++;
	}
}

void pixel2Tmp8(char tekst[])
{
	aantalTmpPixels = 0;
	int lengte = strlen(tekst);
	if(lengte > 300){
		printf("Aantal karakters > 300 %d \n", lengte);
		run = 0;
	}
	int pixels;
	int byte;
	int i = 0;
	while(i < lengte){
		byte = tekst[i];
		pixels = pixels + alfabetArray[byte][0][0];
		if(byte > 31){
			pixels++;
		}
		i++;
	}
	pixels--;
	if(pixels > aantalLeds){
		printf("Aantal pixels > aantal leds %d  %d", pixels, aantalLeds);
		run = 0;
	}
	pixels = (aantalLeds - pixels) / 2;
	i = 0;
	while(i < pixels){
		pixelTmp[0][i] = 0;
		pixelTmp[1][i] = 0;
		pixelTmp[2][i] = 0;
		pixelTmp[3][i] = 0;
		pixelTmp[4][i] = 0;
		pixelTmp[5][i] = 0;
		pixelTmp[6][i] = 0;
		pixelTmp[7][i] = 0;
		i++;
	}
	aantalTmpPixels = pixels;
	int j;
	while(i < lengte){
		byte = tekst[i];
		pixels = alfabetArray[byte][0][0];
		j = 0;
		while(j < pixels){
			pixelTmp[0][aantalTmpPixels + j] = alfabetArray[byte][1][j];
			pixelTmp[1][aantalTmpPixels + j] = alfabetArray[byte][2][j];
			pixelTmp[2][aantalTmpPixels + j] = alfabetArray[byte][3][j];
			pixelTmp[3][aantalTmpPixels + j] = alfabetArray[byte][4][j];
			pixelTmp[4][aantalTmpPixels + j] = alfabetArray[byte][5][j];
			pixelTmp[5][aantalTmpPixels + j] = alfabetArray[byte][6][j];
			pixelTmp[6][aantalTmpPixels + j] = alfabetArray[byte][7][j];
			pixelTmp[7][aantalTmpPixels + j] = alfabetArray[byte][8][j];
			j++;
		}
		if(byte > 31){
			aantalTmpPixels = aantalTmpPixels + pixels;
			pixelTmp[0][aantalTmpPixels] = 0;
			pixelTmp[1][aantalTmpPixels] = 0;
			pixelTmp[2][aantalTmpPixels] = 0;
			pixelTmp[3][aantalTmpPixels] = 0;
			pixelTmp[4][aantalTmpPixels] = 0;
			pixelTmp[5][aantalTmpPixels] = 0;
			pixelTmp[6][aantalTmpPixels] = 0;
			pixelTmp[7][aantalTmpPixels] = 0;
			aantalTmpPixels = aantalTmpPixels + 1;
			pixelTmp[0][aantalTmpPixels] = 0;
			pixelTmp[1][aantalTmpPixels] = 0;
			pixelTmp[2][aantalTmpPixels] = 0;
			pixelTmp[3][aantalTmpPixels] = 0;
			pixelTmp[4][aantalTmpPixels] = 0;
			pixelTmp[5][aantalTmpPixels] = 0;
			pixelTmp[6][aantalTmpPixels] = 0;
			pixelTmp[7][aantalTmpPixels] = 0;
		}
		else{
			aantalTmpPixels = aantalTmpPixels + pixels;
		}
		i++;
	}
}

void cls7()																// Wissen van de verschillende buffers
{
	int i = 0;
	while(i < 2100){
		pixelBuf1[i] = 0x00;
		pixelBuf2[i] = 0x00;
		pixelBuf3[i] = 0x00;
		pixelBuf4[i] = 0x00;
		pixelBuf5[i] = 0x00;
		pixelBuf6[i] = 0x00;
		pixelBuf7[i] = 0x00;
		i++;
	}
	pixelBuf2Kleur7();
}

void cls8()
{
	int i = 0;
	while(i < 2100){
		pixelBuf1[i] = 0x00;
		pixelBuf2[i] = 0x00;
		pixelBuf3[i] = 0x00;
		pixelBuf4[i] = 0x00;
		pixelBuf5[i] = 0x00;
		pixelBuf6[i] = 0x00;
		pixelBuf7[i] = 0x00;
		pixelBuf8[i] = 0x00;
		i++;
	}
	pixelBuf2Kleur8();
}
	
void clsBasis()
{
	int i = 0;
	while(i < 2100){
		pixelBuf8[i] = 0x00;
		i++;
	}
	pixelBuf2KleurBasis();
}

void cls1ste()
{
	int i = 0;
	while(i < 2100){
		pixelBuf1[i] = 0x00;
		i++;
	}
}

void cls7de()
{
	int i = 0;
	while(i < 2100){
		pixelBuf7[i] = 0x00;
		i++;
	}
}	

void cls8ste()
{
	int i = 0;
	while(i < 2100){
		pixelBuf8[i] = 0x00;
		i++;
	}
}
// einde functies tekst naar pixels


// begin tekst manipulatie functies
void shiftRechts(char buffer[])											// Hoofd functie voor shift naar rechta van de pixels
{
	int einde = aantalLeds;
	if(aantalPixels > aantalLeds){
		einde = aantalPixels;
	}
	einde = einde - 1;
	int temp = buffer[einde];
	while(einde > 0){
		buffer[einde] = buffer[einde - 1];
		einde --;
	}
	buffer[0] = temp;
}

void shiftRechts7()														// Shift display nnar rechts
{
	shiftRechts(pixelBuf1);
	shiftRechts(pixelBuf2);
	shiftRechts(pixelBuf3);
	shiftRechts(pixelBuf4);
	shiftRechts(pixelBuf5);
	shiftRechts(pixelBuf6);
	shiftRechts(pixelBuf7);
	pixelBuf2Kleur7();
	}
	
void shiftRechts8()
{
	shiftRechts(pixelBuf1);
	shiftRechts(pixelBuf2);
	shiftRechts(pixelBuf3);
	shiftRechts(pixelBuf4);
	shiftRechts(pixelBuf5);
	shiftRechts(pixelBuf6);
	shiftRechts(pixelBuf7);
	shiftRechts(pixelBuf8);
	pixelBuf2Kleur8();
}
	
void shiftRechtsBasis()
{
	shiftRechts(pixelBuf8);
	pixelBuf2KleurBasis();
}

void shiftLinks(char buffer[])											// Hoofd functie voor shift naar links van de pixels
{
	int einde = aantalLeds;
	if(aantalPixels > aantalLeds){
		einde = aantalPixels;
	}
	int teller = 0;
	int temp = buffer[0];
	einde = einde - 1;
	while(teller < einde){
		buffer[teller] = buffer[teller + 1];
		teller ++;
	}
	buffer[einde] = temp;
}

void shiftLinks7()														// Shift display naar links
{
	shiftLinks(pixelBuf1);
	shiftLinks(pixelBuf2);
	shiftLinks(pixelBuf3);
	shiftLinks(pixelBuf4);
	shiftLinks(pixelBuf5);
	shiftLinks(pixelBuf6);
	shiftLinks(pixelBuf7);
	pixelBuf2Kleur7();
}

void shiftLinks8()
{
	shiftLinks(pixelBuf1);
	shiftLinks(pixelBuf2);
	shiftLinks(pixelBuf3);
	shiftLinks(pixelBuf4);
	shiftLinks(pixelBuf5);
	shiftLinks(pixelBuf6);
	shiftLinks(pixelBuf7);
	shiftLinks(pixelBuf8);
	pixelBuf2Kleur8();
}

void shiftLinksBasis()
{
	shiftLinks(pixelBuf8);
	pixelBuf2KleurBasis();
}

void shiftRechtsLinks7()												// Shift rechts links aantal pixels mag niet groter zij dan aantal leds
{
	now1 = clock();
	if((now1 - then1) > 1200000){
		rechtsTeller = 0;
		linksTeller = 0;
	}
	now1 = then1 = clock();
	int terug = aantalLeds - aantalPixels;
	if(rechtsTeller < terug){
		shiftRechts7();
		rechtsTeller ++;
	}
	if(rechtsTeller == terug){
		if(linksTeller < terug){
			shiftLinks7();
			linksTeller ++;
		}
	}
	if(linksTeller == terug){
		linksTeller = 0;
		rechtsTeller = 0;
	}
	pixelBuf2Kleur7();
}

void shiftRechtsLinks8()
{
	now1 = clock();
	if((now1 - then1) > 1200000){
		rechtsTeller = 0;
		linksTeller = 0;
	}
	now1 = then1 = clock();
	int terug = aantalLeds - aantalPixels;
	if(rechtsTeller < terug){
		shiftRechts8();
		rechtsTeller ++;
	}
	if(rechtsTeller == terug){
		if(linksTeller < terug){
			shiftLinks8();
			linksTeller ++;
		}
	}
	if(linksTeller == terug){
		linksTeller = 0;
		rechtsTeller = 0;
	}
	pixelBuf2Kleur8();
}

void shiftRechtsLinksBasis()
{
	now1 = clock();
	if((now1 - then1) > 1200000){
		rechtsTeller = 0;
		linksTeller = 0;
	}
	now1 = then1 = clock();
	int terug = aantalLeds - aantalPixelsBasis;
	if(rechtsTeller < terug){
		shiftRechtsBasis();
		rechtsTeller ++;
	}
	if(rechtsTeller == terug){
		if(linksTeller < terug){
			shiftLinksBasis();
			linksTeller ++;
		}
	}
	if(linksTeller == terug){
		linksTeller = 0;
		rechtsTeller = 0;
	}
	pixelBuf2KleurBasis();
}

void stringCopy(char doel[], char bron[], int aantal)					// Copy functie van string naat string
{
	int i = 0;
	while(i < aantal){
		doel[i] = bron[i];
		i++;
	}
}

void omhoog7()															// Zet de tekst eerst in het middenen schuift dan omhoog
{																		// aantal pixels mag meer zijn dan aantal leds
	stringCopy(pixelBuf1, pixelBuf2, aantalPixels);						// bij einde int volgendeStap = 1
	stringCopy(pixelBuf2, pixelBuf3, aantalPixels);
	stringCopy(pixelBuf3, pixelBuf4, aantalPixels);
	stringCopy(pixelBuf4, pixelBuf5, aantalPixels);
	stringCopy(pixelBuf5, pixelBuf6, aantalPixels);
	stringCopy(pixelBuf6, pixelBuf7, aantalPixels);
	cls7de();
	stringCopy(pixelBuf7, pixelTmp[rijTeller], aantalTmpPixels);
}

void omhoog8()
{
	stringCopy(pixelBuf1, pixelBuf2, aantalPixels);
	stringCopy(pixelBuf2, pixelBuf3, aantalPixels);
	stringCopy(pixelBuf3, pixelBuf4, aantalPixels);
	stringCopy(pixelBuf4, pixelBuf5, aantalPixels);
	stringCopy(pixelBuf5, pixelBuf6, aantalPixels);
	stringCopy(pixelBuf6, pixelBuf7, aantalPixels);
	stringCopy(pixelBuf7, pixelBuf8, aantalPixels);
	cls8ste();
	stringCopy(pixelBuf8, pixelTmp[rijTeller], aantalTmpPixels);
}

void zevenOmhoog(char tekst[])
{
	if(v == 0){
			pixel2Tmp7(tekst);
			if(aantalTmpPixels > aantalPixels){
				aantalPixels = aantalTmpPixels;
			}
			v = 1;
			rijTeller = 0;
		}
		if(rijTeller < 7){
			omhoog7();
			rijTeller ++;
			
		}
		else{
			aantalPixels = aantalTmpPixels;
			v = 0;
			volgendeStap = 1;
			
		}
		pixelBuf2Kleur7();
}

void achtOmhoog(char tekst[])
{
	if(v == 0){
			pixel2Tmp8(tekst);
			if(aantalTmpPixels > aantalPixels){
				aantalPixels = aantalTmpPixels;
			}
			v = 1;
			rijTeller = 0;
		}
		if(rijTeller < 8){
			omhoog8();
			rijTeller ++;
			
		}
		else{
			aantalPixels = aantalTmpPixels;
			v = 0;
			volgendeStap = 1;
			
		}
		pixelBuf2Kleur8();
}

void omlaag7()															// Zet tekst in het midden en schuift dan omlaag
{																		// aatal pixels mag niet groter dan aantal leds
	stringCopy(pixelBuf7, pixelBuf6, aantalPixels);						// bij einde int volgendeStap = 1
	stringCopy(pixelBuf6, pixelBuf5, aantalPixels);
	stringCopy(pixelBuf5, pixelBuf4, aantalPixels);
	stringCopy(pixelBuf4, pixelBuf3, aantalPixels);
	stringCopy(pixelBuf3, pixelBuf2, aantalPixels);
	stringCopy(pixelBuf2, pixelBuf1, aantalPixels);
	cls1ste();
	stringCopy(pixelBuf1, pixelTmp[6 - rijTeller], aantalTmpPixels);
}

void omlaag8()
{
	stringCopy(pixelBuf8, pixelBuf7, aantalPixels);
	stringCopy(pixelBuf7, pixelBuf6, aantalPixels);
	stringCopy(pixelBuf6, pixelBuf5, aantalPixels);
	stringCopy(pixelBuf5, pixelBuf4, aantalPixels);
	stringCopy(pixelBuf4, pixelBuf3, aantalPixels);
	stringCopy(pixelBuf3, pixelBuf2, aantalPixels);
	stringCopy(pixelBuf2, pixelBuf1, aantalPixels);
	cls1ste();
	stringCopy(pixelBuf1, pixelTmp[6 - rijTeller], aantalTmpPixels);
}

void zevenOmlaag(char tekst[])
{
	if(v == 0){
			pixel2Tmp7(tekst);
			if(aantalTmpPixels > aantalPixels){
				aantalPixels = aantalTmpPixels;
			}
			v = 1;
			rijTeller = 0;
		}
		if(rijTeller < 7){
			omlaag7();
			rijTeller ++;
			
		}
		else{
			aantalPixels = aantalTmpPixels;
			v = 0;
			volgendeStap = 1;
			
		}
		pixelBuf2Kleur7();
}

void achtOmlaag(char tekst[])
{
	if(v == 0){
			pixel2Tmp8(tekst);
			if(aantalTmpPixels > aantalPixels){
				aantalPixels = aantalTmpPixels;
			}
			v = 1;
			rijTeller = 0;
		}
		if(rijTeller < 8){
			omlaag8();
			rijTeller ++;
			
		}
		else{
			aantalPixels = aantalTmpPixels;
			v = 0;
			volgendeStap = 1;
			
		}
		pixelBuf2Kleur8();
}		
// einde tekst manipulatie functies

/* Opmerking bij kleuropmaak
 * het zijn niet de karakters die een kleur hebben, het is het canvas dat de kleur heeft
 * het karakter bepaalt alleen of de led brandt of niet.
 * tekst met elk karakter een andere kleur is zo dus niet mogelijk
 * */

//begin kleur functies
void kleurOpmaak(float kleurMatrix[8][3][3], char rood[], char groen[], char blauw[], int rij) // Omzetten van de kleurMatrixen naar kleurwaardes voor elke led
{
	float roodBegin;
	float roodEind;
	float roodVerdeling;
	float groenBegin;
	float groenEind;
	float groenVerdeling;
	float blauwBegin;
	float blauwEind;
	float blauwVerdeling;
	float roodStap;
	float groenStap;
	float blauwStap;
	int teller;
	float hoogR;
	float laagR;
	float hoogG;
	float laagG;
	float hoogB;
	float laagB;
	
	roodBegin = kleurMatrix[rij][0][0];
	roodEind = kleurMatrix[rij][0][1];
	roodVerdeling = kleurMatrix[rij][0][2] * 2;
	hoogR = roodEind;
	laagR = roodBegin;
	if(roodEind < roodBegin){
		hoogR = roodBegin;
		laagR = roodEind;
	}
	groenBegin = kleurMatrix[rij][1][0];
	groenEind = kleurMatrix[rij][1][1];
	groenVerdeling = kleurMatrix[rij][1][2] * 2;
	hoogG = groenEind;
	laagG = groenBegin;
	if(groenEind < groenBegin){
		hoogG = groenBegin;
		laagG = groenEind;
	}
	blauwBegin = kleurMatrix[rij][2][0];
	blauwEind = kleurMatrix[rij][2][1];
	blauwVerdeling = kleurMatrix[rij][2][2] * 2;
	hoogB = blauwEind;
	laagB = blauwBegin;
	if(blauwEind < blauwBegin){
		hoogB = blauwBegin;
		laagB = blauwEind;
	}
	roodStap = (roodEind - roodBegin) * roodVerdeling / aantalLeds;
	groenStap = (groenEind - groenBegin) * groenVerdeling / aantalLeds;
	blauwStap = (blauwEind - blauwBegin) * blauwVerdeling/ aantalLeds;
	teller = 0;
	while(teller < aantalLeds){
		rood[teller] = roodBegin;
		groen[teller] = groenBegin;
		blauw[teller] = blauwBegin;
		roodBegin = roodBegin + roodStap;
		if((roodBegin < laagR) || (roodBegin > hoogR)){
			roodStap = roodStap * -1;
			roodBegin = roodBegin + roodStap;
		}
		groenBegin = groenBegin + groenStap;
		if((groenBegin < laagG) || (groenBegin > hoogG)){
			groenStap = groenStap * -1;
			groenBegin  = groenBegin + groenStap;
		}
		blauwBegin = blauwBegin + blauwStap;
		if((blauwBegin < laagB) || (blauwBegin > hoogB)){
			blauwStap = blauwStap * -1;
			blauwBegin = blauwBegin + blauwStap;
		}
		teller ++;
	}
}

void kleurOpmaak7(float matrix[8][3][3])
{
	int rij = 0;
	kleurOpmaak(matrix, rood1, groen1, blauw1, rij);
	rij ++;
	kleurOpmaak(matrix, rood2, groen2, blauw2, rij);
	rij ++;
	kleurOpmaak(matrix, rood3, groen3, blauw3, rij);
	rij ++;
	kleurOpmaak(matrix, rood4, groen4, blauw4, rij);
	rij ++;
	kleurOpmaak(matrix, rood5, groen5, blauw5, rij);
	rij ++;
	kleurOpmaak(matrix, rood6, groen6, blauw6, rij);
	rij ++;
	kleurOpmaak(matrix, rood7, groen7, blauw7, rij);
}

void kleurOpmaak8(float matrix[8][3][3])
{
	int rij = 0;
	kleurOpmaak(matrix, rood1, groen1, blauw1, rij);
	rij ++;
	kleurOpmaak(matrix, rood2, groen2, blauw2, rij);
	rij ++;
	kleurOpmaak(matrix, rood3, groen3, blauw3, rij);
	rij ++;
	kleurOpmaak(matrix, rood4, groen4, blauw4, rij);
	rij ++;
	kleurOpmaak(matrix, rood5, groen5, blauw5, rij);
	rij ++;
	kleurOpmaak(matrix, rood6, groen6, blauw6, rij);
	rij ++;
	kleurOpmaak(matrix, rood7, groen7, blauw7, rij);
	rij ++;
	kleurOpmaak(matrix, rood8, groen8, blauw8, rij);
}

void kleurOpmaakBasis(float matrix[8][3][3])
{
	kleurOpmaak(matrix, rood8, groen8, blauw8, 7);
}

void shiftKleurLinks(char rbuf[], char gbuf[], char bbuf[])				
{
	int teller = 0;
	int temp1;
	int temp2;
	int temp3;
	temp1 = rbuf[0];
	temp2 = gbuf[0];
	temp3= bbuf[0];
	while(teller < aantalLeds - 1){
		rbuf[teller] = rbuf[teller + 1];
		gbuf[teller] = gbuf[teller + 1];
		bbuf[teller] = bbuf[teller + 1];
		teller ++;
	}
	rbuf[aantalLeds - 1] = temp1;
	gbuf[aantalLeds - 1] = temp2;
	bbuf[aantalLeds - 1] = temp3;
}

void shiftKleurLinks7()													// Schuift de canvas kleuren naar links
{
	shiftKleurLinks(rood1, groen1, blauw1);
	shiftKleurLinks(rood2, groen2, blauw2);
	shiftKleurLinks(rood3, groen3, blauw3);
	shiftKleurLinks(rood4, groen4, blauw4);
	shiftKleurLinks(rood5, groen5, blauw5);
	shiftKleurLinks(rood6, groen6, blauw6);
	shiftKleurLinks(rood7, groen7, blauw7);
	pixelBuf2Kleur7();
}

void shiftKleurLinks8()
{
	shiftKleurLinks(rood1, groen1, blauw1);
	shiftKleurLinks(rood2, groen2, blauw2);
	shiftKleurLinks(rood3, groen3, blauw3);
	shiftKleurLinks(rood4, groen4, blauw4);
	shiftKleurLinks(rood5, groen5, blauw5);
	shiftKleurLinks(rood6, groen6, blauw6);
	shiftKleurLinks(rood7, groen7, blauw7);
	shiftKleurLinks(rood8, groen8, blauw8);
	pixelBuf2Kleur8();
}

void shiftKleurLinksBasis()
{
	shiftKleurLinks(rood8, groen8, blauw8);
	pixelBuf2KleurBasis();
}

void pixelBuf2Kleur7()													// Indien er op die led een pixel staat bepaald door pixel2Buf de kleurwaardes invullen
{
	int i = 0;
	int einde = aantalLeds;
	if(aantalPixels > aantalLeds){
		einde = aantalPixels;
	}
	while(i < einde){
		strip1Red[i] = rood1[i] * pixelBuf1[i] * maxHelderheid * fade7 / 65025 ;
		strip1Green[i] = groen1[i] * pixelBuf1[i] * maxHelderheid * fade7 / 65025;
		strip1Blue[i] = blauw1[i] * pixelBuf1[i] * maxHelderheid * fade7 / 65025;
		strip2Red[i] = rood2[i] * pixelBuf2[i] * maxHelderheid * fade7 / 65025;
		strip2Green[i] = groen2[i] * pixelBuf2[i] * maxHelderheid * fade7 / 65025;
		strip2Blue[i] = blauw2[i] * pixelBuf2[i] * maxHelderheid * fade7 / 65025;
		strip3Red[i] = rood3[i] * pixelBuf3[i] * maxHelderheid * fade7 / 65025;
		strip3Green[i] = groen3[i] * pixelBuf3[i] * maxHelderheid * fade7 / 65025;
		strip3Blue[i] = blauw3[i] * pixelBuf3[i] * maxHelderheid * fade7 / 65025;
		strip4Red[i] = rood4[i] * pixelBuf4[i] * maxHelderheid * fade7 / 65025;
		strip4Green[i] = groen4[i] * pixelBuf4[i] * maxHelderheid * fade7 / 65025;
		strip4Blue[i] = blauw4[i] * pixelBuf4[i] * maxHelderheid * fade7 / 65025;
		strip5Red[i] = rood5[i] * pixelBuf5[i] * maxHelderheid * fade7 / 65025;
		strip5Green[i] = groen5[i] * pixelBuf5[i] * maxHelderheid * fade7 / 65025;
		strip5Blue[i] = blauw5[i] * pixelBuf5[i] * maxHelderheid * fade7/ 65025;
		strip6Red[i] = rood6[i] * pixelBuf6[i] * maxHelderheid * fade7 / 65025;
		strip6Green[i] = groen6[i] * pixelBuf6[i] * maxHelderheid * fade7 / 65025;
		strip6Blue[i] = blauw6[i] * pixelBuf6[i] * maxHelderheid * fade7 / 65025;
		strip7Red[i] = rood7[i] * pixelBuf7[i] * maxHelderheid * fade7 / 65025;
		strip7Green[i] = groen7[i] * pixelBuf7[i] * maxHelderheid * fade7 / 65025;
		strip7Blue[i] = blauw7[i] * pixelBuf7[i] * maxHelderheid * fade7/ 65025;
		i++;
	}
}

void pixelBuf2Kleur8()
{
	int i = 0;
	int einde = aantalLeds;
	if(aantalPixels > aantalLeds){
		einde = aantalPixels;
	}
	while(i < einde){
		strip1Red[i] = rood1[i] * pixelBuf1[i] * maxHelderheid * fade8 / 65025 ;
		strip1Green[i] = groen1[i] * pixelBuf1[i] * maxHelderheid * fade8 / 65025;
		strip1Blue[i] = blauw1[i] * pixelBuf1[i] * maxHelderheid * fade8 / 65025;
		strip2Red[i] = rood2[i] * pixelBuf2[i] * maxHelderheid * fade8 / 65025;
		strip2Green[i] = groen2[i] * pixelBuf2[i] * maxHelderheid * fade8 / 65025;
		strip2Blue[i] = blauw2[i] * pixelBuf2[i] * maxHelderheid * fade8 / 65025;
		strip3Red[i] = rood3[i] * pixelBuf3[i] * maxHelderheid * fade8 / 65025;
		strip3Green[i] = groen3[i] * pixelBuf3[i] * maxHelderheid * fade8 / 65025;
		strip3Blue[i] = blauw3[i] * pixelBuf3[i] * maxHelderheid * fade8 / 65025;
		strip4Red[i] = rood4[i] * pixelBuf4[i] * maxHelderheid * fade8 / 65025;
		strip4Green[i] = groen4[i] * pixelBuf4[i] * maxHelderheid * fade8 / 65025;
		strip4Blue[i] = blauw4[i] * pixelBuf4[i] * maxHelderheid * fade8 / 65025;
		strip5Red[i] = rood5[i] * pixelBuf5[i] * maxHelderheid * fade8 / 65025;
		strip5Green[i] = groen5[i] * pixelBuf5[i] * maxHelderheid * fade8 / 65025;
		strip5Blue[i] = blauw5[i] * pixelBuf5[i] * maxHelderheid * fade8/ 65025;
		strip6Red[i] = rood6[i] * pixelBuf6[i] * maxHelderheid * fade8 / 65025;
		strip6Green[i] = groen6[i] * pixelBuf6[i] * maxHelderheid * fade8 / 65025;
		strip6Blue[i] = blauw6[i] * pixelBuf6[i] * maxHelderheid * fade8 / 65025;
		strip7Red[i] = rood7[i] * pixelBuf7[i] * maxHelderheid * fade8 / 65025;
		strip7Green[i] = groen7[i] * pixelBuf7[i] * maxHelderheid * fade8 / 65025;
		strip7Blue[i] = blauw7[i] * pixelBuf7[i] * maxHelderheid * fade8/ 65025;
		strip8Red[i] = rood8[i] * pixelBuf8[i] * maxHelderheid * fade8 / 65025;
		strip8Green[i] = groen8[i] * pixelBuf8[i] * maxHelderheid * fade8 / 65025;
		strip8Blue[i] = blauw8[i] * pixelBuf8[i] * maxHelderheid  * fade8 / 65025;
		i++;
	}
}

void pixelBuf2KleurBasis()
{
	int i = 0;
	int einde = aantalLeds;
	if(aantalPixels > aantalLeds){
		einde = aantalPixels;
	}
	while(i < einde){
		strip8Red[i] = rood8[i] * pixelBuf8[i] * maxHelderheid * fadeBasis / 65025;
		strip8Green[i] = groen8[i] * pixelBuf8[i] * maxHelderheid * fadeBasis / 65025;
		strip8Blue[i] = blauw8[i] * pixelBuf8[i] * maxHelderheid  * fadeBasis / 65025;
		i++;
	}
}
// einde kleurfuncties

// begin animatie functies
void sterTekst7(int kleur[], int aantal)								// Ster op tekst
{
	char strip[100] = "";
	int i = 0;
	int j;
	while(i < aantal){
		strip[i] = rand() %7;
		i++;
	}
	i = 0;
	while(i < aantal){
		j = rand() % (aantalLeds - 1);
		
		if(strip[i] == 0){
			strip1Red[j] = kleur[0] * pixelBuf1[j];
			strip1Green[j] = kleur[1] * pixelBuf1[j];
			strip1Blue[j] = kleur[2] * pixelBuf1[j];
		}
		else if(strip[i] == 1){
			strip2Red[j] = kleur[0] * pixelBuf2[j];
			strip2Green[j] = kleur[1] * pixelBuf2[j];
			strip2Blue[j] = kleur[2] * pixelBuf2[j];
		}
		else if(strip[i] == 2){
			strip3Red[j] = kleur[0] * pixelBuf3[j];
			strip3Green[j] = kleur[1] * pixelBuf3[j];
			strip3Blue[j] = kleur[2] * pixelBuf3[j];
		}
		else if(strip[i] == 3){
			strip4Red[j] = kleur[0] * pixelBuf4[j];
			strip4Green[j] = kleur[1] * pixelBuf4[j];
			strip4Blue[j] = kleur[2] * pixelBuf4[j];
		}
		else if(strip[i] == 4){
			strip5Red[j] = kleur[0] * pixelBuf5[j];
			strip5Green[j] = kleur[1] * pixelBuf5[j];
			strip5Blue[j] = kleur[2] * pixelBuf5[j];
		}	
		else if(strip[i] == 5){
			strip6Red[j] = kleur[0] * pixelBuf6[j];
			strip6Green[j] = kleur[1] * pixelBuf6[j];
			strip6Blue[j] = kleur[2] * pixelBuf6[j];
		}
		else if(strip[i] == 6){
			strip7Red[j] = kleur[0] * pixelBuf7[j];
			strip7Green[j] = kleur[1] * pixelBuf7[j];
			strip7Blue[j] = kleur[2] * pixelBuf7[j];
		}
		i++;
	}
}

void sterTekst8(int kleur[], int aantal)
{
	char strip[100] = "";
	int i = 0;
	int j;
	while(i < aantal){
		strip[i] = rand() %8;
		i++;
	}
	i = 0;
	while(i < aantal){
		j = rand() % (aantalLeds - 1);
		
		if(strip[i] == 0){
			strip1Red[j] = kleur[0] * pixelBuf1[j];
			strip1Green[j] = kleur[1] * pixelBuf1[j];
			strip1Blue[j] = kleur[2] * pixelBuf1[j];
		}
		else if(strip[i] == 1){
			strip2Red[j] = kleur[0] * pixelBuf2[j];
			strip2Green[j] = kleur[1] * pixelBuf2[j];
			strip2Blue[j] = kleur[2] * pixelBuf2[j];
		}
		else if(strip[i] == 2){
			strip3Red[j] = kleur[0] * pixelBuf3[j];
			strip3Green[j] = kleur[1] * pixelBuf3[j];
			strip3Blue[j] = kleur[2] * pixelBuf3[j];
		}
		else if(strip[i] == 3){
			strip4Red[j] = kleur[0] * pixelBuf4[j];
			strip4Green[j] = kleur[1] * pixelBuf4[j];
			strip4Blue[j] = kleur[2] * pixelBuf4[j];
		}
		else if(strip[i] == 4){
			strip5Red[j] = kleur[0] * pixelBuf5[j];
			strip5Green[j] = kleur[1] * pixelBuf5[j];
			strip5Blue[j] = kleur[2] * pixelBuf5[j];
		}	
		else if(strip[i] == 5){
			strip6Red[j] = kleur[0] * pixelBuf6[j];
			strip6Green[j] = kleur[1] * pixelBuf6[j];
			strip6Blue[j] = kleur[2] * pixelBuf6[j];
		}
		else if(strip[i] == 6){
			strip7Red[j] = kleur[0] * pixelBuf7[j];
			strip7Green[j] = kleur[1] * pixelBuf7[j];
			strip7Blue[j] = kleur[2] * pixelBuf7[j];
		}
		else if(strip[i] == 7){
			strip8Red[j] = kleur[0] * pixelBuf8[j];
			strip8Green[j] = kleur[1] * pixelBuf8[j];
			strip8Blue[j] = kleur[2] * pixelBuf8[j];
		}
		i++;
	}
}

void sterTekstBasis(int kleur[], int aantal)
{
	char strip[100] = "";
	int i = 0;
	int j;
	while(i < aantal){
		strip[i] = rand() %8;
		i++;
	}
	i = 0;
	while(i < aantal){
		j = rand() % (aantalLeds - 1);
		if(strip[i] == 7){
			strip8Red[j] = kleur[0] * pixelBuf8[j];
			strip8Green[j] = kleur[1] * pixelBuf8[j];
			strip8Blue[j] = kleur[2] * pixelBuf8[j];
		}
		i++;
	}
}

void ster7(int kleur[],int aantal)										// Ster op een willekeurige plaats
{
	char strip[100] = "";
	int i = 0;
	int j;
	while(i < aantal){
		strip[i] = rand() %7;
		i++;
	}
	i = 0;
	while(i < aantal){
		j = rand() % (aantalLeds - 1);
		
		if(strip[i] == 0){
			strip1Red[j] = kleur[0];
			strip1Green[j] = kleur[1];
			strip1Blue[j] = kleur[2];
		}
		else if(strip[i] == 1){
			strip2Red[j] = kleur[0];
			strip2Green[j] = kleur[1];
			strip2Blue[j] = kleur[2];
		}
		else if(strip[i] == 2){
			strip3Red[j] = kleur[0];
			strip3Green[j] = kleur[1];
			strip3Blue[j] = kleur[2];
		}
		else if(strip[i] == 3){
			strip4Red[j] = kleur[0];
			strip4Green[j] = kleur[1];
			strip4Blue[j] = kleur[2];
		}
		else if(strip[i] == 4){
			strip5Red[j] = kleur[0];
			strip5Green[j] = kleur[1];
			strip5Blue[j] = kleur[2];
		}	
		else if(strip[i] == 5){
			strip6Red[j] = kleur[0];
			strip6Green[j] = kleur[1];
			strip6Blue[j] = kleur[2];
		}
		else if(strip[i] == 6){
			strip7Red[j] = kleur[0];
			strip7Green[j] = kleur[1];
			strip7Blue[j] = kleur[2];
		}
		i++;
	}
}
	
void ster8(int kleur[],int aantal)
{
	char strip[100] = "";
	int i = 0;
	int j;
	while(i < aantal){
		strip[i] = rand() %8;
		i++;
	}
	i = 0;
	while(i < aantal){
		j = rand() % (aantalLeds - 1);
		
		if(strip[i] == 0){
			strip1Red[j] = kleur[0];
			strip1Green[j] = kleur[1];
			strip1Blue[j] = kleur[2];
		}
		else if(strip[i] == 1){
			strip2Red[j] = kleur[0];
			strip2Green[j] = kleur[1];
			strip2Blue[j] = kleur[2];
		}
		else if(strip[i] == 2){
			strip3Red[j] = kleur[0];
			strip3Green[j] = kleur[1];
			strip3Blue[j] = kleur[2];
		}
		else if(strip[i] == 3){
			strip4Red[j] = kleur[0];
			strip4Green[j] = kleur[1];
			strip4Blue[j] = kleur[2];
		}
		else if(strip[i] == 4){
			strip5Red[j] = kleur[0];
			strip5Green[j] = kleur[1];
			strip5Blue[j] = kleur[2];
		}	
		else if(strip[i] == 5){
			strip6Red[j] = kleur[0];
			strip6Green[j] = kleur[1];
			strip6Blue[j] = kleur[2];
		}
		else if(strip[i] == 6){
			strip7Red[j] = kleur[0];
			strip7Green[j] = kleur[1];
			strip7Blue[j] = kleur[2];
		}
		else if(strip[i] == 7){
			strip8Red[j] = kleur[0];
			strip8Green[j] = kleur[1];
			strip8Blue[j] = kleur[2];
		}
		i++;
	}
}

void sterBasis(int kleur[],int aantal)
{
	char strip[100] = "";
	int i = 0;
	int j;
	while(i < aantal){
		strip[i] = rand() %8;
		i++;
	}
	i = 0;
	while(i < aantal){
		j = rand() % (aantalLeds - 1);
		if(strip[i] == 7){
			strip8Red[j] = kleur[0];
			strip8Green[j] = kleur[1];
			strip8Blue[j] = kleur[2];
		}
		i++;
	}
}

void klok()																// Klok op display
{
	cls7();
	char tijd[10] = "";
	time_t rawtime;
	struct tm *info;
	time( &rawtime );
	info = localtime( &rawtime );
	sprintf(tijd, "%02d:%02d:%02d", (info -> tm_hour), (info -> tm_min), (info -> tm_sec));
	aantalPixels = 0;
	int pixels;
	int byte;
	int i = 0;
	int j;
	while(i < 8){
		byte = tijd[i];
		pixels = alfabetArray[byte][0][0];
		j = 0;
		while(j < pixels){
			pixelBuf1[aantalPixels + j] = alfabetArray[byte][1][j];
			pixelBuf2[aantalPixels + j] = alfabetArray[byte][2][j];
			pixelBuf3[aantalPixels + j] = alfabetArray[byte][3][j];
			pixelBuf4[aantalPixels + j] = alfabetArray[byte][4][j];
			pixelBuf5[aantalPixels + j] = alfabetArray[byte][5][j];
			pixelBuf6[aantalPixels + j] = alfabetArray[byte][6][j];
			pixelBuf7[aantalPixels + j] = alfabetArray[byte][7][j];
			j++;
		}
		aantalPixels = aantalPixels + pixels;
		pixelBuf1[aantalPixels] = 0;
		pixelBuf2[aantalPixels] = 0;
		pixelBuf3[aantalPixels] = 0;
		pixelBuf4[aantalPixels] = 0;
		pixelBuf5[aantalPixels] = 0;
		pixelBuf6[aantalPixels] = 0;
		pixelBuf7[aantalPixels] = 0;
		aantalPixels = aantalPixels +1;
		pixelBuf1[aantalPixels] = 0;
		pixelBuf2[aantalPixels] = 0;
		pixelBuf3[aantalPixels] = 0;
		pixelBuf4[aantalPixels] = 0;
		pixelBuf5[aantalPixels] = 0;
		pixelBuf6[aantalPixels] = 0;
		pixelBuf7[aantalPixels] = 0;
		i++;
	}
	int aantal = (aantalLeds - 40) / 2;
	int teller = 0;
	while(teller < aantal){
		shiftRechts7();
		teller ++;
	}
	pixelBuf2Kleur7();
}

void fade7Uit()															// Doven display
{																		// einde = int volgendeStap = 1
	if(fade7 > 0){
		fade7--;
	}
	else{
		volgendeStap = 1;
	}
	pixelBuf2Kleur7();
}

void fade8Uit()
{
	if(fade8 > 0){
		fade8--;
	}
	else{
		volgendeStap = 1;
	}
	pixelBuf2Kleur8();
}

void fadeBasisUit()
{
	if(fadeBasis > 0){
		fadeBasis--;
	}
	else{
		volgendeStap = 1;
	}
	pixelBuf2KleurBasis();
}

void fade7In()															// oplichte display maximum helderheid wordt bepaald door maxHelderheid
{																		// einde = int volgendeStap = 1
	if(fade7 < 255){
		fade7++;
	}
	else{
		volgendeStap = 1;
	}
	pixelBuf2Kleur7();
}

void fade8In()
{
	if(fade8 < 255){
		fade8++;
	}
	else{
		volgendeStap = 1;
	}
	pixelBuf2Kleur8();
}

void fadeBasisIn()
{
	if(fadeBasis < 255){
		fadeBasis++;
	}
	else{
		volgendeStap = 1;
	}
	pixelBuf2KleurBasis();
}
// einde animatie functies	

void pauze(int seconden)
{
	if(v == 0){
		nu = vorig = clock();
		v = 1;
	}
	nu = clock();
	if((seconden * 1000000) < (nu - vorig)){
		v = 0;
		volgendeStap = 1;
	}
}

// CTRL-C
void INThandler(int sig)												
{
	
	run = 0;
}



	
	


