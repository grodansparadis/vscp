// userlist.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//


#if !defined(USERLIST__INCLUDED_)
#define USERLIST__INCLUDED_

#include <wx/hashset.h>
#include <wx/socket.h>

#include "../../common/dllist.h"
#include "vscp.h"


#define VSCP_USER_RIGHT_ALLOW_RESTART					0x80000000
// Undefined
// Undefined
#define VSCP_USER_RIGHT_ALLOW_VARIABLE_SAVE				0x10000000

#define VSCP_USER_RIGHT_ALLOW_DM_EDIT					0x08000000
#define VSCP_USER_RIGHT_ALLOW_DM_LOAD					0x04000000
#define VSCP_USER_RIGHT_ALLOW_DM_SAVE					0x02000000
// Undefined

#define VSCP_USER_RIGHT_ALLOW_VSCP_DRV_LOAD				0x00800000
#define VSCP_USER_RIGHT_ALLOW_VSCP_DRV_UNLOAD			0x00400000
// undefined
// undefined

#define VSCP_USER_RIGHT_ALLOW_CANAL_DRV_LOAD			0x00080000
#define VSCP_USER_RIGHT_ALLOW_CANAL_DRV_UNLOAD			0x00040000
// undefined
// undefined

// undefined
// undefined
// undefined
// undefined

// undefined
// undefined
// undefined
// undefined

// undefined
// undefined
// undefined
// undefined

#define VSCP_USER_RIGHT_ALLOW_COMMAND_GROUP4			0x00000008
#define VSCP_USER_RIGHT_ALLOW_COMMAND_GROUP3			0x00000004
#define VSCP_USER_RIGHT_ALLOW_COMMAND_GROUP2			0x00000002
#define VSCP_USER_RIGHT_ALLOW_COMMAND_GROUP1			0x00000001

class CUserItem {
public:

	/// Constructor
	CUserItem(void);

	/// Destructor
	virtual ~CUserItem(void);

    /*!
	  Check if a remote client is allowed to connect.
		 First full ip address is checked against hash set (a.b.c.d)
		 Next LSB byte is replaced with a star and tested. (a.b.c.*)
		 Next the lsb-1 is also replaced with a star and tested. (a.b.*.*)
		 Last the lsb-2 is replaced with a star and tested.(a.*.*.*)
	  @param remote ip-address for remote machine.
	  @return true if the remote machine is allowed to connect.
	 */
	bool isAllowedToConnect( const wxString& remote);

	/*!
        Check if use is allowd to send event.
			First check "*.*"
	        Next check "class:type"
	        Next check "class:*"
        @param vscp_class VSCP class to test.
	    @param vscp_type VSCP type to test.
        @return true if the client is allowed to send event.
	 */
	bool isUserAllowedToSendEvent( const uint32_t vscp_class,
		                            const uint32_t vscp_type);

	/// Username
	wxString m_user;

	/// MD5 of user password
	wxString m_md5Password;

	/*!
		Bitarray with user rights i.e. tells what
		this user is allowed to do.

		"admin" has all rights.
		"driver" can send and receive events.

		Bit 31 -  
		Bit 30 - 
		Bit 29 - 
		Bit 28 -  

		Bit 27 -  
		Bit 26 -  
		Bit 25 -  
		Bit 24 - 

		Bit 23 -  
		Bit 22 -  
		Bit 21 - 
		Bit 20 -

		Bit 19 -  
		Bit 18 -  
		Bit 17 -
		Bit 16 -
	
		Bit 15 - 
		Bit 14 - 
		Bit 13 - 
		Bit 12 -

		Bit 11 -
		Bit 10 - 
		Bit 9 - 
		Bit 8 - 

		Bit 7 -  
		Bit 6 -  
		Bit 5 -  
		Bit 4 -  

		Bit 3 - Privilege MSB
		Bit 2 - Privilege
		Bit 1 - Privilege
		Bit 0 - Privilege LSB
	 */
	unsigned long m_userRights;

	/*!
		Filter associated with this user
	 */
	vscpEventFilter m_filterVSCP;

	/*!
		This list holds ip-addresses for remote
		computers that are allowed to connect to this
		machine.
	 */
	//ALLOWED_REMOTES_HASH m_hashAllowedRemotes;
	wxArrayString m_listAllowedRemotes;

	/*!
		This list holds allowed events that user can send.
		Wildcards can be used and the default is all events 
		allowed.
	 */
	wxArrayString m_listAllowedEvents;

protected:

	/*!
		System assigned ID for user
	 */
	uint32_t m_userID;

};



// hash table for users
WX_DECLARE_HASH_MAP(wxString, CUserItem*, wxStringHash, wxStringEqual, VSCPUSERHASH);

class CUserList {
public:

	/// Constructor
	CUserList(void);

	/// Destructor
	virtual ~CUserList(void);

	/*!
		Add a user that are allowed to access the control interface
		@param user Username for user.
		@param md5 MD5 of user password.
		@param userRights list with user rights on the form right1,right2,right3....
						admin - all rights
						user - standard user rights
						or an unsigned long value
		@param Ponter toa VSCP filter associated with this user.
		@param allowedRemotes List with remote computers that are allowed to connect. 
					Empty list is no restrictions.			
		@param allowedEvents List with allowed events that a remote user is allowed
					to send.
		@return true on success. false on failure.	
	 */
	bool addUser(const wxString& user,
		                    const wxString& md5,
		                    const wxString& userRights = _(""),
		                    const vscpEventFilter *pFilter = NULL,
		                    const wxString& allowedRemotes = _(""),
		                    const wxString& allowedEvents = _(""));


	/*!
		Get user 
		@param user Username
		@return Ponter to user if available else NULL
	*/
	CUserItem * getUser( const wxString& user );

	/*!
	  Check if a username i available.
	  @param user Username to test.
	  @return Pointer to useritem if valid, NULL if not.
	 */
	CUserItem * checkUser(const wxString& user, const wxString& md5password);

	

protected:

	/*!
		hash with user items
	 */
	VSCPUSERHASH m_userhashmap;


};



#endif


