/* keyboard.h - Defines used in interactions with the keyboard interrupt
 * vim:ts=4 noexpandtab
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"


#define KEYBOARD_IRQ		0x01
#define KEYBOARD_ADDR_PORT	0x64
#define KEYBOARD_DATA_PORT	0x60
#define LOWEST_BITMASK		0x01
#define VIDEO       		0xB8000
#define WHITE				0x07
#define STEP_SIZE			0x02
#define KEYMAP_SIZE			128
#define PRESSED				0x01
#define RELEASED			0x00
#define EMPTY				0x00
#define NUM_COLS    		80
#define NUM_ROWS    		25

#define BACKSPACE			0x0E
#define ENTER				0x1C
#define CAPS				0x3A
#define LSHIFT_PRESSED		0x2A
#define RSHIFT_PRESSED		0x36
#define LSHIFT_RELEASED		0xAA
#define RSHIFT_RELEASED		0xB6
#define CTRL_PRESSED		0x1D
#define CTRL_RELEASED		0x9D
#define ALT_PRESSED			0x38
#define ALT_RELEASED		0xB8

#define F1_PRESSED		0x3B
#define F2_PRESSED		0x3C
#define F3_PRESSED		0x3D

#define TERMINAL_ONE		  0
#define TERMINAL_TWO		  1
#define TERMINAL_THREE		  2

#define NEW_LINE			0x0A
#define KEY_MAP_SIZE		128
#define KEY_BUF_SIZE		128

/* Extern variables */
extern int video_mem_pos, last_pos;
extern int prev_pos[NUM_COLS * NUM_ROWS * STEP_SIZE];
extern char key_buffer[KEY_BUF_SIZE];
extern int bufi;

/* Externally-visible functions */

/* Initialize the keyboard input */
void keyboard_init(void);
/* Handle keyboard interrupt (pressing buttons) */
void keyboard_interrupt_handler(void);
/* Convert input char*/
unsigned char convert_char(unsigned char inputChar);

// Terminal Switch Function
void switch_terminal_key_buffer(uint8_t dest, uint8_t src);

#endif /* _KEYBOARD_H */
