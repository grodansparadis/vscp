/*  RawEthernet Application
 *  Written by: Jeremiah Clark, Oct 2003
 *  This code is opensouce and free for you to use.  All I ask is 
 *  that you give credit in your source comments.
 * 
 *  This application will interface with the NDIS Protocol Driver to enable
 *  the sending of raw ethernet packets out of a network device.
 */

using System;
using System.Runtime.InteropServices;	// For DllImport
using System.Text;						// For Encoding

namespace RawEthernet
{
	// Class to perform the sending of raw ethernet packets
	class RawEthernet
	{
		[STAThread]
		static void Main(string[] args)
		{
			// create a new instance of the RawEthernet Class
			// the constructor will also open a handle to the driver
			RawEthernet rawether = new RawEthernet();

			// check to see if we got a valid handle
			if (rawether.m_bDriverOpen)
			{
				// we have a handle to the device
				Console.WriteLine("Handle to NDIS Driver: " + rawether.Handle.ToString());
			}
			else
			{
				// we don't have a handle so alert user and quit
				Console.WriteLine("ERROR: NDIS Driver could not be opened\n");
				Console.WriteLine("Press enter to exit...");
				Console.ReadLine();
				return;
			}

			// OK, now we have a handle to the driver, so let's get a list of the
			// adapters that are using that driver
			AdaptersInfo[] aiAdapters = rawether.EnumerateAdapters();
			// List the adapters so that we can choose which one we want to use
			Console.WriteLine("\nThe following adapters can be used to send packets:");
			foreach (AdaptersInfo ai in aiAdapters)
			{ 
				Console.WriteLine("\t" + ai.ToString());
			}
			// Ask the user to choose the index of the adapter that they want to send from
			Console.Write("\nChoose the adapter number that you want to send from: ");
			// Get the value that they type
			int adIndex = Convert.ToInt32(Console.ReadLine());
			

			// ok, now bind that adapter to the driver handle that we have open
			if (!rawether.BindAdapter(aiAdapters[adIndex].AdapterID))
			{
				// alert user and quit
				Console.WriteLine("ERROR: Could not bind to the adapter.\n");
				Console.WriteLine("Press enter to exit...");
				Console.ReadLine();
				return;
			}
			else
			{
				// tell the user that we are bound to the adapter
				Console.WriteLine("\nAdapter " + aiAdapters[adIndex].AdapterID +
					" is ready for writing...\n");
			}

			// ok, now we are ready to write a raw packet on the adapter
			// create a generic raw packet (must be at least 16 bytes long)
			byte[] packet = new byte[] {0xff,0xff,0xff,0xff,0xff,0xff,  //destination mac
										0x00,0x02,0x3e,0x4c,0x49,0xaa,  //source mac
										0x08,0x00,					    //protocol
										0x01,0x02,0x03,0x04,0x05,0x06}; //generic data
			
			// write the packet to the device a few times
			rawether.DoWrite(packet);
			rawether.DoWrite(packet);
			rawether.DoWrite(packet);
			
			// Prompt user for exit
			Console.WriteLine("\nPress enter to exit...");
			Console.ReadLine();

			// Close the drive an associated resources
			rawether.CloseDriver();
			rawether = null;
		}



		#region ATTRIBUTES
			
			// Path to the NDIS Protocol Driver so we can open it like a file
			private string m_sNdisProtDriver = "\\\\.\\\\NdisProt";

			// IntegerPointer to hold the handle of the driver
			private IntPtr m_iHandle = IntPtr.Zero;

			// Bool to hold whether we have a connection to the driver
			private bool m_bDriverOpen = false;

			// Bool to hold whether we are bound to an adapter
			private bool m_bAdapterBound = false;

		#endregion ATTIBUTES

		#region PROPERTIES

			// public properties for the class
			public IntPtr Handle { get { return this.m_iHandle; } }
			public bool IsDriverOpen { get { return this.m_bDriverOpen; } }
			public bool IsAdapterBound { get { return this.m_bAdapterBound; } }

		#endregion PROPERTIES

		#region CONSTRUCTOR

			public RawEthernet()
			{
				// Open a handle to the NDIS Device driver
				this.m_bDriverOpen = this.OpenDriver();
			}

		#endregion CONSTRUCTOR

		#region METHODS

			// method to open a handle to the driver so we can access it
			// returns true if we get a valid handle, false if otherwise
			private bool OpenDriver()
			{
				// User the CreateFile API to open a handle to the file
				this.m_iHandle = CreateFile(this.m_sNdisProtDriver, 
					GENERIC_WRITE|GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

				// Check to see if we got a valid handle
				if ((int)this.m_iHandle <= 0)
				{
					// If not, then return false and reset the handle to 0
					this.m_iHandle = IntPtr.Zero;
					return false;
				}

				// Otherwise we have a valid handle
				return true;
			}


			// method to return an array of the active Network adapters on your system
			private AdaptersInfo[] EnumerateAdapters()
			{
				int adapterIndex = 0;		// the current adapter index
				bool validAdapter = true;	// are we still getting a valid adapter

				// we are going to look for up to 10 adapters
				// temp array to hold the adapters that we find
				AdaptersInfo[] aiTemp = new AdaptersInfo[10]; 

				//start a loop while we look for adapters one by one starting at index 0
				do
				{
					// buffer to hold the adapter information that we get
					byte[] buf = new byte[1024];
					// uint to hold the number of bytes that we read
					uint iBytesRead = 0;
					// NDISPROT_QUERY_BINDING structure containing the index
					// that we want to query for
					NDISPROT_QUERY_BINDING ndisprot = new NDISPROT_QUERY_BINDING();
					ndisprot.BindingIndex = (ulong)adapterIndex;
					// uint to hold the length of the ndisprot
					uint bufsize = (uint)Marshal.SizeOf(ndisprot);	
					// perform the following in and unsafe context
					unsafe
					{
						// create a void pointer to buf
						fixed (void* vpBuf = buf)
						{
							// use the DeviceIoControl API to query the adapters
							validAdapter = DeviceIoControl(this.m_iHandle,
								IOCTL_NDISPROT_QUERY_BINDING, (void*)&ndisprot, 
								bufsize, vpBuf, (uint)buf.Length, 
								&iBytesRead, 0);
						}
					}
					// if DeviceIoControl returns false, then there are no
					// more valid adapters, so break the loop
					if (!validAdapter) break;

					// add the adapter information to the temp AdaptersInfo struct array
					// first, get a string containing the info from buf
					string tmpinfo = Encoding.Unicode.GetString(buf).Trim((char)0x00);
					tmpinfo = tmpinfo.Substring(tmpinfo.IndexOf("\\"));
					// add the info to aiTemp
					aiTemp[adapterIndex].Index = adapterIndex;
					aiTemp[adapterIndex].AdapterID = tmpinfo.Substring(0,
						tmpinfo.IndexOf("}")+1);
					aiTemp[adapterIndex].AdapterName = tmpinfo.Substring(
						tmpinfo.IndexOf("}")+1).Trim((char)0x00);
					
					// Increment the adapterIndex count
					adapterIndex++;

				// loop while we have a valid adapter
				}while (validAdapter || adapterIndex < 10);	
				
				// Copy the temp adapter struct to the return struct
				AdaptersInfo[] aiReturn = new AdaptersInfo[adapterIndex];
				for (int i=0;i<aiReturn.Length;i++)
					aiReturn[i] = aiTemp[i];

				// return aiReturn struct
				return aiReturn;
			}


			// method to bind an adapter to the a the handle that we have open
			private bool BindAdapter(string adapterID)
			{
				// char array to hold the adapterID string
				char[] ndisAdapter = new char[256];
				// convert the string to a unicode non-localized char array
				int iNameLength = 0, i = 0;
				for (i=0;i<adapterID.Length;i++)
				{
					ndisAdapter[i] = adapterID[i];
					iNameLength++;
				}
				ndisAdapter[i] = '\0';
				
				// uint to hold the number of bytes read from DeviceIoControl
				uint uiBytesReturned;
			
				// do the following in an unsafe context
				unsafe 
				{
					// create a void pointer to ndisAdapter
					fixed (void* vpNdisAdapter = &ndisAdapter[0])
					{
						// Call the DeviceIoControl API to bind the adapter to the handle
						return DeviceIoControl(this.m_iHandle, IOCTL_NDISPROT_OPEN_DEVICE,
							vpNdisAdapter, (uint)(iNameLength*sizeof(char)), null, 0, 
							&uiBytesReturned, 0);
					}
				}
			}


			// method to close the handle to the device driver
			private bool CloseDriver()
			{
				return CloseHandle(this.m_iHandle);
			}


			// method to write a packet of bytes to the adapter
			private bool DoWrite(byte[] packet)
			{
				// uint to hol the number of bytes sent
				uint uiSentCount = 0;
				// bool to hold whether the packet was sent or not
				bool packetSent = false;

				// used an unsafe context
				unsafe
				{
					// set a void pointer to the packet buffer
					fixed (void* pvPacket = packet)
					{
						packetSent = WriteFile(this.m_iHandle, pvPacket, (uint)packet.Length,
							&uiSentCount, 0);
					}
				}

				// check to see if packet was sent
				if (!packetSent)
				{
					Console.WriteLine("ERROR: Packet not sent: 0 bytes written");
					return false;
				}

				// otherwise the packet was sent
				Console.WriteLine("Packet sent: " + uiSentCount.ToString() + "bytes written");
				return true;
			}


		#endregion METHODS

		#region CONSTANTS

			// file access constants
			private const uint GENERIC_READ  = 0x80000000;
			private const uint GENERIC_WRITE = 0x40000000;
			
			// file creation disposition constant
			private const uint OPEN_EXISTING = 0x00000003;

			// file attributes constant
			private const uint FILE_ATTRIBUTE_NORMAL = 0x00000080;

			// invalid handle constant
			private const int INVALID_HANDLE_VALUE = -1;

			// iocontrol code constants
			private const uint IOCTL_NDISPROT_QUERY_BINDING = 0x12C80C;
			private const uint IOCTL_NDISPROT_OPEN_DEVICE   = 0x12C800;

		#endregion CONSTANTS

		#region IMPORTS

			[DllImport("kernel32", SetLastError=true)]
			private static extern IntPtr CreateFile(
				string _lpFileName,				// filename to open
				uint _dwDesiredAccess,			// access permissions for the file
				uint _dwShareMode,				// sharing or locked
				uint _lpSecurityAttributes,		// security attributes
				uint _dwCreationDisposition,	// file creation method (new, existing)
				uint _dwFlagsAndAttributes,		// other flags and sttribs
				uint _hTemplateFile);			// template file for creating

			[DllImport("kernel32", SetLastError=true)]
			private static extern unsafe bool WriteFile(
				IntPtr _hFile,					// handle of the file to write to
				void* _lpBuffer,				// pointer to the buffer to write
				uint _nNumberOfBytesToWrite,	// number of bytes to write from the buffer
				uint* _lpNumberOfBytesWritten,	// [out] number of bytes written to the file
				uint _lpOverlapped);			// used for async reading and writing

			[DllImport("kernel32", SetLastError=true)]
			private static extern bool CloseHandle(
				IntPtr _hObject);				// handle for the object to close

			[DllImport("kernel32", SetLastError=true)]
			private static extern unsafe bool DeviceIoControl(
				IntPtr _hDevice,				// handle of the device
				uint _dwIoControlCode,			// IO control code to execute
				void* _lpInBuffer,				// Input buffer for the execution
				uint _nInBufferSize,			// size of the input buffer
				void* lpOutBuffer,				// [out] output buffer for the execution
				uint _nOutBufferSize,			// [size of the output buffer
				uint* _lpBytesReturned,			// [out] number of bytes returned
				uint _lpOverlapped);			// used for async reading and writing

		#endregion IMPORTS

		#region STRUCTS

		    [StructLayout(LayoutKind.Sequential)]
			private struct NDISPROT_QUERY_BINDING
			{
				public ulong BindingIndex;        // 0-based binding number
				public ulong DeviceNameOffset;    // from start of this struct
				public ulong DeviceNameLength;    // in bytes
				public ulong DeviceDescrOffset;    // from start of this struct
				public ulong DeviceDescrLength;    // in bytes
			}

		#endregion STRUCTS
	}
	
	// Structure to hold the information for a specific adapter
	public struct AdaptersInfo
	{
		#region ATTRIBUTES

			private int m_iIndex;			// The index of the adapter
			private string m_sAdapterID;	// The ID of the adapter
			private string m_sAdapterName;	// The name of the adapter

		#endregion ATTRIBUTES

		#region PROPERTIES

			public int Index {get{return m_iIndex;}set{m_iIndex = value;}}
			public string AdapterID {get{return m_sAdapterID;}set{m_sAdapterID = value;}}
			public string AdapterName {get{return m_sAdapterName;}set{m_sAdapterName = value;}}

		#endregion PROPERTIES

		#region CONSTRUCTOR

			// The constructor for this struct accepts three arguments
			//  index = the adapter index
			//  adapterID = the ID of the adapter
			//  adapterName = the name of the adapter
			public AdaptersInfo(int index, string adapterID, string adapterName)
			{
				// set the attributes according to the passed args
				this.m_iIndex = index;
				this.m_sAdapterID = adapterID;
				this.m_sAdapterName = adapterName;
			}

		#endregion CONSTRUCTOR

		#region METHODS

		public override string ToString()
		{
			return this.m_iIndex + ". " + this.m_sAdapterID + " - " + this.m_sAdapterName;
		}


		#endregion METHODS
	}
}
