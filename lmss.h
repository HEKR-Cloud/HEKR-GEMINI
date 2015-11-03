/* lmss : All In One */

#ifndef _LMSS_H_
#define _LMSS_H_

/* Primitive Data Types */

#ifndef PC
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned int u16;
typedef signed int s16;
#define NULL ((void *)0)
#endif


/* Global Declarations */

/* Initialize LMSS System */
void lmss_init(void);
/* Setup Standard Libraries */
void lmss_stdlib_binds();
/* Setup lmss Arduino Binds */
void lmss_ino_binds();

/* Install Interrupt Handler */
void lmss_ino_interrupt_init();
/* Process received Interrupt Requests */
void lmss_ino_interrupt_process();


#endif

