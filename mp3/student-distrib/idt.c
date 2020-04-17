#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "keyboard.h"
#include "mouse.h"
#include "rtc.h"
#include "itr_hdl.h"
#include "schedule.h"

// Reference: student-notes.pdf 29
// parameter init
#define KEYBOARD    0x21
#define MOUSE        0x2C
#define RTC            0x28
#define SYSCALL        0x80
#define PIT            0x20

/* A bunch of exception handlers
 * Demonstrate the death of exception, identify the exception,
 * loop forever to squash the user-level program.
 *
 * Based on the ia32-ref table
 * Reference: IA32-ref-manual-vol-3.pdf 5-3
 */

void exc_de() {
    cli();
    printf("Divide Error\n");
    while(1);
}
void exc_db() {
    cli();
    printf("Debug Exception\n");
    while(1);
}
void exc_nmi() {
    cli();
    printf("Nonmaskable Interrupt\n");
    while(1);
}
void exc_bp() {
    cli();
    printf("Breakpoint\n");
    while(1);
}
void exc_of() {
    cli();
    printf("Overflow\n");
    while(1);
}
void exc_br() {
    cli();
    printf("Bound Range Exceeded\n");
    while(1);
}
void exc_ud() {
    cli();
    printf("Invalid Opcode (Undefined)\n");
    while(1);
}
void exc_nm() {
    cli();
    printf("Device Not Available (No Math Coprocessor)\n");
    while(1);
}
void exc_df() {
    cli();
    printf("Double Fault\n");
    while(1);
}
void exc_cs() {
    cli();
    printf("Coprocessor Segment Overrun\n");
    while(1);
}
void exc_ts() {
    cli();
    printf("Invalid TSS\n");
    while(1);
}
void exc_np() {
    cli();
    printf("Segment Not Present\n");
    while(1);
}
void exc_ss() {
    cli();
    printf("Stack-Segment Fault\n");
    while(1);
}
void exc_gp() {
    cli();
    printf("General Protection\n");
    while(1);
}
void exc_pf() {
    cli();
    printf("Page Fault\n");
    while(1);
}
void exc_mf() {
    cli();
    printf("x87 FPU Floating-Point Error\n");
    while(1);
}
void exc_ac() {
    cli();
    printf("Alignment Check\n");
    while(1);
}
void exc_mc() {
    cli();
    printf("Machine Check\n");
    while(1);
}
void exc_xf() {
    cli();
    printf("SIMD Floating-Point Exception\n");
    while(1);
}


// This is for undefined interrupts
void default_interrupt() {
    cli();
    printf("Undefined interrupt");
    sti();
}

/*
 * init_idt(): initializes the idt
 * first put values(present, dpl etc.) in all entries; then use the given
 * SET_IDT_ENTRY to set offset; finally load the address of idt to idtr
 *
 * inputs: none
 * retvals: none
 */
void init_idt () {

    int i;                                    //loop index

    for(i = 0; i < NUM_VEC; i++) {

        idt[i].present = 1;                 //present

        idt[i].dpl = 0;                     //0 for interrupt and exception


        idt[i].reserved0 = 0;                //01111 000 00000 for exception i<32, Reference: IA32-ref-manual-vol-3.pdf 5-14
        idt[i].size = 1;                    //1 = 32 bits; 0 = 16 bits. here all 32
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 1;
        idt[i].reserved4 = 0;

        if(i >= 32) {                        //01110 000 00000 for interrupt i>=32, Reference: IA32-ref-manual-vol-3.pdf 5-14
            idt[i].reserved3 = 0;
            SET_IDT_ENTRY(idt[i], default_interrupt);
                                            //haven't defined, may be used in future.
        }

        if(i == SYSCALL){
              idt[i].dpl = 0x3;     //3 for system call
                idt[i].reserved3 = 1;
        }


        idt[i].seg_selector = KERNEL_CS;    //set segment selector = kernel cs

    }

    //set the already defined exceptions according to the ia32-ref
    SET_IDT_ENTRY(idt[0], exc_de);
    SET_IDT_ENTRY(idt[1], exc_db);
    SET_IDT_ENTRY(idt[2], exc_nmi);
    SET_IDT_ENTRY(idt[3], exc_bp);
    SET_IDT_ENTRY(idt[4], exc_of);
    SET_IDT_ENTRY(idt[5], exc_br);
    SET_IDT_ENTRY(idt[6], exc_ud);
    SET_IDT_ENTRY(idt[7], exc_nm);
    SET_IDT_ENTRY(idt[8], exc_df);
    SET_IDT_ENTRY(idt[9], exc_cs);
    SET_IDT_ENTRY(idt[10], exc_ts);
    SET_IDT_ENTRY(idt[11], exc_np);
    SET_IDT_ENTRY(idt[12], exc_ss);
    SET_IDT_ENTRY(idt[13], exc_gp);
    SET_IDT_ENTRY(idt[14], exc_pf);
    SET_IDT_ENTRY(idt[16], exc_mf);
    SET_IDT_ENTRY(idt[17], exc_ac);
    SET_IDT_ENTRY(idt[18], exc_mc);
    SET_IDT_ENTRY(idt[19], exc_xf);

    SET_IDT_ENTRY(idt[PIT], pit_handler);            //set pit interrupt handler
    SET_IDT_ENTRY(idt[KEYBOARD], keyboard_handler);    //set keyboard interrupt handler
    SET_IDT_ENTRY(idt[MOUSE], mouse_handler);        //set mouse interrupt handler
    SET_IDT_ENTRY(idt[RTC], rtc_handler);            //set clock interrupt handler
    SET_IDT_ENTRY(idt[SYSCALL], syscall_handler);    //set system call handler

    lidt(idt_desc_ptr);                                // Load idt to idtr
}
