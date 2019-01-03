// ControlObject.h: interface for the CControlObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CONTROLOBJECT_H__60D17299_AE8A_4BC4_AAA8_091AF6104FD5__INCLUDED_)
#define CONTROLOBJECT_H__60D17299_AE8A_4BC4_AAA8_091AF6104FD5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Maximum # elements in list
#define MAX_ELEMENTS	25

// Maximum # elemnts in group
#define MAX_TYPE_GROUP	10

// XML parser constants
#define XML_STATE_NONE		0
#define XML_STATE_START		1
#define XML_STATE_END		2
#define XML_STATE_DATA		3
#define XML_STATE_ERRROR	4

#define BUFFER_SIZE 8192		// buffer size
#define MAX_WEBPAGE_SIZE 64000	// max WEB page size

#define LIST_MAX_NAME_LEN	80
#define LIST_MAX_DIST_LEN	20

#define TAG_MAX_SIZE		128


struct _listelement {

	/// id for record
	unsigned long m_id;
	
	TCHAR m_name[ LIST_MAX_NAME_LEN ];

	/// Number of symbols/categories/subcategory
	int m_catcnt;
	
	/// Array with cathegorys
	unsigned int m_cathegory[ MAX_TYPE_GROUP ];
	
	/// Array with subcathegorys
	unsigned int m_subcategory[ MAX_TYPE_GROUP ];
		
	/// Logitude for object
	double m_longitude;
	
	/// Latitude for object
	double m_latitude;
	
	/// Distance to object from current point
	unsigned long m_distance;

	/// Formated distance
	TCHAR m_szDistance[ LIST_MAX_DIST_LEN ];
	
	/// pointer to dynamic allocated object description
	//char m_pDescription[ 2048 ];

	
	/// pointer to dynamic allocated object url
	//char m_pur[];
	
	/// pointer to dynamic allocated object email
	//char *m_pemail;
	
	/// pointer to dynamic allocated object address
	//char *m_paddress;
	
	/// pointer to dynamic allocated object phone number
	//char *m_pphone;
};

class CControlObject  
{
public:

	CControlObject();
	virtual ~CControlObject();

	/*!
		convertToASCII

	*/
	void convertToASCII( unsigned char *p );

	/*!
		startElement

	*/
	void startElement( char *name, char **atts );

	/*!
		endElement

	*/
	void endElement( char *name );

	/*!
		characterHandler

	*/
	void characterHandler( char *data, int len );

	/*!
		parseXML

	*/
	bool parseXML( char *pData, unsigned int len );

	/*!
		getXMLValue

	*/
	bool getXMLValue( char *pBuf, 
						char *pTag, 
						char *pValue, 
						unsigned int maxlen );

	/*!
		getDataFromServer

	*/
	bool getDataFromServer( void );

	/*!
		reportObjectToServer

	*/
	bool reportObjectToServer( wxString *strName,  wxString *strDescription );


	/*!
		doHtmlPageFetch

	*/
	long doHtmlPageFetch( char *pszURL, char *pBuf, long bufsize );



	// *****************************
	// XML Parsing
	// *****************************

	int m_depth;
	TCHAR m_curtagname[ 80 ];
	TCHAR m_xmlbuf[ 8192 ];
	bool m_bStartTag;
	bool m_bObjectTag;

	/*!
		Number of elemenst in list
	*/
	unsigned short	m_cntList;

	/*!
		List elements
	*/
	_listelement m_list[ MAX_ELEMENTS ];

	char m_szServerURL[ MAX_PATH ];

};

#endif // !defined(CONTROLOBJECT_H__60D17299_AE8A_4BC4_AAA8_091AF6104FD5__INCLUDED_)
