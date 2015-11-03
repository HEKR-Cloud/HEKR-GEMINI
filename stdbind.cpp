/* lmss : Stdlib binds */

#include "Arduino.h"

#include "lmss.h"
#include "args.h"
#include "vm.h"


/* not(s:bool) */
void proc_stdlib_not(lmssArgs *args)
{
    bool v = args->get_bool();
    args->ret_bool(!v);
}

bool proc_stdlib_eq_raw(lmssArg *arg1, lmssArg *arg2)
{
    char *str1; u8 len1;
    char *str2; u8 len2;

    if (arg1->type() != arg2->type())
    { return false; }

    switch (arg1->type())
    {
        case LMSSARG_NIL:
            return true;
        case LMSSARG_INT:
            return (arg1->get_int() == arg2->get_int());
        case LMSSARG_BOOL:
            return (arg1->get_bool() == arg2->get_bool());
        case LMSSARG_STR:
            arg1->get_str(&str1, &len1);
            arg2->get_str(&str2, &len2);
            if (len1 != len2)
            { return false; }
            while (len1-- != 0)
            {
                if (*str1++ != *str2++)
                { return false; }
            }
            return true;
    }
    return false;
}

/* =(v1:any, v2:any) */
void proc_stdlib_eq(lmssArgs *args)
{
    lmssArg *arg1 = args->get_any();
    lmssArg *arg2 = args->get_any();

    args->ret_bool(proc_stdlib_eq_raw(arg1, arg2));
}

bool proc_stdlib_l_raw(lmssArg *arg1, lmssArg *arg2)
{
    if (arg1->type() != arg2->type())
    { return false; }

    switch (arg1->type())
    {
        case LMSSARG_INT:
            return (arg1->get_int() < arg2->get_int());
        case LMSSARG_NIL:
        case LMSSARG_BOOL:
        case LMSSARG_STR:
            return false;
    }
    return false;
}

bool proc_stdlib_g_raw(lmssArg *arg1, lmssArg *arg2)
{
    if (arg1->type() != arg2->type())
    { return false; }

    switch (arg1->type())
    {
        case LMSSARG_INT:
            return (arg1->get_int() > arg2->get_int());
        case LMSSARG_NIL:
        case LMSSARG_BOOL:
        case LMSSARG_STR:
            return false;
    }
    return false;
}

/* <(v1:any, v2:any) */
void proc_stdlib_l(lmssArgs *args)
{
    lmssArg *arg1 = args->get_any();
    lmssArg *arg2 = args->get_any();
    args->ret_bool(proc_stdlib_l_raw(arg1, arg2));
}

///* >=(v1:any, v2:any) */
//void proc_stdlib_ge(lmssArgs *args)
//{
    //lmssArg *arg1 = args->get_any();
    //lmssArg *arg2 = args->get_any();
    //args->ret_bool(!proc_stdlib_l_raw(arg1, arg2));
//}

///* >(v1:any, v2:any) */
//void proc_stdlib_g(lmssArgs *args)
//{
    //lmssArg *arg1 = args->get_any();
    //lmssArg *arg2 = args->get_any();
    //args->ret_bool(proc_stdlib_g_raw(arg1, arg2));
//}

///* <=(v1:any, v2:any) */
//void proc_stdlib_le(lmssArgs *args)
//{
    //lmssArg *arg1 = args->get_any();
    //lmssArg *arg2 = args->get_any();
    //args->ret_bool(!proc_stdlib_g_raw(arg1, arg2));
//}


/* +(v1:int, v2:int) */
void proc_stdlib_add(lmssArgs *args)
{
    s16 v1 = args->get_int();
    s16 v2 = args->get_int();
    args->ret_int(v1 + v2);
}

///* -(v1:int, v2:int) */
//void proc_stdlib_sub(lmssArgs *args)
//{
    //s16 v1 = args->get_int();
    //s16 v2 = args->get_int();
    //args->ret_int(v1 - v2);
//}

///* *(v1:int, v2:int) */
//void proc_stdlib_mul(lmssArgs *args)
//{
    //s16 v1 = args->get_int();
    //s16 v2 = args->get_int();
    //args->ret_int(v1 * v2);
//}

///* /(v1:int, v2:int) */
//void proc_stdlib_div(lmssArgs *args)
//{
    //s16 v1 = args->get_int();
    //s16 v2 = args->get_int();
    //args->ret_int(v1 / v2);
//}

///* mod(v1:int, v2:int) */
//void proc_stdlib_mod(lmssArgs *args)
//{
    //s16 v1 = args->get_int();
    //s16 v2 = args->get_int();
    //args->ret_int(v1 % v2);
//}


void lmss_stdlib_binds(void)
{
    /* Bind Primitive functions */
    vm->bind_proc("not", 3, proc_stdlib_not);
    vm->bind_proc("=", 1, proc_stdlib_eq);
    //vm->bind_proc("eq?", 3, proc_stdlib_eq);
    vm->bind_proc("<", 1, proc_stdlib_l);
    //vm->bind_proc(">", 1, proc_stdlib_g);
    //vm->bind_proc("<=", 2, proc_stdlib_le);
    //vm->bind_proc(">=", 2, proc_stdlib_ge);
    vm->bind_proc("+", 1, proc_stdlib_add);
    //vm->bind_proc("-", 1, proc_stdlib_sub);
    //vm->bind_proc("*", 1, proc_stdlib_mul);
    //vm->bind_proc("/", 1, proc_stdlib_div);
    //vm->bind_proc("modulo", 6, proc_stdlib_mod);
}

