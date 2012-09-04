/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    private.h

Abstract:

    Contains structure definitions and function prototypes private to
    the driver.

Environment:

    Kernel mode

--*/

#pragma warning(disable:4200)  //
#pragma warning(disable:4201)  // nameless struct/union
#pragma warning(disable:4214)  // bit field types other than int

#include <initguid.h>
#include <ntddk.h>
#include <ntintsafe.h>
#include "usbdi.h"
#include "usbdlib.h"

#pragma warning(default:4200)
#pragma warning(default:4201)
#pragma warning(default:4214)

#include <wdf.h>
#include <wdfusb.h>
#include "public.h"


#ifndef _H
#define _H

#define POOL_TAG (ULONG) 'SBSU'

#undef ExAllocatePool
#define ExAllocatePool(type, size) \
    ExAllocatePoolWithTag(type, size, POOL_TAG);

//#if DBG

//#define UsbSamp_DbgPrint(level, _x_) \
            //if((level) <= DebugLevel) { \
            //    DbgPrint("UsbSamp: "); DbgPrint _x_; \
           //}
#define UsbSamp_DbgPrint(level, _x_) DbgPrint("UsbSamp: "); DbgPrint _x_;

//#else

//#define UsbSamp_DbgPrint(level, _x_)

//#endif


#define MAX_TRANSFER_SIZE   256
#define TEST_BOARD_TRANSFER_BUFFER_SIZE (64 *1024 )
#define REMOTE_WAKEUP_MASK 0x20

#define GetListHeadEntry(ListHead)  ((ListHead)->Flink)

//
// A structure representing the instance information associated with
// this particular device.
//

typedef struct _DEVICE_CONTEXT {

    USB_DEVICE_DESCRIPTOR           UsbDeviceDescriptor;

    PUSB_CONFIGURATION_DESCRIPTOR   UsbConfigurationDescriptor;

    WDFUSBDEVICE                    WdfUsbTargetDevice;

    ULONG                           WaitWakeEnable;

    BOOLEAN                         IsDeviceHighSpeed;

    WDFUSBINTERFACE                 UsbInterface;

    UCHAR                           NumberConfiguredPipes;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT,
                                                          GetDeviceContext)
//
// This context is associated with every open handle.
//
typedef struct _FILE_CONTEXT {

    WDFUSBPIPE Pipe;

} FILE_CONTEXT, *PFILE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FILE_CONTEXT,
                                                        GetFileContext)

//
// This context is associated with every request recevied by the driver
// from the app.
//
typedef struct _REQUEST_CONTEXT {

    WDFMEMORY         UrbMemory;
    PMDL              Mdl;
    ULONG             Length;         // remaining to xfer
    ULONG             Numxfer;        // cumulate xfer
    ULONG_PTR         VirtualAddress; // va for next segment of xfer.
    WDFCOLLECTION     SubRequestCollection; // used for doing Isoch
    WDFSPINLOCK     SubRequestCollectionLock; // used to sync access to collection at DISPATCH_LEVEL
    BOOLEAN           Read; // TRUE if Read

} REQUEST_CONTEXT, * PREQUEST_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT            ,
                                                GetRequestContext)

typedef struct _WORKITEM_CONTEXT {
    WDFDEVICE       Device;
    WDFUSBPIPE      Pipe;
} WORKITEM_CONTEXT, *PWORKITEM_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(WORKITEM_CONTEXT,
                                                GetWorkItemContext)

//
// Required for doing ISOCH transfer. This context is associated with every
// subrequest created by the driver to do ISOCH transfer.
//
typedef struct _SUB_REQUEST_CONTEXT {

    WDFREQUEST  UserRequest;
    PURB        SubUrb;
    PMDL        SubMdl;
    LIST_ENTRY  ListEntry; // used in CancelRoutine

} SUB_REQUEST_CONTEXT, *PSUB_REQUEST_CONTEXT ;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(SUB_REQUEST_CONTEXT, GetSubRequestContext)

extern ULONG DebugLevel;

DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD UsbSamp_EvtDeviceAdd;

EVT_WDF_DEVICE_PREPARE_HARDWARE UsbSamp_EvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE UsbSamp_EvtDeviceReleaseHardware;
EVT_WDF_DEVICE_FILE_CREATE UsbSamp_EvtDeviceFileCreate;

EVT_WDF_IO_QUEUE_IO_READ UsbSamp_EvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE UsbSamp_EvtIoWrite;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL UsbSamp_EvtIoDeviceControl;

EVT_WDF_REQUEST_CANCEL UsbSamp_EvtRequestCancel;
EVT_WDF_REQUEST_COMPLETION_ROUTINE SubRequestCompletionRoutine;
EVT_WDF_REQUEST_COMPLETION_ROUTINE ReadWriteCompletion;

WDFUSBPIPE
GetPipeFromName(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUNICODE_STRING FileName
    );

VOID
ReadWriteIsochEndPoints(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN ULONG Length,
    IN WDF_REQUEST_TYPE RequestType
    );

VOID
ReadWriteBulkEndPoints(
    IN WDFQUEUE Queue,
    IN WDFREQUEST Request,
    IN ULONG Length,
    IN WDF_REQUEST_TYPE RequestType
    );

NTSTATUS
ResetPipe(
    IN WDFUSBPIPE             Pipe
    );

NTSTATUS
ResetDevice(
    IN WDFDEVICE Device
    );

VOID
CancelSelectSuspend(
    IN PDEVICE_CONTEXT DeviceContext
    );


NTSTATUS
ReadAndSelectDescriptors(
    IN WDFDEVICE Device
    );

NTSTATUS
ConfigureDevice(
    IN WDFDEVICE Device
    );

NTSTATUS
SelectInterfaces(
    IN WDFDEVICE Device
    );

NTSTATUS
CallUSBD(
    IN PDEVICE_OBJECT DeviceObject,
    IN PURB           Urb
    );

VOID
ProcessQueuedRequests(
    IN OUT PDEVICE_CONTEXT DeviceContext
    );


NTSTATUS
AbortPipes(
    IN WDFDEVICE Device
    );


NTSTATUS
QueuePassiveLevelCallback(
    IN WDFDEVICE    Device,
    IN WDFUSBPIPE   Pipe
    );

VOID
PerformFullSpeedIsochTransfer(
    IN WDFDEVICE        Device,
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN ULONG            TotalLength,
    IN WDF_REQUEST_TYPE RequestType
    );

VOID
PerformHighSpeedIsochTransfer(
    IN WDFDEVICE        Device,
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN ULONG            TotalLength,
    IN WDF_REQUEST_TYPE RequestType
    );

VOID
DbgPrintRWContext(
    PREQUEST_CONTEXT                 rwContext
    );
    
//////////// GS ///////////////    
VOID
UsbSamp_EvtIoStop(
    __in WDFQUEUE         Queue,
    __in WDFREQUEST       Request,
    __in ULONG            ActionFlags
    );    

#endif

