/* lmss : Arduino Interrupts */

#include "lmss.h"
#include "vm.h"
#include "ref.h"
#include "err.h"


/* Initialize interrupt 
 * Use in setup() */
void lmss_ino_interrupt_init(void)
{

}

/* Initialize interrupt 
 * Use in loop() */
void lmss_ino_interrupt_process(void)
{
    Obj obj, obj_ret;

    for (;;)
    {
        /* Detect event */
        obj = interrupt_vector->get_interrupt_event();
        if (obj.type() == OBJ_NIL) break;

        /* GC */
        vm->gc();

        /* Evaluate */
        obj_ret = vm->eval_blank_lambda_proc(obj);
        if (obj_ret.type() == OBJ_ERROR)
        {
            Serial.print("Error: ");
            Serial.println(err_pool->get());
        }
    }
}

