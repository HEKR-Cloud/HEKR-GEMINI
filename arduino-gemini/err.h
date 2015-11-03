/* lmss : Error */

#ifndef _ERR_H_
#define _ERR_H_

/* Error */
class ErrPool
{
    private:
        const char *p;
    public:
        ErrPool(void)
        { this->p = ""; };
        void set(const char *p)
        { this->p = p; }
        const char *get(void)
        { return this->p; }
};

#endif

