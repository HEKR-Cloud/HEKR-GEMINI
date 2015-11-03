/* lmss : Virtual Machine */

#ifndef _VM_H_
#define _VM_H_

#include "Arduino.h"

#include "lmss.h"
#include "parser.h"
#include "ref.h"

/* Interrupt */

class Interrupt
{
    private:
        u8 isr;
    public:
        Interrupt(void);
        void set(u8 body);
        u8 get(void);

        /* GC Interface */
        void mark_used_mark(void);
};
class InterruptVector
{
    private:
        Interrupt ints[8];
        /* 1 byte should be able to handle 8 interrupts */
        Bitmap<1> bitmap_enabled;
        Bitmap<1> bitmap_triggered;

        void enable(u8 num)
        { this->bitmap_enabled.set(num); }
        void disable(u8 num)
        { this->bitmap_enabled.clr(num); }

    public:

        void attach(u8 num, u8 obj_proc)
        {
            //Serial.print("Attach Interrupt #");
            //Serial.println(num);
            //Serial.print("Procedure:");
            //Serial.println(obj_proc);
            this->enable(num);
            this->ints[num].set(obj_proc);
        }

        void detach(u8 num)
        {
            this->disable(num);
        }

        void trigger(u8 num)
        { this->bitmap_triggered.set(num); }

        Obj get_interrupt_event(void)
        {
            /* Get the trigger interrupt number */
            s8 num = this->bitmap_triggered.busy();
            if (num == -1) return make_nil();
            /* Mark as been confirmed */
            this->bitmap_triggered.clr(num); 

            /* Retrieve the attached procedure with 
             * the interrupt number */
            return Obj(this->ints[num].get());
        }

        /* GC Interface */
#ifdef DEBUG
        void status(void)
        {
            //Serial.print("Interrupts:");
            //Serial.print(this->bitmap_used.num_used());
            //Serial.print("/");
            //Serial.print(this->bitmap_used.num_total());
            //Serial.println("");
        }
#endif
        void mark_used_mark(void)
        {
            for (u8 i = 0; i != 2; i++)
            {
                if (this->bitmap_enabled.value(i) != 0)
                {
                    mark_obj(this->ints[i].get());
                }
            }
        }
};


/* Variable */
class Var
{
    private:
        Obj name;
        Obj value;
    public:
        Var *next;
        Var(Obj name, Obj value) 
        {
            ObjClone(&this->name, name);
            ObjClone(&this->value, value);
            this->next = (Var *)NULL;
        }
        RefItem *get_name_ref_item(void)
        {
            return extract_sym(this->name);
        }
        Obj get_value(void) { return this->value; }
        void set_value(Obj obj) { this->value = obj; }

        /* GC Interface */
        void mark_used_mark(void);
};


/* Environment */
class Env
{
    private:
        Var *var; 
    public:
        Env *prev;

        /* Empty env */
        Env(void) 
        {
            this->var = (Var *)NULL;
            this->prev = (Env *)NULL; 
        }
        /* Normal env (has previous env) */
        Env(Env *prev) 
        {
            this->var = (Var *)NULL;
            this->prev = prev; 
        }
        ~Env(void)
        {
            Var *var_next;

            while (this->var)
            {
                var_next = this->var->next;
                delete(this->var);
                this->var = var_next;
            }
        }
        /* Append new variable in current environment */
        void append_var(Var *var);
        /* Lookup variables */
        Var *lookup_var_by_refitem(RefItem *refitem);
        Var *lookup_var_by_obj(Obj obj);
        Var *lookup_var_by_name(char *name, u8 len);

        /* GC Interface */
        void mark_used_mark(void);
};

class VM 
{
    private:
        class Env *env;

        /* Parse number */
        Obj parse_number(IO *io);
        /* Parse sharp started
         * Objects (#f, #t, #nil) */
        Obj parse_sharp(IO *io);
        /* Parse str */
        Obj parse_str(IO *io);
        /* Parse symbol */
        Obj parse_sym(IO *io);
        /* Parse pair */
        Obj parse_pair(IO *io);
        /* Parse object */
        Obj parse(IO *io);
        /* Evaluate */
        Obj eval_obj(Obj obj);
        /* Evaluate Statements */
        Obj eval_stmts(Obj obj);
        /* Detect labeled pair */
        Obj labeled_pair(Obj obj, const char *lbl, const u8 len);
        /* (lambda (pars) <body>)*/
        Obj eval_lambda(Obj obj);
        /* (begin <body>)*/
        Obj eval_begin(Obj obj);
        /* (define name value) */
        Obj eval_define(Obj obj);
        /* (set! name value) */
        Obj eval_set(Obj obj);
        /* (if <cond> <true-part> [false-part] */
        Obj eval_if(Obj obj);
        /* (while <cond> <body>) */
        Obj eval_while(Obj obj);
        /* (<procedure> [args]) (Primitive Version) */
        Obj eval_proc_call_primitive(PriProcItem *priprocitem_target, \
                Obj obj_args);
        /* (<procedure> [args]) (Lambda Version) */
        Obj eval_proc_call_lambda(Obj pars, Obj body, Obj args);
        /* (<procedure> [args]) */
        Obj eval_proc_call_specific(Obj proc, Obj args);
        /* (<procedure> [args]) */
        Obj eval_proc_call(Obj obj);
        /* Fill argument with given object */
        Obj args_fill(lmssArgs *args, Obj obj);
        /* Get return value from argument */
        Obj args_ret(lmssArgs *args);

    public:
        /* Constructor */
        VM(void)
        {
            /* Set an empty env */
            this->env = new Env();
        };
        /* Bind Integer */
        void bind_int(const char *procname, u8 len, \
                s16 value)
        {
            Var *new_var = new Var( \
                    make_sym((char *)procname, len), \
                    make_int(value));
            this->env->append_var(new_var);
        }
        /* Bind procedure */
        void bind_proc(const char *procname, u8 len, \
                void (*callback)(lmssArgs *args))
        {
            Var *new_var = new Var(make_sym((char *)procname, len), \
                    make_priproc(callback));
            this->env->append_var(new_var);
        }
        /* Bind Object */
        void bind_obj(Obj name, Obj value)
        {
            Var *new_var = new Var(name, value);
            this->env->append_var(new_var);
        }
        void gc()
        {
            /* Clear all marks */
            ref_pool->clr_used_mark();
            pair_pool->clr_used_mark();
            priproc_pool->clr_used_mark();
            lambdaproc_pool->clr_used_mark();
            bignum_pool->clr_used_mark();
            buffer_pool->clr_used_mark();

            /* Mark objects */
            Env *env_cur = this->env;
            while (env_cur != NULL)
            {
                env_cur->mark_used_mark();
                env_cur = env_cur->prev;
            }
            interrupt_vector->mark_used_mark();

            /* Collect */
            buffer_pool->collect();
        }
        Obj eval(const char *p, const u8 len)
        {
            Obj obj_ret = make_nil();

            /* Initialize iostream */
            IO io(p, len);
            for (;;)
            {
                /* Exit when nothing to evaluate */
                if (io.eof()) { break; }

                /* GC */
                gc();

                /* Parse */
                Obj obj = parse(&io);
                if (obj.type() == OBJ_ERROR)
                {
                    ObjClone(&obj_ret, obj);
                    break;
                }

                /* Eval Object */
                obj_ret = eval_obj(obj);
            }
            return obj_ret;
        };

        Obj eval_blank_lambda_proc(Obj obj_proc)
        {
            return this->eval_proc_call_specific(obj_proc, make_emptylist());
        }
};


class VM;
extern VM *vm;

#endif

