/* lmss : Parser */

#include "lmss.h"

#ifndef _PARSER_H_
#define _PARSER_H_

#define IS_NUM(ch) ('0'<=(ch))&&((ch)<='9')
#define IS_WS(ch) (((ch)==' ')||((ch)=='\r')||((ch)=='\n')||((ch)=='\t'))
#define IS_ALPHA(ch) ((('a'<=(ch))&&((ch)<='z'))||(('A'<=(ch))&&((ch)<='Z')))
bool IS_ID1(char ch);
#define IS_ID(ch) ((IS_ID1(ch))||(IS_NUM(ch)))
#define NOT_EOL() ((p)!=(endp))
#define REMAIN(n) ((endp)-(p)>=(n))

/* IO */
class IO
{
    private:
        char *p;
        u8 len;
    public:
        IO(const char *p, u8 len)
        { this->p = (char *)p; this->len = len; }
        /* Get pointer */
        char *get_ptr(void) { return p; }
        /* Move pointer */
        void forward(void) { p++; len--; }
        void backword(void) { p--; len++; }
        void forward(u8 step) { while (step-- != 0) { p++; len--; } }
        void backword(u8 step) { while (step-- != 0) { p--; len++; } }
        /* Get a char */
        char get_char(void)
        { char ch = *p++; len--; return ch; }
        /* Get a char without moving the pointer */
        char peek_char(void)
        { char ch = *p; return ch; }
        /* Test match */
        bool match_char(char ch) { return (*p == ch); }
        bool match_str(const char* str, u8 len) 
        {
            /* Length */
            if (len > this->len) return false;
            /* Content */
            const char *p1 = this->p, *p2 = str;
            while (len-- != 0) { if (*p1++ != *p2++) return false; }
            /* Match */
            return true;
        }
        bool match_digit(void) { return (IS_NUM(*p)); }
        /* Skip whitespace */
        void skip_whitespace(void)
        {
            while (IS_WS(*p))
            { this->forward(); }
        }
        bool eof(void)
        {
            return (len == 0);
        }
};

#endif

