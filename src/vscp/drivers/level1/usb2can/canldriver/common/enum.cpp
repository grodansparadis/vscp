// Enum.cpp: implementation of the CEnum class.
//
//////////////////////////////////////////////////////////////////////

#include "Enum.h"

//#define DEBUG_ENUM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEnum::CEnum()
{
  hDev = NULL;
}

CEnum::~CEnum()
{

}

BOOL CEnum::GetDevicePath(
                 IN  LPGUID InterfaceGuid,
                 __out_ecount(BufLen) PCHAR DevicePath,
                 __in size_t BufLen, 
				 __in_ecount(BufLen) PCHAR DeviceSerialNumber
                     )

	{
	 
    HDEVINFO                            	HardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA	          	DeviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA		DeviceInterfaceDetailData = NULL;
	SP_DEVINFO_DATA							dev_info_data = {sizeof (SP_DEVINFO_DATA)};		
    ULONG		Length, RequiredLength = 0;
    BOOL		bResult;

	UINT		temp,temp1;
	BOOL		bEqual = TRUE;
	ULONG       nBytes;	

//	FILE	    *m_flog;
//	CHAR		Product[256];

////////////////////// TMP /////////////////
	UCHAR	found = 0;

//m_flog = fopen( "c:\\usb2can.txt", "w" );


    HardwareDeviceInfo = SetupDiGetClassDevs(
                             InterfaceGuid,
                             NULL,
                             NULL,
                             (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    if (HardwareDeviceInfo == INVALID_HANDLE_VALUE) {


        return FALSE;
    }

    DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);


///////////////////////////////////////////////////////////////////////////////////

  for(temp = 0;temp < MAX_USB_DEVICES;temp++ )

  {

    bResult = SetupDiEnumDeviceInterfaces(HardwareDeviceInfo,
                                              0,
                                              InterfaceGuid,
                                              temp,
                                              &DeviceInterfaceData);

    if (bResult == FALSE) 
	{
	   continue;    
    }

    SetupDiGetDeviceInterfaceDetail(
        HardwareDeviceInfo,
        &DeviceInterfaceData,
        NULL,
        0,
        &RequiredLength,
        NULL
        );

    DeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) LocalAlloc(LMEM_FIXED, RequiredLength);


    if (DeviceInterfaceDetailData == NULL)
	{
        SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);
        return FALSE;
    }

    DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    Length = RequiredLength;

    bResult = SetupDiGetDeviceInterfaceDetail(
                  HardwareDeviceInfo,
                  &DeviceInterfaceData,
                  DeviceInterfaceDetailData,
                  Length,
                  &RequiredLength,
                  &dev_info_data);

    if (bResult == FALSE)
	{
          SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);
          LocalFree(DeviceInterfaceDetailData);
          return FALSE;
    }




//SetupDiGetDeviceRegistryProperty( HardwareDeviceInfo, &dev_info_data, SPDRP_UI_NUMBER_DESC_FORMAT    , NULL, (PBYTE)Product, 253,NULL);
// fputs(Product,m_flog);
// fputs("\n",m_flog);


    hDev = CreateFile (
                  DeviceInterfaceDetailData->DevicePath,
                  GENERIC_READ | GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL, // no SECURITY_ATTRIBUTES structure
                  OPEN_EXISTING, // No special create flags
                  0, // No special attributes
                  NULL); // No template file

    if (INVALID_HANDLE_VALUE == hDev) {
     SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);
     LocalFree(DeviceInterfaceDetailData);
	 hDev = NULL;
     return FALSE;
    }    


    bResult = DeviceIoControl(hDev,
                              IOCTL_USBSAMP_GET_SERIALNUMBER_STRING,
                              0,
                              0,
                              tmpBuf,
                              sizeof(tmpBuf),
                              (PULONG) &nBytes,
                              NULL);

    if (bResult == FALSE) 
	{
     SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);
     LocalFree(DeviceInterfaceDetailData);

	 if(hDev != NULL){
		   CloseHandle( hDev );
		    hDev = NULL;
	 }


     return FALSE;
	}



    bEqual = TRUE;

	
	for(temp1=0;temp1< nBytes/2;temp1++){

		if(DeviceSerialNumber[temp1] != tmpBuf[temp1*2]){		
		     bEqual = FALSE;				 
			 break;
		}
	}

  	if( bEqual == TRUE )
	{
       StringCchCopy(DevicePath,BufLen,DeviceInterfaceDetailData->DevicePath);
       SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);
       LocalFree(DeviceInterfaceDetailData);

       if(hDev != NULL){   
                CloseHandle( hDev );
                hDev = NULL;
	   }

//fclose (m_flog);

       return TRUE;  
	}


       LocalFree(DeviceInterfaceDetailData);

       if(hDev != NULL){
                CloseHandle( hDev );
				hDev = NULL;
	   }

  } // for(...


   SetupDiDestroyDeviceInfoList(HardwareDeviceInfo);

   if(hDev != NULL){
            CloseHandle( hDev );
			hDev = NULL;
			}


   return FALSE;

}


















///////////////////////////////////////////////////////////////////////////////////////
/*
     hDevInfo = SetupDiGetClassDevs (
                          (_GUID* )&GUID_CLASS_USB2CAN_STR75,
                          NULL, // Define no enumerator (global)
                          NULL, // Define no
                           (DIGCF_PRESENT | // Only Devices present
                            DIGCF_INTERFACEDEVICE)); // Function class devices.


     if(hDevInfo == NULL)
	 {
      return false;
	 }

    deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);


///////////////////////////////////////////////////////////////////////////////////////////
//    Einam per interfeisu sarasa
///


	for(temp = 0;temp < 4;temp++)
	{
	   if(! SetupDiEnumDeviceInterfaces(
		                       hDevInfo,
							   0,
							   (_GUID* )&GUID_CLASS_USB2CAN_STR75,
							   temp,
							   &deviceInfoData))

		   continue;


			    SetupDiGetDeviceInterfaceDetail (
											hDevInfo,
											&deviceInfoData,
											NULL, // probing so no output buffer yet
											0, // probing so output buffer length of zero
											&requiredLength,
											NULL); // not interested in the specific dev-node

				predictedLength = requiredLength;
                functionClassDeviceData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc (predictedLength);		                


                if(NULL == functionClassDeviceData) {                        
                    SetupDiDestroyDeviceInfoList (hDevInfo); 
                    return false;
				}

                functionClassDeviceData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

				//
				// Retrieve the information from Plug and Play.
				//
				if (! SetupDiGetDeviceInterfaceDetail (
					                        	hDevInfo,
												&deviceInfoData,
												functionClassDeviceData,
												predictedLength,
												&requiredLength,
												NULL)) {

				    free( functionClassDeviceData );
                    SetupDiDestroyDeviceInfoList (hDevInfo);
					return false;
				}			


                (void) StringCchCopy (completeDeviceName, MAX_LENGTH, functionClassDeviceData->DevicePath);
				(void) StringCchCat (completeDeviceName, MAX_LENGTH, "\\" ); 
				(void) StringCchCat (completeDeviceName, MAX_LENGTH, "PIPE01");

        //sprintf(dbgbuf,functionClassDeviceData->DevicePath);
        StringCchPrintf(dbgbuf,sizeof(dbgbuf),completeDeviceName);
  		fwrite( dbgbuf, 1, strlen( dbgbuf), m_flog );

                  hDataOut = CreateFile (
								completeDeviceName,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL, // no SECURITY_ATTRIBUTES structure
								OPEN_EXISTING, // No special create flags
								0, // No special attributes
								NULL); // No template file

                   if (INVALID_HANDLE_VALUE == hDataOut) {
                      //printf( "FAILED to open %s\n", devName );
					   fputs("\n FAILED to open \n",m_flog);
					   fclose(m_flog);
					}               				


				   WriteFile(hDataOut,"aaa",3,&nBytesWrite,NULL);
               // WriteFile(hWrite, poutBuf, WriteLen,  (PULONG) &nBytesWrite, NULL);
                //printf("<%s> W (%04.4d) : request %06.6d bytes -- %06.6d bytes written\n",
                        //outPipe, i, WriteLen, nBytesWrite);


                  hDev = CreateFile (
								functionClassDeviceData->DevicePath,
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL, // no SECURITY_ATTRIBUTES structure
								OPEN_EXISTING, // No special create flags
								0, // No special attributes
								NULL); // No template file

                   if (INVALID_HANDLE_VALUE == hDataOut) {
                      //printf( "FAILED to open %s\n", devName );
					   fputs("\n FAILED to open \n",m_flog);
					   fclose(m_flog);
					}  



              free( functionClassDeviceData );   

#ifdef DEBUG_USB2CAN_RECEIVE	
        //sprintf(dbgbuf," Device path=%s ",functionClassDeviceData->DevicePath);
        //sprintf(dbgbuf,functionClassDeviceData->DevicePath);
        //StringCchPrintf(dbgbuf,sizeof(dbgbuf),functionClassDeviceData->DevicePath);
		//fwrite( dbgbuf, 1, strlen( dbgbuf), m_flog );
#endif
		              

	}// for( ...


     SetupDiDestroyDeviceInfoList (hDevInfo);

*/