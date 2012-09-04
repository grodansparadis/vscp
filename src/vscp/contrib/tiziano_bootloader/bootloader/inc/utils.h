//*****************************************************************************
// File name: 
// Function:  
//*****************************************************************************
#ifndef __UTILS_H
#define __UTILS_H

#ifndef MIN
#define MIN(n,m)      (((n) < (m)) ? (n) : (m))
#endif
// --------------------------------------------------------------------------
// ---------------------------   hwAccess Layer -----------------------------
// --------------------------------------------------------------------------

#define    OUTB(x,y)    (*((volatile UINT8 *)(x)) = y)
#define    INB(x)       (*((volatile UINT8 *)(x)))

#define    OUTW(x,y)    (*((volatile UINT32 *)(x)) = y)
#define    INW(x)       (*((volatile UINT32 *)(x)))

// --------------------------------------------------------------------------
extern void delay(UINT32  delaytime);
extern void print_welcomemessage(void);

#endif // __UTILS_H



