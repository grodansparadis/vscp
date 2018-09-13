// dm.h
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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
//

#if !defined(DM__INCLUDED_)
#define DM__INCLUDED_

#include "wx/wx.h"
#include <wx/datetime.h>
#include <wx/dynlib.h>
#include <wx/file.h>
#include <wx/url.h>
//#include <dllist.h>
#include <tables.h>
#include <vscp.h>
#include <remotevariable.h>

class CControlObject;
class CClientItem;
class cvscpvariable;
class dmTimer;

// Holds timer objects with integer keys
WX_DECLARE_HASH_MAP( int, dmTimer*, wxIntegerHash, wxIntegerEqual, DMTIMERS );

WX_DECLARE_LIST ( wxDynamicLibrary, PLUGINLIST ); 	// List with DM plugins
WX_DECLARE_LIST( int, ACTIONTIME );

// Control bits  (deprecated)
#define DM_CONTROL_DONT_CONTINUE_SCAN   0x40000000
#define DM_CONTROL_CHECK_INDEX          0x00000020
#define DM_CONTROL_CHECK_ZONE           0x00000010
#define DM_CONTROL_CHECK_SUBZONE        0x00000008

// Measurement compare constants
#define DM_MEASUREMENT_COMPARE_NOOP     0x00    // No compare
#define DM_MEASUREMENT_COMPARE_EQ       0x01    // Equal to ==
#define DM_MEASUREMENT_COMPARE_NEQ      0x02    // Not equal to !=
#define DM_MEASUREMENT_COMPARE_LT       0x03    // Less than <
#define DM_MEASUREMENT_COMPARE_LTEQ     0x04    // Less than or equal to <=
#define DM_MEASUREMENT_COMPARE_GT       0x05    // Greater than
#define DM_MEASUREMENT_COMPARE_GTEQ     0x06    // Greater than or equal tp >=


// Variable check constants
enum VariableCheckType {
    VARIABLE_CHECK_SET_TRUE = 0,
    VARIABLE_CHECK_SET_FALSE,
    VARIABLE_CHECK_SET_OUTCOME
};

// Forward declaration
class CDM;



/*!
    DM Timer
*/

class dmTimer
{

public:

  /// Constructor
  dmTimer();
  
  /*! 
    Constructor
    @param nameVar Name of variable
    @param id Timer id.
    @param delay Timer value
    @param bStart Run flag for timer
    @param setValue Value to set variable to when timer elapse
  */
  dmTimer( wxString& nameVar,
              uint32_t id,
              uint32_t delay = 0, 
              bool m_bStart = false, 
              bool setValue = false,
              int reloadLimit = -1 );

  /// Destructor
  ~dmTimer();
  
  
  /*! 
    Init
    @param nameVar Name of variable
    @param id Timer id.
    @param delay Timer value
    @param bStart Run flag for timer
    @param setValue Value to set variable to when timer elapse
    @param reloadLimit Limit for reloads (-1 is no limit)
  */
  void init( wxString& nameVar,
              uint32_t id,
              uint32_t delay = 0, 
              bool m_bStart = false, 
              bool setValue = false,
              int reloadLimit = -1 );

  /*!
    Check if the timer is active
    @returns true if active.
  */
  bool isActive( void );

  /*!
    Set the time active state
    @param bState (default true) which sets te active state.
  */
  void setActive( bool bState = true );

  /*!
   * Getter/setter for id
   */
  void setId( uint32_t id ) { m_id = id; };
  uint32_t getId( void ) { return m_id; };
  
  /*!
    Stop timer
  */
  void stopTimer( void );;
  
  /*!
    Pause timer
  */
  void pauseTimer( void );

  /*!
    Start timer
  */
  void startTimer( void );
                            
  /*!
    Resume timer
  */
  void resumeTimer( void );                          

  /*!
    Get current timer value
    @return current timer value.
  */
  uint32_t getDelay( void ) { return m_delay; };
  
  /*!
    Get start timer value
    @return start timer value.
  */
  uint32_t getStartDelay( void ) { return m_delayStart; };

  /*!
    Set timer value
    @param timer value.
  */
  void setDelay( uint32_t delay ) { m_delayStart = m_delay = delay; };

  /*!
    Check if timer is of reloading type
    @return true if reloading, false otherwise
  */
  bool isReloading( void ) { return m_bReload; };

  /*!
    Set the reloading functionality
    @param reload flag
  */
  void setReload( bool bReload ) { m_bReload = bReload; };

  /*!
   * Reload timer
   */
  void reload( void ) { m_delay = m_delayStart; m_reloadCounter--; };
  
  /// Setter/getters for reloadlimit
  void setReloadLimit( int limit ) { m_reloadlimit = limit; };
  int getReloadLimit( void ) { return m_reloadlimit; };
  bool isReloadLimit( void ) { return ( m_reloadlimit > 0 ); };
  
  int getReloadCounter( void ) { return m_reloadCounter; };
  
  /*!
    Decrement timer if greater than zero
  */
  uint32_t decTimer( void );

  /*!
    Get variable name
    @return Variable name
  */
  wxString& getVariableName( void ) { return m_nameVariable; };


  /*!
    Set the variable name
    @param Variable name
  */
  void setVariableName( wxString& nameVariable ) { m_nameVariable = nameVariable; };

  /*!
    *** This is for future usage ***
    Pointer to running thread. This pointer is
    != NULL if thread is running and NULL otherwise.
  */
  wxThread* m_pThread;


private:

  /// Active flag. True if timer should run.
  bool m_bActive;
  
  /// id
  uint32_t m_id;

  /// Delay time
  uint32_t m_delay;
  
  // Delay start time
  uint32_t m_delayStart;

  /// Name of control variable
  wxString m_nameVariable;

  /// Reload value when timer elapsed 
  bool m_bReload;
  
  // Max number of reload to do.
  // -1 is no limit.
  int m_reloadlimit;
  
  // Counter used when reload limit is used
  int m_reloadCounter;
  
  // paused is set after a timer has been pauses
  // it is reseted by a start or a resume
  bool m_bPaused;
};


/*!
    Timed object
*/

class actionTime
{

public:
    
  actionTime();
  ~actionTime();

  /*!
   * Clear all tables
   */
  void clearTables( void );
  

  
  /*!
    Set the weekdays that the action is allowed to occur at. The
    string is on the form 'mtwtfss'  The day can be replaced with a
    '-' to indicate a none valid day.
    @param actionTime strWeek on the format 'mtwtfss'
    @return True on success.
  */
  bool setWeekDays( const wxString& strWeek );


  /*!
    Get the allowed weekdays as a wxWtring on the form 'mtwtfss' where
    '-' is used to indicate a day when the event is not allowe to occur.
    @return A string with the allowed weekdays.
  */
  wxString getWeekDays( void );

  /*!
    Set an allowed weekday in realtext.
    @param strWeekday Weekday to set in real text as "monday", "tuesday", 
        "wednesday", "thursday", "friday", "saturday", "sunday".
    @return true on success.
  */

  bool setWeekDay( const wxString& strWeekday );
  
  /*!
   * Set data so action is always allowed.
   */
  void allowAlways( void );
  
  /*!
    Allow/disallow action to happen on Mondays
    @param bAllow True to allow action on this day.
   */
  void allowMonday( bool bAllow = true ) { m_weekDay[ 0 ] = bAllow; };
  
  /*!
    Allow/disallow action to happen on Tuesday
    @param bAllow True to allow action on this day.
   */
  void allowTuesday( bool bAllow = true ) { m_weekDay[ 1 ] = bAllow; };
  
  /*!
    Allow/disallow action to happen on Wednesday
    @param bAllow True to allow action on this day.
   */
  void allowWednesday( bool bAllow = true ) { m_weekDay[ 2 ] = bAllow; };
  
  /*!
    Allow/disallow action to happen on Thursday
    @param bAllow True to allow action on this day.
   */
  void allowThursday( bool bAllow = true ) { m_weekDay[ 3 ] = bAllow; };
  
  /*!
    Allow/disallow action to happen on Friday
    @param bAllow True to allow action on this day.
   */
  void allowFriday( bool bAllow = true ) { m_weekDay[ 4 ] = bAllow; };
  
  /*!
    Allow/disallow action to happen on Saturday
    @param bAllow True to allow action on this day.
   */
  void allowSaturday( bool bAllow = true ) { m_weekDay[ 5 ] = bAllow; };
  
  /*!
    Allow/disallow action to happen on Sunday
    @param bAllow True to allow action on this day.
   */
  void allowSunday( bool bAllow = true ) { m_weekDay[ 6 ] = bAllow; };

  /*!
    This method parse a string on the form YYYY-MM-DD HH:MM:SS. Both parts can 
    be replaced with a '*' to indicate that it is a no care meaning that * * is
    for all dates and all time while * HH:MM:SS is for all dates but a specific times.
    Further all elements such as YYYY, MM, DD, HH, MM, SS can be replaced with a * to
    represent a no care for each where it's present. Each can also be given as a list 
    separated with '/' characters to indicate several choices. So YYYY-MM-DD HH:0/5/10;SS
    means the action should be performed on a specific date and hour on every full hour, five
    minutes past and ten minutes past.
    @param actionTime Time on the format YYYY-MM-DD HH:MM:SS
    @return True on success.
  */
  bool parseActionTime( const wxString& actionTime );

  /*!
    Get multiple integers when they are in a string separated
    with '/' characters.
    @param item The list separated with '/'.
    @param pList Pointer to list with integer items.
    @return True on success.
  */
  bool getMultiItem( const wxString& items, ACTIONTIME *pList );

  /*!
    Check if we should do actionTime action
    @return True if the actions should be performed else false.
  */
  bool ShouldWeDoAction( void );

  /*!
    Get the actiontime as a readable string
    @return actiontime as a string.
  */
  wxString getActionTimeAsString( void );
  
  /*!
   * Get/set weekday allow
   */
  bool getWeekday( char day ) { return m_weekDay[ day & 0x07 ]; };
  
  bool setWeekday( char day, bool bAllow = true ) { return m_weekDay[ day & 0x07 ] = bAllow ; };

  /*!
   * Get/set from time.
   */
  void setFromTime( wxString strFrom ) {
      
      strFrom.Trim();
      strFrom.Trim(false);
      if ( '*' == strFrom ) {
          strFrom = _("0000-01-01 00:00:00");
      }
      
      m_fromTime.ParseDateTime( strFrom );
      if ( !m_fromTime.IsValid() ) {
          m_fromTime.ParseDateTime( _("0000-01-01 00:00:00") );
      }
  };
  
  void setFromTime( wxDateTime& dt ) { m_fromTime = dt; };
  
  wxDateTime& getFromTime( void ) { return m_fromTime; };

  

  /*!
   * Get/set end time.
   */
  void setEndTime( wxString strEnd ) {
      
      strEnd.Trim();
      strEnd.Trim(false);
      if ( '*' == strEnd ) {
          strEnd = _("9999-12-31 23:59:59");
      }
      
      m_endTime.ParseDateTime( strEnd );
      if ( !m_endTime.IsValid() ) {
          m_endTime.ParseDateTime( _("9999-12-31 23:59:59") );
      }
      
  };
  
  void setEndTime( wxDateTime& dt ) { m_endTime = dt; };
  
  wxDateTime& getEndTime( void ) { return m_endTime; };
  
  
  private:
  
  /*!
    True if working on that day.
  */
  bool m_weekDay[ 7 ];      // Monday - Sunday
  
  /*!
    This is the time (inclusive) from which this action is
    allowed to occur.
  */
  wxDateTime m_fromTime;
  
  /*!
    This is the time up (inclusive) to which this action is
    allowed to occur.
  */
  wxDateTime m_endTime;
  
  // If the year/month/day etc is in any of these arrays it is a positive match
  // Empty is an all positive match

  /// Year when action should be performed, count == 0 is don't care
  ACTIONTIME m_actionYear;

  /// Month when action should be performed, count == 0 is don't care
  ACTIONTIME m_actionMonth;

  /// Day when action should be performed, count == 0 is don't care
  ACTIONTIME m_actionDay;

  /// Hour when action should be performed, count == 0 is don't care
  ACTIONTIME m_actionHour;

  /// Minute when action should be performed, count == 0 is don't care
  ACTIONTIME m_actionMinute;

  /// Second when action should be performed, count == 0 is don't care
  ACTIONTIME m_actionSecond;
  
};


/*!
    Decision Matrix Element
*/

class dmElement
{
    
public:

    /// Constructor
    dmElement( void );

    /// Destructor
    ~dmElement( void );
    
    /*!
        Assignment overload
    */
    dmElement& operator=( const dmElement& dm );
    

    /*!
        Get DM row as a string description.
        @param bCRLF true to have CRLF placed at end (default)
        @return A string representation for the item. This string is on the form 
        enabled,from,to,weekday,time,mask,filter,index,zone,subzone,control-code,
        action-code,action-param,comment,trig-counter,error-counter
        where mask is Priority;Class;Type;GUID an filter is Priority;Class;Type;GUID
        See the specification for a description of the from, to and weekday fields. 
    */
    wxString getAsString( bool bCRLF = true );
    
    /*!
        Set DM row content from string 
        @param strDM DM row in string form.
        @return True on success.
     */
    bool setFromString( wxString& strDM );

    /*!
        Enable row
    */
    void enableRow( void )  { m_bEnable = true; };

    /*!
        Disable row
    */
    void disableRow( void )  { m_bEnable = false; };

    /*!
        Check if it is enabled
        @returns true if enabled false otherwise
    */
    bool isEnabled( void ) { return m_bEnable; };
  
    /*!
        Check if index should be checked
        @returns true if enabled false otherwise
     */
    bool isCheckIndexSet( void ) { return m_bCheckIndex; };
    
    /*!
        Check if zone should be checked
        @returns true if enabled false otherwise
    */    
    bool isCheckZoneSet( void ) 
        { return m_bCheckZone; };
  
    /*!
        Check if zone should be checked
        @returns true if enabled false otherwise
    */
    bool isCheckSubZoneSet( void ) { return m_bCheckSubZone; };
    
    // Check if measurement should be compared.
    bool isCompareMeasurementSet() { return m_bCompareMeasurement; };
    
    /*!
        Get measurement compare code from symbolic compare value
        @parfam strCompare Symbolic compare value
        @return Symbolic code or zero if invalid code (DM_MEASUREMENT_COMPARE_NOOP)
     */
    uint8_t getCompareCodeFromSymbolicMeasurement( wxString& strCompare );
    
    void setSymbolicMeasurementCompareCode( wxString& strCompare ) 
            { m_measurementCompareCode = getCompareCodeFromSymbolicMeasurement( strCompare ); };
            
    wxString getSymbolicMeasurementFromCompareCode( uint8_t cc, uint8_t type = 0 );
  
    /*!
        Handle escape sequences
        @param pEvent Event feed true matrix
        @param str String to replace escapes in
        @return true on success, else false.
    */
    static bool handleEscapes( vscpEvent *pEvent, wxString& str );
    
    /*
        Execute for Unix.
        @param argExec shell command line to execute.
        @return True on success.
    */
#ifndef WIN32    
    bool unixVSCPExecute( wxString& argExec );
#endif    

    /*!
        Perform the selected internal action. The parameter
        is action dependent.
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doAction( vscpEvent *pDMEvent );

    /*!
        Execute action 
        Just executes the external action script. The parameter
        is action dependent.
        @param pDMEvent Event that triggered the action
        @param bCheckExecutable Check if target is an existing file and
                that it is marked as executable.
        @returns true if all went well.
    */
    bool doActionExecute( vscpEvent *pDMEvent, 
                            bool bCheckExecutable = true );

    /*!
        Execute action conditional
        Executes the external action script if variable is true. 
        The parameter is action dependent.
        @param pDMEvent Event that triggered the action
        @param bCheckExecutable Check if target is an existing file and
                that it is marked as executable.
        @returns true if all went well.
    */
    bool doActionExecuteConditional( vscpEvent *pDMEvent, 
                                        bool bCheckExecutable = true );

    /*!
        Timed exeute the external action script.
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionTimedExecute( vscpEvent *pDMEvent );

    /*!
        Send event action 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionSendEvent( vscpEvent *pDMEvent );

    /*!
        Send event conditional action 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionSendEventConditional( vscpEvent *pDMEvent );

    /*!
        Send events from file action 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionSendEventsFromFile( vscpEvent *pDMEvent );
    
    /*!
        Send event to remote VSCP server action 
        @param pDMEvent Event that triggered the action
        @param bSecure Should be set to true for a secure connection (SSL)
        @returns true if all went well.
    */
    bool doActionSendEventRemote( vscpEvent *pDMEvent, bool bSecure = false );

    /*!
        Store in variable action 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionStoreVariable( vscpEvent *pDMEvent );

    /*!
        Add to variable action 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionAddVariable( vscpEvent *pDMEvent );

    /*!
        Subtract from variable action 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionSubtractVariable( vscpEvent *pDMEvent );

    /*!
        Multiply variable action 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionMultiplyVariable( vscpEvent *pDMEvent );

    /*!
        Divide variable action 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionDivideVariable( vscpEvent *pDMEvent );
    
    /*!
     * Check variable against constant
     * @param pDMEvent Event that triggered the action
     * @param type Type of check
     * @returns true if all went well.
     */
    bool doActionCheckVariable( vscpEvent *pDMEvent, VariableCheckType type );
    
    /*!
     * Check variable against measurement
     * @param pDMEvent Event that triggered the action
     * @returns true if all went well.
     */
    bool doActionCheckMeasurement( vscpEvent *pDMEvent );
    
    /*!
     * Store new minimum of measurement in variable if lower than current.
     * @param pDMEvent Event that triggered the action
       @returns true if all went well.
     */
    bool doActionStoreMin( vscpEvent *pDMEvent );
    
    /*!
     * Store new maximum of measurement in variable is higher than current.
     * @param pDMEvent Event that triggered the action
       @returns true if all went well.
     */
    bool doActionStoreMax( vscpEvent *pDMEvent );

    /*!
        Start a timer 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionStartTimer( vscpEvent *pDMEvent );

    /*!
        Pause a timer 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionPauseTimer( vscpEvent *pDMEvent );

    /*!
        Resume a timer 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionResumeTimer( vscpEvent *pDMEvent );

    /*!
        Stop a timer 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionStopTimer( vscpEvent *pDMEvent );

    /*!
        Write to file 
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionWriteFile( vscpEvent *pDMEvent );

    /*!
        Get URL
        @param pDMEvent Event that triggered the action
        @returns true if all went well.
    */
    bool doActionGetURL( vscpEvent *pDMEvent );
    
    
    
    


    // Database index for record ( 0 if loaded from XML file)
    uint32_t m_id;
    
    // This record is loaded from XML file - Can't be edited
    bool m_bStatic;
    
    /// True if row is enabled
    bool m_bEnable;
    
    /// Group ID
    wxString m_strGroupID;
    
    /// DM row filter
    vscpEventFilter m_vscpfilter;

    /// Action code
    uint32_t m_actionCode;

    /// Action parameters
    wxString m_actionparam;

    /// A counter that is updated each time a DM row is matched
    uint32_t m_triggCounter;

    /// A counter that is updated each time an error occurs
    uint32_t m_errorCounter;
    
     /// True if index should be checked
    bool m_bCheckIndex;

    /// If index should be checked this is the one
    uint8_t m_index;

    /// Index should be masked so only the LSB tree bits are checked
    bool m_bCheckMeasurementIndex;
    
    /// True if zone should be checked
    bool m_bCheckZone;
    
    /// If zone should be checked this is the one
    uint8_t m_zone;

    /// True if subzone should be checked
    bool m_bCheckSubZone;
    
    /// If sub zone should be checked this is the one
    uint8_t m_subzone;

    /// Description of last error
    wxString m_strLastError;

    /// Description for entry
    wxString m_comment;

    /// Action time 
    actionTime m_timeAllow;
    
    // Measurement value should be compare with
    // measurement event
    bool m_bCompareMeasurement;

    // Value for measurement comparisons
    double m_measurementValue;
    
    // Unit code for measurement value
    uint8_t m_measurementUnit;

    // Measurement compare code
    uint8_t m_measurementCompareCode;
    
    // Pointer to owner
    CDM *m_pDM;


    /////////////////////////////////////////////////////////////////////////////
    //                    A C T I O N  -  P A R A M E T E R S
    ////////////////////////////////////////////////////////////////////////////


    // When a dm row is added the action code and the action parameter
    // is parsed. Values here are used for storage of parsed data

    /// For action execute dll
    wxDynamicLibrary m_dll;     // dynamic lib holder
    void *m_dllMethod;          // Pointer to dll/dl method

    /// Variable +-*/
    uint8_t m_operation;
    CVSCPVariable m_variable;

    // Event(s)
    // Offset 0 is for single event send and true conditional.
    // Offset 1 is for false conditional event.
    vscpEventEx m_event[2];

    // For file writes
    wxFile m_file;


    /////////////////////////////////////////////////////////////////////////////
    //                     A C T I O N  -  H E L P E R S
    ////////////////////////////////////////////////////////////////////////////



};


// List with DMitems
WX_DECLARE_LIST ( dmElement, DMLIST );


/*!
    Decision Matrix
*/

class CDM
{

public:

    // Constructor
    CDM( CControlObject *ctrlObj = NULL );

    // Destructor
    ~CDM();

    /*!
        Init open logfile etc
    */
    void init( void );

    /*!
        Cleanup DM resources
    */
    void cleanup( void );

    /*!
     * Get compare code from token
     * @param token String token for compare operation
     * @return Code for token or -1 f error.
     */
    static int getCompareCodeFromToken( wxString& token ); 
    
    /*!
     * Get compare token from code
     * @param code Compare codee
     * @return Compare token or empty string if invalid token
     */
    static wxString getTokenFromCompareCode( uint8_t code );

    /*!
        Add Element to matrix
    */
    bool addMemoryElement( dmElement *pItem );

    /*!
        Remove Element from matrix
    */
    bool removeMemoryElement( unsigned short row );

    /*!
        Get number of rows in matrix
    */
    unsigned short getMemoryElementCount( void ) { return m_DMList.GetCount(); };

    /*!
        Get a row from the matrix (from row number)
        @param row DM row to fetch
        @return Pointer to DM row if found, NULL else.
    */
    dmElement *getMemoryElementFromRow( const short row );
    
    /*!
        Get database index from row
        @param row in memory
        @return Database index or zero if there is no
            database item for this record. Note that 
            this can be due to a XML loaded record which
            all have zero indexes.
    */
    uint32_t getDbIndexFromRow( const short row );

    /*!
        Get a row from the matrix (from idx )
        @param idx Database index for DM row to fetch 
        @param prow [out] Pointer to short that will receive row (if no NULL) where
                DM entry was found. 
        @return Pointer to DM row if found, NULL else.
    */
    dmElement *getMemoryElementFromId( const uint32_t idx, short *prow = NULL );
    
    /*!
     * Get all rows in DM as string
     * @param strAllRows All DM rows (crlf separated)
     * @return True on success.
     */
    bool getAllRows( wxString& strAllRows );
    
    /*!
        Add database record
        @param dm Reference to DM element with record data.
        @return True if record inserted.
     */
    bool addDatabaseRecord( dmElement *pdm );
    
    /*!
        Update database record
        @param dm Reference to DM element with record data.
        @return True if record found and updated.
     */
    bool updateDatabaseRecord( dmElement *pdm );
    
    /*
        Update item if database record
     */
    bool updateDatabaseRecordItem( unsigned long id, 
                                    const wxString& strUpdateField, 
                                    const wxString& strUpdateValue );
    
    /*!
        Read DM record from database
        @param idx Database index
        @param pDMitem Pointer to DM element that will be filled with 
                record data.
        @return True if record found.
     */
    bool getDatabaseRecord( uint32_t idx, dmElement *pDMitem );
    
    /*!
        Delete database record
        @param idx Database index
        @return True if record found.
     */
    bool deleteDatabaseRecord( uint32_t idx );
    
    /*!
        Delete database record count
        @param idx Database index
        @return True if record found.
     */
    uint32_t getDatabaseRecordCount( void );
    
    /*!
        Load from Database
    */
    bool loadFromDatabase( void );
    
    /*!
        Load DM from external storage.
    */
    bool loadFromXML( void );

    /*!
        Save DM to external storage.
        @param bAll Save all recotds to XML file if true, else
            only records loaded from XML file.
        @return true on success, false on failure.    
    */
    bool saveToXML( bool bAll = true );

    /*!
        Run an event through the matrix
    */
    bool feed( vscpEvent *pEvent );

    /*!
        Feed periodic events true the matrix
        SECONDS
        MINUTE
        HOUR
        NOON
        MIDNIGHT
        WEEK
        MONTH
        QUARTER
        YEAR
        RANDOM-MINUTE
        RANDOM-HOUR
        RANDOM-DAY
        RANDOM-WEEK
        RANDOM-MONTH
        RANDOM-YEAR
        DUSK
        DAWN
    */
    bool feedPeriodicEvent( void );

    /*!
     * Feed timer started event
     * @param id Timer id
     * @param time Start time in ms
     * return true if event was delivered
     */
    bool feedTimerStarted( uint32_t id, uint32_t time );
    
    /*!
     * Feed timer paused event
     * @param id Timer id
     * @param time Start time in ms
     * return true if event was delivered
     */
    bool feedTimerPaused( uint32_t id, uint32_t time );
    
    /*!
     * Feed timer resumed event
     * @param id Timer id
     * @param time Start time in ms
     * return true if event was delivered
     */
    bool feedTimerResumed( uint32_t id, uint32_t time );
    
    /*!
     * Feed timer stopped event
     * @param id Timer id
     * @param time Start time in ms
     * return true if event was delivered
     */
    bool feedTimerStopped( uint32_t id, uint32_t time );
    
    /*!
     * Feed timer elapsed event
     * @param id Timer id
     * return true if event was delivered
     */
    bool feedTimerElapsed( uint32_t id );
    
    //------------------------------------
    //              Timers
    //------------------------------------

    /*!
        serviceTimers
        This method service all timers and handle there decrement
    */
    void serviceTimers( void );

    /*!
        Add a new timer
        A new timer is created if it is not yet existing. To test this the
        variablename is searched before a new timer is created. If already 
        available the old timer will be initialized with the new values.
        @param nameVar Name of variable
        @param delay Timer value
        @param bStart Run flag for timer
        @param bReload True if reload should be done
        @return a timer if > 0 on success
    */
    int addTimer( uint32_t id,
                    wxString& nameVar, 
                    uint32_t delay = 0, 
                    bool bStart = false, 
                    bool bReload = false,
                    int reloadLimit = -1 );

    /*!
        Start an existing timer. Do nothing if timer does not exist.
        @return true on success
    */
     bool startTimer( uint32_t idTimer );

    /*!
        Start a timer and set 'setvalues' If the timer does not
        exist create it.
        @return timer id or zero on failure
    */
    int startTimer( uint32_t id, 
                        wxString& nameVariable, 
                        uint32_t delay, 
                        bool bSetValue = false,
                        int reloadLimit = -1 );

     /*!
        Stop an existing timer
        @return true on success
    */
     bool stopTimer( uint32_t idTimer );
     
     /*!
        Pause an existing timer
        @return true on success
    */
     bool pauseTimer( uint32_t idTimer );
     
     /*!
        Resume an existing timer
        @return true on success
    */
     bool resumeTimer( uint32_t idTimer );
     
     
     /*!
      *     Create DM table
      *     @return true on success.
      */
     bool doCreateDMTable( void );
     
     
    /*!
     *     Create the in memory DM table
     *     @return true on success.
     */
     bool doCreateInMemoryDMTable( void );
     
    /*!
     *     Fill/Update the in memory DM table
     *     @return true on success.
    */
    bool doFillMemoryDMTable();

  //------------------------------------

    /// Random minute
    uint8_t m_rndMinute;

    /// Random hour
    uint8_t m_rndHour;

    /// Random day
    uint8_t m_rndDay;

    /// Random week
    uint8_t m_rndWeek;

    /// Random month
    uint8_t m_rndMonth;

    /// Random year
    uint8_t m_rndYear;

    /// last update
    wxDateTime m_lastTime;

    /// Path to DM XML file
    wxString m_staticXMLPath;

    /// Allow editing of XML content
    bool bAllowXMLsave;
    
    // Path to the VSCP DM database
    wxFileName m_path_db_vscp_dm;  
    
    // Databases
    sqlite3 *m_db_vscp_dm;                  // External DM database

    /// DM table filter - Filter for all rows of the table
    vscpEventFilter m_DM_Table_filter;

    /*!
        Client item pointer
    */
    CClientItem *m_pClientItem;

    /*!
        Mutex that control DM list access
    */
     wxMutex m_mutexDM;

    /*!
        List with timer objects
    */
    DMTIMERS m_timerHash;


    /*!
        List with Decision Matrix elements
    */
    DMLIST m_DMList;

};


////////////////////////////////////////////////////////////////////////////////
//                            ACTION TREADS
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// actionThread_URL
//

class actionThread_URL : public wxThread
{

public:

    static enum accessmethod_t {
        GET = 0,
        PUT,
        POST,
        OPTIONS,
        DEL,
        PATCH
    } accessmethod;

    /*!
        Constructor
        @param pCtrlObject Pointer to main object
        @param url URL to access
        @param nAccessMethod 0 for GET, 1 for PUT and 2 for POST
        @param putdata Data if PUT is used as accessmethod.
        @param extraheaders Optional extra headers
        @param proxy Optional proxy to use on the form <hostname>:<port number>
        @param kind Threadtype.
    */
    actionThread_URL( CControlObject *pCtrlObject, 
                        wxString& url,
                        accessmethod_t nAccessMethod,
                        wxString& putdata,
                        wxString& extraheaders,
                        wxString& proxy,
                        wxThreadKind kind = wxTHREAD_DETACHED );
  
    /// Destructor
    ~actionThread_URL();

    virtual void *Entry();

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();

private:

    /*!
        URL to access  
    */
    wxURL m_url;
  
    /*!
        Accessmethod for HTTP call
    */
    accessmethod_t m_acessMethod;

    /*!
        Data for PUT
    */
    wxString m_putdata;
    
    /*!
        Optional Extra headers
     */
    wxString m_extraheaders;
    
    // Is set to true if initialization went OK
    bool m_bOK;
};


////////////////////////////////////////////////////////////////////////////////
// actionThread_VSCPSrv
//

class actionThread_VSCPSrv : public wxThread
{

public:

    /*!
        Constructor
        @param pCtrlObject Pointer to main object
        @param strHostname Host to connect to.
        @param port TCP/IP port to use.
        @param strUsername Username.
        @param strUsername Username.
        @param strEvent Event to send
        @param kind Threadtype.
    */
    actionThread_VSCPSrv( CControlObject *pCtrlObject, 
                            wxString& strHostname, 
                            short port, 
                            wxString& strUsername, 
                            wxString& strPassword,
                            wxString& strEvent,
                            wxThreadKind kind = wxTHREAD_DETACHED );
  
    /// Destructor
    ~actionThread_VSCPSrv();

    virtual void *Entry();

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();

private:

    /*!
        Hostname  
    */
    wxString m_strHostname;

    /*!
        Port to access server at
    */
    int m_port;

    /*!
        Username to login with  
    */
    wxString m_strUsername;

    /*!
        Password to login with  
    */
    wxString m_strPassword;

    /*!
        The event to send  
    */
    vscpEventEx m_eventEx;

};





////////////////////////////////////////////////////////////////////////////////
// actionThread_Table
//

class actionThread_Table : public wxThread
{

public:

    /*!
        Constructor
        @param pCtrlObject Pointer to main object.
        @param pTableObj Table object.
        @param kind Threadtype.
    */
    actionThread_Table( wxString &strParam,
                            vscpEvent *pEvent,
                            wxThreadKind kind = wxTHREAD_DETACHED );
  
    /// Destructor
    ~actionThread_Table();

    virtual void *Entry();

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();
    
    /// Escaped DM action parameter
    wxString m_strParam;
    
    /// Feed event
    vscpEvent *m_pFeedEvent;
            
};


#endif // include