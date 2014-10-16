//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: CNTS_MSG_SERVICE_STARTED
//
// MessageText:
//
// "%1" started successfully.
//
#define CNTS_MSG_SERVICE_STARTED         0x40000001L

//
// MessageId: CNTS_MSG_SERVICE_INSTALLED
//
// MessageText:
//
// "%1" was successfully installed.
//
#define CNTS_MSG_SERVICE_INSTALLED       0x40000002L

//
// MessageId: CNTS_MSG_SERVICE_UNINSTALLED
//
// MessageText:
//
// "%1" was successfully uninstalled.
//
#define CNTS_MSG_SERVICE_UNINSTALLED     0x40000003L

//
// MessageId: CNTS_MSG_SERVICE_STOPPED
//
// MessageText:
//
// "%1" stopped successfully.
//
#define CNTS_MSG_SERVICE_STOPPED         0x40000004L

//
// MessageId: CNTS_MSG_SERVICE_PAUSED
//
// MessageText:
//
// "%1" paused successfully.
//
#define CNTS_MSG_SERVICE_PAUSED          0x40000005L

//
// MessageId: CNTS_MSG_SERVICE_CONTINUED
//
// MessageText:
//
// "%1" continued successfully.
//
#define CNTS_MSG_SERVICE_CONTINUED       0x40000006L

//
// MessageId: CNTS_MSG_SERVICE_FAIL_CONNECT_SCM
//
// MessageText:
//
// Failed to connect to the Service Control Manager, ErrorCode:%1.
//
#define CNTS_MSG_SERVICE_FAIL_CONNECT_SCM 0xC0000007L

//
// MessageId: CNTS_MSG_SERVICE_FAIL_OPEN_SERVICE
//
// MessageText:
//
// Failed to open the service, ErrorCode:%1.
//
#define CNTS_MSG_SERVICE_FAIL_OPEN_SERVICE 0xC0000008L

//
// MessageId: CNTS_MSG_SERVICE_FAIL_DELETE_SERVICE
//
// MessageText:
//
// Failed to delete the service, ErrorCode:%1.
//
#define CNTS_MSG_SERVICE_FAIL_DELETE_SERVICE 0xC0000009L

//
// MessageId: MSG_SERVICE_SET_FREQUENCY
//
// MessageText:
//
// Beep Interval has been set to %1 milliseconds.
//
#define MSG_SERVICE_SET_FREQUENCY        0x4000000AL

//
// MessageId: CNTS_MSG_SERVICE_SHUTDOWN
//
// MessageText:
//
// "%1" was successfully shutdown.
//
#define CNTS_MSG_SERVICE_SHUTDOWN        0x4000000BL

