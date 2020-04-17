#ifndef _PCB_H
#define _PCB_H

#include "types.h"
#include "file.h"
#include "system_call.h"

#define FILE_ARRAY_SIZE 8
#define EIGHT_MB         0x800000
#define FOUR_MB          0x400000
#define EIGHT_KB         0x2000
#define BUF_SIZE         128

#define PCB_ARRAY_SIZE  8

// //Process statuses will define later they are flags in a 1 byte element
// // |       |         |       |      |         |          |         |          |
// #define TASK_RUNNING              0x01
// #define TASK_INTERRUPTIBLE        0x02
// #define TASK_UNINTERRUPTIBLE      0x04
// #define TASK_STOPPED              0x08
// #define TASK_TRACED               0x10
// #define EXIT_ZOMBIE               0x20
// #define EXIT_DEAD                 0x40

 //this is the mask from the book depends on where pcb in the book it is below
 //the stack in  our case it should be at the top of the stack
#define ESP_MASK_FOR_PCB


/* struct of Process Control Block
   it contians all the information about a process and information that
   the process needs to run or for processes to switch.
   it has a block that holds memory and the stack
   a block that records devices that the process will use such as files or
   io ports
   and it has a block holding information about related pcbs
*/
typedef struct pcb {
    struct pcb* parent;
//pids are created squentually
    uint32_t process_id;
    uint32_t parent_process_id;
    uint8_t flag;  // if flag is one pcb is in use  // if it is 0 its not used
    uint8_t isRunning;
    //program counter records where the stack currently is
    // where the heap is and where the data section for global vars is

    //cpu registers must be saved and restored when a process is swapped in and
    // out of a cpu.

    //holds location of the stack and the parent process stack
    uint32_t stack_ptr;
    uint32_t base_ptr;
    uint32_t parent_stack_ptr;
    uint32_t parent_base_ptr;
    uint32_t terminal_index;
    //holds argument data for the process we want to complete.
    uint8_t args[BUF_SIZE];

    file_desc_t fd_array[FILE_ARRAY_SIZE];

    uint32_t sch_esp;
    uint32_t sch_ebp;

    uint32_t sch_ss0;
    uint32_t sch_esp0;
    //stack size should be 8KB Can define this at the top.
}pcb_t;

extern pcb_t* pcb_array[PCB_ARRAY_SIZE];

pcb_t* get_pcb_Array();
void init_PCB_array(void);

//___________________________________________________________//
extern void set_pcb_status();
extern void set_parent_id();
extern void set_process_id();

extern uint32_t current_stack();
extern uint32_t current_base();
extern uint32_t parent_stack();
extern uint32_t parent_base();

#endif /* _PCB_H */
