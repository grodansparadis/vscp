/////////////////////////////////////////////////////////////////////////////
// Name:        TestdcApp.h
// Purpose:     wxIndustrialControls Library
// Author:      Marco Cavallini <m.cavallini AT koansoftware.com>
// Modified by: 
// Copyright:   (C)2004-2006 Copyright by Koan s.a.s. - www.koansoftware.com
// Licence:     KWIC License http://www.koansoftware.com/kwic/kwic-license.htm
/////////////////////////////////////////////////////////////////////////////


#ifndef TestdcApp_H
#define TestdcApp_H

// Define a new application type, each program should derive a class from wxApp
class TestdcApp : public wxApp
{
public:
    TestdcApp();
    virtual ~TestdcApp();

    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

DECLARE_APP(TestdcApp)

#endif // TestdcApp_H


