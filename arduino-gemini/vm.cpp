/* lmss : Virtual Machine */

#include "lmss.h"
#include "vm.h"
#include "args.h"


#ifndef DEBUG
#define make_err(x) make_err()
#endif


VM *vm = (class VM *)NULL;


/* Variable */

void Var::mark_used_mark(void)
{
    mark_obj(this->name);
    mark_obj(this->value);
}


/* Environment */

void Env::append_var(Var *var) 
{ 
    if (this->var == NULL) this->var = var;
    else
    {
        var->next = this->var;
        this->var = var;
    }
}

Var *Env::lookup_var_by_refitem(RefItem *refitem)
{
    Var *var_cur = var;

    while (var_cur != NULL)
    {
        if (*refitem == *var_cur->get_name_ref_item())
        { return var_cur; }
        var_cur = var_cur->next; 
    }

    return (Var *)NULL;
}

Var *Env::lookup_var_by_obj(Obj obj)
{
    return lookup_var_by_refitem(obj.extract_ref());
}

Var *Env::lookup_var_by_name(char *name, u8 len)
{
    RefItem tmp_refitem = RefItem(name, len);
    return lookup_var_by_refitem(&tmp_refitem);
}

void Env::mark_used_mark(void)
{
    Var *var_cur = var;

    while (var_cur != NULL)
    {
        var_cur->mark_used_mark();
        var_cur = var_cur->next; 
    }
}


/* Interrupt */

Interrupt::Interrupt(void)
{
    this->isr = 0;
}

void Interrupt::set(u8 body)
{
    this->isr = body; 
}

u8 Interrupt::get(void)
{
    return this->isr; 
}

void Interrupt::mark_used_mark(void)
{
    mark_obj(this->isr);
}


/* Virtual Machine */

Obj VM::parse_number(IO *io)
{
    s16 value = 0;
    while ((!io->eof()) && io->match_digit())
    { value = value * 10 + ((io->get_char()) - '0'); }
    if (value >= OBJ_BIGNUM_MAX)
    { return make_err("number limit exceed"); }
    return make_int(value);
}

/* bool + nil (#f, #t, #nil) */
Obj VM::parse_sharp(IO *io)
{
    if (io->eof())
    { return make_err("invalid constant starts with \'#\'"); }
    if (io->match_char('f')) 
    { io->forward(); return make_f(); }
    else if (io->match_char('t'))
    { io->forward(); return make_t(); }
    else if (io->match_str("nil", 3)) 
    { io->forward(3); return make_nil(); }
    else 
    { return make_err("invalid constant starts with \'#\'"); }
}

Obj VM::parse_str(IO *io)
{
    char *p = io->get_ptr();
    while ((!io->eof()) && (!(io->match_char('\"'))))
    { io->forward(); }
    if (io->get_ptr() - p > REF_ITEM_LEN)
    { return make_err("string length limit exceed"); }
    Obj ret = make_str(p, io->get_ptr() - p);
    io->forward();
    return ret;
}

Obj VM::parse_sym(IO *io)
{
    io->backword();
    char *p = io->get_ptr();
    while ((!io->eof()) && (IS_ID(io->peek_char())))
    { io->forward(); }
    if (io->get_ptr() - p > REF_ITEM_LEN)
    { return make_err("symbol length limit exceed"); }
    Obj ret = make_sym(p, io->get_ptr() - p);
    return ret;
}

Obj VM::parse_pair(IO *io)
{
    Obj car, cdr, pair;

    /* blank pair */
    if (io->peek_char() == ')')
    {
        io->forward();
        return make_emptylist();
    }

    /* Parse CAR */
    car = parse(io);
    if (car.type() == OBJ_ERROR) 
    { return make_err(); }

    /* skip whitespace */
    io->skip_whitespace();

    /* Parse CDR */
    if (io->peek_char() == ')')
    {
        cdr = make_emptylist();
        io->forward();
    }
    else
    {
        cdr = parse_pair(io);
        if (cdr.type() == OBJ_ERROR) 
        { return make_err(); }
    }

    /* Parse CDR */
    pair = make_pair(car, cdr);

    return pair;
}

Obj VM::parse(IO *io)
{

    /* skip whitespace */
    io->skip_whitespace();

    u8 ch = io->get_char();

    if (ch == '#') { return parse_sharp(io); }
    else if (ch == '\"') { return parse_str(io); }
    else if (ch == '(') { return parse_pair(io); }
    else if (IS_NUM(ch)) 
    { 
        io->backword();
        return parse_number(io); 
    }
    else if (IS_ID1(ch))
    {
        return parse_sym(io); 
    }
    else
    {
        return make_err("invalid expression");
    }
}

/* Evaluate */
Obj VM::eval_obj(Obj obj)
{
    Obj obj_ret;

    /* Primitive data */
    switch (obj.type())
    {
        case OBJ_NUM:
        case OBJ_BIGNUM:
        case OBJ_EMPTYLIST:
        case OBJ_NIL:
        case OBJ_F:
        case OBJ_T:
        case OBJ_PRIPROC:
            ObjClone(&obj_ret, obj);
            return obj_ret;
        case OBJ_REF:
            if (is_obj_sym(obj))
            {
                Env *env_cur = this->env;
                Var *var_target = (Var *)NULL;
                while (env_cur != NULL)
                {
                    var_target = env_cur->lookup_var_by_obj(obj);
                    if (var_target != NULL) 
                    {
                        return var_target->get_value();
                    }
                    env_cur = env_cur->prev;
                }
                return make_err();
            }
            else
            {
                /* String */
                ObjClone(&obj_ret, obj);
                return obj_ret;
            }
    }

    /* (define */
    obj_ret = eval_define(obj);
    if (obj_ret.type() != OBJ_NOT_MATCH) { return obj_ret; }
    /* (set! */
    obj_ret = eval_set(obj);
    if (obj_ret.type() != OBJ_NOT_MATCH) { return obj_ret; }
    /* (lambda */
    obj_ret = eval_lambda(obj);
    if (obj_ret.type() != OBJ_NOT_MATCH) { return obj_ret; }
    /* (begin */
    obj_ret = eval_begin(obj);
    if (obj_ret.type() != OBJ_NOT_MATCH) { return obj_ret; }
    /* (if <cond> <true> [false])*/
    obj_ret = eval_if(obj);
    if (obj_ret.type() != OBJ_NOT_MATCH) { return obj_ret; }
    /* (while <cond> [body])*/
    obj_ret = eval_while(obj);
    if (obj_ret.type() != OBJ_NOT_MATCH) { return obj_ret; }

    /* Call */
    obj_ret = eval_proc_call(obj);
    if (obj_ret.type() != OBJ_NOT_MATCH) { return obj_ret; }

    return make_err("invalid expression");
}

Obj VM::eval_stmts(Obj obj)
{
    Obj car, obj_ret;

    while (obj.type() != OBJ_EMPTYLIST)
    {
        car = obj.extract_pair_car();
        obj_ret = eval_obj(car);
        obj = obj.extract_pair_cdr();
    }

    return obj_ret;
}

Obj VM::labeled_pair(Obj obj, const char *lbl, const u8 len)
{
    Obj car;
    if (obj.type() != OBJ_PAIR) return make_not_match();
    car = obj.extract_pair_car();
    if (car.type() != OBJ_REF) return make_not_match();
    if (is_obj_sym(car))
    {
        RefItem *item = extract_sym(car);
        if (!(*item == RefItem((char *)lbl, len)))
        { return make_not_match(); }
    }
    return obj.extract_pair_cdr();
}

/* (lambda (pars) <body>)*/
Obj VM::eval_lambda(Obj obj)
{
    Obj car, rest, pars, body;

    obj = labeled_pair(obj, "lambda", 6);
    if (obj.type() == OBJ_NOT_MATCH) return make_not_match();
    if (obj.type() == OBJ_EMPTYLIST) 
    { return make_err("bad syntax: parameter list and body expected"); }

    pars = obj.extract_pair_car();
    if ((pars.type() != OBJ_PAIR) && (pars.type() != OBJ_EMPTYLIST)) 
    { return make_err("bad syntax: invalid parameter list"); }
    ObjClone(&rest, pars);
    while (rest.type() != OBJ_EMPTYLIST)
    {
        car = rest.extract_pair_car();
        if (car.type() != OBJ_REF) return make_err("no an identifier in parameter list");
        rest = rest.extract_pair_cdr();
    }

    body = obj.extract_pair_cdr();
    if (body.type() == OBJ_EMPTYLIST) return make_err("no expression in body");

    return make_lambdaproc(pars, body);
}

/* (begin <body>)*/
Obj VM::eval_begin(Obj obj)
{
    Obj car, body;

    obj = labeled_pair(obj, "begin", 5);
    if (obj.type() == OBJ_NOT_MATCH) return make_not_match();

    /* body part */
    if (obj.type() == OBJ_EMPTYLIST)
    { body = make_nil(); }
    else
    { body = obj; }

    return eval_stmts(body);
}

/* (define name value) */
Obj VM::eval_define(Obj obj)
{
    Var *var_target = (Var *)NULL;
    Obj car, varname, value;

    obj = labeled_pair(obj, "define", 6);
    if (obj.type() == OBJ_NOT_MATCH) return make_not_match();
    if (obj.type() == OBJ_EMPTYLIST) return make_err("bad syntax");

    /* Extract varname and value */
    varname = obj.extract_pair_car();
    if (varname.type() != OBJ_REF) return make_err("not an identifier");
    obj = obj.extract_pair_cdr();
    if (obj.type() == OBJ_EMPTYLIST) return make_err("bad syntax");
    value = eval_obj(obj.extract_pair_car());
    if (obj.type() == OBJ_ERROR) return obj;

    /* Lookup variable from the top environment */
    var_target = this->env->lookup_var_by_obj(varname);
    if (var_target == NULL)
    {
        /* Variable not found */
        Var *new_var = new Var(varname, \
                value);
        this->env->append_var(new_var);
    }
    else
    {
        var_target->set_value(value);
    }

    return make_nil();
}

/* (set! name value) */
Obj VM::eval_set(Obj obj)
{
    Env *env_cur = this->env;
    Var *var_target = (Var *)NULL;
    Obj car, varname, value;

    obj = labeled_pair(obj, "set!", 4);
    if (obj.type() == OBJ_NOT_MATCH) return make_not_match();
    if (obj.type() == OBJ_EMPTYLIST) return make_err("bad syntax");

    /* Extract varname and value */
    varname = obj.extract_pair_car();
    if (varname.type() != OBJ_REF) return make_err("not an identifier");
    obj = obj.extract_pair_cdr();
    if (obj.type() == OBJ_EMPTYLIST) return make_err("bad syntax");
    value = eval_obj(obj.extract_pair_car());
    if (obj.type() == OBJ_ERROR) return obj;

    /* Lookup variable from environments */
    while (env_cur != NULL)
    {
        var_target = env_cur->lookup_var_by_obj(varname);
        if (var_target != NULL) { break; }
        env_cur = env_cur->prev;
    }
    if (var_target == NULL) 
    { return make_err("no reference to the identifier"); }

    var_target->set_value(value);

    return make_nil();
}

/* (if <cond> <true-part> [false-part] */
Obj VM::eval_if(Obj obj)
{
    Obj car, obj_ret, cond, true_part, false_part;
    false_part = make_err();

    obj = labeled_pair(obj, "if", 2);
    if (obj.type() == OBJ_NOT_MATCH) return make_not_match();
    if (obj.type() == OBJ_EMPTYLIST) return make_err("bad syntax, condition expected");

    /* Condition */
    cond = obj.extract_pair_car();
    obj = obj.extract_pair_cdr();
    if (obj.type() == OBJ_EMPTYLIST) return make_err("bad syntax, true branch expected");

    /* True part */
    true_part = obj.extract_pair_car();
    obj = obj.extract_pair_cdr();

    /* False part */
    if (obj.type() == OBJ_EMPTYLIST)
    { false_part = make_nil(); }
    else
    { false_part = obj; }

    obj_ret = eval_obj(cond);
    if (obj_ret.type() != OBJ_F)
    {
        /* True */
        obj_ret = eval_obj(true_part);
    }
    else
    {
        /* False */
        if (false_part.type() == OBJ_ERROR)
        { obj_ret = make_nil(); }
        else
        { obj_ret = eval_stmts(false_part); }
    }

    return obj_ret;
}

/* (while <cond> <body>) */
Obj VM::eval_while(Obj obj)
{
    Obj car, obj_ret, cond, body;

    obj = labeled_pair(obj, "while", 5);
    if (obj.type() == OBJ_NOT_MATCH) return make_not_match();
    if (obj.type() == OBJ_EMPTYLIST) return make_err("bad syntax, condition expected");

    /* condition */
    cond = obj.extract_pair_car();
    obj = obj.extract_pair_cdr();
    if (obj.type() == OBJ_EMPTYLIST) return make_err("bad syntax, no expression in branch");

    /* body part */
    if (obj.type() == OBJ_EMPTYLIST)
    { body = make_nil(); }
    else
    { body = obj; }

    for (;;)
    {
        obj_ret = eval_obj(cond);
        if (obj_ret.type() == OBJ_F)
        { break; }

        eval_stmts(body);
    }

    return make_nil();
}

/* (<procedure> [args]) (Primitive Version) */
Obj VM::eval_proc_call_primitive(PriProcItem *priprocitem_target, Obj obj_args)
{
    lmssArgs args;

    /* Arguments */
    if (args_fill(&args, obj_args).type() == OBJ_ERROR)
    { return make_err(); }

    /* Call the function */
    priprocitem_target->call(&args); 

    /* Return the result of calling */
    return args_ret(&args);
}


/* (<procedure> [args]) (Lambda Version) */
Obj VM::eval_proc_call_lambda(Obj pars, Obj body, Obj args)
{
    Obj par, arg, obj_ret;

    /* Create a new environment */
    Env *new_env = new Env(this->env);
    if (new_env == NULL) make_err("out of memory");

    /* Set new environment as current one */
    this->env = new_env;

    /* Fill Arguments */
    while (pars.type() != OBJ_EMPTYLIST)
    {
        if (args.type() == OBJ_EMPTYLIST)
        { return make_err("expected number of arguments not match the parameter list"); }

        par = pars.extract_pair_car();
        arg = eval_obj(args.extract_pair_car());
        if (arg.type() == OBJ_ERROR) 
        { 
            ObjClone(&obj_ret, arg);
            goto lbl_release;
        }
        args = args.extract_pair_cdr();

        vm->bind_obj(par, arg);

        pars = pars.extract_pair_cdr();
    }

    if (args.type() != OBJ_EMPTYLIST)
    { return make_err("expected number of arguments not match the parameter list"); }

    /* Evaluate */
    obj_ret = eval_stmts(body);

    /* Return to the previous environment */
lbl_release:
    this->env = new_env->prev;
    delete(new_env);

    /* Return the value to prev environment */
    return obj_ret;
}

/* (<procedure> [args]) */
Obj VM::eval_proc_call_specific(Obj proc, Obj args)
{
    PriProcItem *priprocitem_target = (PriProcItem *)NULL;
    Obj lambda_args, lambda_body;

    /* Call the procedure */
    switch (proc.type())
    {
        case OBJ_PRIPROC:
            priprocitem_target = extract_priproc(proc);
            return eval_proc_call_primitive(priprocitem_target, args);

        case OBJ_LAMBDAPROC:
            lambda_args = extract_lambdaproc_args(proc);
            lambda_body = extract_lambdaproc_body(proc);
            return eval_proc_call_lambda(lambda_args, lambda_body, args);

        default:
            return make_not_match();
    }
}

/* (<procedure> [args]) */
Obj VM::eval_proc_call(Obj obj)
{
    Obj first;

    /* Should be a pair */
    if (obj.type() != OBJ_PAIR) return make_not_match();

    /* First Element */
    first = eval_obj(obj.extract_pair_car());
    obj = obj.extract_pair_cdr();

    return eval_proc_call_specific(first, obj);
}

/* Fill arguments with given object */
Obj VM::args_fill(lmssArgs *args, Obj obj)
{
    Obj car;

    /* Arguments */
    while (obj.type() != OBJ_EMPTYLIST)
    {
        car = obj.extract_pair_car();

        /* Evaluate the node */
        car = eval_obj(car);
        if (car.type() == OBJ_ERROR)
        {
            return make_err();
        }

        switch (car.type())
        {
            case OBJ_NUM:
            case OBJ_BIGNUM:
                args->put_int(extract_int(car));
                break;
            case OBJ_NIL:
                args->put_nil();
                break;
            case OBJ_F:
                args->put_bool(false);
                break;
            case OBJ_T:
                args->put_bool(true);
                break;
            case OBJ_REF:
                if (is_obj_str(car))
                {
                    RefItem *item = extract_str(car);
                    args->put_str(item->str, item->len);
                }
                else
                {
                    args->put_nil();
                }
                break;
            case OBJ_LAMBDAPROC:
            case OBJ_PRIPROC:
                args->put_proc(car.get_body());
                break;
            default:
                args->put_nil();
                break;
        }
        obj = obj.extract_pair_cdr();
    }

    return make_nil();
}

/* Get return value from argument */
Obj VM::args_ret(lmssArgs *args)
{
    char *str; 
    u8 len;

    if (args->has_retval())
    {
        switch (args->retval_type())
        {
            case LMSSARG_INT:
                return make_int(args->get_ret_int());

            case LMSSARG_STR:
                args->get_ret_str(&str, &len);
                return make_str(str, len);

            case LMSSARG_BUF:
                args->get_ret_buf(&str, &len);
                return make_buf(str, len);

            case LMSSARG_BOOL:
                if (args->get_ret_bool())
                { return make_t(); }
                else
                { return make_f(); }

            case LMSSARG_NIL:
            default:
                return make_nil();
        }
    }
    else
    { return make_nil(); }
}

