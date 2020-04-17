#include "multi_terminal.h"
#include "lib.h"
#include "system_call.h"
#include "paging.h"
#include "x86_desc.h"
#include "schedule.h"

term_t term[TERMINAL_NUM];
volatile uint8_t cur_term_index;
uint8_t new_term_excute = 0;
uint8_t pre_term_index = 0;


/* void multi_terminal_init(
 * Inputs: none
 * Return Value: none
 * Function: Initialize multiple terminals */
void multi_terminal_init() {
    int i;
    for( i = 0; i < TERMINAL_NUM; i++){
        term[i].exist = 0;
        memset(term[i].term_key_buffer, 0, KEY_BUF_SIZE);
        term[i].parent_term_index = 0;
        term[i].child_term_index = 0;
        term[i].cur_pid = 0;
        term[i].pre_pid = 0;
        term[i].cursor_x = 0;
        term[i].cursor_y = 0;
        term[i].video_mem_pos = 0;
        term[i].last_pos = 0;
        memset(term[i].prev_pos, 0, NUM_COLS * NUM_ROWS * STEP_SIZE);
        term[i].bufi = 0;
        term[i].enter_flag = 0;
    }
    term[TERMINAL_ONE].exist = 1;
    term[TERMINAL_ONE].term_vm_addr = TERMINAL_ONE_VIDEO_ADDR;
    term[TERMINAL_TWO].term_vm_addr = TERMINAL_TWO_VIDEO_ADDR;
    term[TERMINAL_THREE].term_vm_addr = TERMINAL_THREE_VIDEO_ADDR;
}

/* void multi_terminal_switch(uint8_t next_term_index)(
 * Inputs: none
 * Return Value: none
 * Function: Switch from the current terminal to the next terminal */
void multi_terminal_switch(uint8_t next_term_index) {

    if(next_term_index > TERMINAL_NUM || cur_term_index == next_term_index){
        return;
    }

    cli();


    // switch position of cursor and mouse
    term[cur_term_index].cursor_x = get_cursor_x();
    term[cur_term_index].cursor_y = get_cursor_y();
    set_cursor_x(term[next_term_index].cursor_x);
    set_cursor_y(term[next_term_index].cursor_y);
	old_attr(get_mouse_x(),get_mouse_y());

    // switch keyboard buffer
    switch_terminal_key_buffer(next_term_index, cur_term_index);

    // switch video memory
    switch_terminal_video(next_term_index, cur_term_index);

    // update the position of cursor and mouse
    update_cursor();
	new_attr(get_mouse_x(),get_mouse_y());

    // execute "shell" if we switch to a new terminal
    if(term[next_term_index].exist == 0){
        term[cur_term_sch].cur_pid = cur_pid;
        term[cur_term_sch].pre_pid = pre_pid;

        pcb_t* prev_pcb = pcb_array[term[cur_term_sch].cur_pid];

        new_term_excute = 1;
        pre_term_index = cur_term_sch;
        cur_term_index = next_term_index;

        // store esp and ebp
        uint32_t esp, ebp;
        asm volatile("movl %%esp, %0":"=g"(esp));
        asm volatile("movl %%ebp, %0":"=g"(ebp));
        prev_pcb->sch_esp = esp;
        prev_pcb->sch_ebp = ebp;

        sti();
        execute((const uint8_t*) "shell");
    }


    map_video_memory_into_user();
    if(next_term_sch != cur_term_index){
        map_backing(term[next_term_sch].term_vm_addr);
    }

    //printf("next_term_index: %d\n", next_term_index);
    //printf("cur_pid: %d\n", term[next_term_index].cur_pid);

    cur_term_index = next_term_index;

    sti();
    asm volatile("leave");
    asm volatile("ret");

}
