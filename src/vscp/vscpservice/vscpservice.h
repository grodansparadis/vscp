// myservice.h

#include "../../common/ntservice.h"
#include "../common/controlobject.h"


/*!
	Windows NT Service 
*/

class CVSCPService : public CNTService
{

public:

	/// Constructor
	CVSCPService();
    
	/*! 
		Main loop for service
	*/
	virtual void Run( void );

	/*! 
		Stop service
	*/
	virtual void OnStop( void );
	
	/*! 
		Initialize communication

		@return TRUE if initialisation went well.
	*/
	virtual BOOL OnInit( void );

	/*! 
		Interrogate service
	*/
	virtual void OnInterrogate( void );

	/*! 
		Pause service
	*/
	virtual void OnPause( void );

	/*! 
		Continue service
	*/
	virtual void OnContinue( void );

	/*! 
		Shutdown service
	*/
	virtual void OnShutdown( void );


	/*! 
		Process user control requests

		@return TRUE if opcode handled.
	*/
	virtual BOOL OnUserControl( DWORD dwOpcode );

	/*!
		Save the current status in the registry	
	*/
  void SaveStatus( void );


protected:

	/// Start control parameter
	int m_iStartParam;

	/// Current state of service
	int m_iState;

	/// The object encapsulating control functionality
	CControlObject m_ctrlObj;
};
