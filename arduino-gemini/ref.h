/* lmss : Data Types */

#ifndef _REF_H_
#define _REF_H_

#include "bitmap.h"


class ErrPool;
class BigNumPool;
class BufferPool;
class RefPool;
class PairPool;
class PriProcPool;
class LambdaProcPool;
class InterruptVector;

extern ErrPool *err_pool;
extern BigNumPool *bignum_pool;
extern BufferPool *buffer_pool;
extern RefPool *ref_pool;
extern PairPool *pair_pool;
extern PriProcPool *priproc_pool;
extern LambdaProcPool *lambdaproc_pool;
extern InterruptVector *interrupt_vector;

/* RAM Size 2 KiB (2048 bytes) */

/* Every primitive object (numbers + nil + #f + #t)
 * occupies 1 byte (8 bits).
 * Reference objecs are different, they encode
 * index instead of the value.
 *
 * +----------------------------------+
 * |  8 bits    | Category            |
 * +----------------------------------+
 * |    0 ~  81 | numbers             |
 * |         82 | not match           |
 * |         83 | error               |
 * |         84 | emptylist           |
 * |         85 | nil                 |
 * |         86 | #f                  |
 * |         87 | #t                  |
 * |            |                     |
 * |   88 ~  95 | 'buffer'            |
 * |   96 ~ 127 | 'big' numbers (32)  |
 * |  128 ~ 159 | primitive procs (32)|
 * |  160 ~ 175 | lambda proc (16)    |
 * |  176 ~ 223 | pairs (48)          |
 * |  224 ~ 255 | sym+str (32)        |
 * +----------------------------------+
 */


#define   OBJ_NUM         ((u8)(0))
#define   OBJ_NUM_MAX     ((u8)(81))
#define   OBJ_NOT_MATCH   ((u8)(82))
#define   OBJ_ERROR       ((u8)(83))
#define   OBJ_EMPTYLIST   ((u8)(84))
#define   OBJ_NIL         ((u8)(85))
#define   OBJ_F           ((u8)(86))
#define   OBJ_T           ((u8)(87))
#define   OBJ_BUFFER      ((u8)(88))
#define   OBJ_BIGNUM      ((u8)(96))
#define   OBJ_BIGNUM_MIN  ((s16)(-32768))
#define   OBJ_BIGNUM_MAX  ((s16)(32767))
#define   OBJ_PRIPROC     ((u8)(128))
#define   OBJ_LAMBDAPROC  ((u8)(160))
#define   OBJ_PAIR        ((u8)(176))
#define   OBJ_REF         ((u8)(224))

/* Type */
u8 obj_type(u8 body);


/*------------------------------------------------------
 * 'BIG' NUMBER 
 *------------------------------------------------------*/

/* The current implementation of 'Big' number uses 
 * signed 16-bit number, which is just the normal 'int' type 
 * on arduino platform. */

class BigNumItem
{
    private:
        s16 value;
    public:
        BigNumItem(void);
        void set(s16 value);
        s16 get(void);
};
#define BIGNUM_ITEM_COUNT 32
#define BIGNUM_ITEM_NOT_FOUND BIGNUM_ITEM_COUNT 
class BigNumPool
{
    private:
        /* Counter */
        u8 size_free;
        /* 32 items -> 32 bits -> 4 bytes */
        Bitmap<4> bitmap_used;

        /* Make something and returns the record index */
        int m_make(s16 value);
    public:
        /* Items */
        BigNumItem items[BIGNUM_ITEM_COUNT];

        BigNumPool(void);
        /* Allocate a new bignum */
        u8 make_bignum(s16 value);

        /* GC Interface */
#ifdef DEBUG
        void status(void)
        {
            Serial.print("Bignum:");
            Serial.print(this->bitmap_used.num_used());
            Serial.print("/");
            Serial.print(this->bitmap_used.num_total());
            Serial.println("");
        }
#endif
        void clr_used_mark(void);
        void mark_used_mark(u8 idx);
};


/*------------------------------------------------------
 * 'BUFFER'
 *------------------------------------------------------*/

/* A data structure for storing a 'block' of bytes into memory */

class BufferItem
{
    private:
        char *buf;
        u16 len;
    public:
        BufferItem(void);
        void set(char *buf, u16 len);
        char *get_buf(void);
        s16 get_len(void);
        void collect();
};
#define BUFFER_ITEM_COUNT 8
#define BUFFER_ITEM_NOT_FOUND BUFFER_ITEM_COUNT 
class BufferPool
{
    private:
        /* Counter */
        u8 size_free;

        /* 8 items -> 8 bits -> 1 bytes */
        Bitmap<1> bitmap_used;

        /* Make something and returns the record index */
        int m_make(char *buf, s16 len);

    public:
        /* Items */
        BufferItem items[BUFFER_ITEM_COUNT];

        BufferPool(void);

        /* Allocate a new buffer */
        u8 make_buffer(char *buf, s16 len);

        /* GC Interface */
#ifdef DEBUG
        void status(void)
        {
            Serial.print("Buffer:");
            Serial.print(this->bitmap_used.num_used());
            Serial.print("/");
            Serial.print(this->bitmap_used.num_total());
            Serial.println("");
        }
#endif
        void clr_used_mark(void);
        void mark_used_mark(u8 idx);
        void collect(void);
};


/*------------------------------------------------------
 * REFERENCE ITEM (STRING + SYMBOL)
 *------------------------------------------------------*/

/*
 * Reference Objects (symbols + strings share the same pool), 
 * each item has:
 * 14 bytes for content (14 bytes usable);
 * 1 byte for length;
 * 1 byte for check;
 * 16 bytes * 32 items = 512 bytes
 */

#define REF_ITEM_LEN 14
#define REF_ITEM_COUNT 32
#define REF_ITEM_NOT_FOUND REF_ITEM_COUNT 
#define REF_ITEM_SYM 0
#define REF_ITEM_STR 1
class RefItem
{
    public:
        char str[REF_ITEM_LEN];
        u8 len;
        u8 check;

        RefItem(void);
        RefItem(char *p_src, u8 len);

        void set(char *p_src, u8 len);
        bool operator==(RefItem rhs);
};
class RefPool
{
    private:
        /* Counter */
        u8 size_free;

        /* 32 items -> 32 bits -> 4 bytes 
         * 0 indicates symbol
         * 1 indicates string */
        Bitmap<4> bitmap_type;
        Bitmap<4> bitmap_used;

        /* Make something and returns the record index */
        int m_make(char *p, u8 len);
        int m_exists(char *p, u8 len, int type);
    public:
        /* Items */
        /* Is providing direct access a good idea? */
        RefItem items[REF_ITEM_COUNT];

        RefPool(void);

        /* Allocate a new string */
        u8 make_str(char *p, u8 len);
        /* Allocate a new Symbol */
        u8 make_sym(char *p, u8 len);
        /* Type */
        u8 type(u8 idx);

        /* GC Interface */
#ifdef DEBUG
        void status(void)
        {
            Serial.print("Ref:");
            Serial.print(this->bitmap_used.num_used());
            Serial.print("/");
            Serial.print(this->bitmap_used.num_total());
            Serial.println("");
        }
#endif
        void clr_used_mark(void);
        void mark_used_mark(u8 idx);
};


/*------------------------------------------------------
 * PAIR
 *------------------------------------------------------*/

/* Pair Items are used for store pairs.
 * Each pair has 2 slots, car and cdr. */
class PairItem
{
    public:
        u8 car;
        u8 cdr;
        PairItem(void);
        PairItem(u8 car, u8 cdr);
        void set(u8 car, u8 cdr);

        /* GC Interface */
        void mark_used_mark(void);
};
#define PAIR_ITEM_COUNT 64
#define PAIR_ITEM_NOT_FOUND PAIR_ITEM_COUNT
class PairPool
{
    private:
        u8 size_free;

        Bitmap<8> bitmap_used;
        /* Make something and returns the record index */
        int m_make(u8 car_in, u8 cdr_in);

    public:
        PairItem items[PAIR_ITEM_COUNT];
        PairPool(void);
        /* Allocate a new pair */
        int make_pair(u8 car_in, u8 cdr_in);

        /* GC Interface */
#ifdef DEBUG
        void status(void)
        {
            Serial.print("Pair:");
            Serial.print(this->bitmap_used.num_used());
            Serial.print("/");
            Serial.print(this->bitmap_used.num_total());
            Serial.println("");
        }
#endif
        void clr_used_mark(void);
        void mark_used_mark(u8 idx);
};



/*------------------------------------------------------
 * PRIMITIVE PROC
 *------------------------------------------------------*/

class lmssArgs;

class PriProcItem
{
    private:
        void (*callback)(lmssArgs *args);
    public:
        void set(void (*callback)(lmssArgs *args));
        void call(lmssArgs *args);
};
#define PRIPROC_ITEM_COUNT 32
#define PRIPROC_ITEM_NOT_FOUND PRIPROC_ITEM_COUNT
class PriProcPool
{
    private:
        /* Counter */
        u8 size_free;

        Bitmap<4> bitmap_used;
        /* Make something and returns the record index */
        int m_make(void (*callback)(lmssArgs *args));
    public:
        /* Items */
        PriProcItem items[PRIPROC_ITEM_COUNT];
        PriProcPool(void);
        /* Primitive a new procedure */
        int make_priproc(void (*callback)(lmssArgs *args));

        /* GC Interface */
#ifdef DEBUG
        void status(void)
        {
            Serial.print("Primitive Proc:");
            Serial.print(this->bitmap_used.num_used());
            Serial.print("/");
            Serial.print(this->bitmap_used.num_total());
            Serial.println("");
        }
#endif
        void clr_used_mark(void);
        void mark_used_mark(u8 idx);
};


/*------------------------------------------------------
 * LAMBDA PROC
 *------------------------------------------------------*/

class LambdaProcItem
{
    private:
        u8 args;
        u8 body;
    public:
        void set(u8 args, u8 body);
        u8 get_args(void);
        u8 get_body(void);
        void call(lmssArgs *args);

        /* GC Interface */
        void mark_used_mark(void);
};
#define LAMBDAPROC_ITEM_COUNT 16
#define LAMBDAPROC_ITEM_NOT_FOUND LAMBDAPROC_ITEM_COUNT 
class LambdaProcPool
{
    private:
        /* Counter */
        u8 size_free;
        Bitmap<2> bitmap_used;

        /* Make something and returns the record index */
        int m_make(u8 args, u8 body);
    public:
        /* Items */
        LambdaProcItem items[LAMBDAPROC_ITEM_COUNT];
        LambdaProcPool(void);

        /* Primitive a new procedure */
        int make_lambdaproc(u8 args, u8 body);

        /* GC Interface */
#ifdef DEBUG
        void status(void)
        {
            Serial.print("Lambda Proc:");
            Serial.print(this->bitmap_used.num_used());
            Serial.print("/");
            Serial.print(this->bitmap_used.num_total());
            Serial.println("");
        }
#endif
        void clr_used_mark(void);
        void mark_used_mark(u8 idx);
};


/* Object */
class Obj
{
    private:
        u8 body;
    public:
        Obj(void) : body(0) {};
        Obj(u8 v) : body(v) {};
        u8 get_body(void) { return this->body; }
        void set_body(u8 body) { this->body = body; }

        /* type */
        u8 type(void) { return obj_type(this->get_body()); }
        RefItem *extract_ref() 
        { return &ref_pool->items[this->get_body() - OBJ_REF]; }
        Obj extract_pair_car()
        { return Obj(pair_pool->items[this->get_body() - OBJ_PAIR].car); }
        Obj extract_pair_cdr()
        { return Obj(pair_pool->items[this->get_body() - OBJ_PAIR].cdr); }
};

/* Clone */
void ObjClone(Obj *dst, Obj src);

/* Integer */
Obj make_int(s16 v);
s16 extract_int(Obj obj);

/* Error */
Obj make_not_match(void);
Obj make_err(void);
Obj make_err(const char *msg);

/* Others */
Obj make_emptylist(void);
Obj make_nil(void);
Obj make_f(void);
Obj make_t(void);

/* Pair */
Obj make_pair(Obj car, Obj cdr);

/* Symbol/String */
Obj make_sym(char *p, u8 len);
Obj make_str(char *p, u8 len);
RefItem *extract_sym(Obj obj);
RefItem *extract_str(Obj obj);
bool is_obj_str(Obj obj);
bool is_obj_sym(Obj obj); 

/* Buffer */
Obj make_buf(char *p, u8 len);

/* Primitive Procedure */
Obj make_priproc(void (*callback)(lmssArgs *args));
PriProcItem *extract_priproc(Obj obj);

/* Lambda Procedure */
Obj make_lambdaproc(Obj args, Obj body);
Obj extract_lambdaproc_args(Obj obj);
Obj extract_lambdaproc_body(Obj obj);


/* Mark Object */
void mark_obj(u8 obj_body);
void mark_obj(Obj obj);


#endif

