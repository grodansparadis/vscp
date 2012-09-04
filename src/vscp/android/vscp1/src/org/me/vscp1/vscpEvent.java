/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.me.vscp1;

/**
 *
 * @author ahedman1
 */
public class vscpEvent {
	long m_crc;          // crc checksum - currently only used for UDP and RF
	byte[] m_data;        // Pointer to data. Max 487 (512- 25) bytes
        // Following two are for daemon internal use
	long m_obid;          // Used by driver for channel info etc.
	long m_timestamp;     // Relative time stamp for package in microseconds
        // CRC should be calculated from
        // here to end + datablock
	short m_head;         // Bit 16 GUID is IP v.6 address.
                              // bit 765 prioriy, Priority 0-7 where 0 is highest.
                              // bit 4 = hardcoded, true for a hardcoded device.
                              // bit 3 = Dont calculate CRC, false for CRC usage.
                              // bit 2 = Set means this is CAN message.
                              // bit 1 = If bit 2 is set; Extended CAN message if set
                              // bit 0 = If bit 2 is set: Remote frame if set.
	short m_vscp_class;   // VSCP class
	short m_vscp_type;    // VSCP type
	GUID m_guid;        // Node address MSB(0) -> LSB(15)
	short m_sizeData;     // Number of valid data bytes

        /// Constructor
        public vscpEvent() {
            m_data = new byte[ 487 ];
        }
}
