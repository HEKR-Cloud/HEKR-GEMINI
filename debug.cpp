/* lmss : Debug */

#include "Arduino.h" /* Serial */

#include "lmss.h"
#include "ref.h"

void lmss_print(Obj obj)
{
    u8 value = obj.get_body();
    Obj obj1, obj2;
    u8 idx;
    if (value >= OBJ_REF)
    {
        idx = value - OBJ_REF;
        if (ref_pool->type(idx) == REF_ITEM_SYM)
        {
            Serial.print(ref_pool->items[idx].str);
        }
        else
        {
            Serial.print("\"");
            Serial.print(ref_pool->items[idx].str);
            Serial.print("\"");
        }
    }
    else if (value >= OBJ_PAIR)
    {
        idx = value - OBJ_PAIR;
        obj1 = Obj(pair_pool->items[idx].car);
        obj2 = Obj(pair_pool->items[idx].cdr);

        Serial.print("(cons ");
        lmss_print(obj1);
        Serial.print(" ");
        lmss_print(obj2);
        Serial.print(")");
    }
    else if (value >= OBJ_LAMBDAPROC)
    { Serial.print("#lambda-proc"); }
    else if (value >= OBJ_PRIPROC)
    { Serial.print("#primitive-proc"); }
    else if (value >= OBJ_BIGNUM)
    {
        idx = value - OBJ_BIGNUM;
        Serial.print(bignum_pool->items[idx].get());
    }
    else if (value >= OBJ_BUFFER)
    { Serial.print("#buffer"); }
    else if (value == OBJ_T)
    { Serial.print("#t"); }
    else if (value == OBJ_F)
    { Serial.print("#f"); }
    else if (value == OBJ_NIL)
    { Serial.print("#nil"); }
    else if (value == OBJ_EMPTYLIST)
    { Serial.print("'()"); }
    else if (value == OBJ_ERROR)
    { Serial.print("#error"); }
    else
    { Serial.print(value); }
    Serial.flush();
}

void lmss_println(Obj obj)
{
    lmss_print(obj);
    Serial.println("");
    Serial.flush();
}

#ifdef DEBUG
void obj_print(Obj obj)
{
    lmss_print(obj);
}


void debug_newline(void)
{
    Serial.println("");
}
void debug_print(Obj obj)
{
    //Serial.print(">> ");
    obj_print(obj);
}
void debug_print(const char *s)
{
    //Serial.print(">> ");
    Serial.print(s);
}
void debug_println(Obj obj)
{
    debug_print(obj);
    debug_newline();
}
void debug_println(const char *s)
{
    debug_print(s);
    debug_newline();
}
#endif

