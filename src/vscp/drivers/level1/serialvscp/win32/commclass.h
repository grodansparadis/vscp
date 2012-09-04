WaitForString() enables you to wait for a string.

Added Also a wait for string option to WriteToPort().


- The Wait does not block the receiving thread.

- Windows Messages are processed while waiting.

Added some improvements found in the previous comments.

To end a program during a WaitForString(), use PostQuitMessage(0). Add PostQuitMessage(0) in OnCancel() if you use a main frame based on CDialog.


/*
** FILENAME CSerialPort.cpp
**
** PURPOSE This class can read, write and watch one serial port.
** It sends messages to its owner when something happends on the port
** The class creates a thread for reading and writing so the main
** program is not blocked.
**
** CREATION DATE 15-09-1997
** LAST MODIFICATION 12-11-1997
**
** AUTHOR Remon Spekreijse
**
**
*/

#include "stdafx.h"
#include "SerialPort.h"
#include <time.h>
#include <sys/timeb.h>

#include <assert.h>

//
// Constructor
//
CSerialPort::CSerialPort()
{
m_hComm = NULL;

// initialize overlapped structure members to zero
m_ov.Offset = 0;
m_ov.OffsetHigh = 0;

// create events
m_ov.hEvent = NULL;
m_hWriteEvent = NULL;
m_hShutdownEvent = NULL;
m_hReceivedRequestedString = NULL;

m_szWriteBuffer = NULL;

m_bThreadAlive = FALSE;

}

//
// Delete dynamic memory
//
CSerialPort::~CSerialPort()
{
do {
SetEvent(m_hShutdownEvent);
}
while (m_bThreadAlive);

TRACE("Thread ended\n");

// close handles to avoid memory leaks
CloseHandle(m_hReceivedRequestedString);

CloseHandle(m_ov.hEvent);
CloseHandle(m_hWriteEvent);
CloseHandle(m_hComm);
CloseHandle(m_hShutdownEvent);

m_hComm = NULL;
m_ov.hEvent = NULL;
m_hWriteEvent = NULL;
m_hShutdownEvent = NULL;
m_hReceivedRequestedString = NULL;

if (m_szWriteBuffer != NULL)
delete [] m_szWriteBuffer;

//DeleteCriticalSection(&m_csCommunicationSync);
}

//
// Initialize the port. This can be port 1 to 4.
//
BOOL CSerialPort::InitPort(CWnd* pPortOwner, // the owner (CWnd) of the port (receives message)
UINT writebuffersize, // size to the writebuffer
UINT portnr, // portnumber (1..4)
UINT baud, // baudrate
char parity, // parity
UINT databits, // databits
UINT stopbits, // stopbits
DWORD dwCommEvents) // EV_RXCHAR, EV_CTS etc
{
assert(portnr > 0 && portnr < 5);
assert(pPortOwner != NULL);

// if the thread is alive: Kill
if (m_bThreadAlive)
{
do
{
SetEvent(m_hShutdownEvent);
} while (m_bThreadAlive);
TRACE("Thread ended\n");
}

// See WaitForString()
m_DoCollectChars = FALSE;
m_StringToWaitFor.Empty();
m_StringToWaitForLength = 0;

// create events
if (m_ov.hEvent != NULL)
ResetEvent(m_ov.hEvent);
m_ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

if (m_hWriteEvent != NULL)
ResetEvent(m_hWriteEvent);
m_hWriteEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

if (m_hShutdownEvent != NULL)
ResetEvent(m_hShutdownEvent);
m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

if (m_hReceivedRequestedString != NULL)
ResetEvent(m_hReceivedRequestedString);
m_hReceivedRequestedString = CreateEvent(NULL, TRUE, FALSE, NULL);

// initialize the event objects
m_hEventArray[0] = m_hShutdownEvent; // highest priority
m_hEventArray[1] = m_ov.hEvent;
m_hEventArray[2] = m_hWriteEvent;

// initialize critical section
InitializeCriticalSection(&m_csCommunicationSync);

// set buffersize for writing and save the owner
m_pOwner = pPortOwner;

if (m_szWriteBuffer != NULL)
delete [] m_szWriteBuffer;
m_szWriteBuffer = new char[writebuffersize];

m_nPortNr = portnr;

m_nWriteBufferSize = writebuffersize;
m_dwCommEvents = dwCommEvents;

BOOL bResult = FALSE;
char *szPort = new char[50];
char *szBaud = new char[50];

// now it critical!
EnterCriticalSection(&m_csCommunicationSync);

// if the port is already opened: close it
if (m_hComm != NULL)
{
CloseHandle(m_hComm);
m_hComm = NULL;
}

// prepare port strings
sprintf(szPort, "COM%d", portnr);
sprintf(szBaud, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopbits);

// get a handle to the port
m_hComm = CreateFile(szPort, // communication port string (COMX)
GENERIC_READ | GENERIC_WRITE, // read/write types
0, // comm devices must be opened with exclusive access
NULL, // no security attributes
OPEN_EXISTING, // comm devices must use OPEN_EXISTING
FILE_FLAG_OVERLAPPED, // Async I/O
0); // template must be 0 for comm devices

if (m_hComm == INVALID_HANDLE_VALUE)
{
// port not found
delete [] szPort;
delete [] szBaud;

return FALSE;
}

// set the timeout values
m_CommTimeouts.ReadIntervalTimeout = 1000;
m_CommTimeouts.ReadTotalTimeoutMultiplier = 1000;
m_CommTimeouts.ReadTotalTimeoutConstant = 1000;
m_CommTimeouts.WriteTotalTimeoutMultiplier = 1000;
m_CommTimeouts.WriteTotalTimeoutConstant = 1000;

// configure
if (SetCommTimeouts(m_hComm, &m_CommTimeouts))
{
if (SetCommMask(m_hComm, dwCommEvents))
{
if (GetCommState(m_hComm, &m_dcb))
{
m_dcb.fRtsControl = RTS_CONTROL_ENABLE; // set RTS bit high!
m_dcb.fInX = TRUE; // XON/XOFF ON
m_dcb.fOutX = TRUE;
m_dcb.XonChar = 17;
m_dcb.XoffChar = 19;
m_dcb.XonLim = 100;
m_dcb.XoffLim = 100;
if (BuildCommDCB(szBaud, &m_dcb))
{
if (SetCommState(m_hComm, &m_dcb))
; // normal operation... continue
else
ProcessErrorMessage("SetCommState()");
}
else
ProcessErrorMessage("BuildCommDCB()");
}
else
ProcessErrorMessage("GetCommState()");
}
else
ProcessErrorMessage("SetCommMask()");
}
else
ProcessErrorMessage("SetCommTimeouts()");

delete [] szPort;
delete [] szBaud;

// flush the port
PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

// release critical section
LeaveCriticalSection(&m_csCommunicationSync);

TRACE("Initialisation for communicationport %d completed.\nUse Startmonitor to communicate.\n", portnr);

return TRUE;
}

//
// The CommThread Function.
//
UINT CSerialPort::CommThread(LPVOID pParam)
{
// Cast the void pointer passed to the thread back to
// a pointer of CSerialPort class
CSerialPort *port = (CSerialPort*)pParam;

// Set the status variable in the dialog class to
// TRUE to indicate the thread is running.
port->m_bThreadAlive = TRUE;

// Misc. variables
DWORD BytesTransfered = 0;
DWORD Event = 0;
DWORD CommEvent = 0;
DWORD dwError = 0;
COMSTAT comstat;
BOOL bResult = TRUE;

// Clear comm buffers at startup
if (port->m_hComm) // check if the port is opened
PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

// begin forever loop. This loop will run as long as the thread is alive.
for (;;)
{

// Make a call to WaitCommEvent(). This call will return immediatly
// because our port was created as an async port (FILE_FLAG_OVERLAPPED
// and an m_OverlappedStructerlapped structure specified). This call will cause the
// m_OverlappedStructerlapped element m_OverlappedStruct.hEvent, which is part of the m_hEventArray to
// be placed in a non-signeled state if there are no bytes available to be read,
// or to a signeled state if there are bytes available. If this event handle
// is set to the non-signeled state, it will be set to signeled when a
// character arrives at the port.

// we do this for each port!

bResult = WaitCommEvent(port->m_hComm, &Event, &port->m_ov);

if (!bResult)
{
// If WaitCommEvent() returns FALSE, process the last error to determin
// the reason..
switch (dwError = GetLastError())
{
case ERROR_IO_PENDING:
{
// This is a normal return value if there are no bytes
// to read at the port.
// Do nothing and continue
break;
}
case 87:
{
// Under Windows NT, this value is returned for some reason.
// I have not investigated why, but it is also a valid reply
// Also do nothing and continue.
break;
}
default:
{
// All other error codes indicate a serious error has
// occured. Process this error.
port->ProcessErrorMessage("WaitCommEvent()");
break;
}
}
}
else
{
// If WaitCommEvent() returns TRUE, check to be sure there are
// actually bytes in the buffer to read.
//
// If you are reading more than one byte at a time from the buffer
// (which this program does not do) you will have the situation occur
// where the first byte to arrive will cause the WaitForMultipleObjects()
// function to stop waiting. The WaitForMultipleObjects() function
// resets the event handle in m_OverlappedStruct.hEvent to the non-signelead state
// as it returns.
//
// If in the time between the reset of this event and the call to
// ReadFile() more bytes arrive, the m_OverlappedStruct.hEvent handle will be set again
// to the signeled state. When the call to ReadFile() occurs, it will
// read all of the bytes from the buffer, and the program will
// loop back around to WaitCommEvent().
//
// At this point you will be in the situation where m_OverlappedStruct.hEvent is set,
// but there are no bytes available to read. If you proceed and call
// ReadFile(), it will return immediatly due to the async port setup, but
// GetOverlappedResults() will not return until the next character arrives.
//
// It is not desirable for the GetOverlappedResults() function to be in
// this state. The thread shutdown event (event 0) and the WriteFile()
// event (Event2) will not work if the thread is blocked by GetOverlappedResults().
//
// The solution to this is to check the buffer with a call to ClearCommError().
// This call will reset the event handle, and if there are no bytes to read
// we can loop back through WaitCommEvent() again, then proceed.
// If there are really bytes to read, do nothing and proceed.

bResult = ClearCommError(port->m_hComm, &dwError, &comstat);

if (comstat.cbInQue == 0)
continue;
} // end if bResult

// Main wait function. This function will normally block the thread
// until one of nine events occur that require action.

Event = WaitForMultipleObjects(3, port->m_hEventArray, FALSE, INFINITE);

switch (Event)
{
case 0:
{
// Shutdown event. This is event zero so it will be
// the higest priority and be serviced first.

port->m_bThreadAlive = FALSE;

// Kill this thread. break is not needed, but makes me feel better.
AfxEndThread(100);
break;
}
case 1: // read event
{
GetCommMask(port->m_hComm, &CommEvent);
if (CommEvent & EV_CTS)
::PostMessage(port->m_pOwner->m_hWnd, WM_COMM_CTS_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);
if (CommEvent & EV_RXFLAG)
::PostMessage(port->m_pOwner->m_hWnd, WM_COMM_RXFLAG_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);
if (CommEvent & EV_BREAK)
::PostMessage(port->m_pOwner->m_hWnd, WM_COMM_BREAK_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);
if (CommEvent & EV_ERR)
::PostMessage(port->m_pOwner->m_hWnd, WM_COMM_ERR_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);
if (CommEvent & EV_RING)
::PostMessage(port->m_pOwner->m_hWnd, WM_COMM_RING_DETECTED, (WPARAM) 0, (LPARAM) port->m_nPortNr);

if (CommEvent & EV_RXCHAR)
// Receive character event from port.
ReceiveChar(port, comstat);

break;
}
case 2: // write event
{
// Write character event from port
WriteChar(port);
break;
}

} // end switch

} // close forever loop

return 0;
}

//
// start comm watching
//
BOOL CSerialPort::StartMonitoring()
{
if (!(m_Thread = AfxBeginThread(CommThread, this)))
return FALSE;
TRACE("Thread started\n");
return TRUE;
}

//
// Restart the comm thread
//
BOOL CSerialPort::RestartMonitoring()
{
TRACE("Thread resumed\n");
m_Thread->ResumeThread();
return TRUE;
}


//
// Suspend the comm thread
//
BOOL CSerialPort::StopMonitoring()
{
TRACE("Thread suspended\n");
m_Thread->SuspendThread();
return TRUE;
}


//
// If there is a error, give the right message
//
void CSerialPort::ProcessErrorMessage(char* ErrorText)
{
char *Temp = new char[200];

LPVOID lpMsgBuf;

FormatMessage(
FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
NULL,
GetLastError(),
MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
(LPTSTR) &lpMsgBuf,
0,
NULL
);

sprintf(Temp, "WARNING: %s Failed with the following error: \n%s\nPort: %d\n", (char*)ErrorText, lpMsgBuf, m_nPortNr);
MessageBox(NULL, Temp, "Application Error", MB_ICONSTOP);

LocalFree(lpMsgBuf);
delete[] Temp;
}

//
// Write a character.
//
void CSerialPort::WriteChar(CSerialPort* port)
{
BOOL bWrite = TRUE;
BOOL bResult = TRUE;

DWORD BytesSent = 0;
#ifdef _DEBUG
TRACE("WriteChar\n");
#endif

ResetEvent(port->m_hWriteEvent);

// Gain ownership of the critical section
EnterCriticalSection(&port->m_csCommunicationSync);

if (bWrite)
{
// Initailize variables
port->m_ov.Offset = 0;
port->m_ov.OffsetHigh = 0;

// Clear buffer
PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

if(port->m_bByteData) {
bResult = WriteFile(port->m_hComm,
port->m_szWriteBuffer,
port->m_nByteCount,
&BytesSent,
&port->m_ov);
}
else {
bResult = WriteFile(port->m_hComm,
port->m_szWriteBuffer,
strlen((char*)port->m_szWriteBuffer),
&BytesSent,
&port->m_ov);
}

// deal with any error codes
if (!bResult)
{
DWORD dwError = GetLastError();
switch (dwError)
{
case ERROR_IO_PENDING:
{
// continue to GetOverlappedResults()
BytesSent = 0;
bWrite = FALSE;
break;
}
default:
{
// all other error codes
port->ProcessErrorMessage("WriteFile()");
}
}
}
else
{
LeaveCriticalSection(&port->m_csCommunicationSync);
#ifdef _DEBUG
TRACE("%c", port->m_szWriteBuffer[0]);
#endif

}
} // end if(bWrite)

if (!bWrite)
{
bWrite = TRUE;

bResult = GetOverlappedResult(port->m_hComm, // Handle to COMM port
&port->m_ov, // Overlapped structure
&BytesSent, // Stores number of bytes sent
TRUE); // Wait flag

LeaveCriticalSection(&port->m_csCommunicationSync);

// deal with the error code
if (!bResult)
{
port->ProcessErrorMessage("GetOverlappedResults() in WriteFile()");
}
} // end if (!bWrite)

// Verify that the data size send equals what we tried to send
if (BytesSent != (DWORD) port->m_nByteCount)
{
TRACE("WARNING: WriteFile() error.. Bytes Sent: %d; Message Length: %d\n", BytesSent, port->m_nByteCount);
}
#ifdef _DEBUG
TRACE("%d cars ecrits...\n", BytesSent);
#endif
}

//
// Character received. Inform the owner
//
void CSerialPort::ReceiveChar(CSerialPort* port, COMSTAT comstat)
{
BOOL bRead = TRUE;
BOOL bResult = TRUE;
DWORD dwError = 0;
DWORD BytesRead = 0;
DWORD BytesToRead = 1; //modified
unsigned char RXBuff[MAXBYTESTOREAD];//modified
DWORD i;
int j;
static int indexStringRead = 0;

memset(RXBuff, 0, sizeof(RXBuff));

for (;;)
{
// Gain ownership of the comm port critical section.
// This process guarantees no other part of this program
// is using the port object.
EnterCriticalSection(&port->m_csCommunicationSync);

// ClearCommError() will update the COMSTAT structure and
// clear any other errors.

bResult = ClearCommError(port->m_hComm, &dwError, &comstat);

LeaveCriticalSection(&port->m_csCommunicationSync);

// start forever loop. I use this type of loop because I
// do not know at runtime how many loops this will have to
// run. My solution is to start a forever loop and to
// break out of it when I have processed all of the
// data available. Be careful with this approach and
// be sure your loop will exit.
// My reasons for this are not as clear in this sample
// as it is in my production code, but I have found this
// solutiion to be the most efficient way to do this.

if (comstat.cbInQue == 0)
{
// break out when all bytes have been read
break;
}

EnterCriticalSection(&port->m_csCommunicationSync);

if (bRead)
{
bResult = ReadFile(port->m_hComm, // Handle to COMM port
RXBuff, // RX Buffer Pointer
BytesToRead, // byte to read depend on que
&BytesRead, // Stores number of bytes read
&port->m_ov); // pointer to the m_ov structure
// deal with the error code
if (!bResult)
{
switch (dwError = GetLastError())
{
case ERROR_IO_PENDING:
{
// asynchronous i/o is still in progress
// Proceed on to GetOverlappedResults();
bRead = FALSE;
break;
}
default:
{
// Another error has occured. Process this error.
port->ProcessErrorMessage("ReadFile()");
break;
}
}
}
else
{
// ReadFile() returned complete. It is not necessary to call GetOverlappedResults()
bRead = TRUE;
}
} // close if (bRead)

if (!bRead)
{
bRead = TRUE;
bResult = GetOverlappedResult(port->m_hComm, // Handle to COMM port
&port->m_ov, // Overlapped structure
&BytesRead, // Stores number of bytes read
TRUE); // Wait flag

// deal with the error code
if (!bResult)
{
port->ProcessErrorMessage("GetOverlappedResults() in ReadFile()");
}
} // close if (!bRead)

LeaveCriticalSection(&port->m_csCommunicationSync);

for (i = 0; i < BytesRead; i++) {
// If requested, looks for a given string
if (port->m_DoCollectChars){
// The Receive buffer is not full yet
if (indexStringRead < port->m_StringToWaitForLength){
port->m_StringRead [indexStringRead] = RXBuff[i];
++indexStringRead;
}
// The Receive buffer is full
if (indexStringRead >= port->m_StringToWaitForLength){
if (indexStringRead > port->m_StringToWaitForLength) {
// Decalage vers la gauche.
for (j = 0; j < port->m_StringToWaitForLength; j++)
port->m_StringRead[j] = port->m_StringRead[j +1];
port->m_StringRead[--j] = RXBuff[i];
}
else
++indexStringRead;
if (!memcmp(port->m_StringRead, port->m_StringToWaitFor, port->m_StringToWaitForLength)){
// Notify parent (waiting for it in WriteToPort()) that the string was found
SetEvent(port->m_hReceivedRequestedString);
indexStringRead = 0;
}
}
}
// notify parent that a byte was received
::PostMessage((port->m_pOwner)->m_hWnd, WM_COMM_RXCHAR, (WPARAM) RXBuff[i], (LPARAM) port->m_nPortNr);
}
/*#ifdef _DEBUG
TRACE("%c", RXBuff);
#endif*/
} // end forever loop
}

//
// Write a string to the port
//
BOOL CSerialPort::WriteToPort(CString string, BOOL waitString, const CString &string_to_wait_for, DWORD seconds_to_wait)
{
int size = string.GetLength();
return WriteToPort(string.GetBuffer(size), size, waitString, string_to_wait_for, seconds_to_wait);
}

BOOL CSerialPort::WriteToPort(char* string, int nCount, BOOL waitString, const CString &string_to_wait_for, DWORD seconds_to_wait)
{
DWORD Event;

assert(m_hComm != 0);

memset(m_szWriteBuffer, 0, sizeof(m_szWriteBuffer));

if (nCount == -1) {
m_bByteData = FALSE;
strcpy(m_szWriteBuffer, string);
m_nByteCount = strlen(string);
}
else {
m_bByteData = TRUE;
m_nByteCount = nCount;
memcpy(m_szWriteBuffer, string, nCount);
}

if (waitString){
m_DoCollectChars = TRUE; // Used in ReceiveChar()
m_StringToWaitFor = string_to_wait_for;
m_StringToWaitForLength = string_to_wait_for.GetLength();
if (m_StringToWaitForLength > MAXBYTESTOCOMPARE)
return FALSE;
}
// set event for write
SetEvent(m_hWriteEvent);

if (waitString){
BOOL bIdle = TRUE;
LONG lIdleCount = 0;
ResetEvent(m_hReceivedRequestedString);
struct _timeb Start_time;
struct _timeb Current_time;
_ftime(&Start_time);

LONG ms_remaining = seconds_to_wait*1000;

for (;;){
Event = MsgWaitForMultipleObjects(1, &m_hReceivedRequestedString, FALSE, (DWORD)ms_remaining, QS_ALLEVENTS);
switch (Event)
{
case WAIT_OBJECT_0:
#ifdef _DEBUG
TRACE("Dans WriteToPort : Chaine Trouvee!!\n");
#endif
ResetEvent(m_hReceivedRequestedString);
m_DoCollectChars = FALSE;
return TRUE;
// Dispatch Messages sent by ReceiveChar()
case WAIT_OBJECT_0 + 1:
MSG msg;
#ifdef _DEBUG
TRACE("Dans WriteToPort : Message a traiter...\n");
#endif
// This to avoid a deadLock with the ReceiveChar() thread, and also to let the app receive messages
// See MFC Source file "Thrdcore.cpp" and OnIdle in MSDN
while (bIdle && !PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE)) {
// call OnIdle while in bIdle state
if (!AfxGetApp()->OnIdle(lIdleCount++))
bIdle = FALSE; // assume "no idle" state
}
// phase2: pump messages while available
do {
// pump message, but quit on WM_QUIT
if (!AfxGetApp()->PumpMessage()){
//AfxGetApp()->ExitInstance();
// We are not in Run(), so let's repost the WM_QUIT message.
PostQuitMessage(0);
return FALSE;
}

// reset "no idle" state after pumping "normal" message
if (AfxGetApp()->IsIdleMessage(&msg)) {
bIdle = TRUE;
lIdleCount = 0;
}
}
while (PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE));
_ftime(&Current_time);
ms_remaining -= ((Current_time.time*1000 +Current_time.millitm) - (Start_time.time *1000 + Start_time.millitm));

if (ms_remaining > 0){
#ifdef _DEBUG
TRACE ("Reste: %d ms\n", ms_remaining);
#endif
_ftime(&Start_time);
break;
}
#ifdef _DEBUG
TRACE ("Pas de break\n");
#endif
case WAIT_TIMEOUT:
#ifdef _DEBUG
m_StringRead[m_StringToWaitForLength -1] = 0;
TRACE("Dans WriteToPort : Timeout!!(%ds) lu=->%s<-\n", seconds_to_wait, m_StringRead);
#endif
m_DoCollectChars = FALSE;
return FALSE;
default:
m_DoCollectChars = FALSE;
return FALSE;
}
}
}
return TRUE;
}

//
// Return the device control block
//
DCB CSerialPort::GetDCB()
{
return m_dcb;
}

//
// Return the communication event masks
//
DWORD CSerialPort::GetCommEvents()
{
return m_dwCommEvents;
}

//
// Return the output buffer size
//
DWORD CSerialPort::GetWriteBufferSize()
{
return m_nWriteBufferSize;
}

BOOL CSerialPort::WaitForString(const CString &string_to_wait_for, DWORD seconds_to_wait)
{
DWORD Event;

m_DoCollectChars = TRUE; // Used in ReceiveChar()
m_StringToWaitFor = string_to_wait_for;
m_StringToWaitForLength = string_to_wait_for.GetLength();
if (m_StringToWaitForLength > MAXBYTESTOCOMPARE)
return FALSE;

BOOL bIdle = TRUE;
LONG lIdleCount = 0;
ResetEvent(m_hReceivedRequestedString);
struct _timeb Start_time;
struct _timeb Current_time;
_ftime(&Start_time);
LONG ms_remaining = seconds_to_wait*1000;

for (;;){
Event = MsgWaitForMultipleObjects(1, &m_hReceivedRequestedString, FALSE, (DWORD)ms_remaining, QS_ALLEVENTS);
switch (Event)
{
case WAIT_OBJECT_0:
#ifdef _DEBUG
TRACE("Dans WaitForString : Chaine Trouvee!!\n");
#endif
ResetEvent(m_hReceivedRequestedString);
m_DoCollectChars = FALSE;
return TRUE;
// Dispatch Messages sent by ReceiveChar()
case WAIT_OBJECT_0 + 1:
MSG msg;
// This to avoid a deadLock with the ReceiveChar() thread, and also to let the app receive messages
// See MFC Source file "Thrdcore.cpp" and OnIdle in MSDN
while (bIdle && !PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE)) {
// call OnIdle while in bIdle state
if (!AfxGetApp()->OnIdle(lIdleCount++))
bIdle = FALSE; // assume "no idle" state
}

// phase2: pump messages while available
do {
// pump message, but quit on WM_QUIT
if (!AfxGetApp()->PumpMessage()){
//AfxGetApp()->ExitInstance();
// We are not in Run(), so let's repost the WM_QUIT message.
PostQuitMessage(0);
return FALSE;
}
// reset "no idle" state after pumping "normal" message
if (AfxGetApp()->IsIdleMessage(&msg)) {
bIdle = TRUE;
lIdleCount = 0;
}
}
while (PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE));
_ftime(&Current_time);
ms_remaining -= ((Current_time.time*1000 +Current_time.millitm) - (Start_time.time *1000 + Start_time.millitm));

if (ms_remaining > 0){
#ifdef _DEBUG
TRACE ("Reste: %d ms\n", ms_remaining);
#endif
_ftime(&Start_time);
break;
}
#ifdef _DEBUG
TRACE ("Pas de break\n");
#endif
case WAIT_TIMEOUT:
#ifdef _DEBUG
m_StringRead[m_StringToWaitForLength -1] = 0;
TRACE("Dans WaitForString : Timeout!! lu=->%s<-\n", m_StringRead);
#endif
m_DoCollectChars = FALSE;
return FALSE;
default:
m_DoCollectChars = FALSE;
return FALSE;
}
}
return TRUE;
}
/*
** FILENAME CSerialPort.h
**
** PURPOSE This class can read, write and watch one serial port.
** It sends messages to its owner when something happends on the port
** The class creates a thread for reading and writing so the main
** program is not blocked.
**
** CREATION DATE 15-09-1997
** LAST MODIFICATION 12-11-1997
**
** AUTHOR Remon Spekreijse
**
**
*/

#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__

#define WM_COMM_BREAK_DETECTED WM_USER+1 // A break was detected on input.
#define WM_COMM_CTS_DETECTED WM_USER+2 // The CTS (clear-to-send) signal changed state.
#define WM_COMM_DSR_DETECTED WM_USER+3 // The DSR (data-set-ready) signal changed state.
#define WM_COMM_ERR_DETECTED WM_USER+4 // A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY.
#define WM_COMM_RING_DETECTED WM_USER+5 // A ring indicator was detected.
#define WM_COMM_RLSD_DETECTED WM_USER+6 // The RLSD (receive-line-signal-detect) signal changed state.
#define WM_COMM_RXCHAR WM_USER+7 // A character was received and placed in the input buffer.
#define WM_COMM_RXFLAG_DETECTED WM_USER+8 // The event character was received and placed in the input buffer.
#define WM_COMM_TXEMPTY_DETECTED WM_USER+9 // The last character in the output buffer was sent.
#define MAXBYTESTOREAD 512
#define MAXBYTESTOCOMPARE 512

class CSerialPort
{
public:
// contruction and destruction
CSerialPort();
virtual ~CSerialPort();

// port initialisation
BOOL InitPort(CWnd* pPortOwner, UINT nBufferSize = 4096, UINT portnr = 1, UINT baud = 9600, char parity = 'N', UINT databits = 8, UINT stopsbits = 1, DWORD dwCommEvents = EV_RXCHAR);
BOOL CSerialPort::WaitForString(const CString &string_to_wait_for, DWORD seconds_to_wait = 1);
// start/stop comm watching
BOOL StartMonitoring();
BOOL RestartMonitoring();
BOOL StopMonitoring();

DWORD GetWriteBufferSize();
DWORD GetCommEvents();
DCB GetDCB();

BOOL WriteToPort(char* string, int nCount = -1, BOOL waitString = FALSE, const CString &string_to_wait_for = (char*)0, DWORD seconds_to_wait = 0);
BOOL WriteToPort(CString string, BOOL waitString = FALSE, const CString &string_to_wait_for = (char*)0, DWORD seconds_to_wait = 0);

protected:
// protected memberfunctions
void ProcessErrorMessage(char* ErrorText);
static UINT CommThread(LPVOID pParam);
static void ReceiveChar(CSerialPort* port, COMSTAT comstat);
static void WriteChar(CSerialPort* port);

// thread
CWinThread* m_Thread;

// synchronisation objects
CRITICAL_SECTION m_csCommunicationSync;
BOOL m_bThreadAlive;

// handles
HANDLE m_hShutdownEvent;
HANDLE m_hComm;
HANDLE m_hWriteEvent;
HANDLE m_hReceivedRequestedString;
// Event array.
// One element is used for each event. There are two event handles for each port.
// A Write event and a receive character event which is located in the overlapped structure (m_ov.hEvent).
// There is a general shutdown when the port is closed.
HANDLE m_hEventArray[3];

// structures
OVERLAPPED m_ov;
COMMTIMEOUTS m_CommTimeouts;
DCB m_dcb;

// owner window
CWnd* m_pOwner;

// misc
UINT m_nPortNr;
char* m_szWriteBuffer;
DWORD m_dwCommEvents;
DWORD m_nWriteBufferSize;

BOOL m_bByteData;
int m_nByteCount;

unsigned char m_StringRead[MAXBYTESTOCOMPARE];
CString m_StringToWaitFor;
int m_StringToWaitForLength;
BOOL m_DoCollectChars;
};

#endif __SERIALPORT_H__ 