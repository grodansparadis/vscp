//*****************************************************************************
// FILENAME:                                                    
//                                                                           
// VERSION:                                                              
//                                                                          
// DESCRIPTION:                                   
//                                                                           
// TOOLS:                             		  
//                                                                           
// REVISION HISTORY                                                          
// Version  Author           Date        Remarks							  
//                                      
//*****************************************************************************

// @file
// CAN Controller Acceptance Filter Configuration
// 
// This header file configures the acceptance filter, i.e. defines Identifier
// ranges for all sorts of message filtering desired for a certain CAN network
// This file contains some certain definitions (actually an example) and can
// be used as a template for further modifications

#ifndef __CANGAF_H
#define __CANGAF_H

//*****************************************************************************
// Standard include files                                                    
//*****************************************************************************

#include "config.h"

//*****************************************************************************
// Project include files                                                     
//*****************************************************************************

//*****************************************************************************
// Types                                                                     
//*****************************************************************************

//
// Structure used for 11-bit CAN Identifiers
//
typedef struct
{
	UINT8    Channel;
	UINT16   ID;
} CAN_ACF;

//
// Structure used for 29-bit CAN Identifiers
//
typedef struct
{
	UINT8    Channel;
	UINT32   IDx;
} CAN_ACFx;

#endif // __CANGAF_H */
