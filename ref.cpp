/* lmss : Data Types */

#include "lmss.h"
#include "ref.h"
#include "err.h"

/* Definitions of pools */
class BigNumPool *bignum_pool = (class BigNumPool *)NULL;
class BufferPool *buffer_pool = (class BufferPool *)NULL;
class RefPool *ref_pool = (class RefPool *)NULL;
class PairPool *pair_pool = (class PairPool *)NULL;
class PriProcPool *priproc_pool = (class PriProcPool *)NULL;
class LambdaProcPool *lambdaproc_pool = (class LambdaProcPool *)NULL;
class InterruptVector *interrupt_vector = (class InterruptVector *)NULL;


/* Type */
u8 obj_type(u8 body)
{
    u8 value = body;
    if (value >= OBJ_REF) return OBJ_REF;
    else if (value >= OBJ_PAIR) return OBJ_PAIR;
    else if (value >= OBJ_LAMBDAPROC) return OBJ_LAMBDAPROC;
    else if (value >= OBJ_PRIPROC) return OBJ_PRIPROC;
    else if (value >= OBJ_BIGNUM) return OBJ_BIGNUM;
    else if (value >= OBJ_BUFFER) return OBJ_BUFFER;
    else if (value == OBJ_T) return OBJ_T;
    else if (value == OBJ_F) return OBJ_F;
    else if (value == OBJ_NIL) return OBJ_NIL;
    else if (value == OBJ_EMPTYLIST) return OBJ_EMPTYLIST;
    else if (value == OBJ_ERROR) return OBJ_ERROR;
    else if (value == OBJ_NOT_MATCH) return OBJ_NOT_MATCH;
    else return OBJ_NUM;
}


/*------------------------------------------------------
 * 'BIG' NUMBER 
 *------------------------------------------------------*/

BigNumItem::BigNumItem(void)
{
    this->value = 0; 
}

void BigNumItem::set(s16 value)
{
    this->value = value; 
}

s16 BigNumItem::get(void)
{
    return this->value; 
}

int BigNumPool::m_make(s16 value)
{
    /* Find a spare space */
    s8 idx_spare = this->bitmap_used.spare();
    if (idx_spare == -1) 
    { return BIGNUM_ITEM_NOT_FOUND; }
    /* Copy Contents */
    this->items[idx_spare].set(value);
    /* Used */
    this->bitmap_used.set((u8)idx_spare);
    /* Update counter */
    size_free -= 1;
    return idx_spare;
}

BigNumPool::BigNumPool(void)
{
    this->size_free = BIGNUM_ITEM_COUNT;
}

u8 BigNumPool::make_bignum(s16 value)
{
    u8 idx = m_make(value);
    if (idx == BIGNUM_ITEM_NOT_FOUND)
    { return BIGNUM_ITEM_NOT_FOUND; }
    return idx;
}

void BigNumPool::clr_used_mark(void)
{
    this->bitmap_used.clr_all();
}
void BigNumPool::mark_used_mark(u8 idx)
{
    this->bitmap_used.set(idx);
}


/*------------------------------------------------------
 * 'BUFFER'
 *------------------------------------------------------*/

RefItem::RefItem(void) 
{ 
    this->len = 0;
    this->check = 0;
}

RefItem::RefItem(char *p_src, u8 len) 
{
    this->set(p_src, len); 
}

void RefItem::set(char *p_src, u8 len)
{
    char *p_dst = str;
    this->len = len;
    this->check = 0;
    while (len-- != 0)
    { 
        this->check ^= *p_src;
        *p_dst++ = *p_src++; 
    }
    *p_dst = '\0';
}

bool RefItem::operator==(RefItem rhs)
{
    if (rhs.len != this->len) return false;
    if (rhs.check != this->check) return false;
    u8 len = rhs.len;
    while (len-- != 0)
    {
        if (rhs.str[len] != this->str[len]) return false;
    }
    return true;
}


/*------------------------------------------------------
 * 'BUFFER'
 *------------------------------------------------------*/

BufferItem::BufferItem(void)
{
    this->buf = (char *)NULL; len = 0; 
}

void BufferItem::set(char *buf, u16 len)
{
    this->buf = buf; 
    this->len = len; 
}

char *BufferItem::get_buf(void)
{
    return this->buf; 
}

s16 BufferItem::get_len(void)
{
    return this->len; 
}

void BufferItem::collect()
{
    delete this->buf;
    this->buf = (char *)NULL; 
    len = 0;
}

int BufferPool::m_make(char *buf, s16 len)
{
    /* Find a spare space */
    s8 idx_spare = this->bitmap_used.spare();
    if (idx_spare == -1) 
    { return BUFFER_ITEM_NOT_FOUND; }
    /* Copy Contents */
    this->items[idx_spare].set(buf, len);
    /* Used */
    this->bitmap_used.set((u8)idx_spare);
    /* Update counter */
    size_free -= 1;
    return idx_spare;
}

BufferPool::BufferPool(void)
{
    this->size_free = BUFFER_ITEM_COUNT;
}

u8 BufferPool::make_buffer(char *buf, s16 len)
{
    u8 idx = m_make(buf, len);
    if (idx == BUFFER_ITEM_NOT_FOUND)
    { return BUFFER_ITEM_NOT_FOUND; }
    return idx;
}

void BufferPool::clr_used_mark(void)
{
    this->bitmap_used.clr_all();
}
void BufferPool::mark_used_mark(u8 idx)
{
    this->bitmap_used.set(idx);
}
void BufferPool::collect(void)
{
    u8 i, j, idx;
    /* Find out exist equal item */
    for (i = 0; i != bitmap_used.size; i++) 
        if (bitmap_used.body[i] != 0x00)
        {
            for (j = 0; j != 8; j++)
            {
                if ((bitmap_used.body[i] & (1<<j)) == 0)
                {
                    idx = (i << 3) + j;
                    if (items[idx].get_buf() != NULL)
                    { items[idx].collect(); }
                }
            }
        }
}


/*------------------------------------------------------
 * REFERENCE ITEM (STRING + SYMBOL)
 *------------------------------------------------------*/

int RefPool::m_make(char *p, u8 len)
{
    /* Find a spare space */
    s8 idx_spare = this->bitmap_used.spare();
    if (idx_spare == -1) 
    { return REF_ITEM_NOT_FOUND; }
    /* Copy Contents */
    this->items[idx_spare].set(p, len);
    /* Used */
    this->bitmap_used.set((u8)idx_spare);
    /* Update counter */
    size_free -= 1;
    return idx_spare;
}

int strncmp(char *s1, char *s2, u8 len)
{
    while (len-- != 0)
    {
        if (*s1 != *s2) return *s1 - *s2;
        s1++; s2++;
    }
    return 0;
}

int RefPool::m_exists(char *p, u8 len, int type)
{
    u8 i, j, idx;
    u8 check = 0;
    /* Calculate check */
    for (i = 0; i != len; i++)
    { check ^= *(p + i); }
    /* Find out exist equal item */
    for (i = 0; i != bitmap_used.size; i++) 
        if (bitmap_used.body[i] != 0x00)
        {
            for (j = 0; j != 8; j++)
            {
                if ((bitmap_used.body[i] & (1<<j)) != 0)
                {
                    idx = (i << 3) + j;

                    if ((this->type(idx) == type) && \
                            ((len == items[idx].len) && \
                             (check == items[idx].check) && \
                             (strncmp(p, items[idx].str, len) == 0)))
                    {
                        return idx; 
                    }
                }
            }
        }
    return REF_ITEM_NOT_FOUND;
}

RefPool::RefPool(void)
{
    this->size_free = REF_ITEM_COUNT;
}

u8 RefPool::make_str(char *p, u8 len)
{
    s8 idx;
    /* Find if the value exists */
    idx = m_exists(p, len, REF_ITEM_STR);
    if (idx != REF_ITEM_NOT_FOUND) 
    { return idx; }
    /* Create a new one */
    idx = m_make(p, len);
    if (idx == REF_ITEM_NOT_FOUND)
    { return REF_ITEM_NOT_FOUND; }
    this->bitmap_type.set(idx);
    return idx;
}

u8 RefPool::make_sym(char *p, u8 len)
{
    s8 idx;
    /* Find if the value exists */
    idx = m_exists(p, len, REF_ITEM_SYM);
    if (idx != REF_ITEM_NOT_FOUND) 
    { return idx; }
    /* Create a new one */
    idx = m_make(p, len);
    if (idx == REF_ITEM_NOT_FOUND)
    { return REF_ITEM_NOT_FOUND; }
    this->bitmap_type.clr(idx);
    return idx;
}

u8 RefPool::type(u8 idx)
{
    return this->bitmap_type.value(idx);
}

void RefPool::clr_used_mark(void)
{
    this->bitmap_used.clr_all();
}

void RefPool::mark_used_mark(u8 idx)
{
    this->bitmap_used.set(idx);
}


/*------------------------------------------------------
 * PAIR
 *------------------------------------------------------*/

PairItem::PairItem(void) 
{
    this->car = 0;
    this->cdr = 0;
}

PairItem::PairItem(u8 car, u8 cdr) 
{
    this->car = car;
    this->cdr = cdr;
}

void PairItem::set(u8 car, u8 cdr) 
{
    this->car = car;
    this->cdr = cdr;
}

void PairItem::mark_used_mark(void)
{
    mark_obj(this->car);
    mark_obj(this->cdr);
}

int PairPool::m_make(u8 car_in, u8 cdr_in)
{
    /* Find a spare space */
    s8 idx_spare = this->bitmap_used.spare();
    if (idx_spare == -1) 
    { return PAIR_ITEM_NOT_FOUND; }
    /* Copy Contents */
    this->items[idx_spare].set(car_in, cdr_in);
    /* Used */
    this->bitmap_used.set((u8)idx_spare);
    /* Update counter */
    size_free -= 1;
    return idx_spare;
}

PairPool::PairPool(void) 
{
    this->size_free = PAIR_ITEM_COUNT;
}

int PairPool::make_pair(u8 car_in, u8 cdr_in)
{
    u8 idx = m_make(car_in, cdr_in);
    if (idx == PAIR_ITEM_NOT_FOUND)
    { return PAIR_ITEM_NOT_FOUND; }
    return idx;
}

void PairPool::clr_used_mark(void)
{
    this->bitmap_used.clr_all();
}

void PairPool::mark_used_mark(u8 idx)
{
    this->bitmap_used.set(idx);
    /* Inside */
    this->items[idx].mark_used_mark();
}


/*------------------------------------------------------
 * PRIMITIVE PROC
 *------------------------------------------------------*/

void PriProcItem::set(void (*callback)(lmssArgs *args))
{
    this->callback = callback;
}
void PriProcItem::call(lmssArgs *args)
{
    this->callback(args);
}

int PriProcPool::m_make(void (*callback)(lmssArgs *args))
{
    /* Find a spare space */
    s8 idx_spare = this->bitmap_used.spare();
    if (idx_spare == -1) 
    { return PRIPROC_ITEM_COUNT; }
    /* Copy Contents */
    this->items[idx_spare].set(callback);
    /* Used */
    this->bitmap_used.set((u8)idx_spare);
    /* Update counter */
    size_free -= 1;
    return idx_spare;
}

PriProcPool::PriProcPool(void)
{
    this->size_free = PRIPROC_ITEM_COUNT;
}

int PriProcPool::make_priproc(void (*callback)(lmssArgs *args))
{
    u8 idx = m_make(callback);
    if (idx == PRIPROC_ITEM_NOT_FOUND)
    { return PRIPROC_ITEM_NOT_FOUND; }
    return idx;
}

void PriProcPool::clr_used_mark(void)
{
    this->bitmap_used.clr_all();
}

void PriProcPool::mark_used_mark(u8 idx)
{
    this->bitmap_used.set(idx);
}


/*------------------------------------------------------
 * LAMBDA PROC
 *------------------------------------------------------*/

void LambdaProcItem::set(u8 args, u8 body)
{
    this->args = args;
    this->body = body;
}

u8 LambdaProcItem::get_args(void)
{
    return this->args; 
}

u8 LambdaProcItem::get_body(void)
{
    return this->body; 
}

void LambdaProcItem::call(lmssArgs *args)
{
    (void)args;
}

void LambdaProcItem::mark_used_mark(void)
{
    mark_obj(this->args);
    mark_obj(this->body);
}

int LambdaProcPool::m_make(u8 args, u8 body)
{
    /* Find a spare space */
    s8 idx_spare = this->bitmap_used.spare();
    if (idx_spare == -1) 
    { return LAMBDAPROC_ITEM_COUNT; }
    /* Copy Contents */
    this->items[idx_spare].set(args, body);
    /* Used */
    this->bitmap_used.set((u8)idx_spare);
    /* Update counter */
    size_free -= 1;
    return idx_spare;
}

LambdaProcPool::LambdaProcPool(void)
{
    this->size_free = LAMBDAPROC_ITEM_COUNT;
}

int LambdaProcPool::make_lambdaproc(u8 args, u8 body)
{
    u8 idx = m_make(args, body);
    if (idx == LAMBDAPROC_ITEM_NOT_FOUND)
    { return  LAMBDAPROC_ITEM_NOT_FOUND; }
    return idx;
}

void LambdaProcPool::clr_used_mark(void)
{
    this->bitmap_used.clr_all();
}

void LambdaProcPool::mark_used_mark(u8 idx)
{
    this->bitmap_used.set(idx);
    /* Internal */
    this->items[idx].mark_used_mark();
}


/* Clone */
void ObjClone(Obj *dst, Obj src)
{
    dst->set_body(src.get_body()); 
}

/* Integer */
Obj make_int(s16 v) 
{
    if (v > OBJ_NUM_MAX)
    { return Obj(OBJ_BIGNUM + bignum_pool->make_bignum(v)); }
    else
    { return Obj(v); }
}
s16 extract_int(Obj obj)
{
    if (obj.type() == OBJ_BIGNUM)
    {
        return bignum_pool->items[obj.get_body() - OBJ_BIGNUM].get(); 
    }
    else
    { return obj.get_body() - OBJ_NUM; }
}

/* Error */
Obj make_not_match(void) {
    return Obj(OBJ_NOT_MATCH); 
}
Obj make_err(void) {
    return Obj(OBJ_ERROR); 
}
Obj make_err(const char *msg) {
    err_pool->set(msg);
    return Obj(OBJ_ERROR); 
}


#ifndef DEBUG
#define make_err(x) make_err()
#endif


Obj make_emptylist(void) { return Obj(OBJ_EMPTYLIST); }
Obj make_nil(void) { return Obj(OBJ_NIL); }
Obj make_f(void) { return Obj(OBJ_F); }
Obj make_t(void) { return Obj(OBJ_T); }

/* Pair */
Obj make_pair(Obj car, Obj cdr) 
{
    u8 idx = pair_pool->make_pair(car.get_body(), cdr.get_body());
    if (idx == PAIR_ITEM_NOT_FOUND) 
    { return make_err("out of memory: pair"); }
    return Obj(OBJ_PAIR + idx);
}

/* Symbol/String */
Obj make_sym(char *p, u8 len)
{
    u8 idx = ref_pool->make_sym(p, len); 
    if (idx == REF_ITEM_NOT_FOUND) 
    { return make_err("out of memory: sym"); }
    return Obj(OBJ_REF + idx); 
}
Obj make_str(char *p, u8 len) 
{
    u8 idx = ref_pool->make_str(p, len); 
    if (idx == REF_ITEM_NOT_FOUND)
    { return make_err("out of memory: str"); }
    return Obj(OBJ_REF + idx); 
}
RefItem *extract_sym(Obj obj) { return &ref_pool->items[obj.get_body() - OBJ_REF]; }
RefItem *extract_str(Obj obj) { return extract_sym(obj); }
bool is_obj_str(Obj obj) 
{ return ref_pool->type(obj.get_body() - OBJ_REF) == REF_ITEM_STR; }
bool is_obj_sym(Obj obj) 
{ return ref_pool->type(obj.get_body() - OBJ_REF) == REF_ITEM_SYM; }

/* Buffer */
Obj make_buf(char *p, u8 len) 
{
    u8 idx = buffer_pool->make_buffer(p, len); 
    if (idx == BUFFER_ITEM_NOT_FOUND)
    { return make_err("out of memory: buf"); }
    return Obj(OBJ_BUFFER + idx); 
}

/* Primitive Procedure */
Obj make_priproc(void (*callback)(lmssArgs *args))
{
    u8 idx = priproc_pool->make_priproc(callback);
    if (idx == PRIPROC_ITEM_NOT_FOUND) 
    { return make_err("out of memory: primitive procedure"); }
    return Obj(OBJ_PRIPROC + idx); 
}
PriProcItem *extract_priproc(Obj obj) 
{ return &priproc_pool->items[obj.get_body() - OBJ_PRIPROC]; }

/* Lambda Procedure */
Obj make_lambdaproc(Obj args, Obj body)
{
    u8 idx = lambdaproc_pool->make_lambdaproc(args.get_body(), body.get_body());
    if (idx == LAMBDAPROC_ITEM_NOT_FOUND) 
    { return make_err("out of memory: lambda procedure"); }
    return Obj(OBJ_LAMBDAPROC + idx);
}
Obj extract_lambdaproc_args(Obj obj) 
{ return Obj(lambdaproc_pool->items[obj.get_body() - OBJ_LAMBDAPROC].get_args()); }
Obj extract_lambdaproc_body(Obj obj) 
{ return Obj(lambdaproc_pool->items[obj.get_body() - OBJ_LAMBDAPROC].get_body()); }


void mark_obj(u8 obj_body)
{
    switch (obj_type(obj_body))
    {
        case OBJ_BUFFER:
            buffer_pool->mark_used_mark(obj_body - OBJ_BUFFER);
            break;
        case OBJ_BIGNUM:
            bignum_pool->mark_used_mark(obj_body - OBJ_BIGNUM);
            break;
        case OBJ_REF:
            ref_pool->mark_used_mark(obj_body - OBJ_REF);
            break;
        case OBJ_PAIR:
            pair_pool->mark_used_mark(obj_body - OBJ_PAIR);
            break;
        case OBJ_PRIPROC:
            priproc_pool->mark_used_mark(obj_body - OBJ_PRIPROC);
            break;
        case OBJ_LAMBDAPROC:
            lambdaproc_pool->mark_used_mark(obj_body - OBJ_LAMBDAPROC);
            break;
    }
}

void mark_obj(Obj obj)
{
    mark_obj(obj.get_body());
}


