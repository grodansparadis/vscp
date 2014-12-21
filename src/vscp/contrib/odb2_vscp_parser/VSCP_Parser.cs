/*
 * Created by SharpDevelop.
 * User: mrx23dot
 * Date: 2014.06.25.
 * Time: 19:19
 *
 * To change this template use Tools | Options | Coding | Edit Standard Headers.
 */
using System;
using System.Collections;

namespace VSCPLib
{
  /// <summary>
  /// Description of VSCP_Parser.
  /// </summary>
  public class VSCP_Parser
  {
    public VSCP_Parser()
    {
    }

/** ELM327 init commands:
      UInt16 DELAY = 50;
      SendMessage("dummy\r");
      Thread.Sleep(DELAY);
      SendMessage("AT WS\r");
      Thread.Sleep(DELAY);
      SendMessage("AT E0\r");
      Thread.Sleep(DELAY);
      SendMessage("AT SP9\r");
      Thread.Sleep(DELAY);
      SendMessage("AT H1\r");
      Thread.Sleep(DELAY);
      SendMessage("AT D1\r");
      Thread.Sleep(DELAY);
      SendMessage("AT CAF0\r");
      Thread.Sleep(DELAY);
      SendMessage("AT FE\r");
      Thread.Sleep(DELAY);
      SendMessage("AT MA\r");
      */
    public void ProcessFrame(string frame) {
      /* Some sanity check */
      byte HDR_LEN = 8; /* nibble */
      byte PAYLOAD_MAXLEN = 8; /* nibble */
      byte PAYLOAD_LEN_FIELD_IDX = HDR_LEN; /* 0 based, in nibble */
      byte PAYLOAD_LEN_FIELD_SIZE = 1; /* in nibble */
      byte PAYLOAD_IDX = (byte)(PAYLOAD_LEN_FIELD_IDX + PAYLOAD_LEN_FIELD_SIZE);
      frame = frame.Replace(" ", "").Trim(); /* safety */

      /* check min length */
      if ((frame.Length >= HDR_LEN + PAYLOAD_LEN_FIELD_SIZE) &&
          frame.Length <= (HDR_LEN+PAYLOAD_MAXLEN+PAYLOAD_LEN_FIELD_SIZE) ) {
        byte BYTE_LEN = 2; /* in nibble */
        string Header = frame.Substring(0, HDR_LEN);
        string Payload = frame.Substring(PAYLOAD_IDX);
        byte Payload_len = (byte) (frame.Substring(PAYLOAD_IDX).Length / BYTE_LEN);

        if (((frame.Length - HDR_LEN) - PAYLOAD_LEN_FIELD_SIZE == Payload_len * BYTE_LEN)
            && Payload.Length % 2 == 0) {
          System.Diagnostics.Debug.WriteLine(Header);
          System.Diagnostics.Debug.WriteLine(Payload);

          VSCP_Packet vscp_packet1 = new VSCP_Packet();
          vscp_packet1.orig_addr = Convert.ToByte(Header.Substring(Header.Length - BYTE_LEN, BYTE_LEN), 16);
          vscp_packet1.Type = Convert.ToByte(Header.Substring(2*BYTE_LEN, BYTE_LEN), 16);
          vscp_packet1.Class = (ushort)(Convert.ToUInt16(Header.Substring(0, 2*BYTE_LEN), 16) & (UInt16)511);
          vscp_packet1.Hard_coded = (Convert.ToBoolean(( Convert.ToByte(Header.Substring(0, BYTE_LEN), 16)>>1) & 1));
          vscp_packet1.Priority = (byte)((Convert.ToByte(Header.Substring(0, BYTE_LEN), 16)>>2) & 7);
          vscp_packet1.Payload_Len = Payload_len;


          string oneByte_hex = "";
          byte idx = 0;
          for (uint i = 0; i < Payload.Length; i++) {
            oneByte_hex += Payload.ToCharArray().GetValue(i);
            if (oneByte_hex.Length == 2){
              vscp_packet1.Payload[idx] = Convert.ToByte(oneByte_hex, 16);
              oneByte_hex = "";
              idx++;
            }
          }



          System.Diagnostics.Debug.WriteLine(vscp_packet1.Print());

          VSCP_Classes class1 = new VSCP_Classes();
          System.Diagnostics.Debug.WriteLine(class1.getLev1_ClassName(vscp_packet1.Class));

          /*http://www.mono-project.com/Mono_DataConvert */
          //	SendVSCP(vscp_packet1);
        }
      }

      //   VSCP_ZoneDef z1 =  new VSCP_ZoneDef();
      //   System.Diagnostics.Debug.WriteLine(z1.Category_To_Type(3));

    }


    public void SendVSCP(VSCP_Packet vscp_packet) {
      string Header_out = Convert.ToUInt16(vscp_packet.Priority << 10 | Convert.ToUInt16(vscp_packet.Hard_coded)<<9 | vscp_packet.Class).ToString("X4");
      Header_out += vscp_packet.Type.ToString("X2");
      Header_out += vscp_packet.orig_addr.ToString("X2");
      System.Diagnostics.Debug.WriteLine(Header_out);
    }


  }
}
