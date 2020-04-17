#ifndef IDT_H
#define IDT_H

// A bunch of declaration of exception handlers,Reference: IA32-ref-manual-vol-3.pdf 5-3
void exc_de();
void exc_db();
void exc_nmi();
void exc_bp();
void exc_of();
void exc_br();
void exc_ud();
void exc_nm();
void exc_df();
void exc_cs();
void exc_ts();
void exc_np();
void exc_ss();
void exc_gp();
void exc_pf();
void exc_mf();
void exc_ac();
void exc_mc();
void exc_xf();

//For undefined interrupts, like current syscalls
void default_interrupt();
// init IDT 
void init_idt();


#endif
