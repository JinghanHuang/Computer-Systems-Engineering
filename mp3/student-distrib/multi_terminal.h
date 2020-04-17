#ifndef _MULTI_TERMINAL_H
#define _MULTI_TERMINAL_H

#include "types.h"
#include "keyboard.h"

#define KEY_BUF_SIZE		128
#define TERMINAL_NUM    3
#define PAGE_BORDER     4

#define TERMINAL_ONE		  0
#define TERMINAL_TWO		  1
#define TERMINAL_THREE		  2

// struct of terminal
typedef struct term {
    int exist;
    uint32_t term_vm_addr;
    char term_key_buffer[KEY_BUF_SIZE];
    uint8_t parent_term_index;
    uint8_t child_term_index;
    uint32_t cur_pid;
    uint32_t pre_pid;
    int cursor_x;
    int cursor_y;
    int video_mem_pos;
    int last_pos;
    int prev_pos[NUM_COLS * NUM_ROWS * STEP_SIZE];
    int bufi;
    uint32_t esp;
    uint32_t ebp;
    volatile int enter_flag;
} term_t;

extern term_t term[TERMINAL_NUM];
extern volatile uint8_t cur_term_index;
extern uint8_t new_term_excute;
extern uint8_t pre_term_index;

void multi_terminal_init();
void multi_terminal_switch(uint8_t next_term_index);

#endif /* _MULTIPLE_TERMINAL_H */
