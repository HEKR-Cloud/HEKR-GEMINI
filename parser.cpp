/* lmss : Parser */

#include "parser.h"

bool IS_ID1(char ch)
{
    if (IS_ALPHA(ch)) return true;
    switch (ch)
    {
        case '!': case '$': case '%': case '&':
        case '*': case '+': case '-': case '.':
        case '/': case ':': case '<': case '=':
        case '>': case '?': case '@': case '^':
        case '_': case '~':
            return true;
        default:
            return false;
    }
}

