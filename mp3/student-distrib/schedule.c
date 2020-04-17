#include "schedule.h"
#include "lib.h"
#include "i8259.h"
#include "system_call.h"
#include "multi_terminal.h"
#include "pcb.h"
#include "paging.h"
#include "x86_desc.h"


uint32_t page_dir_addr1;
volatile uint8_t cur_term_sch = 0;
volatile uint8_t next_term_sch;
static int openAll = 0;
static int wait = WAIT_TIME;
/* pit_init()
 * initializes the pit.
 * input: none
 * return: none
 */
void pit_init(void)
{
    //set the freq to 40. ref: linux kernel book pg269.

    outb(VAL_1, PIT_PORT_1);
    outb(SET_40HZ & PIT_MASK, PIT_PORT_2);
    outb(SET_40HZ >> PIT_BIT_MOVE, PIT_PORT_2);

    //enable irq channel 0
    enable_irq(PIT_IRQ);
}

/* pit_interrupt_handler()
 * pit_interrupt_handler, schedule the tasks.
 * input: none
 * return: none
 */
void pit_interrupt_handler(void)
{
    uint8_t next_pcb_sch;
    send_eoi(PIT_IRQ);
    cli();

	//start all terminals
    if(wait == 0){
        if(openAll == 0){
            openAll += 1;
            clear();
            execute((const uint8_t*) "shell");
        }
        else if (term[TERMINAL_TWO].exist == 0){
            openAll += 1;
            wait = WAIT_TIME;
            sti();
            multi_terminal_switch(TERMINAL_TWO);
        }
        else if (term[TERMINAL_THREE].exist == 0){
            openAll += 1;
            wait = WAIT_TIME;
            sti();
            multi_terminal_switch(TERMINAL_THREE);
        }
        else if(openAll == TERMINAL_NUM){
            openAll = -1;
            wait = WAIT_TIME;
            sti();
            multi_terminal_switch(TERMINAL_ONE);
            enable_irq(KEYBOARD_IRQ);
        }
    }

	//get next index
    next_pcb_sch = get_next();

    // switch previous and current process index
    term[cur_term_sch].cur_pid = cur_pid;
    term[cur_term_sch].pre_pid = pre_pid;
    cur_pid = term[next_term_sch].cur_pid;
    pre_pid = term[next_term_sch].pre_pid;

	//redirect page
    set_up_page(next_pcb_sch);
    flush_TLB();

	//gert pcb structures
    pcb_t* prev_pcb = pcb_array[term[cur_term_sch].cur_pid];
    pcb_t* next_pcb = pcb_array[next_pcb_sch];

	//switch the video memory and backing video memory
    map_video_memory_into_user();
    if(next_term_sch != cur_term_index){
        map_backing(term[next_term_sch].term_vm_addr);
    }

	//save to tss
    prev_pcb->sch_ss0 = tss.ss0;
    prev_pcb->sch_esp0 = tss.esp0;
    tss.ss0 = next_pcb->sch_ss0;
    tss.esp0 = next_pcb->sch_esp0;
    //tss.ss0 = KERNEL_DS;
    //tss.esp0 = EIGHT_MB - next_pcb_sch * EIGHT_KB - PAGE_BORDER;

	//set term_sch index
    cur_term_sch = next_term_sch;
    wait -= 1;

	//follwing two block set esp and ebp
    asm volatile(
        "movl %%esp, %0;"
        "movl %%ebp, %1;"
        :"=r"(prev_pcb->sch_esp), "=r"(prev_pcb->sch_ebp)
        :
    );

    asm volatile(
        "movl %0, %%esp;"
        "movl %1, %%ebp;"
        :
        :"r"(next_pcb->sch_esp), "r"(next_pcb->sch_ebp)
    );

    sti();

    asm volatile("leave");
    asm volatile("ret");
}


/* get_next()
 * to get the index of next pcb we switch to
 * input:
 * return:
 */
uint32_t get_next(){
    int i;
    next_term_sch = cur_term_sch;
    for (i = 0; i < TERMINAL_NUM; i++){
        next_term_sch = (next_term_sch + 1) % TERMINAL_NUM; //3 is total number of terms
        if (term[next_term_sch].exist == 1){
            break;
        }
    }
    return term[next_term_sch].cur_pid;
}
