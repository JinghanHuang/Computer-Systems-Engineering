/* keyboard.c - Functions to interact with the keyboard
 * vim:ts=4 noexpandtab
 */

#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "terminal.h"
#include "multi_terminal.h"

// keymap copied from http://www.osdever.net/bkerndev/Docs/keyboard.htm
static unsigned char key_map[KEY_MAP_SIZE] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
  '9', '0', '-', '=', '\b',     /* Backspace */
  '\t',                 /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
    0,                  /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
 '\'', '`',   0,                /* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
  'm', ',', '.', '/',   0,                              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};
static unsigned char key_map_shift[KEY_MAP_SIZE] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',     /* 9 */
  '(', ')', '_', '+', '\b',     /* Backspace */
  '\t',                 /* Tab */
  'Q', 'W', 'E', 'R',   /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter key */
    0,                  /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',     /* 39 */
 '\'', '~',   0,                /* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',                    /* 49 */
  'M', '<', '>', '?',   0,                              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};
static unsigned char key_map_caps[KEY_MAP_SIZE] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
  '9', '0', '-', '=', '\b',     /* Backspace */
  '\t',                 /* Tab */
  'Q', 'W', 'E', 'R',   /* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', /* Enter key */
    0,                  /* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',     /* 39 */
 '\'', '`',   0,                /* Left shift */
 '\\', 'Z', 'X', 'C', 'V', 'B', 'N',                    /* 49 */
  'M', ',', '.', '/',   0,                              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};
static unsigned char key_map_caps_shift[KEY_MAP_SIZE] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',     /* 9 */
  '(', ')', '_', '+', '\b',     /* Backspace */
  '\t',                 /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', /* Enter key */
    0,                  /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':',     /* 39 */
 '\'', '~',   0,                /* Left shift */
 '|', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
  'm', '<', '>', '?',   0,                              /* Right shift */
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};


/* the position in the video memory*/
int video_mem_pos = 0, last_pos = 0;
int prev_pos[NUM_COLS * NUM_ROWS * STEP_SIZE];
/* video memory array */
static char* video_mem = (char *)VIDEO;

unsigned int caps_state = RELEASED, shift_state = RELEASED,
            ctrl_state = RELEASED, alt_state = RELEASED;

/*keyboard buffer array, 128-character is the max size*/
char key_buffer[KEY_BUF_SIZE];
int bufi = 0;//buffer index

/* void keyboard_init(void)
 * Inputs:       void
 * Return Value: void
 * Function: Initialize the keyboard */
void keyboard_init(void) {
    //enable_irq(KEYBOARD_IRQ);    // enable the Interrupt ReQuest for keyboard
    update_cursor();
}


/* void keyboard_handler()
 * Inputs:  uint32_t irq_num = the Interrupt ReQuest number which
 *                                the EOI signal will be sent to
 * Return Value: void
 * Function: Handle keyboard interrupt (pressing buttons) */
void keyboard_interrupt_handler(void) {
    // Reference: https://wiki.osdev.org/%228042%22_PS/2_Controller
    // Reference: https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
    unsigned char convertedChar, dataIN;
    send_eoi(KEYBOARD_IRQ);
    cli();
    // read data from the keyboard data port
    dataIN = inb(KEYBOARD_DATA_PORT);
    convertedChar = convert_char(dataIN);

    //if(dataIN == ENTER)
        //return;
    if ((convertedChar) && (bufi < KEY_BUF_SIZE - 1)) {
        // printf(":%c",convertedChar);
        key_buffer[bufi] = convertedChar;// store in the keybuffer
        bufi ++;
        if(video_mem_pos >= NUM_COLS * NUM_ROWS * STEP_SIZE){
            last_pos = SCROLL_SIZE - STEP_SIZE;
            vertical_scroll();
        }
        video_mem[video_mem_pos] = convertedChar; // outputs the string to the video memory
        prev_pos[video_mem_pos + STEP_SIZE] = video_mem_pos;
        video_mem_pos += STEP_SIZE;                 // go ahead for the next char
        update_cursor();
    }
    sti();
}

/* unsigned char convert_char(unsigned char inputChar)
 * Inputs:  unsigned char inputChar
 * Return Value: retChar
 * Function: Conver input char in different situations */
unsigned char convert_char(unsigned char inputChar){
    int i, prev;
    unsigned char retChar = EMPTY;
    switch (inputChar) {
        // press backspace
        case BACKSPACE:
            prev_pos[EMPTY] = EMPTY;
			if (prev_pos[video_mem_pos]==video_mem_pos) break;
            video_mem_pos = prev_pos[video_mem_pos];
            video_mem[video_mem_pos] = EMPTY;
            update_cursor();
            bufi--;
            key_buffer[bufi] = EMPTY;
            break;
        // press enter
        case ENTER:
            prev = video_mem_pos;
            last_pos = prev - NUM_COLS * STEP_SIZE;
            for (; video_mem_pos < (prev / (STEP_SIZE * NUM_COLS) + 1) * (STEP_SIZE * NUM_COLS); video_mem_pos += STEP_SIZE)
                video_mem[video_mem_pos] = EMPTY;
            prev_pos[video_mem_pos] = prev; // for txt model
            //prev_pos[video_mem_pos] = video_mem_pos; // for terminal model
            if(video_mem_pos >= NUM_COLS * NUM_ROWS * STEP_SIZE){
                //last_pos = SCROLL_SIZE; // for terminal model
                vertical_scroll();
            }
            update_cursor();
            key_buffer[bufi] = NEW_LINE;//ascii of enter
            term[cur_term_index].enter_flag = 1;
            break;
        // press Caps Lock
        case CAPS:
            caps_state ^= PRESSED;
            break;
        // press Shift
        case LSHIFT_PRESSED:
        case RSHIFT_PRESSED:
            shift_state = PRESSED;
            break;
        case LSHIFT_RELEASED:
        case RSHIFT_RELEASED:
            shift_state = RELEASED;
            break;
        // press Ctrl
        case CTRL_PRESSED:
            ctrl_state = PRESSED;
            break;
        case CTRL_RELEASED:
            ctrl_state = RELEASED;
            break;
        // press Alt
        case ALT_PRESSED:
            alt_state = PRESSED;
            break;
        case ALT_RELEASED:
            alt_state = RELEASED;
            break;
        default:
            retChar = PRESSED;
            break;
    }
    // check if it is out of range
    if (!retChar || inputChar >= KEYMAP_SIZE) return EMPTY;
    // CTRL + L to clean the screen
    if (ctrl_state && key_map[inputChar] == 'l')
    {
        for (i = 0; i < NUM_COLS * NUM_ROWS* STEP_SIZE; i += STEP_SIZE)
            video_mem[i] = EMPTY;
        video_mem_pos = 0;
        update_cursor();
        return EMPTY;
    }
    // when Caps Lock is pressed
    if (caps_state)
    {
        if (shift_state)
            return key_map_caps_shift[inputChar];
        return key_map_caps[inputChar];
    }
    // when Shift is pressed
    if (shift_state)
        return key_map_shift[inputChar];

    // when Alt is pressed
    if(alt_state){
        switch (inputChar) {
            case F1_PRESSED:
                sti();
                multi_terminal_switch(TERMINAL_ONE);
                break;
            case F2_PRESSED:
                sti();
                multi_terminal_switch(TERMINAL_TWO);
                break;
            case F3_PRESSED:
                sti();
                multi_terminal_switch(TERMINAL_THREE);
                break;
            default:
                break;
        }
    }

    return key_map[inputChar];
}


/**************** Function added for Checkpoint 5 *****************/

/* switch_terminal_key_buffer(uint8_t dest, uint8_t src)
 * Inputs: dest - the index for dest terminal, src - the index for src terminal
 * Return Value: none
 * Function:  switch keyboard buffers between diffrent terminals */
void switch_terminal_key_buffer(uint8_t dest, uint8_t src) {
    if(dest >= TERMINAL_NUM || src >= TERMINAL_NUM){
        return;
    }

    cli();
    int i;
    term[src].video_mem_pos = video_mem_pos;
    video_mem_pos = term[dest].video_mem_pos;
    term[src].last_pos = last_pos;
    last_pos = term[dest].last_pos;
    term[src].bufi = bufi;
    bufi = term[dest].bufi;
    //memcpy((char*) term[src].prev_pos, (const char*) prev_pos, NUM_COLS * NUM_ROWS * STEP_SIZE);
    //memcpy((char*) prev_pos, (const char*) term[dest].prev_pos, NUM_COLS * NUM_ROWS * STEP_SIZE);
    for(i = 0; i < NUM_COLS * NUM_ROWS * STEP_SIZE; i+= STEP_SIZE){
        term[src].prev_pos[i] = prev_pos[i];
        prev_pos[i] = term[dest].prev_pos[i];
    }
    memcpy((char*) term[src].term_key_buffer, (const char*) key_buffer, KEY_BUF_SIZE);
    memcpy((char*) key_buffer, (const char*) term[dest].term_key_buffer, KEY_BUF_SIZE);
    sti();
}
