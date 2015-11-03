#include <lmss.h>
#include <args.h>
#include <err.h>
#include <bitmap.h>
#include <debug.h>
#include <inoint.h>
#include <ref.h>
#include <parser.h>
#include <vm.h>


/* LMSS Template */

/* Usage: Everything you should included is "lmss.h":
 *   #include "lmss.h"
 *
 * then initialize the system by using:
 *   lmss_init();
 *
 * vm global variable is the way you communicate between
 * C and Scheme part.
 *
 * Method: bind(name, len, pointer to)
 *
 */

/* Main Routines */

void setup()
{
    /* Initialize */
    lmss_init();
    /* Setup Stdlib */
    lmss_stdlib_binds();
    /* Setup lmss Arduino Binds */
    lmss_ino_binds();
    /* Install Interrupt Handler */
    lmss_ino_interrupt_init();

    const char *s = ""
        "(serial-begin 9600)"
        "(dev.gpio.direction led-builtin out)"
        "(attach-int 0 (lambda () (display \"interrupted\")) change)"
        ;
    u8 len = strlen(s);
    vm->eval(s, len);
}

#ifndef BUFFER_LEN 
#define BUFFER_LEN 32
#endif
void loop()
{
    char buffer[BUFFER_LEN];
    char *buffer_p;
    Obj obj_ret;
    u8 len;
    s16 ch;

    /* Process received Interrupt Requests */
    lmss_ino_interrupt_process();

    if ((len = Serial.available()) > 0)
    {
        buffer_p = buffer;
        len = 0;

        /* Read */
        for (;;)
        {
            /* Read a char */
            do { ch = Serial.read(); } while (ch == -1);
            if ((ch == '\r') || (ch == '\n')) break;

            *buffer_p++ = ch;
            len++;
        }

        /* Evaluate */
        obj_ret = vm->eval(buffer, buffer_p - buffer);
        if (obj_ret.type() == OBJ_ERROR)
        {
            Serial.print("Error: ");
            Serial.println(err_pool->get());
        }
        else
        {
            /* Print */
            lmss_println(obj_ret);
        }
    }
}


