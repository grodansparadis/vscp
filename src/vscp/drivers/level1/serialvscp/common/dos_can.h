// dos_can.h
//
// Copyright (C) 2005-2014
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//

#ifndef __DOSCANH__         
#define __DOSCANH__


#ifdef __cplusplus
  extern "C" {
#endif

// Types
typedef unsigned char _u8;
typedef unsigned short _u16;
typedef unsigned long _u32;



// Filter mask settings
#define CANUSB_ACCEPTANCE_CODE_ALL		0x00000000
#define CANUSB_ACCEPTANCE_MASK_ALL		0xFFFFFFFF




// Prototypes




#ifdef __cplusplus
}
#endif

#endif // __DOSCANH__
