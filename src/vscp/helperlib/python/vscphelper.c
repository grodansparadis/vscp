// FILE: vscphelper.h
//
// Copyright (C) 2002-2014 Ake Hedman akhe@grodansparadis.com
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

// HISTORY:
//    130814 - AKHE Started this file
//
// http://docs.python.org/2/extending/extending.html
// http://docs.python.org/release/1.5.2/ext/parseTuple.html
//

#include <Python.h>

//-----------------------------------------------------------------------------
//								H E L P E R S
//-----------------------------------------------------------------------------
/*!
  Get bit-array from coded event data
  \param pNorm Pointer to normalised integer.
  \param length Number of bytes it consist of including
  the first normalise byte.
  \return Bit-array as a unsigned 64-bit integer.
 */
static PyObject *
vscphlp_getDataCodingBitArray( PyObject *self, PyObject *args )
{
    const char *pNorm;
    const char *length;
    
    if ( !PyArg_ParseTuple( args, "sb", &pNorm, &length ) ) {
        return NULL;
    }
    
    return getDataCodingBitArray(pNorm,length);
}

extern "C" unsigned long vscphlp_getDataCodingBitArray(const unsigned char *pNorm,
		const unsigned char length)
{
	return getDataCodingBitArray(pNorm,length);
}


static PyObject *
vscphelper_system(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system( command );
    return Py_BuildValue("i", sts);
}

