/* lmss : Arduino binds */

#include "Arduino.h"

#include "lmss.h"
#include "args.h"
#include "vm.h"


/* Basic Functions */


/* Arduino Things */
/* http://arduino.cc/en/Reference/HomePage */

/* pinMode(num:int, mode:int) */
void proc_pinMode(lmssArgs *args)
{
    u8 pin = args->get_int();
    u8 mode = args->get_int();
    pinMode(pin, mode);
}

/* digitalWrite(num:int, value:int) */
void proc_digitalWrite(lmssArgs *args)
{
    u8 pin = args->get_int();
    u8 value = args->get_int();
    digitalWrite(pin, value);
}

/* delay(ms:int) */
void proc_delay(lmssArgs *args)
{
    s16 ms = args->get_int();
    delay(ms);
}

/* serial-begin() */
void proc_serial_begin(lmssArgs *args)
{
    s16 baud = args->get_int();
    Serial.begin(baud);
}


/* print(obj:any) */
void proc_print(lmssArgs *args)
{
    s16 value_int;
    bool value_bool;
    char *str;

    switch (args->type())
    {
        case LMSSARG_INT:
            value_int = args->get_int();
            Serial.print(value_int);
            break;
        case LMSSARG_STR:
            args->get_str(&str, (u8 *)NULL);
            Serial.print(str);
            break;
        case LMSSARG_BOOL:
            value_bool = args->get_bool();
            if (value_bool) Serial.print("#t");
            else Serial.print("#f");
            break;
        case LMSSARG_NIL:
            Serial.print("#nil");
            break;
    }
    Serial.flush();
}

/* println(obj:any) */
void proc_println(lmssArgs *args)
{
    proc_print(args);
    Serial.println("");
    Serial.flush();
}

/* Native ISR transporter */
void native_isr_transporter(u8 num)
{
    interrupt_vector->trigger(num);
}

void native_isr_transporter_0(void) { native_isr_transporter(0); }
void native_isr_transporter_1(void) { native_isr_transporter(1); }

/* attach_interrupt(num:int, isr:proc, mode:int) */
void proc_attach_int(lmssArgs *args)
{
    u8 num = args->get_int();
    u8 isr = args->get_proc();
    u8 mode = args->get_int();
    void (*func)(void) = (void (*)())NULL;

    /* Native bind */
    switch (num)
    {
        case 0: func = native_isr_transporter_0; break;
        case 1: func = native_isr_transporter_1; break;
        default: return;
    }
    attachInterrupt(num, func, mode);
    interrupt_vector->attach(num, isr);
}

/* detach_interrupt(num:int) */
void proc_detach_int(lmssArgs *args)
{
    u8 num = args->get_int();

    detachInterrupt(num);
    interrupt_vector->detach(num);
}

///* avaliable(obj:any):int */
//void proc_avaliable(lmssArgs *args)
//{
    //args->ret_int(Serial.available());
//}

///* parse-int():int */
//void proc_parse_int(lmssArgs *args)
//{
    //args->ret_int(Serial.parseInt());
//}
//

///* freemem():int */
//void proc_freemem(lmssArgs *args)
//{
    //args->ret_int(freeRam());
//}


#ifdef DEBUG
/* freeres() */
void proc_freeres(lmssArgs *args)
{
    (void)args;
    ref_pool->status();
    pair_pool->status();
    priproc_pool->status();
    bignum_pool->status();
    buffer_pool->status();
}
#endif


void lmss_ino_binds(void)
{
    /* Bind Primitive functions */
    vm->bind_int("led-builtin", 11, LED_BUILTIN);
    //vm->bind_int("high", 4, HIGH);
    //vm->bind_int("low", 3, LOW);
    vm->bind_int("in", 2, INPUT);
    vm->bind_int("out", 3, OUTPUT);
    vm->bind_int("change", 4, CHANGE);
    vm->bind_int("rising", 6, RISING);
    vm->bind_int("falling", 7, FALLING);
    vm->bind_proc("dev.gpio.direction", 18, &proc_pinMode);
    vm->bind_proc("dev.gpio.value", 14, &proc_digitalWrite);
    vm->bind_proc("thread.msleep", 13, &proc_delay);

    /* Interrupt */
    vm->bind_proc("attach-int", 10, &proc_attach_int);
    vm->bind_proc("detach-int", 10, &proc_detach_int);

    /* Serial */
    vm->bind_proc("serial-begin", 12, &proc_serial_begin);
    vm->bind_proc("display", 7, &proc_println);
    //vm->bind_proc("print", 5, &proc_print);
    //vm->bind_proc("avaliable", 9, &proc_avaliable);
    //vm->bind_proc("parse-int", 9, &proc_parse_int);

    /* Freemem */
    //vm->bind_proc("freemem", 7, &proc_freemem);
#ifdef DEBUG
    vm->bind_proc("freeres", 7, &proc_freeres);
#endif
}

