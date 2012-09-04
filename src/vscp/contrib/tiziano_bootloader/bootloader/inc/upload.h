// ****************************************************************************
// 
// DESCRIPTION: 
// 
// AUTHOR:
// 
// HISTORY:    
// 
// ****************************************************************************


#ifndef __UPLOAD_H
#define __UPLOAD_H

//-----------------------------------------------------------------------------
extern void open_upload(void);
extern void exec_upload(UINT32 ch);
extern int HandleStartBlock(unsigned char* pindata,unsigned char* poutdata);
extern int HandleBlockData(unsigned char* pindata,unsigned char* poutdata);
extern int HandleProgramBlock(unsigned char* pindata,unsigned char* poutdata);
extern int HandleActivateNewImage(unsigned char* pindata,unsigned char* poutdata);
extern void ResetChecksum(void);
//-----------------------------------------------------------------------------
#endif

