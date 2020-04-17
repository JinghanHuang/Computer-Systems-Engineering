#ifndef ITR_HDL_H
#define ITR_HDL_H


// assemble linkage for keyboard interrupt handler
extern void keyboard_handler();

// assemble linkage for mouse interrupt handler
extern void mouse_handler();

// assemble linkage for rtc interrupt handler
extern void rtc_handler();

// assemble linkage for system call handler
extern void syscall_handler();

// assemble linkage for pit interrrupt handler
extern void pit_handler();
#endif
