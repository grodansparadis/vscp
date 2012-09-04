/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    isorwr.c

Abstract:

    This file has dispatch routines for read and write.

Environment:

    Kernel mode

Notes:

--*/

#include "private.h"
       
VOID
DoSubRequestCleanup(
    PREQUEST_CONTEXT    MainRequestContext,
    PLIST_ENTRY         SubRequestsList,
    PBOOLEAN            CompleteRequest
    )  ;

VOID
ReadWriteIsochEndPoints(
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN ULONG            Length,
    IN WDF_REQUEST_TYPE RequestType
    )
/*++

Routine Description:

    This routine does some validation and
    invokes appropriate function to perform
    Isoch transfer

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    ULONG                   totalLength;
    ULONG                   packetSize;
    NTSTATUS               status;
    PDEVICE_CONTEXT         deviceContext;
    WDF_USB_PIPE_INFORMATION   pipeInfo;
    PFILE_CONTEXT           fileContext;
    WDFUSBPIPE              pipe;

    UsbSamp_DbgPrint(3, ("ReadWriteIsochEndPoints - begins\n"));

    //
    // initialize vars
    //
    totalLength = Length;

    deviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));

    //
    // Get the pipe associate with this request.
    //
    fileContext = GetFileContext(WdfRequestGetFileObject(Request));
    pipe = fileContext->Pipe;
    WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);
    WdfUsbTargetPipeGetInformation(pipe, &pipeInfo);

    if((WdfUsbPipeTypeIsochronous != pipeInfo.PipeType)) {
        UsbSamp_DbgPrint(1, ("Usbd pipe type is not Isochronous\n"));
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto Exit;

    }

    //
    // each packet can hold this much info
    //
    packetSize = pipeInfo.MaximumPacketSize;

    if(packetSize == 0) {
        UsbSamp_DbgPrint(1, ("Invalid parameter\n"));
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    //
    // atleast packet worth of data to be transferred.
    //
    if(totalLength < packetSize) {

        UsbSamp_DbgPrint(1, ("Atleast packet worth of data..\n"));
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    //
    // Perform reset. if there are some active transfers queued up
    // for this endpoint then the reset pipe will fail.
    //
    ResetPipe(pipe);

    if(deviceContext->IsDeviceHighSpeed) {

        PerformHighSpeedIsochTransfer(WdfIoQueueGetDevice(Queue),
                                      Queue,
                                      Request,
                                      Length,
                                      RequestType);
        return;
    }
    else {

        PerformFullSpeedIsochTransfer(WdfIoQueueGetDevice(Queue),
                                      Queue,
                                      Request,
                                      Length,
                                      RequestType);
        return;
    }

Exit:

    WdfRequestCompleteWithInformation(Request, status, 0);
    return;
}

VOID
PerformFullSpeedIsochTransfer(
    IN WDFDEVICE        Device,
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN ULONG            TotalLength,
    IN WDF_REQUEST_TYPE RequestType
    )
/*++

Routine Description:

    This routine
    1. Creates a Sub Request for each irp/urb pair.
       (Each irp/urb pair can transfer a max of 1024 packets.)
    2. All the irp/urb pairs are initialized
    3. The subsidiary irps (of the irp/urb pair) are passed
       down the stack at once.
    4. The main Read/Write is completed in the SubRequestCompletionRoutine
       even if one SubRequest is sent successfully.

Arguments:

    Device - Device handle
    Request - Default queue handle
    Request - Read/Write Request received from the user app.
    TotalLength - Length of the user buffer.

Return Value:

    VOID
--*/
{
    ULONG                   packetSize;
    ULONG                   numSubRequests;
    ULONG                   stageSize;
    PUCHAR                  virtualAddress;
    NTSTATUS               status;
    PDEVICE_CONTEXT         deviceContext;
    PREQUEST_CONTEXT        rwContext;
    PMDL                    requestMdl;
    WDF_USB_PIPE_INFORMATION   pipeInfo;
    WDFUSBPIPE              pipe = NULL;
    WDFCOLLECTION           hCollection = NULL;
    WDFSPINLOCK            hSpinLock = NULL;
    PFILE_CONTEXT           fileContext;
    WDF_OBJECT_ATTRIBUTES   attributes;
    WDFREQUEST              subRequest = NULL;
    PSUB_REQUEST_CONTEXT    subReqContext = NULL;
    ULONG                   i, j;
    PLIST_ENTRY             thisEntry;
    LIST_ENTRY              subRequestsList;
    USBD_PIPE_HANDLE        usbdPipeHandle;

    UNREFERENCED_PARAMETER(Device);

    InitializeListHead(&subRequestsList);

    rwContext = GetRequestContext(Request);


    if(RequestType == WdfRequestTypeRead) {

        rwContext->Read = TRUE;
    } else {
        rwContext->Read = FALSE;
    }

    UsbSamp_DbgPrint(3, ("PerformFullSpeedIsochTransfer %s - begins\n",
                                rwContext->Read ? "Read":"Write"));

    deviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));

    //
    // Create a collection to store all the sub requests.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = Request;
    status = WdfCollectionCreate(&attributes,
                                &hCollection);
    if (!NT_SUCCESS(status))
    {
        UsbSamp_DbgPrint(1, ("WdfCollectionCreate failed with status 0x%x\n", status));
        goto Exit;
    }

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = hCollection;
    status = WdfSpinLockCreate(&attributes, &hSpinLock);
    if (!NT_SUCCESS(status))
    {
        UsbSamp_DbgPrint(1,  ("WdfSpinLockCreate failed with status 0x%x\n", status));
        goto Exit;
    }

    //
    // Get the pipe associate with this request.
    //
    fileContext = GetFileContext(WdfRequestGetFileObject(Request));
    pipe = fileContext->Pipe;
    WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);
    WdfUsbTargetPipeGetInformation(pipe, &pipeInfo);

    //
    // Each packet can hold this much info
    //
    packetSize = pipeInfo.MaximumPacketSize;

    UsbSamp_DbgPrint(3, ("totalLength = %d\n", TotalLength));
    UsbSamp_DbgPrint(3, ("packetSize = %d\n", packetSize));

    //
    // there is an inherent limit on the number of packets
    // that can be passed down the stack with each
    // irp/urb pair (255)
    // if the number of required packets is > 255,
    // we shall create "required-packets / 255 + 1" number
    // of irp/urb pairs.
    // Each irp/urb pair transfer is also called a stage transfer.
    //
    if(TotalLength > (packetSize * 255)) {

        stageSize = packetSize * 255;
    }
    else {

        stageSize = TotalLength;
    }

    UsbSamp_DbgPrint(3, ("PerformFullSpeedIsochTransfer::stageSize = %d\n", stageSize));

    //
    // determine how many stages of transfer needs to be done.
    // in other words, how many irp/urb pairs required.
    // this irp/urb pair is also called the subsidiary irp/urb pair
    //
    numSubRequests = (TotalLength + stageSize - 1) / stageSize;

    UsbSamp_DbgPrint(3, ("PerformFullSpeedIsochTransfer::numSubRequests = %d\n", numSubRequests));

    rwContext->SubRequestCollection = hCollection;
    rwContext->SubRequestCollectionLock = hSpinLock;

    if(RequestType == WdfRequestTypeRead) {
        status = WdfRequestRetrieveOutputWdmMdl(Request, &requestMdl);
        if(!NT_SUCCESS(status)){
            UsbSamp_DbgPrint(1, ("WdfRequestRetrieveOutputWdmMdl failed %x\n", status));
            goto Exit;
        }
    } else {
        status = WdfRequestRetrieveInputWdmMdl(Request, &requestMdl);
        if(!NT_SUCCESS(status)){
            UsbSamp_DbgPrint(1, ("WdfRequestRetrieveInputWdmMdl failed %x\n", status));
            goto Exit;
        }
    }
      
    virtualAddress = (PUCHAR) MmGetMdlVirtualAddress(requestMdl);

    for(i = 0; i < numSubRequests; i++) {

        WDFMEMORY               subUrbMemory;
        PURB                    subUrb;
        PMDL                    subMdl;
        ULONG                   nPackets;
        ULONG                   siz;
        ULONG                   offset;

        //
        // For every stage of transfer we need to do the following
        // tasks
        // 1. allocate a request
        // 2. allocate an urb
        // 3. allocate a mdl.
        // 4. Format the request for transfering URB
        // 5. Send the Request.

        //

        nPackets = (stageSize + packetSize - 1) / packetSize;

        UsbSamp_DbgPrint(3, ("nPackets = %d for Irp/URB pair %d\n", nPackets, i));

        ASSERT(nPackets <= 255);
        siz = GET_ISO_URB_SIZE(nPackets);

        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attributes, SUB_REQUEST_CONTEXT);

        status = WdfRequestCreate(
            &attributes,
            WdfUsbTargetDeviceGetIoTarget(deviceContext->WdfUsbTargetDevice),
            &subRequest);

        if(!NT_SUCCESS(status)) {

            UsbSamp_DbgPrint(1, ("WdfRequestCreate failed 0x%x\n", status));
            goto Exit;
        }

        subReqContext = GetSubRequestContext(subRequest);
        subReqContext->UserRequest = Request;

        //
        // Allocate memory for URB.
        //
        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        attributes.ParentObject = subRequest;
        status = WdfMemoryCreate(
                            &attributes,
                            NonPagedPool,
                            POOL_TAG,
                            siz,
                            &subUrbMemory,
                            (PVOID*) &subUrb);

        if (!NT_SUCCESS(status)) {
            WdfObjectDelete(subRequest);
            UsbSamp_DbgPrint(1, ("Failed to alloc MemoryBuffer for suburb\n"));
            goto Exit;
        }

        subReqContext->SubUrb = subUrb;

        //
        // Allocate a mdl and build the MDL to describe the staged buffer.
        //
        subMdl = IoAllocateMdl((PVOID) virtualAddress,
                            stageSize,
                            FALSE,
                            FALSE,
                            NULL);

        if(subMdl == NULL) {

            UsbSamp_DbgPrint(1, ("failed to alloc mem for sub context mdl\n"));
            WdfObjectDelete(subRequest);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }

        IoBuildPartialMdl(requestMdl,
                          subMdl,
                          (PVOID) virtualAddress,
                          stageSize);

        subReqContext->SubMdl = subMdl;

        virtualAddress += stageSize;
        TotalLength -= stageSize;

        //
        // Initialize the subsidiary urb
        //
        usbdPipeHandle = WdfUsbTargetPipeWdmGetPipeHandle(pipe);

        subUrb->UrbIsochronousTransfer.Hdr.Length = (USHORT) siz;
        subUrb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;
        subUrb->UrbIsochronousTransfer.PipeHandle = usbdPipeHandle;
        if(rwContext->Read) {

            subUrb->UrbIsochronousTransfer.TransferFlags =
                                                     USBD_TRANSFER_DIRECTION_IN;
        }
        else {

            subUrb->UrbIsochronousTransfer.TransferFlags =
                                                     USBD_TRANSFER_DIRECTION_OUT;
        }

        subUrb->UrbIsochronousTransfer.TransferBufferLength = stageSize;
        subUrb->UrbIsochronousTransfer.TransferBufferMDL = subMdl;

#if 0
        //
        // This is a way to set the start frame and NOT specify ASAP flag.
        //
        status = WdfUsbTargetDeviceRetrieveCurrentFrameNumber(wdfUsbDevice, &frameNumber);
        subUrb->UrbIsochronousTransfer.StartFrame = frameNumber  + SOME_LATENCY;
#endif

        //
        // when the client driver sets the ASAP flag, it basically
        // guarantees that it will make data available to the HC
        // and that the HC should transfer it in the next transfer frame
        // for the endpoint.(The HC maintains a next transfer frame
        // state variable for each endpoint). If the data does not get to the HC
        // fast enough, the USBD_ISO_PACKET_DESCRIPTOR - Status is
        // USBD_STATUS_BAD_START_FRAME on uhci. On ohci it is 0xC000000E.
        //

        subUrb->UrbIsochronousTransfer.TransferFlags |=
                                    USBD_START_ISO_TRANSFER_ASAP;

        subUrb->UrbIsochronousTransfer.NumberOfPackets = nPackets;
        subUrb->UrbIsochronousTransfer.UrbLink = NULL;

        //
        // set the offsets for every packet for reads/writes
        //
        if(rwContext->Read) {

            offset = 0;

            for(j = 0; j < nPackets; j++) {

                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = 0;

                if(stageSize > packetSize) {

                    offset += packetSize;
                    stageSize -= packetSize;
                }
                else {

                    offset += stageSize;
                    stageSize = 0;
                }
            }
        }
        else {

            offset = 0;

            for(j = 0; j < nPackets; j++) {

                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;

                if(stageSize > packetSize) {

                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = packetSize;
                    offset += packetSize;
                    stageSize -= packetSize;
                }
                else {

                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = stageSize;
                    offset += stageSize;
                    stageSize = 0;
                    ASSERT(offset == (subUrb->UrbIsochronousTransfer.IsoPacket[j].Length +
                                      subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset));
                }
            }
        }


        //
        // Associate the URB with the request.
        //
        status = WdfUsbTargetPipeFormatRequestForUrb(pipe,
                                          subRequest,
                                          subUrbMemory,
                                          NULL);

        if (!NT_SUCCESS(status)) {
            UsbSamp_DbgPrint(1, ("Failed to format requset for urb\n"));
            WdfObjectDelete(subRequest);
            IoFreeMdl(subMdl);
            goto Exit;
        }

        WdfRequestSetCompletionRoutine(subRequest,
                                        SubRequestCompletionRoutine,
                                        rwContext);

        if(TotalLength > (packetSize * 255)) {

            stageSize = packetSize * 255;
        }
        else {

            stageSize = TotalLength;
        }

        //
        // WdfCollectionAdd takes a reference on the request object and removes
        // it when you call WdfCollectionRemove.
        //
        status = WdfCollectionAdd(hCollection, subRequest);
        if (!NT_SUCCESS(status)) {
            UsbSamp_DbgPrint(1, ("WdfCollectionAdd failed 0x%x\n", status));
            WdfObjectDelete(subRequest);
            IoFreeMdl(subMdl);
            goto Exit;
        }

        InsertTailList(&subRequestsList, &subReqContext->ListEntry);

    }


    //
    // Mark the main request cancelable so that we can cancel the subrequests
    // if the main requests gets cancelled for any reason.
    //
    WdfRequestMarkCancelable(Request, UsbSamp_EvtRequestCancel);

    while(!IsListEmpty(&subRequestsList)) {

        thisEntry = GetListHeadEntry(&subRequestsList);
        subReqContext = CONTAINING_RECORD(thisEntry, SUB_REQUEST_CONTEXT, ListEntry);
        subRequest = WdfObjectContextGetObject(subReqContext);
        UsbSamp_DbgPrint(3, ("Sending subRequest 0x%p\n", subRequest));

        if (WdfRequestSend(subRequest, WdfUsbTargetPipeGetIoTarget(pipe), WDF_NO_SEND_OPTIONS) == FALSE) {
            status = WdfRequestGetStatus(subRequest);
            UsbSamp_DbgPrint(1, ("WdfRequestSend failed with status code 0x%x\n", status));
            ASSERT(!NT_SUCCESS(status));
            goto Exit;
        }

        //
        // Don't touch the subrequest after it has been sent.
        // Make a note that at least one subrequest is sent. This will be used
        // in deciding whether we should free the subrequests in case of failure.
        //

        RemoveHeadList(&subRequestsList);

    }

Exit:

    //
    // Checking the status besides the number of list entries will help differentiate
    // failures where everything succeeded vs where there were failures before adding
    // list entries.
    //
    if(NT_SUCCESS(status) && IsListEmpty(&subRequestsList)) {
        //
        // We will let the completion routine to cleanup and complete the
        // main request.
        //
        return;
    }
    else {
        BOOLEAN  completeRequest;

        completeRequest = TRUE;
        
        if(hCollection) {                       
            DoSubRequestCleanup(rwContext, &subRequestsList, &completeRequest);  
        }

        if (completeRequest) {
            if (rwContext->Numxfer > 0 ) {
                WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, rwContext->Numxfer);
            }
            else {
               WdfRequestCompleteWithInformation(Request, status, rwContext->Numxfer); 
            }

        }

    }

    UsbSamp_DbgPrint(3, ("PerformFullSpeedIsochTransfer -- ends\n"));
    return;
}

VOID
PerformHighSpeedIsochTransfer(
    IN WDFDEVICE        Device,
    IN WDFQUEUE         Queue,
    IN WDFREQUEST       Request,
    IN ULONG            TotalLength,
    IN WDF_REQUEST_TYPE RequestType
    )
/*++

Routine Description:

    High Speed Isoch Transfer requires packets in multiples of 8.
    (Argument: 8 micro-frames per ms frame)
    Another restriction is that each Irp/Urb pair can be associated
    with a max of 1024 packets.

    Here is one of the ways of creating Irp/Urb pairs.
    Depending on the characteristics of real-world device,
    the algorithm may be different

    This algorithm will distribute data evenly among all the packets.

    Input:
    TotalLength - no. of bytes to be transferred.

    Other parameters:
    packetSize - max size of each packet for this pipe.

    Implementation Details:

    Step 1:
    ASSERT(TotalLength >= 8)

    Step 2:
    Find the exact number of packets required to transfer all of this data

    numberOfPackets = (TotalLength + packetSize - 1) / packetSize

    Step 3:
    Number of packets in multiples of 8.

    if(0 == (numberOfPackets % 8)) {

        actualPackets = numberOfPackets;
    }
    else {

        actualPackets = numberOfPackets +
                        (8 - (numberOfPackets % 8));
    }

    Step 4:
    Determine the min. data in each packet.

    minDataInEachPacket = TotalLength / actualPackets;

    Step 5:
    After placing min data in each packet,
    determine how much data is left to be distributed.

    dataLeftToBeDistributed = TotalLength -
                              (minDataInEachPacket * actualPackets);

    Step 6:
    Start placing the left over data in the packets
    (above the min data already placed)

    numberOfPacketsFilledToBrim = dataLeftToBeDistributed /
                                  (packetSize - minDataInEachPacket);

    Step 7:
    determine if there is any more data left.

    dataLeftToBeDistributed -= (numberOfPacketsFilledToBrim *
                                (packetSize - minDataInEachPacket));

    Step 8:
    The "dataLeftToBeDistributed" is placed in the packet at index
    "numberOfPacketsFilledToBrim"

    Algorithm at play:

    TotalLength  = 8193
    packetSize   = 8
    Step 1

    Step 2
    numberOfPackets = (8193 + 8 - 1) / 8 = 1025

    Step 3
    actualPackets = 1025 + 7 = 1032

    Step 4
    minDataInEachPacket = 8193 / 1032 = 7 bytes

    Step 5
    dataLeftToBeDistributed = 8193 - (7 * 1032) = 969.

    Step 6
    numberOfPacketsFilledToBrim = 969 / (8 - 7) = 969.

    Step 7
    dataLeftToBeDistributed = 969 - (969 * 1) = 0.

    Step 8
    Done :)

    Another algorithm
    Completely fill up (as far as possible) the early packets.
    Place 1 byte each in the rest of them.
    Ensure that the total number of packets is multiple of 8.

    This routine then
    1. Creates a Sub Request for each irp/urb pair.
       (Each irp/urb pair can transfer a max of 1024 packets.)
    2. All the irp/urb pairs are initialized
    3. The subsidiary irps (of the irp/urb pair) are passed
       down the stack at once.
    4. The main Read/Write is completed in the SubRequestCompletionRoutine
       even if one SubRequest is sent successfully.

Arguments:

    Device - Device handle
    Request - Default queue handle
    Request - Read/Write Request received from the user app.
    TotalLength - Length of the user buffer.

Return Value:

    VOID
--*/
{
    ULONG                   numberOfPackets;
    ULONG                   actualPackets;
    ULONG                   minDataInEachPacket;
    ULONG                   dataLeftToBeDistributed;
    ULONG                   numberOfPacketsFilledToBrim;
    ULONG                   packetSize;
    ULONG                   numSubRequests;
    ULONG                   stageSize;
    PUCHAR                  virtualAddress;
    NTSTATUS                status;
    PDEVICE_CONTEXT         deviceContext;
    PREQUEST_CONTEXT        rwContext;
    PMDL                    requestMdl;
    WDF_USB_PIPE_INFORMATION   pipeInfo;
    WDFUSBPIPE              pipe;
    WDFCOLLECTION           hCollection = NULL;
    WDFSPINLOCK             hSpinLock = NULL;
    PFILE_CONTEXT           fileContext;
    WDF_OBJECT_ATTRIBUTES   attributes;
    WDFREQUEST              subRequest;
    PSUB_REQUEST_CONTEXT    subReqContext;
    ULONG                   i, j;
    PLIST_ENTRY             thisEntry;
    LIST_ENTRY              subRequestsList;
    USBD_PIPE_HANDLE        usbdPipeHandle;

    UNREFERENCED_PARAMETER(Device);

    InitializeListHead(&subRequestsList); 
    rwContext = GetRequestContext(Request);

    if(RequestType == WdfRequestTypeRead) {

        rwContext->Read = TRUE;
    } else {
        rwContext->Read = FALSE;
    }

    UsbSamp_DbgPrint(3, ("PerformFullSpeedIsochTransfer %s - begins\n",
                                rwContext->Read ? "Read":"Write"));
    if(TotalLength < 8) {

        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    deviceContext = GetDeviceContext(WdfIoQueueGetDevice(Queue));

    //
    // Create a collection to store all the sub requests.
    //
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = Request;
    status = WdfCollectionCreate(&attributes,
                                &hCollection);
    if (!NT_SUCCESS(status))
    {
        UsbSamp_DbgPrint(1, ("WdfCollectionCreate failed with status 0x%x\n", status));
        goto Exit;
    }

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.ParentObject = hCollection;
    status = WdfSpinLockCreate(&attributes, &hSpinLock);
    if (!NT_SUCCESS(status))
    {
        UsbSamp_DbgPrint(1, ("WdfSpinLockCreate failed with status 0x%x\n", status));
        goto Exit;
    }

    //
    // Get the pipe associate with this request.
    //
    fileContext = GetFileContext(WdfRequestGetFileObject(Request));
    pipe = fileContext->Pipe;
    WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);
    WdfUsbTargetPipeGetInformation(pipe, &pipeInfo);


    //
    // each packet can hold this much info
    //
    packetSize = pipeInfo.MaximumPacketSize;

    numberOfPackets = (TotalLength + packetSize - 1) / packetSize;

    if(0 == (numberOfPackets % 8)) {

        actualPackets = numberOfPackets;
    }
    else {

        //
        // we need multiple of 8 packets only.
        //
        actualPackets = numberOfPackets +
                        (8 - (numberOfPackets % 8));
    }

    minDataInEachPacket = TotalLength / actualPackets;

    if(minDataInEachPacket == packetSize) {

        numberOfPacketsFilledToBrim = actualPackets;
        dataLeftToBeDistributed     = 0;

        UsbSamp_DbgPrint(1, ("TotalLength = %d\n", TotalLength));
        UsbSamp_DbgPrint(1, ("PacketSize  = %d\n", packetSize));
        UsbSamp_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                            numberOfPacketsFilledToBrim,
                            packetSize));
    }
    else {

        dataLeftToBeDistributed = TotalLength -
                              (minDataInEachPacket * actualPackets);

        numberOfPacketsFilledToBrim = dataLeftToBeDistributed /
                                  (packetSize - minDataInEachPacket);

        dataLeftToBeDistributed -= (numberOfPacketsFilledToBrim *
                                (packetSize - minDataInEachPacket));


        UsbSamp_DbgPrint(1, ("TotalLength = %d\n", TotalLength));
        UsbSamp_DbgPrint(1, ("PacketSize  = %d\n", packetSize));
        UsbSamp_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                            numberOfPacketsFilledToBrim,
                            packetSize));
        if(dataLeftToBeDistributed) {

            UsbSamp_DbgPrint(1, ("One packet has %d bytes\n",
                                minDataInEachPacket + dataLeftToBeDistributed));
            UsbSamp_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                                actualPackets - (numberOfPacketsFilledToBrim + 1),
                                minDataInEachPacket));
        }
        else {
            UsbSamp_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                                actualPackets - numberOfPacketsFilledToBrim,
                                minDataInEachPacket));
        }
    }

    //
    // determine how many stages of transfer needs to be done.
    // in other words, how many irp/urb pairs required.
    // this irp/urb pair is also called the subsidiary irp/urb pair
    //
    numSubRequests = (actualPackets + 1023) / 1024;

    UsbSamp_DbgPrint(1, ("PeformHighSpeedIsochTransfer::numSubRequests = %d\n", numSubRequests));

    rwContext->SubRequestCollection = hCollection;
    rwContext->SubRequestCollectionLock = hSpinLock;

    if(RequestType == WdfRequestTypeRead) {
        status = WdfRequestRetrieveOutputWdmMdl(Request, &requestMdl);
        if(!NT_SUCCESS(status)){
            UsbSamp_DbgPrint(1, ("WdfRequestRetrieveOutputWdmMdl failed %x\n", status));
            goto Exit;
        }
    } else {
        status = WdfRequestRetrieveInputWdmMdl(Request, &requestMdl);
        if(!NT_SUCCESS(status)){
            UsbSamp_DbgPrint(1, ("WdfRequestRetrieveInputWdmMdl failed %x\n", status));
            goto Exit;
        }
    }

    virtualAddress = (PUCHAR) MmGetMdlVirtualAddress(requestMdl);


    for(i = 0; i < numSubRequests; i++) {

        WDFMEMORY               subUrbMemory;
        PURB                    subUrb;
        PMDL                    subMdl;
        ULONG                   nPackets;
        ULONG                   siz;
        ULONG                   offset;


        //
        // For every stage of transfer we need to do the following
        // tasks
        // 1. allocate a request
        // 2. allocate an urb
        // 3. allocate a mdl.
        // 4. Format the request for transfering URB
        // 5. Send the Request.

        //
        if(actualPackets <= 1024) {

            nPackets = actualPackets;
            actualPackets = 0;
        }
        else {

            nPackets = 1024;
            actualPackets -= 1024;
        }

        UsbSamp_DbgPrint(1, ("nPackets = %d for Irp/URB pair %d\n", nPackets, i));

        ASSERT(nPackets <= 1024);

        siz = GET_ISO_URB_SIZE(nPackets);


        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attributes, SUB_REQUEST_CONTEXT            );

        status = WdfRequestCreate(
            &attributes,
            WdfUsbTargetDeviceGetIoTarget(deviceContext->WdfUsbTargetDevice),
            &subRequest);

        if(!NT_SUCCESS(status)) {

            UsbSamp_DbgPrint(1, ("WdfRequestCreate failed 0x%x\n", status));
            goto Exit;
        }

        subReqContext = GetSubRequestContext(subRequest);
        subReqContext->UserRequest = Request;

        //
        // Allocate memory for URB.
        //
        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
        attributes.ParentObject = subRequest;
        status = WdfMemoryCreate(
                            &attributes,
                            NonPagedPool,
                            POOL_TAG,
                            siz,
                            &subUrbMemory,
                            (PVOID*) &subUrb);

        if (!NT_SUCCESS(status)) {
            WdfObjectDelete(subRequest);
            UsbSamp_DbgPrint(1, ("Failed to alloc MemoryBuffer for suburb\n"));
            goto Exit;
        }

        subReqContext->SubUrb = subUrb;


        if(nPackets > numberOfPacketsFilledToBrim) {

            stageSize =  packetSize * numberOfPacketsFilledToBrim;
            stageSize += (minDataInEachPacket *
                          (nPackets - numberOfPacketsFilledToBrim));
            stageSize += dataLeftToBeDistributed;
        }
        else {

            stageSize = packetSize * nPackets;
        }

        //
        // allocate a mdl.
        //
        subMdl = IoAllocateMdl((PVOID) virtualAddress,
                               stageSize,
                               FALSE,
                               FALSE,
                               NULL);

        if(subMdl == NULL) {

            UsbSamp_DbgPrint(1, ("failed to alloc mem for sub context mdl\n"));
            WdfObjectDelete(subRequest);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit;
        }

        IoBuildPartialMdl(requestMdl,
                          subMdl,
                          (PVOID) virtualAddress,
                          stageSize);

        subReqContext->SubMdl = subMdl;

        virtualAddress += stageSize;
        TotalLength -= stageSize;

        usbdPipeHandle = WdfUsbTargetPipeWdmGetPipeHandle(pipe);
        subUrb->UrbIsochronousTransfer.Hdr.Length = (USHORT) siz;
        subUrb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;
        subUrb->UrbIsochronousTransfer.PipeHandle = usbdPipeHandle;

        if(rwContext->Read) {

            subUrb->UrbIsochronousTransfer.TransferFlags =
                                                     USBD_TRANSFER_DIRECTION_IN;
        }
        else {

            subUrb->UrbIsochronousTransfer.TransferFlags =
                                                     USBD_TRANSFER_DIRECTION_OUT;
        }

        subUrb->UrbIsochronousTransfer.TransferBufferLength = stageSize;
        subUrb->UrbIsochronousTransfer.TransferBufferMDL = subMdl;
/*
        This is a way to set the start frame and NOT specify ASAP flag.

        status = WdfUsbTargetDeviceRetrieveCurrentFrameNumber(wdfUsbDevice, &frameNumber);
        subUrb->UrbIsochronousTransfer.StartFrame = frameNumber  + SOME_LATENCY;
*/
        subUrb->UrbIsochronousTransfer.TransferFlags |=
                                        USBD_START_ISO_TRANSFER_ASAP;

        subUrb->UrbIsochronousTransfer.NumberOfPackets = nPackets;
        subUrb->UrbIsochronousTransfer.UrbLink = NULL;

        //
        // set the offsets for every packet for reads/writes
        //
        if(rwContext->Read) {

            offset = 0;

            for(j = 0; j < nPackets; j++) {

                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = 0;

                if(numberOfPacketsFilledToBrim) {

                    offset += packetSize;
                    numberOfPacketsFilledToBrim--;
                    status = RtlULongSub(stageSize, packetSize, &stageSize);
                    ASSERT(NT_SUCCESS(status));
                }
                else if(dataLeftToBeDistributed) {

                    offset += (minDataInEachPacket + dataLeftToBeDistributed);
                    stageSize -= (minDataInEachPacket + dataLeftToBeDistributed);
                    dataLeftToBeDistributed = 0;
                }
                else {

                    offset += minDataInEachPacket;
                    stageSize -= minDataInEachPacket;
                }
            }

            ASSERT(stageSize == 0);
        }
        else {

            offset = 0;

            for(j = 0; j < nPackets; j++) {

                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;

                if(numberOfPacketsFilledToBrim) {

                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = packetSize;
                    offset += packetSize;
                    numberOfPacketsFilledToBrim--;
                    stageSize -= packetSize;
                }
                else if(dataLeftToBeDistributed) {

                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length =
                                        minDataInEachPacket + dataLeftToBeDistributed;
                    offset += (minDataInEachPacket + dataLeftToBeDistributed);
                    stageSize -= (minDataInEachPacket + dataLeftToBeDistributed);
                    dataLeftToBeDistributed = 0;

                }
                else {
                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = minDataInEachPacket;
                    offset += minDataInEachPacket;
                    stageSize -= minDataInEachPacket;
                }
            }

            ASSERT(stageSize == 0);
        }

        //
        // Associate the URB with the request.
        //
        status = WdfUsbTargetPipeFormatRequestForUrb(pipe,
                                          subRequest,
                                          subUrbMemory,
                                          NULL );
        if (!NT_SUCCESS(status)) {
            UsbSamp_DbgPrint(1, ("Failed to format requset for urb\n"));
            WdfObjectDelete(subRequest);
            IoFreeMdl(subMdl);
            goto Exit;
        }

        WdfRequestSetCompletionRoutine(subRequest,
                                        SubRequestCompletionRoutine,
                                        rwContext);

        //
        // WdfCollectionAdd takes a reference on the request object and removes
        // it when you call WdfCollectionRemove.
        //
        status = WdfCollectionAdd(hCollection, subRequest);
        if (!NT_SUCCESS(status)) {
            UsbSamp_DbgPrint(1, ("WdfCollectionAdd failed 0x%x\n", status));
            WdfObjectDelete(subRequest);
            IoFreeMdl(subMdl);
            goto Exit;
        }

        InsertTailList(&subRequestsList, &subReqContext->ListEntry);

    }

    //
    // Mark the main request cancelable so that we can cancel the subrequests
    // if the main requests gets cancelled for any reason.
    //
    WdfRequestMarkCancelable(Request, UsbSamp_EvtRequestCancel);

    while(!IsListEmpty(&subRequestsList)) {

        //
        // Don't remove the entry till the request has been sent.
        // This wil insure in case of failure that the List accurately reflects
        // the number of unsent sub-requests.
        //
        thisEntry = GetListHeadEntry(&subRequestsList);
        subReqContext = CONTAINING_RECORD(thisEntry, SUB_REQUEST_CONTEXT, ListEntry);
        subRequest = WdfObjectContextGetObject(subReqContext);
        UsbSamp_DbgPrint(3, ("Sending subRequest 0x%p\n", subRequest));

        if (WdfRequestSend(subRequest, WdfUsbTargetPipeGetIoTarget(pipe), WDF_NO_SEND_OPTIONS) == FALSE) {
            status = WdfRequestGetStatus(subRequest);
            UsbSamp_DbgPrint(1, ("WdfRequestSend failed with status code 0x%x\n", status));
            WdfVerifierDbgBreakPoint(); // break into the debugger if the registry value is set.
            goto Exit;
        }

        RemoveHeadList(&subRequestsList);
        //
        // Don't touch the subrequest after it has been sent.
        // Make a note that at least one subrequest is sent. This will be used
        // in deciding whether we should free the subrequests in case of failure.
        //
    }



Exit:

    //
    // Checking the status besides the number of list entries will help differentiate
    // failures where everything succeeded vs where there were failures before adding
    // list entries.
    //
    if(NT_SUCCESS(status) && IsListEmpty(&subRequestsList)) {
        //
        // We will let the completion routine to cleanup and complete the
        // main request.
        //
        return;
    }
    else {
        BOOLEAN  completeRequest;

        completeRequest = TRUE;
        
        if(hCollection) {                       
            DoSubRequestCleanup(rwContext, &subRequestsList, &completeRequest);  
        }

        if (completeRequest) {
            
            if (rwContext->Numxfer > 0 ) {
                WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, rwContext->Numxfer);
            }
            else {
               WdfRequestCompleteWithInformation(Request, status, rwContext->Numxfer); 
            }
        }

    }

    UsbSamp_DbgPrint(3, ("PerformHighSpeedIsochTransfer -- ends\n"));
    return;
}

VOID
DoSubRequestCleanup(
    PREQUEST_CONTEXT    MainRequestContext,
    PLIST_ENTRY         SubRequestsList,
    PBOOLEAN            CompleteRequest
    )
{   
    PLIST_ENTRY           thisEntry;
    PSUB_REQUEST_CONTEXT  subReqContext;
    WDFREQUEST            subRequest;
    ULONG                 numPendingRequests;

    *CompleteRequest = TRUE;
    WdfSpinLockAcquire(MainRequestContext->SubRequestCollectionLock);

    while(!IsListEmpty(SubRequestsList)) {
        thisEntry = RemoveHeadList(SubRequestsList);
        subReqContext = CONTAINING_RECORD(thisEntry, SUB_REQUEST_CONTEXT, ListEntry);
        subRequest = WdfObjectContextGetObject(subReqContext);
        WdfCollectionRemove(MainRequestContext->SubRequestCollection, subRequest);

        if(subReqContext->SubMdl) {
            IoFreeMdl(subReqContext->SubMdl);
            subReqContext->SubMdl = NULL;
        }
        
        WdfObjectDelete(subRequest);
    }
    numPendingRequests = WdfCollectionGetCount(MainRequestContext->SubRequestCollection);
    WdfSpinLockRelease(MainRequestContext->SubRequestCollectionLock);

    if (numPendingRequests > 0) {
        *CompleteRequest = FALSE;
    }       
       
    return; 
}

VOID
SubRequestCompletionRoutine(
    IN WDFREQUEST                  Request,
    IN WDFIOTARGET                 Target,
    PWDF_REQUEST_COMPLETION_PARAMS CompletionParams,
    IN WDFCONTEXT                  Context
    )
/*++

Routine Description:

    Completion Routine

Arguments:

    Context - Driver supplied context
    Target - Target handle
    Request - Request handle
    Params - request completion params


Return Value:

    VOID

--*/
{
    PURB                    urb;
    ULONG                   i;
    ULONG                   numPendingRequests;
    NTSTATUS                status;
    PREQUEST_CONTEXT        rwContext;
    WDFREQUEST              mainRequest;
    PSUB_REQUEST_CONTEXT    subReqContext;

    UNREFERENCED_PARAMETER(Target);

    UsbSamp_DbgPrint(3, ("SubRequestCompletionRoutine - begins\n"));

    subReqContext = GetSubRequestContext(Request);

    urb = (PURB) subReqContext->SubUrb;

    IoFreeMdl(subReqContext->SubMdl);
    subReqContext->SubMdl = NULL;

    rwContext = (PREQUEST_CONTEXT) Context;
    ASSERT(rwContext);

    status = CompletionParams->IoStatus.Status;

    if(NT_SUCCESS(status) &&
       USBD_SUCCESS(urb->UrbHeader.Status)) {

        rwContext->Numxfer +=
                urb->UrbIsochronousTransfer.TransferBufferLength;

        UsbSamp_DbgPrint(1, ("rwContext->Numxfer = %d\n", rwContext->Numxfer));
    }
    else {

        UsbSamp_DbgPrint(1, ("read-write irp failed with status %X\n", status));
        UsbSamp_DbgPrint(1, ("urb header status %X\n", urb->UrbHeader.Status));
    }

    for(i = 0; i < urb->UrbIsochronousTransfer.NumberOfPackets; i++) {

        UsbSamp_DbgPrint(4, ("IsoPacket[%d].Length = %X IsoPacket[%d].Status = %X\n",
                            i,
                            urb->UrbIsochronousTransfer.IsoPacket[i].Length,
                            i,
                            urb->UrbIsochronousTransfer.IsoPacket[i].Status));
    }

    //
    // Remove the SubRequest from the collection.
    //
    WdfSpinLockAcquire(rwContext->SubRequestCollectionLock);

    WdfCollectionRemove(rwContext->SubRequestCollection, Request);

    numPendingRequests = WdfCollectionGetCount(rwContext->SubRequestCollection);

    WdfSpinLockRelease(rwContext->SubRequestCollectionLock);

    //
    // If all the sub requests are completed. Complete the main request sent
    // by the user application.
    //
    if(numPendingRequests == 0) {

        UsbSamp_DbgPrint(3, ("no more pending sub requests\n"));

        if(NT_SUCCESS(status)) {

            UsbSamp_DbgPrint(1, ("urb start frame %X\n",
                                urb->UrbIsochronousTransfer.StartFrame));
        }

        mainRequest = WdfObjectContextGetObject(rwContext);

        //
        // if we transferred some data, main Irp completes with success
        //
        UsbSamp_DbgPrint(1, ("Total data transferred = %X\n", rwContext->Numxfer));

        UsbSamp_DbgPrint(3, ("SubRequestCompletionRoutine %s completed\n",
                            rwContext->Read?"Read":"Write"));
        //
        // Mark the main request as not cancelable before completing it.
        // Since we don't complete the Request in our Cancel routine,
        // we don't really have to worry about checking the status for
        // STATUS_CANCELLED.
        //
        (VOID)WdfRequestUnmarkCancelable(mainRequest);
        
        if (rwContext->Numxfer > 0 ) {
            WdfRequestCompleteWithInformation(mainRequest, STATUS_SUCCESS, rwContext->Numxfer);
        }
        else {
            UsbSamp_DbgPrint(3, ("SubRequestCompletionRoutine completiong with failure status %x\n",
                                 CompletionParams->IoStatus.Status));
            WdfRequestCompleteWithInformation(mainRequest, CompletionParams->IoStatus.Status, rwContext->Numxfer); 
        }       
        
    }

    //
    // Since we created the subrequests, we should free it by removing the
    // reference.
    //
    WdfObjectDelete(Request);

    UsbSamp_DbgPrint(3, ("SubRequestCompletionRoutine - ends\n"));

    return;
}

VOID
UsbSamp_EvtRequestCancel(
    WDFREQUEST Request
    )
/*++

Routine Description:

    This is the cancellation routine for the main read/write Irp.

Arguments:


Return Value:

    None

--*/
{
    PREQUEST_CONTEXT        rwContext;
    LIST_ENTRY              cancelList;
    PSUB_REQUEST_CONTEXT    subReqContext;
    WDFREQUEST              subRequest;
    ULONG                   i;
    PLIST_ENTRY             thisEntry;

    UsbSamp_DbgPrint(3, ("UsbSamp_EvtIoCancel - begins\n"));

    rwContext = GetRequestContext(Request);

    if(!rwContext->SubRequestCollection) {
        ASSERTMSG("Very unlikely, collection is created before\
                        the request is made cancellable", FALSE);
        return;
    }

    InitializeListHead(&cancelList);

    //
    // We cannot call the WdfRequestCancelSentRequest with the collection lock
    // acquired because that call can recurse into our completion routine,
    // when the lower driver completes the request in the CancelRoutine,
    // and can cause deadlock when we acquire the collection to remove the
    // subrequest. So to workaround that, we will get the item from the
    // collection, take an extra reference, and put them in the local list.
    // Then we drop the lock, walk the list and call WdfRequestCancelSentRequest and
    // remove the extra reference.
    //
    WdfSpinLockAcquire(rwContext->SubRequestCollectionLock);

    for(i=0; i<WdfCollectionGetCount(rwContext->SubRequestCollection); i++) {


        subRequest = WdfCollectionGetItem(rwContext->SubRequestCollection, i);

        subReqContext = GetSubRequestContext(subRequest);

        WdfObjectReference(subRequest);

        InsertTailList(&cancelList, &subReqContext->ListEntry);
    }

    WdfSpinLockRelease(rwContext->SubRequestCollectionLock);

    while(!IsListEmpty(&cancelList))
    {
        thisEntry = RemoveHeadList(&cancelList);

        subReqContext = CONTAINING_RECORD(thisEntry, SUB_REQUEST_CONTEXT, ListEntry);

        subRequest = WdfObjectContextGetObject(subReqContext);

        UsbSamp_DbgPrint(3, ("Cancelling subRequest 0x%p\n", subRequest));

        if(!WdfRequestCancelSentRequest(subRequest)) {
            UsbSamp_DbgPrint(3, ("WdfRequestCancelSentRequest failed\n"));
        }

        WdfObjectDereference(subRequest);
    }

    UsbSamp_DbgPrint(3, ("UsbSamp_EvtIoCancel - ends\n"));

    return;
}

