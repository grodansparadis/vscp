// bttest.cpp : Defines the entry point for the console application.
//

#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <bthdef.h>
#include <BluetoothAPIs.h>

#pragma comment(lib, "Irprops.lib")

BLUETOOTH_FIND_RADIO_PARAMS m_bt_find_radio = {
  sizeof(BLUETOOTH_FIND_RADIO_PARAMS)
};

BLUETOOTH_RADIO_INFO m_bt_info = {
  sizeof(BLUETOOTH_RADIO_INFO),
  0,
};

BLUETOOTH_DEVICE_SEARCH_PARAMS m_search_params = {
  sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
  1,
  0,
  1,
  1,
  1,
  15,
  NULL
};

BLUETOOTH_DEVICE_INFO m_device_info = {
  sizeof(BLUETOOTH_DEVICE_INFO),
  0,
};

HANDLE m_radio = NULL;
HBLUETOOTH_RADIO_FIND m_bt = NULL;
HBLUETOOTH_DEVICE_FIND m_bt_dev = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	  while(true) {
    m_bt = BluetoothFindFirstRadio(&m_bt_find_radio, &m_radio);
    
    int m_radio_id = 0;
    do {
      m_radio_id++;

      BluetoothGetRadioInfo(m_radio, &m_bt_info);

      wprintf(L"Radio %d:\r\n", m_radio_id);
      wprintf(L"\tName: %s\r\n", m_bt_info.szName);
      wprintf(L"\tAddress: %02x:%02x:%02x:%02x:%02x:%02x\r\n", m_bt_info.address.rgBytes[1], m_bt_info.address.rgBytes[0], m_bt_info.address.rgBytes[2], m_bt_info.address.rgBytes[3], m_bt_info.address.rgBytes[4], m_bt_info.address.rgBytes[5]);
      wprintf(L"\tClass: 0x%08x\r\n", m_bt_info.ulClassofDevice);
      wprintf(L"\tManufacturer: 0x%04x\r\n", m_bt_info.manufacturer);

      m_search_params.hRadio = m_radio;

      ::ZeroMemory(&m_device_info, sizeof(BLUETOOTH_DEVICE_INFO));
      m_device_info.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

      m_bt_dev = BluetoothFindFirstDevice(&m_search_params, &m_device_info);

      if ( NULL == m_bt_dev ) {
          wprintf( L"\t\tNo Bluetooth device detected.\r\n" );
          Sleep( 1000 );
          continue;
      }

      int m_device_id = 0;
      do {
        m_device_id++;

        wprintf(L"\tDevice %d:\r\n", m_device_id);
        wprintf(L"\t\tName: %s\r\n", m_device_info.szName);
        wprintf(L"\t\tAddress: %02x:%02x:%02x:%02x:%02x:%02x\r\n", m_device_info.Address.rgBytes[1], m_device_info.Address.rgBytes[0], m_device_info.Address.rgBytes[2], m_device_info.Address.rgBytes[3], m_device_info.Address.rgBytes[4], m_device_info.Address.rgBytes[5]);
        wprintf(L"\t\tClass: 0x%08x\r\n", m_device_info.ulClassofDevice);
        wprintf(L"\t\tConnected: %s\r\n", m_device_info.fConnected ? L"true" : L"false");
        wprintf(L"\t\tAuthenticated: %s\r\n", m_device_info.fAuthenticated ? L"true" : L"false");
        wprintf(L"\t\tRemembered: %s\r\n", m_device_info.fRemembered ? L"true" : L"false");

      } while(BluetoothFindNextDevice(m_bt_dev, &m_device_info));

      BluetoothFindDeviceClose(m_bt_dev);

    } while(BluetoothFindNextRadio(&m_bt_find_radio, &m_radio));

    BluetoothFindRadioClose(m_bt);

    Sleep(1000);
  }

  return 0;
}

