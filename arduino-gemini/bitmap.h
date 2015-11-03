/* lmss : Bitmap */

#ifndef _BITMAP_H_
#define _BITMAP_H_


/* Bitmap */
template <int n>
class Bitmap
{
    public:
        u8 body[n];
        u8 size;

        Bitmap(void)
        {
            this->size = n;
            for (u8 i = 0; i != n; i++)
            { body[i] = 0; }
        }
        /* Find spare */
        s8 spare(void)
        {
            for (s8 i = 0; i != n; i++)
                if (body[i] != 0xFF)
                    for (s8 j = 0; j != 8; j++)
                        if ((body[i] & (1<<j)) == 0)
                            return (i << 3) + j;
            return -1;
        }
        /* Find busy */
        s8 busy(void)
        {
            for (s8 i = 0; i != n; i++)
                if (body[i] != 0x00)
                    for (s8 j = 0; j != 8; j++)
                        if ((body[i] & (1<<j)) != 0)
                            return (i << 3) + j;
            return -1;
        }
        /* Set */
        void set(u8 index)
        { body[index >> 3] |= 1 << (index & 0x7); }
        /* Clear */
        void clr(u8 index)
        { body[index >> 3] &= ~(1 << (index & 0x7)); }
        void clr_all(void)
        {
            for (u8 i = 0; i != n; i++)
            { body[i] = 0; }
        }
        /* Value */
        u8 value(u8 index)
        { return (body[index >> 3] >> (index & 0x7)) & 0x1; }

        /* used bit */
        u8 num_used(void)
        {
            u8 count = 0;

            for (s8 i = 0; i != n; i++)
                if (body[i] != 0xFF)
                    for (s8 j = 0; j != 8; j++)
                        if ((body[i] & (1<<j)) == 0)
                            count++;
            return count;
        }
        u8 num_total(void)
        {
            return n << 3;
        }
};


#endif

