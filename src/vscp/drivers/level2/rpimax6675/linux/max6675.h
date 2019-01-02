///////////////////////////////////////////////////////////////////////////////
// max6675.h:
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
//
// Copyright -2018 Matthew Robinson - https://github.com/mttrb
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MAX6675_H
#define MAX6675_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    MAX6675_CELSIUS,
    MAX6675_KELVIN,
    MAX6675_FAHRENHEIT
} MAX6675TempScale;


typedef struct max6675 {	
    int m_pi;			// Used by pigpioif2 
    int m_handle;		// Used by pigpio/pigpioif2
    int m_SpiChannel;
    int m_OpenSensor;           // != 0 if open sensor detected
    MAX6675TempScale m_scale; 	// Temperature unit
} max6675_t;

max6675_t * MAX6675Setup( int spi_channel ); 
void MAX6675Free( max6675_t *handle );

void MAX6675SetScale( max6675_t *handle, MAX6675TempScale scale );
MAX6675TempScale MAX6675GetScale( max6675_t *handle );

float MAX6675GetTempC( max6675_t *handle );
float MAX6675GetTempK( max6675_t *handle );
float MAX6675GetTempF( max6675_t *handle );

float MAX6675GetTemp( max6675_t *handle );

#ifdef __cplusplus
}
#endif

#endif
