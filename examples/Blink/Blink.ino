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
        "(pin-mode led-builtin output)";
    u8 len = strlen(s);
    vm->eval(s, len);
}

void loop()
{
    const char *s = ""
        "(dev.gpio.value led-builtin 1)"
        "(thread.msleep 1000)"
        "(dev.gpio.value led-builtin 0)"
        "(thread.msleep 1000)";
    u8 len = strlen(s);
    vm->eval(s, len);
}


