/* lmss : Arduino Interrupts */

#ifndef _INOINT_H_
#define _INOINT_H_

/* Initialize interrupt 
 * Use in setup() */
void lmss_ino_interrupt_init(void);

/* Initialize interrupt 
 * Use in loop() */
void lmss_ino_interrupt_process(void);

#endif

