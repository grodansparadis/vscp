/////////////////////////////////////////////////////////////////////////////
// Name:        platform.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     Mon 23 Apr 2007 21:41:07 CEST
// RCS-ID:      
// Copyright:   (C) Copyright 2007 Ake Hedman, eurosource
// Licence: 
//
// This files includes other files in the project that are
// different for different platforms    
/////////////////////////////////////////////////////////////////////////////

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#ifdef WIN32
#include "../../common/canalshmem_level1_win32.cpp"
#include "../../common/canalshmem_level2_win32.cpp"
#else
#include "../../common/canalshmem_level1_unix.cpp"
#include "../../common/canalshmem_level2_unix.cpp"
#endif


#endif

