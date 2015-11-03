/* lmss : Arguments */

#ifndef _ARGS_H_
#define _ARGS_H_

enum
{
    LMSSARG_INT = 0,
    LMSSARG_STR,
    LMSSARG_BUF,
    LMSSARG_BOOL,
    LMSSARG_NIL,
    LMSSARG_PROC,
};

class lmssArg
{
    private:
        u8 type_val;
        union
        {
            s16 value_int;
            bool value_bool;
            struct
            {
                char *str;
                u8 len;
            }
            value_str;
            struct
            {
                char *buf;
                u8 len;
            }
            value_buf;
            u8 value_proc;
        } body;

    public:
        lmssArg *prev, *next;

        lmssArg(u8 value)
        {
            this->type_val = LMSSARG_PROC;
            this->prev = this->next = (class lmssArg *)NULL;
            this->body.value_proc = value;
        }
        lmssArg(s16 value)
        {
            this->type_val = LMSSARG_INT;
            this->prev = this->next = (class lmssArg *)NULL;
            this->body.value_int = value;
        }
        lmssArg(bool value)
        {
            this->type_val = LMSSARG_BOOL;
            this->prev = this->next = (class lmssArg *)NULL;
            this->body.value_bool = value;
        }
        lmssArg(int type, char *str, u8 len)
        {
            switch (type)
            {
                case LMSSARG_STR:
                    this->type_val = LMSSARG_STR;
                    this->body.value_str.str = str;
                    this->body.value_str.len = len;
                    break;
                case LMSSARG_BUF:
                    this->type_val = LMSSARG_BUF;
                    this->body.value_buf.buf = str;
                    this->body.value_buf.len = len;
                    break;
            }
            this->prev = this->next = (class lmssArg *)NULL;
        }
        lmssArg(void)
        {
            this->type_val = LMSSARG_NIL;
            this->prev = this->next = (class lmssArg *)NULL;
        }

        u8 type(void)
        { return this->type_val; }

        s16 get_int(void)
        { return this->body.value_int; }
        u8 get_proc(void)
        { return this->body.value_proc; }
        bool get_bool(void)
        { return this->body.value_bool; }
        void get_str(char **str_out, u8 *len_out)
        {
            if (str_out != NULL) *str_out = this->body.value_str.str; 
            if (len_out != NULL) *len_out = this->body.value_str.len; 
        }
        void get_buf(char **buf_out, u8 *len_out)
        {
            if (buf_out != NULL) *buf_out = this->body.value_buf.buf; 
            if (len_out != NULL) *len_out = this->body.value_str.len; 
        }
        void get_nil(void)
        {
            /* Do nothing */
        }
};

class lmssArgs
{
    private:
        lmssArg *begin;
        lmssArg *end;
        u8 size;

        lmssArg *ret_val;

        void m_append(lmssArg *new_arg)
        {
            if (this->begin == NULL)
            {
                this->begin = this->end = new_arg;
            }
            else
            {
                this->end->next = new_arg;
                new_arg->prev = this->end;
                this->end = new_arg;
            }
            this->size++;
        }

        lmssArg *m_pop(void)
        {
            lmssArg *arg = this->begin;
            if (this->begin->next != NULL)
            {
                this->begin->next->prev = (class lmssArg *)NULL;
            }
            this->begin = this->begin->next;
            this->size--;
            return arg;
        }

    public:
        lmssArgs(void)
        { 
            this->begin = this->end = this->ret_val = (class lmssArg *)NULL; 
            this->size = 0;
        }

        /* Put values into args (used by VM) */
        void put_int(s16 value)
        { this->m_append(new lmssArg(value)); }
        void put_proc(u8 value)
        { this->m_append(new lmssArg(value)); }
        void put_bool(bool value)
        { this->m_append(new lmssArg(value)); }
        void put_str(char *str, u8 len)
        { this->m_append(new lmssArg(LMSSARG_STR, str, len)); }
        void put_buf(char *buf, u8 len)
        { this->m_append(new lmssArg(LMSSARG_BUF, buf, len)); }
        void put_nil(void)
        { this->m_append(new lmssArg()); }

        /* Get return value (used by VM) */
        bool has_retval(void)
        { return this->ret_val != NULL; }
        u8 retval_type(void)
        { return this->ret_val->type(); }

        s16 get_ret_int(void)
        { return this->ret_val->get_int(); }
        u8 get_ret_proc(void)
        { return this->ret_val->get_proc(); }
        bool get_ret_bool(void)
        { return this->ret_val->get_bool(); }
        void get_ret_str(char **str_out, u8 *len_out)
        { return this->ret_val->get_str(str_out, len_out); }
        void get_ret_buf(char **str_out, u8 *len_out)
        { return this->ret_val->get_buf(str_out, len_out); }


        /* Get argument (used by user) */
        bool has(void)
        { return this->begin != NULL; }
        u8 type(void)
        {
            return this->begin->type();
        }
        lmssArg *get_any(void)
        {
            lmssArg *arg = m_pop();
            return arg;
        }
        s16 get_int(void)
        {
            s16 value;
            lmssArg *arg = m_pop();
            value = arg->get_int();
            delete arg;
            return value;
        }
        u8 get_proc(void)
        {
            u8 value;
            lmssArg *arg = m_pop();
            value = arg->get_int();
            delete arg;
            return value;
        }
        bool get_bool(void)
        {
            bool value;
            lmssArg *arg = m_pop();
            value = arg->get_bool();
            delete arg;
            return value;
        }
        void get_str(char **str_out, u8 *len_out)
        {
            lmssArg *arg = m_pop();
            arg->get_str(str_out, len_out);
            delete arg;
        }
        void get_buf(char **buf_out, u8 *len_out)
        {
            lmssArg *arg = m_pop();
            arg->get_buf(buf_out, len_out);
            delete arg;
        }

        /* Return value (used by user) */
        void ret_int(s16 value)
        {
            if (this->ret_val != NULL) 
            { delete this->ret_val; this->ret_val = (class lmssArg *)NULL; }
            this->ret_val = new lmssArg(value); 
        }
        void ret_proc(u8 value)
        { 
            if (this->ret_val != NULL) 
            { delete this->ret_val; this->ret_val = (class lmssArg *)NULL; }
            this->ret_val = new lmssArg(value); 
        }
        void ret_bool(bool value)
        { 
            if (this->ret_val != NULL) 
            { delete this->ret_val; this->ret_val = (class lmssArg *)NULL; }
            this->ret_val = new lmssArg(value); 
        }
        void ret_str(char *str, u8 len)
        {
            if (this->ret_val != NULL) 
            { delete this->ret_val; this->ret_val = (class lmssArg *)NULL; }
            this->ret_val = new lmssArg(LMSSARG_STR, str, len); 
        }
        void ret_buf(char *buf, u8 len)
        {
            if (this->ret_val != NULL) 
            { delete this->ret_val; this->ret_val = (class lmssArg *)NULL; }
            this->ret_val = new lmssArg(LMSSARG_BUF, buf, len); 
        }
};



#endif

