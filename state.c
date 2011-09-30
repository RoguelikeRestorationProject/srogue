/*
    state.c - Portable Rogue Save State Code

    Copyright (C) 1999, 2000, 2005 Nicholas J. Kisseberth

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

/************************************************************************/
/* Save State Code                                                      */
/************************************************************************/

#define RSID_STATS        0xABCD0001
#define RSID_THING        0xABCD0002
#define RSID_THING_NULL   0xDEAD0002
#define RSID_OBJECT       0xABCD0003
#define RSID_MAGICITEMS   0xABCD0004
#define RSID_KNOWS        0xABCD0005
#define RSID_GUESSES      0xABCD0006
#define RSID_OBJECTLIST   0xABCD0007
#define RSID_BAGOBJECT    0xABCD0008
#define RSID_MONSTERLIST  0xABCD0009
#define RSID_MONSTERSTATS 0xABCD000A
#define RSID_MONSTERS     0xABCD000B
#define RSID_TRAP         0xABCD000C
#define RSID_WINDOW       0xABCD000D
#define RSID_DAEMONS      0xABCD000E
#define RSID_IWEAPS       0xABCD000F
#define RSID_IARMOR       0xABCD0010
#define RSID_SPELLS       0xABCD0011
#define RSID_ILIST        0xABCD0012
#define RSID_HLIST        0xABCD0013
#define RSID_DEATHTYPE    0xABCD0014
#define RSID_CTYPES       0XABCD0015
#define RSID_COORDLIST    0XABCD0016
#define RSID_ROOMS        0XABCD0017


#include <curses.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rogue.h"
#include "rogue.ext"

#define READSTAT ((format_error == 0) && (read_error == 0))
#define WRITESTAT (write_error == 0)

int read_error   = FALSE;
int write_error  = FALSE;
int format_error = FALSE;
int end_of_file  = FALSE;
int big_endian   = 0;
const char *fmterr = "";

char encstr[] = "\354\251\243\332A\201|\301\321p\210\251\327\"\257\365t\341%3\271^`~\203z{\341};\f\341\231\222e\234\351]\321";

/*
 * perform an encrypted write
 */
encwrite(starta, size, outf)
register void *starta;
unsigned int size;
register FILE *outf;
{
    register char *ep;
    register char *start = starta;

    ep = encstr;

    while (size--)
    {
        putc(*start++ ^ *ep++, outf);
        if (*ep == '\0')
            ep = encstr;
    }
}

/*
 * perform an encrypted read
 */
encread(starta, size, inf)
register void *starta;
unsigned int size;
register int inf;
{
    register char *ep;
    register int read_size;
    register char *start = starta;

    if ((read_size = read(inf, start, size)) == -1 || read_size == 0)
        return read_size;

    ep = encstr;

    while (size--)
    {
        *start++ ^= *ep++;
        if (*ep == '\0')
            ep = encstr;
    }
    return read_size;
}

void *
get_list_item(struct linked_list *l, int i)
{
    int count = 0;

    while(l != NULL)
    {   
        if (count == i)
            return(l->l_data);
                        
        l = l->l_next;
        
        count++;
    }
    
    return(NULL);
}

int
find_list_ptr(struct linked_list *l, void *ptr)
{
    int count = 0;

    while(l != NULL)
    {
        if (l->l_data == ptr)
            return(count);
            
        l = l->l_next;
        count++;
    }
    
    return(-1);
}

int
list_size(struct linked_list *l)
{
    int count = 0;
    
    while(l != NULL)
    {
        if (l->l_data == NULL)
            return(count);
            
        count++;
        
        l = l->l_next;
    }
    
    return(count);
}

int
rs_write(FILE *savef, void *ptr, int size)
{
    if (!write_error)
        encwrite(ptr,size,savef);

    if (0)
        write_error = TRUE;
        
    assert(write_error == 0);

    return(WRITESTAT);
}

int
rs_write_char(FILE *savef, char c)
{
    rs_write(savef, &c, 1);
    
    return(WRITESTAT);
}

int
rs_write_boolean(FILE *savef, bool c)
{
    unsigned char buf = (c == 0) ? 0 : 1;
    
    rs_write(savef, &buf, 1);

    return(WRITESTAT);
}

int
rs_write_booleans(FILE *savef, bool *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_boolean(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_shint(FILE *savef, unsigned char c)
{
    unsigned char buf = c;

    rs_write(savef, &buf, 1);

    return(WRITESTAT);
}

int
rs_write_short(FILE *savef, short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_write_shorts(FILE *savef, short *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_short(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_ushort(FILE *savef, unsigned short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_write_int(FILE *savef, int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_ints(FILE *savef, int *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_int(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_uint(FILE *savef, unsigned int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_long(FILE *savef, long c)
{
    int c2;
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *)&c;

    if (sizeof(long) == 8)
    {
        c2 = c;
        buf = (unsigned char *) &c2;
    }

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_longs(FILE *savef, long *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_long(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_ulong(FILE *savef, unsigned long c)
{
    unsigned int c2;
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *)&c;

    if ( (sizeof(long) == 8) && (sizeof(int) == 4) )
    {
        c2 = c;
        buf = (unsigned char *) &c2;
    }

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_write_ulongs(FILE *savef, unsigned long *c, int count)
{
    int n = 0;

    rs_write_int(savef,count);
    
    for(n = 0; n < count; n++)
        rs_write_ulong(savef,c[n]);

    return(WRITESTAT);
}

int
rs_write_string(FILE *savef, char *s)
{
    int len = 0;

    len = (s == NULL) ? 0 : strlen(s) + 1;

    rs_write_int(savef, len);
    rs_write(savef, s, len);
            
    return(WRITESTAT);
}

int
rs_write_string_index(FILE *savef, char *master[], int max, char *str)
{
    int i;

    for(i = 0; i < max; i++)
    {
        if (str == master[i])
        {
            rs_write_int(savef,i);
            return(WRITESTAT);
        }
    }

    rs_write_int(savef,-1);

    return(WRITESTAT);
}

int
rs_write_strings(FILE *savef, char *s[], int count)
{
    int len = 0;
    int n = 0;

    rs_write_int(savef,count);

    for(n = 0; n < count; n++)
    {
        len = (s[n] == NULL) ? 0L : strlen(s[n]) + 1;
        rs_write_int(savef, len);
        rs_write(savef, s[n], len);
    }
    
    return(WRITESTAT);
}

int
rs_read(int inf, void *ptr, int size)
{
    int actual;

    end_of_file = FALSE;

    if (!read_error && !format_error)
    {
        actual = encread(ptr, size, inf);

        if ((actual == 0) && (size != 0))
           end_of_file = TRUE;
    }
       
    if (read_error)
    {
        printf("read error has occurred. restore short-circuited.\n");
        abort();
    }

    if (format_error)
    {
        printf("format error: %s\r\n", fmterr);
        printf("game format invalid. restore short-circuited.\n");
        abort();
    }

    return(READSTAT);
}

int
rs_read_char(int inf, char *c)
{
    rs_read(inf, c, 1);
    
    return(READSTAT);
}

int
rs_read_boolean(int inf, bool *i)
{
    unsigned char buf;
    
    rs_read(inf, &buf, 1);
    
    *i = (bool) buf;
    
    return(READSTAT);
}

int
rs_read_booleans(int inf, bool *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Invalid booleans block. %d != requested %d\n",value,count); 
            format_error = TRUE;
        }
        else
        {
            for(n = 0; n < value; n++)
                rs_read_boolean(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_shint(int inf, unsigned char *i)
{
    unsigned char buf;
    
    rs_read(inf, &buf, 1);
    
    *i = (unsigned char) buf;
    
    return(READSTAT);
}

int
rs_read_short(int inf, short *i)
{
    unsigned char bytes[2];
    short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 2);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((short *) buf);

    return(READSTAT);
} 

int
rs_read_shorts(int inf, short *i, int count)
{
    int n = 0, value = 0;

    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_short(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_ushort(int inf, unsigned short *i)
{
    unsigned char bytes[2];
    unsigned short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 2);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((unsigned short *) buf);

    return(READSTAT);
} 

int
rs_read_int(int inf, int *i)
{
    unsigned char bytes[4];
    int  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((int *) buf);

    return(READSTAT);
}

int
rs_read_ints(int inf, int *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_int(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_uint(int inf, unsigned int *i)
{
    unsigned char bytes[4];
    int  input;
    unsigned char *buf = (unsigned char *)&input;
    
    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned int *) buf);

    return(READSTAT);
}

int
rs_read_long(int inf, long *i)
{
    unsigned char bytes[4];
    long input;
    unsigned char *buf = (unsigned char *) &input;
    
    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((long *) buf);

    return(READSTAT);
}

int
rs_read_longs(int inf, long *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_long(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_ulong(int inf, unsigned long *i)
{
    unsigned char bytes[4];
    unsigned long input;
    unsigned char *buf = (unsigned char *) &input;
    
    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned long *) buf);

    return(READSTAT);
}

int
rs_read_ulongs(int inf, unsigned long *i, int count)
{
    int n = 0, value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
            format_error = TRUE;
        else
        {
            for(n = 0; n < value; n++)
                rs_read_ulong(inf, &i[n]);
        }
    }
    
    return(READSTAT);
}

int
rs_read_string(int inf, char *s, int max)
{
    int len = 0;

    if (rs_read_int(inf, &len) != FALSE)
    {
        if (len > max)
        {
            printf("String too long to restore. %d > %d\n",len,max);
            printf("Sorry, invalid save game format\n");
            format_error = TRUE;
        }
    
        rs_read(inf, s, len);
    }
    
    return(READSTAT);
}

int
rs_read_new_string(int inf, char **s)
{
    int len=0;
    char *buf=0;

    if (rs_read_int(inf, &len) != 0)
    {
        if (len == 0)
            *s = NULL;
        else
        { 
            buf = malloc(len);

            if (buf == NULL)            
                read_error = TRUE;
            else
            {
                rs_read(inf, buf, len);
                *s = buf;
            }
        }
    }

    return(READSTAT);
}

int
rs_read_string_index(int inf, char *master[], int maxindex, char **str)
{
    int i;

    if (rs_read_int(inf,&i) != 0)
    {
        if (i > maxindex)
        {
            printf("String index is out of range. %d > %d\n", i, maxindex);
            printf("Sorry, invalid save game format\n");
            format_error = TRUE;
        }
        else if (i >= 0)
            *str = master[i];
        else
            *str = NULL;
    }

    return(READSTAT);
}

int
rs_read_strings(int inf, char **s, int count, int max)
{
    int len   = 0;
    int n     = 0;
    int value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Incorrect number of strings in block. %d > %d.", 
                value, count);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
        {
            for(n = 0; n < value; n++)
            {
                rs_read_string(inf, s[n], max);
            }
        }
    }
    
    return(READSTAT);
}

int
rs_read_new_strings(int inf, char **s, int count)
{
    int len   = 0;
    int n     = 0;
    int value = 0;
    
    if (rs_read_int(inf,&value) != 0)
    {
        if (value != count)
        {
            printf("Incorrect number of new strings in block. %d > %d.",
                value,count);abort();
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
            for(n=0; n<value; n++)
            {
                rs_read_int(inf, &len);
            
                if (len == 0)
                    s[n]=0;
                else 
                {
                    s[n] = malloc(len);
                    rs_read(inf,s[n],len);
                }
            }
    }
    
    return(READSTAT);
}

/******************************************************************************/

int
rs_write_coord(FILE *savef, struct coord c)
{
    rs_write_int(savef, c.x);
    rs_write_int(savef, c.y);
    
    return(WRITESTAT);
}

int
rs_read_coord(int inf, struct coord *c)
{
    rs_read_int(inf,&c->x);
    rs_read_int(inf,&c->y);
    
    return(READSTAT);
}

int
rs_write_window(FILE *savef, WINDOW *win)
{
    int row,col,height,width;
    width = getmaxx(win);
    height = getmaxy(win);

    rs_write_int(savef,RSID_WINDOW);
    rs_write_int(savef,height);
    rs_write_int(savef,width);
    
    for(row=0;row<height;row++)
        for(col=0;col<width;col++)
            rs_write_int(savef, mvwinch(win,row,col));
}

int
rs_read_window(int inf, WINDOW *win)
{
    int id,row,col,maxlines,maxcols,value,width,height;
    
    width = getmaxx(win);
    height = getmaxy(win);

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_WINDOW)
        {
            printf("Invalid head id. %x != %x(RSID_WINDOW)\n", id, RSID_WINDOW);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }   
        else
        {
            rs_read_int(inf,&maxlines);
            rs_read_int(inf,&maxcols);
            if (maxlines > height)
               abort();
            if (maxcols > width)
               abort();
               
            for(row=0;row<maxlines;row++)
                for(col=0;col<maxcols;col++)
                {
                    rs_read_int(inf, &value);
                    mvwaddch(win,row,col,value);
                }
        }
    }
        
    return(READSTAT);
}

int
rs_write_daemons(FILE *savef, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
        
    rs_write_int(savef, RSID_DAEMONS);
    rs_write_int(savef, count);
        
    for(i = 0; i < count; i++)
    {
        if (d_list[i].d_func == rollwand)
            func = 1;
        else if (d_list[i].d_func == doctor)
            func = 2;
        else if (d_list[i].d_func == stomach)
            func = 3;
        else if (d_list[i].d_func == runners)
            func = 4;
        else if (d_list[i].d_func == swander)
            func = 5;
        else if (d_list[i].d_func == nohaste)
            func = 6;
        else if (d_list[i].d_func == unconfuse)
            func = 7;
        else if (d_list[i].d_func == unsee)
            func = 8;
        else if (d_list[i].d_func == sight)
            func = 9;
        else if (d_list[i].d_func == noteth)
            func = 10;
        else if (d_list[i].d_func == sapem)
            func = 11;
        else if (d_list[i].d_func == notslow)
            func = 12;
        else if (d_list[i].d_func == notregen)
            func = 13;
        else if (d_list[i].d_func == notinvinc)
            func = 14;
        else if (d_list[i].d_func == rchg_str)
            func = 15;
        else if (d_list[i].d_func == wghtchk)
            func = 16;
        else if (d_list[i].d_func == status)
            func = 17;
        else
            func = 0;

        rs_write_int(savef, d_list[i].d_type);
        rs_write_int(savef, func);
        rs_write_int(savef, d_list[i].d_arg);
        rs_write_int(savef, d_list[i].d_time);
    }
    
    return(WRITESTAT);
}       

int
rs_read_daemons(int inf, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
    int value = 0;
    int id = 0;
    
    if (d_list == NULL)
        printf("HELP THERE ARE NO DAEMONS\n");
    
    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_DAEMONS)
        {
            printf("Invalid id. %x != %x(RSID_DAEMONS)\n", id, RSID_DAEMONS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf, &value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of daemons in block. %d > %d.",
                    value, count);
                printf("Sorry, invalid save game format");
                format_error = TRUE;
            }
            else
            {
                for(i=0; i < value; i++)
                {
                    func = 0;
                    rs_read_int(inf, &d_list[i].d_type);
                    rs_read_int(inf, &func);
                    rs_read_int(inf, &d_list[i].d_arg);
                    rs_read_int(inf, &d_list[i].d_time);
                    
                    switch(func)
                    {
                        case 1: d_list[i].d_func = rollwand;
                                break;
                        case 2: d_list[i].d_func = doctor;
                                break;
                        case 3: d_list[i].d_func = stomach;
                                break;
                        case 4: d_list[i].d_func = runners;
                                break;
                        case 5: d_list[i].d_func = swander;
                                break;
                        case 6: d_list[i].d_func = nohaste;
                                break;
                        case 7: d_list[i].d_func = unconfuse;
                                break;
                        case 8: d_list[i].d_func = unsee;
                                break;
                        case 9: d_list[i].d_func = sight;
                                break;
                        case 10: d_list[i].d_func = noteth;
                                break;
                        case 11: d_list[i].d_func = sapem;
                                break;
                        case 12: d_list[i].d_func = notslow;
                                break;
                        case 13: d_list[i].d_func = notregen;
                                break;
                        case 14: d_list[i].d_func = notinvinc;
                                break;
                        case 15: d_list[i].d_func = rchg_str;
                                break;
                        case 16: d_list[i].d_func = wghtchk;
                                break;
                        case 17: d_list[i].d_func = status;
                                break;
                        default: d_list[i].d_func = NULL;
                                break;
                    }   
                }
            }
        }
    }
    
    return(READSTAT);
}                 

int
rs_write_room_reference(FILE *savef, struct room *rp)
{
    int i, room = -1;
    
    for (i = 0; i < MAXROOMS; i++)
        if (&rooms[i] == rp)
            room = i;

    rs_write_int(savef, room);

    return(WRITESTAT);
}

int
rs_read_room_reference(int inf, struct room **rp)
{
    int i;
    
    rs_read_int(inf, &i);

    *rp = &rooms[i];
            
    return(READSTAT);
}

int
rs_write_rooms(FILE *savef, struct room r[], int count)
{
    int n = 0;

    rs_write_int(savef, count);
    
    for(n=0; n<count; n++)
    {
        rs_write_coord(savef, r[n].r_pos);
        rs_write_coord(savef, r[n].r_max);
        rs_write_coord(savef, r[n].r_gold);
        rs_write_coord(savef, r[n].r_exit[0]);
        rs_write_coord(savef, r[n].r_exit[1]);
        rs_write_coord(savef, r[n].r_exit[2]);
        rs_write_coord(savef, r[n].r_exit[3]);
        rs_write_room_reference(savef,r[n].r_ptr[0]);
        rs_write_room_reference(savef,r[n].r_ptr[1]);
        rs_write_room_reference(savef,r[n].r_ptr[2]);
        rs_write_room_reference(savef,r[n].r_ptr[3]);
        rs_write_int(savef, r[n].r_goldval);
        rs_write_int(savef, r[n].r_flags);
        rs_write_int(savef, r[n].r_nexits);
    }
    
    return(WRITESTAT);
}

int
rs_read_rooms(int inf, struct room *r, int count)
{
    int value = 0, n = 0;

    if (rs_read_int(inf,&value) != 0)
    {
        if (value > count)
        {
            printf("Incorrect number of rooms in block. %d > %d.",
                value,count);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
            for(n = 0; n < value; n++)
            {
                rs_read_coord(inf,&r[n].r_pos);
                rs_read_coord(inf,&r[n].r_max);
                rs_read_coord(inf,&r[n].r_gold);
                rs_read_coord(inf,&r[n].r_exit[0]);
                rs_read_coord(inf,&r[n].r_exit[1]);
                rs_read_coord(inf,&r[n].r_exit[2]);
                rs_read_coord(inf,&r[n].r_exit[3]);
                rs_read_room_reference(inf,&r[n].r_ptr[0]);
                rs_read_room_reference(inf,&r[n].r_ptr[1]);
                rs_read_room_reference(inf,&r[n].r_ptr[2]);
                rs_read_room_reference(inf,&r[n].r_ptr[3]);
                rs_read_int(inf,&r[n].r_goldval);
                rs_read_int(inf,&r[n].r_flags);
                rs_read_int(inf,&r[n].r_nexits);
            }
    }

    return(READSTAT);
}

int
rs_write_monlev(FILE *savef, struct monlev m)
{
    rs_write_int(savef, m.l_lev);
    rs_write_int(savef, m.h_lev);
    rs_write_boolean(savef, m.d_wand);

    return(WRITESTAT);
}

int
rs_read_monlev(int inf, struct monlev *m)
{
    rs_read_int(inf, &m->l_lev);
    rs_read_int(inf, &m->h_lev);
    rs_read_boolean(inf, &m->d_wand);

    return(READSTAT);
}

int
rs_write_magic_items(FILE *savef, struct magic_item *i, int count)
{
    int n;
    
    rs_write_int(savef, RSID_MAGICITEMS);
    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
    {
        /* mi_name is constant, defined at compile time in all cases */
        rs_write_int(savef,i[n].mi_prob);
    }
    
    return(WRITESTAT);
}

int
rs_read_magic_items(int inf, struct magic_item *mi, int count)
{
    int id;
    int n;
    int value;

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_MAGICITEMS)
        {
            printf("Invalid id. %x != %x(RSID_MAGICITEMS)\n",
                id, RSID_MAGICITEMS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }   
        else if (rs_read_int(inf, &value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of magic items in block. %d > %d.",
                    value, count);
                printf("Sorry, invalid save game format");
                format_error = TRUE;
            }
            else
            {
                for(n = 0; n < value; n++)
                {
                    rs_read_int(inf,&mi[n].mi_prob);
                }
            }
        }
    }
    
    return(READSTAT);
}


int
rs_write_real(FILE *savef, struct real r)
{
    rs_write_int(savef, r.a_str);
    rs_write_int(savef, r.a_dex);
    rs_write_int(savef, r.a_wis);
    rs_write_int(savef, r.a_con);

    return(WRITESTAT);
}

int
rs_read_real(int inf, struct real *r)
{
    rs_read_int(inf,&r->a_str);
    rs_read_int(inf,&r->a_dex);
    rs_read_int(inf,&r->a_wis);
    rs_read_int(inf,&r->a_con);

    return(READSTAT);
}

int
rs_write_stats(FILE *savef, struct stats *s)
{
    rs_write_int(savef, RSID_STATS);
    rs_write_real(savef, s->s_re);
    rs_write_real(savef, s->s_ef);
    rs_write_long(savef, s->s_exp);
    rs_write_int(savef, s->s_lvl);
    rs_write_int(savef, s->s_arm);
    rs_write_int(savef, s->s_hpt);
    rs_write_int(savef, s->s_maxhp);
    rs_write_int(savef, s->s_pack);
    rs_write_int(savef, s->s_carry);
    rs_write(savef, s->s_dmg, sizeof(s->s_dmg));

    return(WRITESTAT);
}

int
rs_read_stats(int inf, struct stats *s)
{
    int id;

    rs_read_int(inf, &id);
    rs_read_real(inf, &s->s_re);
    rs_read_real(inf, &s->s_ef);

    rs_read_long(inf,&s->s_exp);
    rs_read_int(inf,&s->s_lvl);
    rs_read_int(inf,&s->s_arm);
    rs_read_int(inf,&s->s_hpt);
    rs_read_int(inf,&s->s_maxhp);
    rs_read_int(inf,&s->s_pack);
    rs_read_int(inf,&s->s_carry);

    rs_read(inf,s->s_dmg,sizeof(s->s_dmg));
    
    return(READSTAT);
}

int
rs_write_monster_reference(FILE *savef, struct monster *m)
{
    int i, mon = -1;
    
    for (i = 0; i < (MAXMONS+1); i++)
        if (&monsters[i] == m)
            mon = i;

    rs_write_int(savef, mon);

    return(WRITESTAT);
}

int
rs_read_monster_reference(int inf, struct monster **mp)
{
    int i;
    
    rs_read_int(inf, &i);

    if (i < 0)
        *mp = NULL;
    else
        *mp = &monsters[i];
            
    return(READSTAT);
}

int
rs_write_monster_references(FILE *savef, struct monster *marray[], int count)
{
    int i;

    for(i = 0; i < count; i++)
        rs_write_monster_reference(savef, marray[i]);
}

int 
rs_read_monster_references(int inf, struct monster *marray[], int count)
{
    int i;

    for(i = 0; i < count; i++)
        rs_read_monster_reference(inf, &marray[i]);
}

int
rs_write_object(FILE *savef, struct object *o)
{
    rs_write_int(savef, RSID_OBJECT);
    rs_write_coord(savef, o->o_pos);
    rs_write(savef, o->o_damage, sizeof(o->o_damage));
    rs_write(savef, o->o_hurldmg, sizeof(o->o_hurldmg));

    if (o->o_type == ARMOR)
        assert( strcmp(o->o_typname,things[TYP_ARMOR].mi_name) == 0 );
    else if (o->o_type == FOOD)
        assert( strcmp(o->o_typname,things[TYP_FOOD].mi_name) == 0 );
    else if (o->o_type == RING)
        assert( strcmp(o->o_typname,things[TYP_RING].mi_name) == 0 );
    else if (o->o_type == WEAPON)
        assert( strcmp(o->o_typname,things[TYP_WEAPON].mi_name) == 0 );
    else if (o->o_type == POTION)
        assert( strcmp(o->o_typname,things[TYP_POTION].mi_name) == 0 );
    else if (o->o_type == SCROLL)
        assert( strcmp(o->o_typname,things[TYP_SCROLL].mi_name) == 0 );
    else if (o->o_type == STICK)
        assert( strcmp(o->o_typname,things[TYP_STICK].mi_name) == 0 );
    else if (o->o_type == AMULET)
        assert( strcmp(o->o_typname,things[TYP_AMULET].mi_name) == 0 );
    else
        assert(0 == 1);

    rs_write_int(savef, o->o_type);
    rs_write_int(savef, o->o_count);
    rs_write_int(savef, o->o_which);
    rs_write_int(savef, o->o_hplus);
    rs_write_int(savef, o->o_dplus);
    rs_write_int(savef, o->o_ac);
    rs_write_int(savef, o->o_flags);
    rs_write_int(savef, o->o_group);
    rs_write_int(savef, o->o_weight);
    rs_write_int(savef, o->o_vol);

    rs_write_char(savef, o->o_launch);

    return(WRITESTAT);
}

int
rs_read_object(int inf, struct object *o)
{
    int id;

    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_OBJECT)
        {
            printf("Invalid id. %x != %x(RSID_OBJECT)\n",
                id,RSID_OBJECT);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else
        {
            rs_read_coord(inf, &o->o_pos);
            rs_read(inf, &o->o_damage, sizeof(o->o_damage));
            rs_read(inf, &o->o_hurldmg, sizeof(o->o_hurldmg));
            rs_read_int(inf, &o->o_type);

            if (o->o_type == ARMOR)
                o->o_typname = things[TYP_ARMOR].mi_name;
            else if (o->o_type == FOOD)
                o->o_typname = things[TYP_FOOD].mi_name;
            else if (o->o_type == RING)
                o->o_typname = things[TYP_RING].mi_name;
            else if (o->o_type == WEAPON)
                o->o_typname = things[TYP_WEAPON].mi_name;
            else if (o->o_type == POTION)
                o->o_typname = things[TYP_POTION].mi_name;
            else if (o->o_type == SCROLL)
                o->o_typname = things[TYP_SCROLL].mi_name;
            else if (o->o_type == STICK)
                o->o_typname = things[TYP_STICK].mi_name;
            else if (o->o_type == AMULET)
                o->o_typname = things[TYP_AMULET].mi_name;
            else
                assert(0 == 1);

            rs_read_int(inf, &o->o_count);
            rs_read_int(inf, &o->o_which);
            rs_read_int(inf, &o->o_hplus);
            rs_read_int(inf, &o->o_dplus);
            rs_read_int(inf, &o->o_ac);
            rs_read_int(inf, &o->o_flags);
            rs_read_int(inf, &o->o_group);
            rs_read_int(inf, &o->o_weight);
            rs_read_int(inf, &o->o_vol);
            rs_read_char(inf, &o->o_launch);
        }
    }
    
    return(READSTAT);
}

int
rs_read_object_list(int inf, struct linked_list **list)
{
    int id;
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    if (rs_read_int(inf,&id) != 0)
    {
        if (rs_read_int(inf,&cnt) != 0)
        {
            for (i = 0; i < cnt; i++) 
            {
                l = new_item(sizeof(struct object));
                memset(l->l_data,0,sizeof(struct object));
                l->l_prev = previous;
                if (previous != NULL)
                    previous->l_next = l;
                rs_read_object(inf,(struct object *) l->l_data);
                if (previous == NULL)
                    head = l;
                previous = l;
            }
            
            if (l != NULL)
                l->l_next = NULL;
    
            *list = head;
        }
        else
            format_error = TRUE;
    }
    else
        format_error = TRUE;


    return(READSTAT);
}

int
rs_write_object_list(FILE *savef, struct linked_list *l)
{
    rs_write_int(savef, RSID_OBJECTLIST);
    rs_write_int(savef, list_size(l));

    while (l != NULL) 
    {
        rs_write_object(savef, (struct object *) l->l_data);
        l = l->l_next;
    }
    
    return(WRITESTAT);
}

int
rs_write_traps(FILE *savef, struct trap *trap,int count)
{
    int n;

    rs_write_int(savef, RSID_TRAP);
    rs_write_int(savef, count);
    
    for(n=0; n<count; n++)
    {
        rs_write_coord(savef, trap[n].tr_pos);
        rs_write_coord(savef, trap[n].tr_goto);
        rs_write_int(savef, trap[n].tr_flags);
        rs_write_char(savef, trap[n].tr_type);
    }
}

rs_read_traps(int inf, struct trap *trap, int count)
{
    int id = 0, value = 0, n = 0;

    if (rs_read_int(inf,&id) != 0)
    {
        if (id != RSID_TRAP)
        {
            printf("Invalid id. %x != %x(RSID_TRAP)\n",
                id,RSID_TRAP);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf,&value) != 0)
        {
            if (value > count)
            {
                printf("Incorrect number of traps in block. %d > %d.",
                    value,count);
                printf("Sorry, invalid save game format\n");
                format_error = TRUE;
            }
            else
            {
                for(n=0;n<value;n++)
                {   
                    rs_read_coord(inf,&trap[n].tr_pos);
                    rs_read_coord(inf,&trap[n].tr_goto);
                    rs_read_int(inf,&trap[n].tr_flags);
                    rs_read_char(inf,&trap[n].tr_type);
                }
            }
        }
        else
            format_error = TRUE;
    }
    
    return(READSTAT);
}

int
rs_write_monsters(FILE * savef, struct monster * m, int count)
{
    int n;
    
    rs_write_int(savef, RSID_MONSTERS);
    rs_write_int(savef, count);

    for(n=0;n<count;n++)
    {
        rs_write_monlev(savef, m[n].m_lev);
        rs_write_stats(savef, &m[n].m_stats);
    }
    
    return(WRITESTAT);
}

int
rs_read_monsters(int inf, struct monster *m, int count)
{
    int id = 0, value = 0, n = 0;
    
    if (rs_read_int(inf, &id) != 0)
    {
        if (id != RSID_MONSTERS)
        {
            printf("Invalid id. %x != %x(RSID_MONSTERS)\n",
                id,RSID_MONSTERS);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf, &value) != 0)
        {
            for(n=0;n<value;n++)
            {
                rs_read_monlev(inf, &m[n].m_lev);
                rs_read_stats(inf, &m[n].m_stats);
            }
        }
        else
            format_error = TRUE;
    }
    
    return(READSTAT);
}

int
find_thing_coord(struct linked_list *monlist, struct coord *c)
{
    struct linked_list *mitem;
    struct thing *tp;
    int i = 0;

    for(mitem = monlist; mitem != NULL; mitem = mitem->l_next)
    {
        tp = THINGPTR(mitem);
        if (c == &tp->t_pos)
            return(i);
        i++;
    }

    return(-1);
}

int
find_object_coord(struct linked_list *objlist, struct coord *c)
{
    struct linked_list *oitem;
    struct object *obj;
    int i = 0;

    for(oitem = objlist; oitem != NULL; oitem = oitem->l_next)
    {
        obj = OBJPTR(oitem);
        if (c == &obj->o_pos)
            return(i);
        i++;
    }

    return(-1);
}

void
rs_fix_thing(struct thing *t)
{
    struct linked_list *item;
    struct thing *tp;

    if (t->t_reserved < 0)
        return;

    item = get_list_item(mlist,t->t_reserved);

    if (item != NULL)
    {
        tp = THINGPTR(item);
        t->t_dest = &tp->t_pos;
    }
}

int
find_room_coord(struct room *rmlist, struct coord *c, int n)
{
    int i = 0;
    
    for(i=0; i < n; i++)
        if(&rmlist[i].r_gold == c)
            return(i);
    
    return(-1);
}

int
rs_write_thing(FILE *savef, struct thing *t)
{
    int i = -1;
    
    if (t == NULL)
    {
        rs_write_int(savef, RSID_THING_NULL);
        return(WRITESTAT);
    }
    
    rs_write_int(savef, RSID_THING);

    rs_write_stats(savef, &t->t_stats);
    rs_write_coord(savef, t->t_pos);
    rs_write_coord(savef, t->t_oldpos);

    /* 
        t_dest can be:
        0,0: NULL
        0,1: location of hero
        0,3: global coord 'delta'
        1,i: location of a thing (monster)
        2,i: location of an object
        3,i: location of gold in a room

        We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */

    if (t->t_dest == &hero)
    {
        rs_write_int(savef,0);
        rs_write_int(savef,1);
    }
    else if (t->t_dest != NULL)
    {
        i = find_thing_coord(mlist, t->t_dest);
            
        if (i >=0 )
        {
            rs_write_int(savef,1);
            rs_write_int(savef,i);
        }
        else
        {
            i = find_object_coord(lvl_obj, t->t_dest);
            
            if (i >= 0)
            {
                rs_write_int(savef,2);
                rs_write_int(savef,i);
            }
            else
            {
                i = find_room_coord(rooms, t->t_dest, MAXROOMS);
        
                if (i >= 0) 
                {
                    rs_write_int(savef,3);
                    rs_write_int(savef,i);
                }
                else 
                {
                    rs_write_int(savef, 0);
                    rs_write_int(savef,1); /* chase the hero anyway */
                }
            }
        }
    }
    else
    {
        rs_write_int(savef,0);
        rs_write_int(savef,0);
    }
    
    rs_write_object_list(savef, t->t_pack);
    rs_write_room_reference(savef, t->t_room);
    rs_write_long(savef, t->t_flags);
    rs_write_int(savef, t->t_indx);
    rs_write_int(savef, t->t_nomove);
    rs_write_int(savef, t->t_nocmd);
    rs_write_boolean(savef, t->t_turn);
    rs_write_char(savef, t->t_type);
    rs_write_char(savef, t->t_disguise);
    rs_write_char(savef, t->t_oldch);
    
    return(WRITESTAT);
}

int
rs_read_thing(int inf, struct thing *t)
{
    int id;
    int listid = 0, index = -1;
    struct linked_list *item;
        
    if (rs_read_int(inf, &id) != 0)
    {
        if ((id != RSID_THING) && (id != RSID_THING_NULL)) {
            fmterr = "RSID_THING mismatch";
            format_error = TRUE;
        }
        else if (id == RSID_THING_NULL)
        {
            printf("NULL Thing?\n\r");
        }
        else
        {
            rs_read_stats(inf, &t->t_stats);
            rs_read_coord(inf, &t->t_pos);
            rs_read_coord(inf, &t->t_oldpos);

            /* 
                t_dest can be (listid,index):
                0,0: NULL
                0,1: location of hero
                1,i: location of a thing (monster)
                2,i: location of an object
                3,i: location of gold in a room

                We need to remember what we are chasing rather than 
                the current location of what we are chasing.
            */
            
            rs_read_int(inf, &listid);
            rs_read_int(inf, &index);
            t->t_reserved = -1;

            if (listid == 0) /* hero or NULL */
            {
                if (index == 1)
                    t->t_dest = &hero;
                else
                    t->t_dest = NULL;
            }
            else if (listid == 1) /* monster/thing */
            {
                t->t_dest     = NULL;
                t->t_reserved = index;
            }
            else if (listid == 2) /* object */
            {
                struct object *obj;

                item = get_list_item(lvl_obj, index);

                if (item != NULL)
                {
                    obj = OBJPTR(item);
                    t->t_dest = &obj->o_pos;
                }
            }
            else if (listid == 3) /* gold */
            {
                t->t_dest = &rooms[index].r_gold;
            }
            else
                t->t_dest = NULL;

            rs_read_object_list(inf, &t->t_pack);
            rs_read_room_reference(inf, &t->t_room);
            rs_read_long(inf, &t->t_flags);
            rs_read_int(inf, &t->t_indx);
            rs_read_int(inf, &t->t_nomove);
            rs_read_int(inf, &t->t_nocmd);
            rs_read_boolean(inf, &t->t_turn);
            rs_read_char(inf, &t->t_type);
            rs_read_char(inf, &t->t_disguise);
            rs_read_char(inf, &t->t_oldch);
        }
    }
    else format_error = TRUE;
    
    return(READSTAT);
}

rs_fix_monster_list(list)
struct linked_list *list;
{
    struct linked_list *item;

    for(item = list; item != NULL; item = item->l_next)
        rs_fix_thing(THINGPTR(item));
}

int
rs_write_monster_list(FILE *savef, struct linked_list *l)
{
    int cnt = 0;
    
    rs_write_int(savef, RSID_MONSTERLIST);

    cnt = list_size(l);

    rs_write_int(savef, cnt);

    if (cnt < 1)
        return(WRITESTAT);

    while (l != NULL) {
        rs_write_thing(savef, (struct thing *)l->l_data);
        l = l->l_next;
    }
    
    return(WRITESTAT);
}

int
rs_read_monster_list(int inf, struct linked_list **list)
{
    int id;
    int i, cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    if (rs_read_int(inf,&id) != 0)
    {
        if (id != RSID_MONSTERLIST)
        {
            printf("Invalid id. %x != %x(RSID_MONSTERLIST)\n",
                id,RSID_MONSTERLIST);
            printf("Sorry, invalid save game format");
            format_error = TRUE;
        }
        else if (rs_read_int(inf,&cnt) != 0)
        {
            for (i = 0; i < cnt; i++) 
            {
                l = new_item(sizeof(struct thing));
                l->l_prev = previous;
                if (previous != NULL)
                    previous->l_next = l;
                rs_read_thing(inf,(struct thing *)l->l_data);
                if (previous == NULL)
                    head = l;
                previous = l;
            }
        

            if (l != NULL)
                l->l_next = NULL;

            *list = head;
        }
    }
    else format_error = TRUE;
    
    return(READSTAT);
}

int
rs_write_object_reference(FILE *savef, struct linked_list *list, 
    struct object *item)
{
    int i;
    
    i = find_list_ptr(list, item);
    rs_write_int(savef, i);

    return(WRITESTAT);
}

rs_read_object_reference(int inf, struct linked_list *list, 
    struct object **item)
{
    int i;
    
    rs_read_int(inf, &i);
    *item = get_list_item(list,i);
            
    return(READSTAT);
}



int
rs_read_scrolls(int inf)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_read_new_string(inf,&s_names[i]);
        rs_read_boolean(inf,&s_know[i]);
        rs_read_new_string(inf,&s_guess[i]);
    }

    return(READSTAT);
}

int
rs_write_scrolls(FILE *savef)
{
    int i;

    for(i = 0; i < MAXSCROLLS; i++)
    {
        rs_write_string(savef,s_names[i]);
        rs_write_boolean(savef,s_know[i]);
        rs_write_string(savef,s_guess[i]);
    }
    return(READSTAT);
}

int
rs_read_potions(int inf)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
        rs_read_string_index(inf,rainbow,NCOLORS,&p_colors[i]);
        rs_read_boolean(inf,&p_know[i]);
        rs_read_new_string(inf,&p_guess[i]);
    }

    return(READSTAT);
}

int
rs_write_potions(FILE *savef)
{
    int i;

    for(i = 0; i < MAXPOTIONS; i++)
    {
        rs_write_string_index(savef,rainbow,NCOLORS,p_colors[i]);
        rs_write_boolean(savef,p_know[i]);
        rs_write_string(savef,p_guess[i]);
    }

    return(WRITESTAT);
}

int
rs_read_rings(int inf)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_read_string_index(inf,stones,NSTONES,&r_stones[i]);
        rs_read_boolean(inf,&r_know[i]);
        rs_read_new_string(inf,&r_guess[i]);
    }

    return(READSTAT);
}

int
rs_write_rings(FILE *savef)
{
    int i;

    for(i = 0; i < MAXRINGS; i++)
    {
        rs_write_string_index(savef,stones,NSTONES,r_stones[i]);
        rs_write_boolean(savef,r_know[i]);
        rs_write_string(savef,r_guess[i]);
    }

    return(WRITESTAT);
}

int
rs_write_sticks(FILE *savef)
{
    int i;

    for (i = 0; i < MAXSTICKS; i++)
    {
        if (strcmp(ws_stuff[i].ws_type,"staff") == 0)
        {
            rs_write_int(savef,0);
            rs_write_string_index(savef, wood, NWOOD, ws_stuff[i].ws_made);
        }
        else
        {
            rs_write_int(savef,1);
            rs_write_string_index(savef, metal, NMETAL, ws_stuff[i].ws_made);
        }
        rs_write_int(savef, ws_stuff[i].ws_vol);
        rs_write_int(savef, ws_stuff[i].ws_wght);
        rs_write_boolean(savef, ws_know[i]);
        rs_write_string(savef, ws_guess[i]);
    }

    return(WRITESTAT);
}

int
rs_read_sticks(int inf)
{
    int i = 0, list = 0;

    for(i = 0; i < MAXSTICKS; i++)
    {
        rs_read_int(inf,&list);
        if (list == 0)
        {
            rs_read_string_index(inf,wood,NWOOD,&ws_stuff[i].ws_made);
            ws_stuff[i].ws_type = "staff";
        }
        else
        {
            rs_read_string_index(inf,metal,NMETAL,&ws_stuff[i].ws_made);
            ws_stuff[i].ws_type = "wand";
        }
        rs_read_int(inf, &ws_stuff[i].ws_vol);
        rs_read_int(inf, &ws_stuff[i].ws_wght);

        rs_read_boolean(inf, &ws_know[i]);
        rs_read_new_string(inf, &ws_guess[i]);
    }

    return(READSTAT);
}

int
rs_save_file(FILE *savef)
{
    int endian = 0x01020304;
    big_endian = ( *((char *)&endian) == 0x01 );

    rs_write_daemons(savef, d_list, MAXDAEMONS);
    rs_write_int(savef, between);
    rs_write_rooms(savef, rooms, MAXROOMS);
    rs_write_room_reference(savef, oldrp);
    rs_write_monster_list(savef, mlist);
    rs_write_thing(savef, &player);
    rs_write_stats(savef,&max_stats);                   
    rs_write_object_list(savef, lvl_obj);
    rs_write_object_reference(savef, player.t_pack, cur_weapon); 
    rs_write_object_reference(savef, player.t_pack, cur_armor);
    rs_write_object_reference(savef, player.t_pack, cur_ring[0]);
    rs_write_object_reference(savef, player.t_pack, cur_ring[1]);
    assert(him == &player.t_stats);
    rs_write_traps(savef, traps, MAXTRAPS);             
    rs_write_int(savef, level);
    rs_write_int(savef, levcount);
    rs_write_int(savef, levtype);
    rs_write_int(savef, trader);
    rs_write_int(savef, curprice);
    rs_write_int(savef, purse);
    rs_write_int(savef, ntraps);
    rs_write_int(savef, packvol);
    rs_write_int(savef, demoncnt);
    rs_write_int(savef, lastscore);
    rs_write_int(savef, no_food);
    rs_write_int(savef, seed);
    rs_write_int(savef, dnum);
    rs_write_int(savef, count);
    rs_write_int(savef, fung_hit);
    rs_write_int(savef, quiet);
    rs_write_int(savef, max_level);
    rs_write_int(savef, food_left);
    rs_write_int(savef, group);
    rs_write_int(savef, hungry_state);
    rs_write_int(savef, foodlev);
    rs_write_int(savef, ringfood);
    rs_write_char(savef, take);
    rs_write_char(savef, runch);
    rs_write(savef, curpurch, 15);

    rs_write(savef, prbuf, LINLEN);
    rs_write(savef, whoami, LINLEN);
    rs_write(savef, fruit, LINLEN);
    
    rs_write_boolean(savef, isfight); 
    rs_write_boolean(savef, nlmove);
    rs_write_boolean(savef, inpool);
    rs_write_boolean(savef, inwhgt);
    rs_write_boolean(savef, running);
    rs_write_boolean(savef, playing);
#ifdef WIZARD
    rs_write_boolean(savef, wizard);
#else 
    rs_write_boolean(savef, 0);
#endif
    rs_write_boolean(savef, after);
    rs_write_boolean(savef, door_stop);
    rs_write_boolean(savef, firstmove);
    rs_write_boolean(savef, waswizard);
    rs_write_boolean(savef, amulet);
    rs_write_boolean(savef, in_shell);
    rs_write_boolean(savef, nochange);
    
    rs_write_coord(savef, oldpos);
    rs_write_coord(savef, delta);
    rs_write_coord(savef, stairs);
    rs_write_coord(savef, rndspot);

    rs_write_monsters(savef, monsters, MAXMONS+1);
    rs_write_monster_references(savef, mtlev, MONRANGE);

    rs_write_scrolls(savef);
    rs_write_potions(savef);
    rs_write_rings(savef);
    rs_write_sticks(savef);

    rs_write_magic_items(savef, things, NUMTHINGS+1);
    rs_write_magic_items(savef, a_magic, MAXARMORS+1);
    rs_write_magic_items(savef, w_magic, MAXWEAPONS+1);
    rs_write_magic_items(savef, s_magic, MAXSCROLLS+1);
    rs_write_magic_items(savef, p_magic, MAXPOTIONS+1);
    rs_write_magic_items(savef, r_magic, MAXRINGS+1);
    rs_write_magic_items(savef, ws_magic, MAXSTICKS+1);

    rs_write_window(savef, cw);
    rs_write_window(savef, mw);
    rs_write_window(savef, stdscr);

    fflush(savef);

    return(WRITESTAT);
}

rs_restore_file(int inf)
{
    bool junk;
    int endian = 0x01020304;
    big_endian = ( *((char *)&endian) == 0x01 );
    
    rs_read_daemons(inf, d_list, MAXDAEMONS);
    rs_read_int(inf, &between);
    rs_read_rooms(inf, rooms, MAXROOMS);
    rs_read_room_reference(inf, &oldrp);
    rs_read_monster_list(inf, &mlist);
    rs_read_thing(inf, &player);
    rs_read_stats(inf,&max_stats);                   
    rs_read_object_list(inf, &lvl_obj);
    rs_read_object_reference(inf, player.t_pack, &cur_weapon); 
    rs_read_object_reference(inf, player.t_pack, &cur_armor);
    rs_read_object_reference(inf, player.t_pack, &cur_ring[0]);
    rs_read_object_reference(inf, player.t_pack, &cur_ring[1]);
    him = &player.t_stats;
    rs_read_traps(inf, traps, MAXTRAPS);             

    rs_read_int(inf, &level);
    rs_read_int(inf, &levcount);
    rs_read_int(inf, &levtype);
    rs_read_int(inf, &trader);
    rs_read_int(inf, &curprice);
    rs_read_int(inf, &purse);
    rs_read_int(inf, &ntraps);
    rs_read_int(inf, &packvol);
    rs_read_int(inf, &demoncnt);
    rs_read_int(inf, &lastscore);
    rs_read_int(inf, &no_food);
    rs_read_int(inf, &seed);
    rs_read_int(inf, &dnum);
    rs_read_int(inf, &count);
    rs_read_int(inf, &fung_hit);
    rs_read_int(inf, &quiet);
    rs_read_int(inf, &max_level);
    rs_read_int(inf, &food_left);
    rs_read_int(inf, &group);
    rs_read_int(inf, &hungry_state);
    rs_read_int(inf, &foodlev);
    rs_read_int(inf, &ringfood);
    rs_read_char(inf, &take);
    rs_read_char(inf, &runch);
    rs_read(inf, curpurch, 15);

    rs_read(inf, prbuf, LINLEN);
    rs_read(inf, whoami, LINLEN);
    rs_read(inf, fruit, LINLEN);
    
    rs_read_boolean(inf, &isfight); 
    rs_read_boolean(inf, &nlmove);
    rs_read_boolean(inf, &inpool);
    rs_read_boolean(inf, &inwhgt);
    rs_read_boolean(inf, &running);
    rs_read_boolean(inf, &playing);
#ifdef WIZARD
    rs_read_boolean(inf, &wizard);
#else
    rs_read_boolean(inf, &junk);
#endif
    rs_read_boolean(inf, &after);
    rs_read_boolean(inf, &door_stop);
    rs_read_boolean(inf, &firstmove);
    rs_read_boolean(inf, &waswizard);
    rs_read_boolean(inf, &amulet);
    rs_read_boolean(inf, &in_shell);
    rs_read_boolean(inf, &nochange);
    
    rs_read_coord(inf, &oldpos);
    rs_read_coord(inf, &delta);
    rs_read_coord(inf, &stairs);
    rs_read_coord(inf, &rndspot);

    rs_read_monsters(inf, monsters, MAXMONS+1);
    rs_read_monster_references(inf, mtlev, MONRANGE);

    rs_read_scrolls(inf);
    rs_read_potions(inf);
    rs_read_rings(inf);
    rs_read_sticks(inf);

    rs_read_magic_items(inf, things, NUMTHINGS+1);
    rs_read_magic_items(inf, a_magic, MAXARMORS+1);
    rs_read_magic_items(inf, w_magic, MAXWEAPONS+1);
    rs_read_magic_items(inf, s_magic, MAXSCROLLS+1);
    rs_read_magic_items(inf, p_magic, MAXPOTIONS+1);
    rs_read_magic_items(inf, r_magic, MAXRINGS+1);
    rs_read_magic_items(inf, ws_magic, MAXSTICKS+1);

    rs_read_window(inf, cw);
    rs_read_window(inf, mw);
    rs_read_window(inf, stdscr);

    return(READSTAT);
}
