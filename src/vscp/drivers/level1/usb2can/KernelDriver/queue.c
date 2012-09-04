/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    Queue.c

Abstract:

    This file contains dispatch routines for create,
    close, device-control, read & write.

Environment:

    Kernel mode

--*/

#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UsbSamp_EvtDeviceFileCreate)
#pragma alloc_text(PAGE, UsbSamp_EvtIoDeviceControl)
#pragma alloc_text(PAGE, UsbSamp_EvtIoRead)
#pragma alloc_text(PAGE, UsbSamp_EvtIoWrite)
#pragma alloc_text(PAGE, GetPipeFromName)
#pragma alloc_text(PAGE, ResetPipe)
#pragma alloc_text(PAGE, ResetDevice)
#endif


VOID
EvtRequestReadCompletionRoutine(
    __in WDFREQUEST                  Request,
    __in WDFIOTARGET                 Target,
    __in PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
    __in WDFCONTEXT                  Context
    );

VOID
EvtRequestWriteCompletionRoutine(
    __in WDFREQUEST                  Request,
    __in WDFIOTARGET                 Target,
    __in PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
    __in WDFCONTEXT                  Context
    );

VOID
UsbSamp_EvtDeviceFileCreate(
    IN WDFDEVICE            Device,
    IN WDFREQUEST           Request,
    IN WDFFILEOBJECT        FileObject
    )
/*++

Routine Description:

    The framework calls a driver's EvtDeviceFileCreate callback
    when the framework receives an IRP_MJ_CREATE request.
    The system sends this request when a user application opens the
    device to perform an I/O operation, such as reading or writing a file.
    This callback is called synchronously, in the context of the thread
    that created the IRP_MJ_CREATE request.

Arguments:

    Device - Handle to a framework device object.
    FileObject - Pointer to fileobject that represents the open handle.
    CreateParams - copy of the create IO_STACK_LOCATION

Return Value:

   NT status code

--*/
{
    NTSTATUS                    status = STATUS_UNSUCCESSFUL;
    PUNICODE_STRING             fileName;
    PFILE_CONTEXT               pFileContext;
    PDEVICE_CONTEXT             pDevContext;
    WDFUSBPIPE                  pipe;

    UsbSamp_DbgPrint(3, ("EvtDeviceFileCreate - begins\n"));

    PAGED_CODE();

    //
    // initialize variables
    //
    pDevContext = GetDeviceContext(Device);
    pFileContext = GetFileContext(FileObject);


    fileName = WdfFileObjectGetFileName(FileObject);

    if (0 == fileName->Length) {
        //
        // opening a device as opposed to pipe.
        //
        status = STATUS_SUCCESS;
    }
    else {
        pipe = GetPipeFromName(pDevContext, fileName);

        if (pipe != NULL) {
            //
            // found a match
            //
            pFileContext->Pipe = pipe;

            WdfUsbTargetPipeSetNoMaximumPacketSizeCheck(pipe);

            status = STATUS_SUCCESS;
        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    WdfRequestComplete(Request, status);

    UsbSamp_DbgPrint(3, ("EvtDeviceFileCreate - ends\n"));

    return;
}

VOID
UsbSamp_EvtIoDeviceControl(
    IN WDFQUEUE   Queue,
    IN WDFREQUEST Request,
    IN size_t     OutputBufferLength,
    IN size_t     InputBufferLength,
    IN ULONG      IoControlCode
    )
/*++

Routine Description:

    This event is called when the framework receives IRP_MJ_DEVICE_CONTROL
    requests from the system.

Arguments:

    Queue - Handle to the framework queue object that is associated
            with the I/O request.
    Request - Handle to a framework request object.

    OutputBufferLength - length of the request's output buffer,
                        if an output buffer is available.
    InputBufferLength - length of the request's input buffer,
                        if an input buffer is available.

    IoControlCode - the driver-defined or system-defined I/O control code
                    (IOCTL) that is associated with the request.
Return Value:

    VOID

--*/
{
    WDFDEVICE          device;
    PVOID              ioBuffer = NULL;
    size_t             bufLength;
    NTSTATUS           status;
    PDEVICE_CONTEXT    pDevContext;
    PFILE_CONTEXT      pFileContext;
    ULONG              length = 0;
    
    PUSHORT 	       stringBuf;
    WDFMEMORY          memoryHandle;

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    UsbSamp_DbgPrint(3, ("Entered UsbSamp_DispatchDevCtrl\n"));
    //
    // initialize variables
    //
    device = WdfIoQueueGetDevice(Queue);
    pDevContext = GetDeviceContext(device);

    switch(IoControlCode) {

/////////////////////////////////////////////////////////////////////////////////////////////

    case IOCTL_USBSAMP_GET_SERIALNUMBER_STRING: // GS
       
   
       status = WdfUsbTargetDeviceQueryString(
                                           pDevContext->WdfUsbTargetDevice,
                                           NULL,
                                           NULL,
                                           NULL,
                                           (PUSHORT)&length,
                                           pDevContext->UsbDeviceDescriptor.iSerialNumber,
                                           0x0409
                                           );

            if(!NT_SUCCESS(status)){
                UsbSamp_DbgPrint(1, ("WdfUsbTargetDeviceQueryString failed\n"));
                length = 0;
                break;
            }

      status = WdfRequestRetrieveOutputBuffer(
                                           Request,
                                           (size_t)length*2, //GS UNICODE
                                           &ioBuffer,
                                           NULL);
      
            if(!NT_SUCCESS(status)){
                UsbSamp_DbgPrint(1, ("WdfRequestRetrieveInputBuffer failed\n"));
                length = 0;
                break;
            }
      
      status = WdfUsbTargetDeviceQueryString(
                                          pDevContext->WdfUsbTargetDevice,
                                          NULL,
                                          NULL,
                                          ioBuffer,
                                          (PUSHORT)&length,
                                          pDevContext->UsbDeviceDescriptor.iSerialNumber,
                                          0x0409
                                          );    
            if(!NT_SUCCESS(status)){
                UsbSamp_DbgPrint(1, ("WdfUsbTargetDeviceQueryString failed\n"));
                length = 0;
                break;
            }            
            
         length*=2;   
                                                                   
    break; 
  
/////////////////////////////////////////////////////////////////////////////////////////////  

    case IOCTL_USBSAMP_RESET_PIPE: 

        pFileContext = GetFileContext(WdfRequestGetFileObject(Request));

        if (pFileContext->Pipe == NULL) {
            status = STATUS_INVALID_PARAMETER;
        }
        else {
            status = ResetPipe(pFileContext->Pipe);
        }

        break;

    case IOCTL_USBSAMP_GET_CONFIG_DESCRIPTOR:


        if (pDevContext->UsbConfigurationDescriptor) {

            length = pDevContext->UsbConfigurationDescriptor->wTotalLength;

            status = WdfRequestRetrieveOutputBuffer(Request, length, &ioBuffer, &bufLength);
            if(!NT_SUCCESS(status)){
                UsbSamp_DbgPrint(1, ("WdfRequestRetrieveInputBuffer failed\n"));
                break;
            }

            RtlCopyMemory(ioBuffer,
                          pDevContext->UsbConfigurationDescriptor,
                          length);

            status = STATUS_SUCCESS;
        }
        else {
            status = STATUS_INVALID_DEVICE_STATE;
        }

        break;

    case IOCTL_USBSAMP_RESET_DEVICE:

        status = ResetDevice(device);
        break;

    default :
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    WdfRequestCompleteWithInformation(Request, status, length);

    UsbSamp_DbgPrint(3, ("Exit UsbSamp_DispatchDevCtrl\n"));

    return;
}

///////////////////////////// READ //////////////////////////////////

VOID
UsbSamp_EvtIoRead(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t           Length
    )
/*++

Routine Description:

    Called by the framework when it receives Read requests.

Arguments:

    Queue - Default queue handle
    Request - Handle to the read/write request
    Lenght - Length of the data buffer associated with the request.
                 The default property of the queue is to not dispatch
                 zero lenght read & write requests to the driver and
                 complete is with status success. So we will never get
                 a zero length request.

Return Value:


--*/

{
    WDFUSBPIPE                  pipe;
    NTSTATUS                    status;
    WDFMEMORY                   reqMemory;
    //PDEVICE_CONTEXT            pDeviceContext;
    PFILE_CONTEXT          	fileContext = NULL;
    WDF_REQUEST_SEND_OPTIONS    opt;

    UNREFERENCED_PARAMETER(Queue);

    UsbSamp_DbgPrint(3,( "-->OsrFxEvtIoRead\n"));

    //
    // First validate input parameters.
    //
    if (Length > 256) {
        UsbSamp_DbgPrint(1,("Transfer exceeds %d\n",TEST_BOARD_TRANSFER_BUFFER_SIZE));
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    //pDeviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));
    //pipe = pDeviceContext->BulkReadPipe;
    
    fileContext = GetFileContext(WdfRequestGetFileObject(Request));
    pipe = fileContext->Pipe;

    status = WdfRequestRetrieveOutputMemory(Request, &reqMemory);
    if(!NT_SUCCESS(status)){
        UsbSamp_DbgPrint(3,( "WdfRequestRetrieveOutputMemory failed %!STATUS!\n", status));
        goto Exit;
    }

    //
    // The format call validates to make sure that you are reading or
    // writing to the right pipe type, sets the appropriate transfer flags,
    // creates an URB and initializes the request.
    //
    status = WdfUsbTargetPipeFormatRequestForRead(pipe,
                            Request,
                            reqMemory,
                            NULL //Offsets
                            );
    if (!NT_SUCCESS(status)) {
        UsbSamp_DbgPrint(1,("WdfUsbTargetPipeFormatRequestForRead failed 0x%x\n", status));
        goto Exit;
    }

    WdfRequestSetCompletionRoutine(
                            Request,
                            EvtRequestReadCompletionRoutine,
                            pipe);                                                
 
    WDF_REQUEST_SEND_OPTIONS_INIT(
                                 &opt,
                                 WDF_REQUEST_SEND_OPTION_TIMEOUT
                                 );
                              
    WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(
                                        &opt,
                                        WDF_REL_TIMEOUT_IN_MS(50)
                                        //WDF_REL_TIMEOUT_IN_SEC(50)
                                        );                              
                                                        
    //
    // Send the request asynchronously.
    //
    if (WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pipe),&opt /*WDF_NO_SEND_OPTIONS*/) == FALSE) {
        //
        // Framework couldn't send the request for some reason.
        //
        UsbSamp_DbgPrint(3,( "WdfRequestSend failed\n"));
        
        status = WdfRequestGetStatus(Request);
        
        goto Exit;
    }


Exit:
    if (!NT_SUCCESS(status)) {
        WdfRequestCompleteWithInformation(Request, status, 0);
    }

    UsbSamp_DbgPrint( 3,( "<-- OsrFxEvtIoRead\n"));

    return;
}

VOID
EvtRequestReadCompletionRoutine(
    __in WDFREQUEST                  Request,
    __in WDFIOTARGET                 Target,
    __in PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
    __in WDFCONTEXT                  Context
    )
/*++

Routine Description:

    This is the completion routine for reads/writes
    If the irp completes with success, we check if we
    need to recirculate this irp for another stage of
    transfer.

Arguments:

    Context - Driver supplied context
    Device - Device handle
    Request - Request handle
    Params - request completion params

Return Value:
    None

--*/
{
    NTSTATUS    status;
    size_t      bytesRead = 0;
    PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;
    WDFUSBPIPE              pipe;//GS

    UNREFERENCED_PARAMETER(Target);
    UNREFERENCED_PARAMETER(Context);

    status = CompletionParams->IoStatus.Status;
    
    pipe = (WDFUSBPIPE) Target;
    
//#define STATUS_IO_TIMEOUT                ((NTSTATUS)0xC00000B5L)  // GS
 
    if (!NT_SUCCESS(status) && status != STATUS_IO_TIMEOUT){
           
        // Queue a workitem to reset the pipe because the completion could be
        // running at DISPATCH_LEVEL.
        //
        QueuePassiveLevelCallback(WdfIoTargetGetDevice(Target), pipe);
        bytesRead = 0;
        goto End;
        //WdfRequestComplete(Request, status);
        //return;//goto End;
    }    

       usbCompletionParams = CompletionParams->Parameters.Usb.Completion;
       bytesRead =  usbCompletionParams->Parameters.PipeRead.Length;

    if (NT_SUCCESS(status)){
        UsbSamp_DbgPrint(3,("Number of bytes read: %I64d\n", (INT64)bytesRead));
    } else {
        UsbSamp_DbgPrint(3,("Read failed - request status 0x%x UsbdStatus 0x%x\n",
                status, usbCompletionParams->UsbdStatus));

    }

End:

    WdfRequestCompleteWithInformation(Request, status, bytesRead);

    return;
}

///////////////////////////////////// WRITE //////////////////////////////////

VOID
UsbSamp_EvtIoWrite(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN size_t           Length
    )
    
/*++

Routine Description:

    Called by the framework when it receives Write requests.

Arguments:

    Queue - Default queue handle
    Request - Handle to the read/write request
    Lenght - Length of the data buffer associated with the request.
                 The default property of the queue is to not dispatch
                 zero lenght read & write requests to the driver and
                 complete is with status success. So we will never get
                 a zero length request.

Return Value:


--*/
{
    NTSTATUS                    status;
    WDFUSBPIPE                  pipe;
    WDFMEMORY                   reqMemory;
    //PDEVICE_CONTEXT             pDeviceContext;
    PFILE_CONTEXT          	fileContext = NULL;
    WDF_REQUEST_SEND_OPTIONS    opt;

    UNREFERENCED_PARAMETER(Queue);

   UsbSamp_DbgPrint(1,( "-->OsrFxEvtIoWrite\n"));

    //
    // First validate input parameters.
    //
    if (Length > 256 /*TEST_BOARD_TRANSFER_BUFFER_SIZE*/) {
        UsbSamp_DbgPrint(1,( "Transfer exceeds %d\n",
                            TEST_BOARD_TRANSFER_BUFFER_SIZE));
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }


    //pDeviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));
    //pipe = pDeviceContext->BulkWritePipe;
    
    fileContext = GetFileContext(WdfRequestGetFileObject(Request));
    pipe = fileContext->Pipe;

    status = WdfRequestRetrieveInputMemory(Request, &reqMemory);
    if(!NT_SUCCESS(status)){
        UsbSamp_DbgPrint(1,( "WdfRequestRetrieveInputBuffer failed\n"));
        goto Exit;
    }

    status = WdfUsbTargetPipeFormatRequestForWrite(pipe,
                            Request,
                            reqMemory,
                            NULL); // Offset


    if (!NT_SUCCESS(status)) {
        UsbSamp_DbgPrint(TRACE_LEVEL_ERROR,("WdfUsbTargetPipeFormatRequest failed 0x%x\n", status));
        goto Exit;
    }

    WdfRequestSetCompletionRoutine(
                            Request,
                            EvtRequestWriteCompletionRoutine,
                            pipe);

    WDF_REQUEST_SEND_OPTIONS_INIT(
                                 &opt,
                                 WDF_REQUEST_SEND_OPTION_TIMEOUT
                                 );
                              
    WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(
                                        &opt,
                                        WDF_REL_TIMEOUT_IN_MS(50)
                                        );  

    //
    // Send the request asynchronously.
    //
    
    if (WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pipe),&opt/* WDF_NO_SEND_OPTIONS*/) == FALSE) {
        //
        // Framework couldn't send the request for some reason.
        //
        UsbSamp_DbgPrint(1, ("WdfRequestSend failed\n"));
        status = WdfRequestGetStatus(Request);
        goto Exit;
    }

Exit:

    if (!NT_SUCCESS(status)) {
        WdfRequestCompleteWithInformation(Request, status, 0);
    }

    UsbSamp_DbgPrint(1,("<-- OsrFxEvtIoWrite\n"));

    return;
}


VOID
EvtRequestWriteCompletionRoutine(
    __in WDFREQUEST                  Request,
    __in WDFIOTARGET                 Target,
    __in PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
    __in WDFCONTEXT                  Context
    )
/*++

Routine Description:

    This is the completion routine for reads/writes
    If the irp completes with success, we check if we
    need to recirculate this irp for another stage of
    transfer.

Arguments:

    Context - Driver supplied context
    Device - Device handle
    Request - Request handle
    Params - request completion params

Return Value:
    None

--*/
{
    NTSTATUS    status;
    size_t      bytesWritten = 0;
    PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;

    UNREFERENCED_PARAMETER(Target);
    UNREFERENCED_PARAMETER(Context);

    status = CompletionParams->IoStatus.Status;

    //
    // For usb devices, we should look at the Usb.Completion param.
    //
    usbCompletionParams = CompletionParams->Parameters.Usb.Completion;

    bytesWritten =  usbCompletionParams->Parameters.PipeWrite.Length;

    if (NT_SUCCESS(status)){
        UsbSamp_DbgPrint(1 ,( "Number of bytes written: %I64d\n", (INT64)bytesWritten));
    } else {
        UsbSamp_DbgPrint(1,("Write failed: request Status 0x%x UsbdStatus 0x%x\n",
                status, usbCompletionParams->UsbdStatus));
    }

    WdfRequestCompleteWithInformation(Request, status, bytesWritten);

    return;
}

VOID
UsbSamp_EvtIoStop(
    __in WDFQUEUE         Queue,
    __in WDFREQUEST       Request,
    __in ULONG            ActionFlags
    )
/*++

Routine Description:

    This callback is invoked on every inflight request when the device
    is suspended or removed. Since our inflight read and write requests
    are actually pending in the target device, we will just acknowledge
    its presence. Until we acknowledge, complete, or requeue the requests
    framework will wait before allowing the device suspend or remove to
    proceeed. When the underlying USB stack gets the request to suspend or
    remove, it will fail all the pending requests.

Arguments:

Return Value:
    None

--*/
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(ActionFlags);

    if (ActionFlags == WdfRequestStopActionSuspend ) {
        WdfRequestStopAcknowledge(Request, FALSE); // Don't requeue
    } else if(ActionFlags == WdfRequestStopActionPurge) {
        WdfRequestCancelSentRequest(Request);
    }
    return;
}


WDFUSBPIPE
GetPipeFromName(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PUNICODE_STRING FileName
    )
/*++

Routine Description:

    This routine will pass the string pipe name and
    fetch the pipe handle.

Arguments:

    DeviceContext - pointer to Device Context

    FileName - string pipe name

Return Value:

    The device extension maintains a pipe context for
    the pipes on 82930 board.

--*/
{
    LONG                  ix;
    ULONG                 uval;
    ULONG                 nameLength;
    ULONG                 umultiplier;
    WDFUSBPIPE            pipe = NULL;

    //
    // typedef WCHAR *PWSTR;
    //
    nameLength = (FileName->Length / sizeof(WCHAR));

    UsbSamp_DbgPrint(3, ("UsbSamp_PipeWithName - begins\n"));

    if(nameLength != 0) {

        UsbSamp_DbgPrint(3, ("Filename = %wZ nameLength = %d\n", FileName, nameLength));

        //
        // Parse the pipe#
        //
        ix = nameLength - 1;

        // if last char isn't digit, decrement it.
        while((ix > -1) &&
              ((FileName->Buffer[ix] < (WCHAR) '0')  ||
               (FileName->Buffer[ix] > (WCHAR) '9')))             {

            ix--;
        }

        if (ix > -1) {

            uval = 0;
            umultiplier = 1;

            // traversing least to most significant digits.

            while((ix > -1) &&
                  (FileName->Buffer[ix] >= (WCHAR) '0') &&
                  (FileName->Buffer[ix] <= (WCHAR) '9'))          {

                uval += (umultiplier *
                         (ULONG) (FileName->Buffer[ix] - (WCHAR) '0'));

                ix--;
                umultiplier *= 10;
            }
            pipe = WdfUsbInterfaceGetConfiguredPipe(
                DeviceContext->UsbInterface,
                (UCHAR)uval, //PipeIndex,
                NULL
                );

        }
    }

    UsbSamp_DbgPrint(3, ("UsbSamp_PipeWithName - ends\n"));

    return pipe;
}

NTSTATUS
ResetPipe(
    IN WDFUSBPIPE             Pipe
    )
/*++

Routine Description:

    This routine resets the pipe.

Arguments:

    Pipe - framework pipe handle

Return Value:

    NT status value

--*/
{
    NTSTATUS          status;

    //
    //  This routine synchronously submits a URB_FUNCTION_RESET_PIPE
    // request down the stack.
    //
    status = WdfUsbTargetPipeResetSynchronously(Pipe,
                                WDF_NO_HANDLE, // WDFREQUEST
                                NULL  // PWDF_REQUEST_SEND_OPTIONS
                                );

    if (NT_SUCCESS(status)) {
        UsbSamp_DbgPrint(3, ("ResetPipe - success\n"));
        status = STATUS_SUCCESS;
    }
    else {
        UsbSamp_DbgPrint(1, ("ResetPipe - failed\n"));
    }

    return status;
}

VOID
StopAllPipes(
    IN PDEVICE_CONTEXT DeviceContext
    )
{
    UCHAR count,i;
    
    count = DeviceContext->NumberConfiguredPipes;
    for (i = 0; i < count; i++) {
        WDFUSBPIPE pipe;
        pipe = WdfUsbInterfaceGetConfiguredPipe(DeviceContext->UsbInterface,
                                                i, //PipeIndex,
                                                NULL
                                                );
        WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(pipe),
                        WdfIoTargetCancelSentIo);
    }
}


VOID
StartAllPipes(
    IN PDEVICE_CONTEXT DeviceContext
    )
{
    UCHAR count,i;
    
    count = DeviceContext->NumberConfiguredPipes;
    for (i = 0; i < count; i++) {
        WDFUSBPIPE pipe;
        pipe = WdfUsbInterfaceGetConfiguredPipe(DeviceContext->UsbInterface,
                                                i, //PipeIndex,
                                                NULL
                                                );
        WdfIoTargetStart(WdfUsbTargetPipeGetIoTarget(pipe));
    }
}

BOOLEAN
IsItWin2k(
    VOID
    )
{
    ULONG majorVersion, minorVersion, buildNumber;

    //
    // Check for win2k.
    // PsGetVersion is obsolete in Windows XP and later versions 
    // of the operating system. Use RtlGetVersion instead.
    //
    PsGetVersion(&majorVersion, &minorVersion, &buildNumber, NULL);   
    if (majorVersion == 5 && minorVersion == 0) {
        return TRUE;
    }
    else{
        return FALSE;
    }   
}

NTSTATUS
ResetDevice(
    IN WDFDEVICE Device
    )
/*++

Routine Description:

    This routine calls WdfUsbTargetDeviceResetPortSynchronously to reset the device if it's still
    connected.

Arguments:

    Device - Handle to a framework device

Return Value:

    NT status value

--*/
{
    PDEVICE_CONTEXT pDeviceContext;
    NTSTATUS status;
    BOOLEAN win2k;
    
    UsbSamp_DbgPrint(3, ("ResetDevice - begins\n"));
    win2k = IsItWin2k();
    pDeviceContext = GetDeviceContext(Device);
    if (win2k) {
        //
        // On Windows 2000, a USB device cannot be reset if there are pending transactions. 
        // This osrusbfx2 driver  configures a continuous reader on the interrupt pipe, 
        // which is in effect a pending transfer. Therefore, if you send a reset-device 
        // request, the request will be stuck in the USB until the pending transactions 
        // have been canceled. Similarly, if there are pending tranasfers on the BULK 
        // IN/OUT pipe cancel them.
        // To work around this issue, the driver should stop the continuous reader 
        // (by calling WdfIoTargetStop) before resetting the device, and restart the 
        // continuous reader (by calling WdfIoTargetStart) after the request completes. 
        //
        StopAllPipes(pDeviceContext);
    }
    
    //
    // It may not be necessary to check whether device is connected before
    // resetting the port.
    //
    status = WdfUsbTargetDeviceIsConnectedSynchronous(pDeviceContext->WdfUsbTargetDevice);

    if(NT_SUCCESS(status)) {
        status = WdfUsbTargetDeviceResetPortSynchronously(pDeviceContext->WdfUsbTargetDevice);
    }
    if (win2k) {
        StartAllPipes(pDeviceContext);
    }
    UsbSamp_DbgPrint(3, ("ResetDevice - ends\n"));

    return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

VOID
ReadWriteWorkItem(
    IN WDFWORKITEM  WorkItem
    )
{
    PWORKITEM_CONTEXT pItemContext;
    NTSTATUS status;

    UsbSamp_DbgPrint(3, ("ReadWriteWorkItem called\n"));

    pItemContext = GetWorkItemContext(WorkItem);

    status = ResetPipe(pItemContext->Pipe);
    if (!NT_SUCCESS(status)) {

        UsbSamp_DbgPrint(1, ("ResetPipe failed 0x%x\n", status));

        status = ResetDevice(pItemContext->Device);
        if(!NT_SUCCESS(status)){

            UsbSamp_DbgPrint(1, ("ResetDevice failed 0x%x\n", status));
        }
    }

    WdfObjectDelete(WorkItem);

    return;
}

NTSTATUS
QueuePassiveLevelCallback(
    IN WDFDEVICE    Device,
    IN WDFUSBPIPE   Pipe
    )
/*++

Routine Description:

    This routine is used to queue workitems so that the callback
    functions can be executed at PASSIVE_LEVEL in the conext of
    a system thread.

Arguments:


Return Value:

--*/
{
    NTSTATUS                       status = STATUS_SUCCESS;
    PWORKITEM_CONTEXT               context;
    WDF_OBJECT_ATTRIBUTES           attributes;
    WDF_WORKITEM_CONFIG             workitemConfig;
    WDFWORKITEM                     hWorkItem;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attributes, WORKITEM_CONTEXT);
    attributes.ParentObject = Device;

    WDF_WORKITEM_CONFIG_INIT(&workitemConfig, ReadWriteWorkItem);

    status = WdfWorkItemCreate( &workitemConfig,
                                &attributes,
                                &hWorkItem);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    context = GetWorkItemContext(hWorkItem);

    context->Device = Device;
    context->Pipe = Pipe;

    //
    // Execute this work item.
    //
    WdfWorkItemEnqueue(hWorkItem);

    return STATUS_SUCCESS;
}


