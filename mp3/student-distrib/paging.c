#include "paging.h"
#include "lib.h"

// declare page directory and page table
pde_t page_directory[ENTRY_NUMBER] __attribute__((aligned (PAGE_SIZE)));
pte_t page_table[ENTRY_NUMBER] __attribute__((aligned (PAGE_SIZE)));
pte_t vidmap_table[ENTRY_NUMBER] __attribute__((aligned (PAGE_SIZE)));

/* void paging_init()
 * Inputs: none
 * Return Value: none
 * Function: Initialize paging */
void paging_init() {

    uint32_t i; // loop index

	// Initialize all the page directory and page table
    for(i = 0; i < PAGE_SIZE; i++){
        page_directory[i].val =  i * PAGE_SIZE;
        page_table[i].val = 0;
        page_table[i].page_base_addr = i;
        vidmap_table[i].val = 0;
        vidmap_table[i].user_supervisor  = 1;
        vidmap_table[i].page_base_addr = i;
    }

    // for video memory
    page_table[VIDEO_MEM_ADDR >> ADDR_SHIFT].present = 1;
    page_table[VIDEO_MEM_ADDR >> ADDR_SHIFT].read_write = 1;
    page_table[VIDEO_MEM_ADDR >> ADDR_SHIFT].page_base_addr = VIDEO_MEM_ADDR >> ADDR_SHIFT;
    // terminal one video memory
    page_table[TERMINAL_ONE_VIDEO_ADDR >> ADDR_SHIFT].present = 1;
    page_table[TERMINAL_ONE_VIDEO_ADDR >> ADDR_SHIFT].read_write = 1;
    page_table[TERMINAL_ONE_VIDEO_ADDR >> ADDR_SHIFT].page_base_addr = TERMINAL_ONE_VIDEO_ADDR >> ADDR_SHIFT;
    // terminal two video memory
    page_table[TERMINAL_TWO_VIDEO_ADDR >> ADDR_SHIFT].present = 1;
    page_table[TERMINAL_TWO_VIDEO_ADDR >> ADDR_SHIFT].read_write = 1;
    page_table[TERMINAL_TWO_VIDEO_ADDR >> ADDR_SHIFT].page_base_addr = TERMINAL_TWO_VIDEO_ADDR >> ADDR_SHIFT;
    // terminal three video memory
    page_table[TERMINAL_THREE_VIDEO_ADDR >> ADDR_SHIFT].present = 1;
    page_table[TERMINAL_THREE_VIDEO_ADDR >> ADDR_SHIFT].read_write = 1;
    page_table[TERMINAL_THREE_VIDEO_ADDR >> ADDR_SHIFT].page_base_addr = TERMINAL_THREE_VIDEO_ADDR >> ADDR_SHIFT;

    // let the first page directory point to the page table containing video memory
    page_directory[0].pde_4kb.present = 1;
    page_directory[0].pde_4kb.read_write = 1;
    page_directory[0].pde_4kb.page_table_base_addr = (uint32_t) (page_table) >> ADDR_SHIFT;


    // for kernel code at 4-8MB
    page_directory[1].pde_4mb.present = 1;
    page_directory[1].pde_4mb.read_write = 1;
    page_directory[1].pde_4mb.page_size = 1; // let the page directory for 4-MByte Page
    page_directory[1].pde_4mb.global_page = 1;
    page_directory[1].pde_4mb.page_base_addr = (uint32_t) KERNEL_ADDR >> ADDR_SHIFT_4MB;

    // initialize registers
    // Register initialization reference: https://wiki.osdev.org/Paging#Enabling
    store_pd_addr((uint32_t) page_directory);
	  enable_4mb_paging();
	  enable_paging();
}

/* static void store_pd_addr(uint32_t page_directory_addr)()
 * Inputs: page_directory_addr - the address of page directory
 * Return Value: none
 * Function: store the address of page directory in CR3 */
void store_pd_addr(uint32_t page_directory_addr) {
    asm volatile("                                                  \n\
        movl %0, %%eax                                              \n\
        movl %%eax, %%cr3                                           \n\
        "
        : /* no outputs */
        : "A"(page_directory_addr)
        : "eax"
    );
}

/* static void enable_paging()
 * Inputs: none
 * Return Value: none
 * Function: set bit 31 in CR0 (PG flag) to 1 to enable paging */
void enable_paging() {
    asm volatile("                                                  \n\
        movl %%cr0, %%eax                                           \n\
        orl  $0x80000000, %%eax     /* bit 31 in CR0 */             \n\
        movl %%eax, %%cr0           /* set PG flag to 1 */          \n\
        "
        : /* no outputs */
        : /* no inputs */
        : "eax"
    );
}

/* static void enable_4mb_paging()
 * Inputs: none
 * Return Value: none
 * Function: set bit 4 in CR4 (PSE flag) to 1 to enable the page size extension for 4-Mbyte pages */
void enable_4mb_paging() {
    asm volatile("                                                  \n\
        movl %%cr4, %%eax                                           \n\
        orl  $0x00000010, %%eax     /* bit 4 in CR4 */              \n\
        movl %%eax, %%cr4           /* set PSE flag to 1 */         \n\
        "
        : /* no outputs */
        : /* no inputs */
        : "eax"
    );
}

/**************** Function added for Checkpoint 3 *****************/

/* void set_up_page()
 * Inputs: cur_pid - process ID
 * Return Value: none
 * Function: set up a page for the corresponding process */
void set_up_page(uint32_t cur_pid) {
    page_directory[PI_INDEX].pde_4mb.present = 1;
    page_directory[PI_INDEX].pde_4mb.read_write = 1;
    page_directory[PI_INDEX].pde_4mb.user_supervisor = 1;
    page_directory[PI_INDEX].pde_4mb.page_size = 1;
    page_directory[PI_INDEX].pde_4mb.page_base_addr = (EIGHT_MB + cur_pid * FOUR_MB) >> ADDR_SHIFT_4MB;
}

/* void flush_TLB()
 * Inputs: none
 * Return Value: none
 * Function: flush TLB by changing cr3 */
void flush_TLB() {
  asm volatile("                                                  \n\
      movl %%cr3, %%eax                                           \n\
      movl %%eax, %%cr3                                           \n\
      "
      : /* no outputs */
      : /* no inputs */
      : "eax"
  );
}

/**************** Function added for Checkpoint 4 *****************/

/* void map_video_memory_into_user()
 * Inputs: none
 * Return Value: none
 * Function:  maps the text-mode video memory into user space at a pre-set virtual address */
void map_video_memory_into_user() {
    vidmap_table[1].present = 1;
    vidmap_table[1].read_write = 1;
    vidmap_table[1].user_supervisor = 1;
    vidmap_table[1].page_base_addr = VIDEO_MEM_ADDR >> ADDR_SHIFT;

    page_directory[VM_PD_IDX].pde_4kb.present = 1;
    page_directory[VM_PD_IDX].pde_4kb.read_write = 1;
    page_directory[VM_PD_IDX].pde_4kb.user_supervisor = 1;
    page_directory[VM_PD_IDX].pde_4kb.page_table_base_addr = (uint32_t) (vidmap_table) >> ADDR_SHIFT;
}

/**************** Function added for Checkpoint 5 *****************/

/* void switch_terminal_video(uint8_t dest, uint8_t src)
 * Inputs: dest - the index for dest terminal, src - the index for src terminal
 * Return Value: none
 * Function:  switch videos between diffrent terminals */
void switch_terminal_video(uint8_t dest, uint8_t src) {
    if(dest >= TERMINAL_NUM || src >= TERMINAL_NUM){
        return;
    }

    cli();
    memcpy((char*) (TERMINAL_ONE_VIDEO_ADDR + (uint32_t)(src * FOUR_KB)), (const char*) VIDEO_MEM_ADDR, FOUR_KB);
    clearvideo();
    memcpy((char*) VIDEO_MEM_ADDR, (const char*) (TERMINAL_ONE_VIDEO_ADDR + (uint32_t)(dest * FOUR_KB)), FOUR_KB);
    sti();
}

/* void map_backing()
 * Inputs: none
 * Return Value: none
 * Function: (for schedule) maps the text-mode video memory into user space at a pre-set virtual address */
void map_backing(uint32_t terminal_video_memory_addr) {
    vidmap_table[1].present = 1;
    vidmap_table[1].read_write = 1;
    vidmap_table[1].user_supervisor = 1;
    vidmap_table[1].page_base_addr = terminal_video_memory_addr >> ADDR_SHIFT;

    page_directory[VM_PD_IDX].pde_4kb.present = 1;
    page_directory[VM_PD_IDX].pde_4kb.read_write = 1;
    page_directory[VM_PD_IDX].pde_4kb.user_supervisor = 1;
    page_directory[VM_PD_IDX].pde_4kb.page_table_base_addr = (uint32_t) (vidmap_table) >> ADDR_SHIFT;
}
