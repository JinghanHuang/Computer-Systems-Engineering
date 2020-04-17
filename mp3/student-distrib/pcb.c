#include "pcb.h"

pcb_t* pcb_array[PCB_ARRAY_SIZE];


/*
  Returns the address for a Process's pcb
  description: returns the address of the pcb for the current process. This is
  used to get more information about the current processes
  inputs: the process id for which we want to find the pcb
  outputs: an address of the pcb to be dereferenced.
  side effects: none
*/
pcb_t* get_pcb_Array(){
  uint32_t pcb_ptr = (EIGHT_MB - EIGHT_KB - EIGHT_KB * cur_pid);
  return (pcb_t*) pcb_ptr;
}



/*
  Initializes the pcb array.
  later will be larger.
  description: initalizes the beginning two processes leaving them blank for
  any exectution to take over and manage.
  inputs: none
  outputs: none
  side effects: creates two stacks in on the kernel page for different processes
  to run and stores data in a pcb about the processes.
*/
void init_PCB_array(void){
  int i,j;
  for(i=0; i < PCB_ARRAY_SIZE; i++){
      pcb_array[i] = (pcb_t*)(EIGHT_MB - EIGHT_KB - EIGHT_KB*i);
      pcb_array[i]->flag = 0;
      pcb_array[i]->isRunning = 0;
      pcb_array[i]->stack_ptr = (EIGHT_MB - EIGHT_KB*i + 1);
      pcb_array[i]->base_ptr = (EIGHT_MB - EIGHT_KB*i + 1);
      pcb_array[i]->terminal_index = -1;

      for(j = FD_START; j < FD_MAX; j++){
          pcb_array[i]->fd_array[j].flags = 0;
          pcb_array[i]->fd_array[j].inode = -1;
          pcb_array[i]->fd_array[j].file_pos = 0;
          pcb_array[i]->fd_array[j].file_op = NULL;
      }
  }
}
