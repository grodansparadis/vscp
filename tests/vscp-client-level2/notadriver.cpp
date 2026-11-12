///////////////////////////////////////////////////////////////////////////////
// notadriver.cpp
//
// A shared library that is not a VSCP Level II driver (no VSCPRead).
// Used to test that the client refuses to use it.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project
// <info@vscp.org>
//

#if defined(_WIN32)
#define FAKE_EXPORT extern "C" __declspec(dllexport)
#else
#define FAKE_EXPORT extern "C" __attribute__((visibility("default")))
#endif

FAKE_EXPORT long
VSCPOpen(const char * /*pPathConfig*/, const unsigned char * /*pguid*/)
{
  return 1;
}
