// vscpvariable.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 
// Ake Hedman, eurosource, <akhe@eurosource.se>
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


// VSCP daemon variable codes are defined in this file

#if !defined(VSCPVARIABLE__INCLUDED_)
#define VSCPVARIABLE__INCLUDED_

#include <wx/hashset.h>
#include <wx/datetime.h>
#include <wx/tokenzr.h>
#include "variablecodes.h"


// Class that holds one VSCP variable
// 		Peristant variables should have names staring with $

class CVSCPVariable {
public:

	/// Constructor
	CVSCPVariable(void);

	// Destructor
	virtual ~CVSCPVariable(void);

	/*!
	Set variable name
	If the variable has $ in front of the name it is persistent.
	If the variable has @ in front of the name its an array.
	@param strName Name of variable
	 */
	void setName(const wxString& strName);

	/*!
		Get variable type as string
		@param type Variable numerical type
		@return Variable type as a string.
	*/
	static const char * getVariableTypeAsString( int type );

	/*
	Get variable name
	@return Variable name.
	 */
	wxString& getName(void)
	{
		return m_name;
	};

	/*!
	Set the variable type

	@param type Variabe type code.
	 */
	void setType(uint8_t type)
	{
		m_type = type;
	};

	/*!
	Get the variable type

	@return The variable type code.
	 */
	uint8_t getType(void)
	{
		return m_type;
	};

	/*!
	Set variable note
	@param strNote Note for variable
	 */
	void setNote(const wxString& strNote)
	{
		m_note = strNote;
	};

	/*
	Get variable note
	@return Variable note.
	 */
	wxString& getNote(void)
	{
		return m_note;
	};


	/*!
	Get numerical variable type from symbol or numeric version
	@parm strVariableType Variable type in string form
	@param variable type in numerical form.
	 */
	static uint8_t getVariableTypeFromString(const wxString& strVariableType);

	/*!
	Check if a variable is persistent
	@return true if persistent else false.
	 */
	bool isPersistent(void)
	{
		return m_bPersistent;
	};

	/*!
	Set persistence

	@param b True if the variable should be persistent
	 */
	void setPersistent(bool b)
	{
		m_bPersistent = b;
	};


	/*!
	Set variable value from string
	@param type Type of value
	@param strValue Value in string form
	@return true on success.
	 */
	bool setValueFromString(int type, const wxString& strValue);

	/*!
	Get the variable as a string value
	@param str String that will get string representation of variable.
	 */
	bool writeVariableToString(wxString& strValue);

	/*!
	Get variable information as a string value
	Format is: "variable name",type,"persistence","value"
	example 1: test_int,3,true,24000
	example 2: test_string,1,true,"This is a string"
	example 3: test_event,7,false,0,20,1,2,3,4,5,6

	@param str String that will get string represenation of variable with all info.
	 */
	bool getVariableFromString(const wxString& strVariable);

	/*
	Set persistent value
	 */
	void setPersistatValue(const wxString& str)
	{
		m_persistant = str;
	};


	/*!
	Reset 
	Set variable to default values

	 */
	void Reset(void);


	/*!
	  isTrue 
	  @return Return true if the variable is of type bool and is true. 
	  True is also returned for a numerical varable (int, long, float) if 
	  it has a non zero value. In all other cases false will be returned.
	 */
	bool isTrue(void);

	/*!
	  setTrue
	  Set a variable to true. A boolean variable will be set to true.
	  A numerical variable (int, long, float) is set to -1. A string is set to "true".
	 */
	void setTrue(void);

	/*!
	  setFalse
	  Set a variable to false. A boolean variable will be set to false.
	  A numerical variable (int, long, float) is set to 0. A string is set to "false".
	 */
	void setFalse(void);

	/*!
	  setValue
	  @param val String to set value to.
	 */
	void setValue(wxString& val)
	{
		m_strValue = val;
	};

	/*!
	  getValue
	  @param value String that will receive value.
	 */
	void getValue(wxString *pValue)
	{
		*pValue = m_strValue;
	};

	/*!
	  setValue
	  @param val long to set value to.
	 */
	void setValue(int val)
	{
		m_longValue = val;
	};

	/*!
	  getValue
	  @param value Int that will receive value.
	 */
	void getValue(int *pValue)
	{
		*pValue = m_longValue;
	};
	
	/*!
	  setValue
	  @param val long to set value to.
	 */
	void setValue(long val)
	{
		m_longValue = val;
	};

	/*!
	  getValue
	  @param value Int that will receive value.
	 */
	void getValue(long *pValue)
	{
		*pValue = m_longValue;
	};

	/*!
	  setValue
	  @param val double to set value to.
	 */
	void setValue(double val)
	{
		m_floatValue = val;
	};

	/*!
	  getValue
	  @param value Double that will receive the value.
	 */
	void getValue(double *pValue)
	{
		*pValue = m_floatValue;
	};

	/*!
	  setValue
	  @param val Boolean to set value to.
	 */
	void setValue(bool val)
	{
		m_boolValue = val;
	};

	/*!
	  getValue
	  @param value Bool that will receive the value.
	 */
	void getValue(bool *pValue)
	{
		*pValue = m_boolValue;
	};



private:

	/// Name of variable
	wxString m_name;

	/// type of variable
	unsigned char m_type;

	/// Flag to make variable persisent
	bool m_bPersistent;

	/*! 
	Flag to mark a variable as an array
	Arrays are never persistent!
	 */
	bool m_bArray;

public:

	/// Note about variable
	wxString m_note;

	/*! 
		Storage
		A "class" variable is stored in the VSCP event.
		A "type" variable is stored in the VSCP event.
		A "GUID" variable is stored in the VSCP event.
		A VSCP data variables s stored in the VSCP event.
	 */
	wxString m_strValue;			// String
	vscpEvent m_event;				// VSCP event
	bool m_boolValue;				// Logical values
	long m_longValue;				// Byte, Integer and long values
	double m_floatValue;			// Floating point values
	uint8_t m_normIntSize;			// Size for Normalised integer	
	uint8_t m_normInteger[8];		// Normalised integer data
	unsigned char m_GUID[16];		// GUID
	wxDateTime m_timestamp;			// Timestamp

	wxString m_persistant;			// Original value is here
};


// Variable hash table define
WX_DECLARE_HASH_MAP(wxString, CVSCPVariable*, wxStringHash, wxStringEqual, VscpVariableHash);

// Variable list
//	This is needed because the iteration through the map does not work!!!
WX_DECLARE_LIST(CVSCPVariable, listVscpVariable);

class CVariableStorage {
public:
	/// Constructor
	CVariableStorage();

	/// Destructor
	virtual ~CVariableStorage();


	/*!
	Find variable from its name
	@param name Variable to search. Name can be preceded with
	persistence marker "$" or array marker "@"
	@return Pointer to variable or NULL if not found.
	 */
	CVSCPVariable * find(const wxString& name);

	/*!
	  Add a variable.
	  If the variable has $ in front of the name it is persistent.
	  If the variable has @ in front of the name its an array.
	  @param name Name of variable
	  @param value Value for variable
	  @param type Type of variable. Defaults to string.
	  @param bPersistent True if the variable should be saved to
	  persistent storage.
	  @return true on success, false on failure.
	 */
	bool add(const wxString& varName,
		const wxString& value,
		uint8_t type = VSCP_DAEMON_VARIABLE_CODE_STRING,
		bool bPersistent = false);

	/*!
	  Add a variable.
	  @param var Pointer to variable object.
	  @return true on success, false on failure.
	 */
	bool add(CVSCPVariable *pVar);

	/*!
	Remove named variable
	@param name Name of variable
	@return true on success, false on failure.
	 */
	bool remove(wxString& name);

	/*!
	Read persistent variables
	@return Returns true on success false on failure.
	 */
	bool load(void);

	/*!
	Write persistent variables to configured storage
	@return Returns true on success false on failure.
	 */
	bool save(void);

	/*!
	  Write persistent variables to file
	  @param strcfgfile path to variable file where data should be written.
	  @return Returns true on success false on failure.
	 */
	bool save(wxString& path);

	/// Hash table for variables with variable name as key
	VscpVariableHash m_hashVariable;

	/// List with variables  
	listVscpVariable m_listVariable;

public:

	/*!
	  Configuration path for variable persistant storage
	 */
	wxString m_configPath;
};


#endif

