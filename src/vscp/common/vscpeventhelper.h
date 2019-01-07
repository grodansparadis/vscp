// FILE: vscpeventhelper.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

// HISTORY:
//    140131 - AKHE Moved from vscphelper
//

/*!
    \file vscphelper.h
    \brief	The vscphelper file contains often used functionality when working
   with VSCP. \details vscphealper have common used functionality to do things
   in the VSCP world. It can be seens as the main toolbox for the VSCP
   programmer.
 */

#if !defined(VSCPEVENTHELPER_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_)
#define VSCPEVENTHELPER_H__C2A773AD_8886_40F0_96C4_4DCA663402B2__INCLUDED_

#include <map>
#include <string>

#include <canal.h>
#include <crc.h>
#include <crc8.h>
#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>

// Forward declaration
class CMDF;

#ifdef __cplusplus
extern "C"
{
#endif

#define MAKE_CLASSTYPE_LONG(a, b) ((((unsigned long)a) << 16) + b)

    enum VSCPInformationFormat
    {
        DEFAULT,
        WITH_DECIMAL_PREFIX,
        WITH_HEX_PREFIX,
        WITH_DECIMAL_SUFFIX,
        WITH_HEX_SUFFIX
    };

    /*!
      @brief This class holds information about VSCP events.
     */

    class VSCPInformation
    {

      public:
        // Constructores/Destructors
        VSCPInformation(void);
        ~VSCPInformation(void);

        /*!
          Get a pointer to the VSCP class hashtable.
         */
        std::map<unsigned long, std::string> *getClassHashPointer(void);

        /*!
          Get a pointer to the VSCP type hashtable.
         */
        std::map<unsigned long, std::string> *getTypeHashPointer(void);

        /*!
          Get class description from class id
         */
        std::string &getClassDescription(int vscp_class);

        /*!
          Get type description from class id and type id
         */
        std::string &getTypeDescription(int vscp_class, int vscp_type);

        /*!
            Fills a string  array with class descriptions
            \param strArray String array to fill.
            \param format Format for list. 0 is just description, 1 is
                id + description
         */

        void fillClassDescriptions(std::deque<std::string> &strArray,
                                   VSCPInformationFormat format = DEFAULT);

        /*!
            Fills a string array with type descriptions
            \param strArray String array to fill.
            \param format Format for list. 0 is just description, 1 is
                id + description
         */

        void fillTypeDescriptions(std::deque<std::string> &strArray,
                                  unsigned int vscp_class,
                                  VSCPInformationFormat format = DEFAULT);

        // We don't want the graphcal UI on apps that don't use it

      private:
        /// Hash for classes
        std::map<unsigned long, std::string> m_hashClass;
        /// Hashtable for known event VSCP classes
        // XX_DECLARE_HASH_MAP(unsigned long, std::string, wxIntegerHash,
        // wxIntegerEqual, VSCPHashClass);

        /// Hash for types
        std::map<unsigned long, std::string> m_hashType;
        /// Hashtable for known event VSCP types
        // XX_DECLARE_HASH_MAP(unsigned long, std::string, wxIntegerHash,
        // wxIntegerEqual, VSCPHashType);
    };

#ifdef __cplusplus
}
#endif

#endif // include define
