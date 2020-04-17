#include "rtc.h"
#include "lib.h"
#include "i8259.h"
#include "multi_terminal.h"
#include "schedule.h"

#define VIRTUAL_RTC 1

#if (VIRTUAL_RTC == 1)
volatile int Interrupt_RTC[TERMINAL_NUM] = {0, 0, 0};
volatile int read_speed[TERMINAL_NUM] = {1, 1, 1};
#endif

#if (VIRTUAL_RTC == 0)
volatile int Interrupt_RTC = 0;
#endif

//int RTC_Counter = 0;
/*
   description:  Initalize the RTC to the IRQL on the pic and enable PERIODIC
   interrupts to go off at the default frequency.

   INPUTS:  None
   OUTPUTS: none
   Side effects:  will change the pic so that the IRQ8 is connected to the RTC
*/
void init_rtc(void){
    //disable the interrupts for the initialization
    cli();
    //look at the clear flags function in library.c
    outb(REG_SET | REG_B, RTC_PORT);  //selects register b
    char prev = inb(CMOS_PORT);
    outb(REG_SET | REG_B, RTC_PORT);
    outb(prev|PERIODIC_INT, CMOS_PORT);
    //turns on with defalt 1024HZ rate
    //enable the interrupts again accoriding to lib.c
    //enable the irq line whent the pic is set up and he makes a function
    sti();
    enable_irq(IRQ_LINE);

}

/*
  description: this is the handler for the RTC for checkpoint one the
  handler only needs to run the provided code in the lib.c file. handler
  should be added to the IDT before the rtc is connected to the PICs

  INPUTS: None
  OUTPUTS: None
  side effects:  will exucute the code provided in lib.c

*/
//for cp 1 should the handler just call the test handler from lib.c
// for cp2 were changing the votile int Interrupt_RTC so that reading
//works correctly
void rtc_interrupt_handler(void){

    send_eoi(IRQ_LINE);
    cli();
    int i;
    //clear the interrupt on the rtc or it wont interrupt again
    #if (VIRTUAL_RTC == 1)
    for(i = 0; i < TERMINAL_NUM; i++){
        if(term[i].exist == 1){
            Interrupt_RTC[i] += VIRTUAL_READ_SPEED;
        }
    }
    #endif

    #if (VIRTUAL_RTC == 0)
    for(i = 0; i < TERMINAL_NUM; i++){
        Interrupt_RTC = 1;
    }
    #endif

    //line below is used for checking the frequency of the device
    //should have a test for after write and after read and open and
    //close
    outb(REG_C , RTC_PORT);
    inb(CMOS_PORT);
    //From Library.c for cp one
    //test_interrupts();
    sti();
}

/*
  description: set_Freqency changes the periodic interrupt Frequency
  of the rtc.  First the input is checked to see if it is vaild if
  it is not an error is returned. If the input is correct the rate
  is calculated and the rtc frequency is changed.
  INPUTS: Frequency
  OUTPUTS: returns 0 on success and -1 on failure
  side effects:  changes the frequency of the RTC
*/
int32_t set_Freq(int frequency){
    //linux limits the amount of intterupts from rtc to 1024 per second
    //we do the same here so higher frequencies are rejected
    if(frequency > 1024){
      return -1;
    }
    //all rtc frequencies are a multiple of 2
    if(frequency%2 !=0){
      return -1;
    }
    // the minimun frequency for the rtc is 2hz
    if(frequency < 2){
      return -1;
    }
    //conver frequency into a valid rate using 4 bytes for the rtc
    char rate;
    //default rate is x6 or 1024hz
    rate = RATE_10;
    switch(frequency){
      //case for 1024Hz
      case 1024:
                rate = RATE_10;
                break;
      //case for 512Hz
      case 512:
                rate = RATE_9;
                break;
      //case for 256Hz
      case 256:
                rate = RATE_8;
                break;
      //case for 128Hz
      case 128:
                rate = RATE_7;
                break;
      //case for 64Hz
      case 64:
                rate = RATE_6;
                break;
      //case for 32Hz
      case 32:
                rate = RATE_5;
                break;
      //case for 16Hz
      case 16:
                rate = RATE_4;
                break;
      //case for 8Hz
      case 8:
                rate = RATE_3;
                break;
      //case for 4Hz
      case 4:
                rate = RATE_2;
                break;
      //case for 2Hz
      case 2:
                rate = RATE_1;
                break;
      //case for frequency that the rtc cannot use
      default:
                return -1;
    }

    //have a switch statement to assign the right frequency to the
    //correct rate then set the rate in the A register
    //process for chaning frequency from osDEV.com on rtc

    #if (VIRTUAL_RTC == 1)
    read_speed[cur_term_sch] = DEFAULT_FOC_VIRTUAL / rate;
    #endif


    #if (VIRTUAL_RTC == 0)
    cli();
    outb(REG_SET|REG_A, RTC_PORT);
    char prev = inb(CMOS_PORT);
    outb(REG_SET|REG_A, RTC_PORT);
    outb((prev & RATE_SET)|rate, CMOS_PORT);
    sti();
    #endif

    return 0;
}


/*
  description: read always returns 0 for the rtc but only after an
  interrupt has occured.  To insure this the read function spins in
  place until an rtc interupt occurs then it sets its flag to 0 again
  and returns 0
  INPUTS: the correct file to identify the rtc read. the buffer and
  the number of bytes in the buffer
  OUTPUTS: none
  side effects: returns 0 when rtc interrupt occurs
*/
//create a votile integer that is changed by the interrupt handler
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    #if (VIRTUAL_RTC == 1)
    while(Interrupt_RTC[cur_term_sch] < read_speed[cur_term_sch]);
    Interrupt_RTC[cur_term_sch] = 0;
    #endif

    #if (VIRTUAL_RTC == 0)
    while(Interrupt_RTC != 1);
    Interrupt_RTC = 0;
    #endif

    return 0;
}

/*
  description: Changes the frequency of the RTC to what is specified
  INPUTS: file description the buffer and the size of the buffer
  OUTPUTS: returns 0 on success -1 on failure
  side effects: changes the rtc frequency
*/
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    //check the length of nbytes to make sure it is 4 long
    if(nbytes != BYTE_SIZE){
        return -1;
    }
    //insure there is information in the buffer
    if((int32_t)buf == NULL){
        return -1;
    }

    int32_t frequency;
    int32_t * buffer= (int32_t*) buf;
    frequency = *buffer;
    cli();
    //check if set_freq worked else return number of bytes written
    int check = set_Freq(frequency);
    sti();
    if(check==-1){
        return -1;
    }
    else{
        return nbytes;
    }
}

/*
  description: Opens the RTC and sets it to a default frequency of 2 Hz
  INPUTS: file descriptor
  OUTPUTS: none
  side effects: returns 0 and changes the rtc frequency to 2 Hz
*/
int32_t rtc_open(const uint8_t* fname){

    #if (VIRTUAL_RTC == 0)
    set_Freq(DEFAULT_FOC);
    #endif

    #if (VIRTUAL_RTC == 1)
    cli();
    outb(REG_SET|REG_A, RTC_PORT);
    char prev = inb(CMOS_PORT);
    outb(REG_SET|REG_A, RTC_PORT);
    outb((prev & RATE_SET)|DEFAULT_FOC_VIRTUAL, CMOS_PORT);
    sti();
    #endif

    return 0;
}

/*
  description: closes the RTC and sets it a a default frequncy of 2 Hz
  INPUTS:
  OUTPUTS:
  side effects: returns 0 or -1 if there is a invalid file descriptor
*/
int32_t rtc_close(int32_t fd){
    set_Freq(DEFAULT_FOC);
    return 0;
}
