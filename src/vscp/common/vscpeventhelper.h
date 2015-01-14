// FILE: vscpeventhelper.h
//
// Copyright (C) 2002-2015 Ake Hedman akhe@grodansparadis.com
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
//    140131 - AKHE Moved from vscphelper
//

/*!
    \file vscphelper.h
    \brief	The vscphelper file contains often used functionality when working with VSCP.
    \details vscphealper have common used functionality to do things in the VSCP world. 
    It can be seens as the main toolbox for the VSCP programmer.
 */


#if !defined(VSCPEVENTHELPER_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define VSCPEVENTHELPER_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_

#ifdef VSCP_QT

#include <QString>
#include <QStringList>

#else


#include <wx/wx.h>
#include <wx/hashmap.h>
#ifndef WIN32
#include <sys/times.h>
#endif

#endif

#include "vscp.h"
#include "vscp_class.h"
#include "vscp_type.h"
#include "canal.h"
#include "../../common/crc.h"

// Forward declaration
class CMDF;

#ifdef __cplusplus
extern "C" {
#endif


    /// Hashtable for known event VSCP classes
    WX_DECLARE_HASH_MAP(unsigned long, wxString, wxIntegerHash, wxIntegerEqual, VSCPHashClass);

    /// Hashtable for known event VSCP types
    WX_DECLARE_HASH_MAP(unsigned long, wxString, wxIntegerHash, wxIntegerEqual, VSCPHashType);

#define MAKE_CLASSTYPE_LONG( a, b ) ((((unsigned long)a)<<16) + b)

    enum VSCPInformationFormat {
        DEFAULT, 
	WITH_DECIMAL_PREFIX, 
	WITH_HEX_PREFIX, 
	WITH_DECIMAL_SUFFIX, 
	WITH_HEX_SUFFIX
    };


    /*!
      @brief This class holds information about VSCP events.
     */

    class VSCPInformation {
        
    public:

        // Constructores/Destructors
        VSCPInformation(void);
        ~VSCPInformation(void);

        /*!
          Get a pointer to the VSCP class hashtable.
         */
        VSCPHashClass *getClassHashPointer(void);

        /*!
          Get a pointer to the VSCP type hashtable.
         */
        VSCPHashType *getTypeHashPointer(void);

        /*!
          Get class description from class id
         */
#ifdef VSCP_QT
#else		 
        wxString& getClassDescription(int vscp_class);
#endif		

        /*!
          Get type description from class id and type id
         */
#ifdef VSCP_QT
#else		 
        wxString& getTypeDescription(int vscp_class, int vscp_type);
#endif		


        /*!
            Fills a string  array with class descriptions
            \param strArray String array to fill.
            \param format Format for list. 0 is just description, 1 is
                id + description
         */
#ifdef VSCP_QT
#else		 
        void fillClassDescriptions( wxArrayString& strArray, 
					VSCPInformationFormat format = DEFAULT );
#endif		


#ifdef VSCP_QT
#else
        // We don't want the graphcal UI on apps that don't use it 
#if ( wxUSE_GUI != 0 )
        /*!
            Fills a combobox with class descriptions
            \param pctrl Pointer to control to fill.
            \param format Format for list. 0 is just description, 1 is
                id + description
         */
        void fillClassDescriptions(wxControlWithItems *pctrl, 
					VSCPInformationFormat format = DEFAULT);

#endif
#endif
        /*!
            Fills a string array with type descriptions
            \param strArray String array to fill.
            \param format Format for list. 0 is just description, 1 is
                id + description
         */
#ifdef VSCP_QT
#else		 
        void fillTypeDescriptions(wxArrayString& strArray, 
					unsigned int vscp_class, 
					VSCPInformationFormat format = DEFAULT);
#endif		

        // We don't want the graphcal UI on apps that don't use it 
#ifdef VSCP_QT
#else		
#if ( wxUSE_GUI != 0 )
        /*!
            Fills a combobox with type descriptions
            \param pctrl Pointer to control to fill.
            \param format Format for list. 0 is just description, 1 is
                id + description
         */
        void fillTypeDescriptions(wxControlWithItems *pctrl, 
					unsigned int vscp_class, 
					VSCPInformationFormat format = DEFAULT);

#endif
#endif

 private:
        /// Hash for classes
        VSCPHashClass m_hashClass;

        /// Hash for types
        VSCPHashType m_hashType;
    };


#ifdef __cplusplus
}
#endif


#endif // include define

