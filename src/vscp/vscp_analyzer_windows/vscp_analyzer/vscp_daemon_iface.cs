using System;
using System.Collections.Generic;
using System.Text;
using tcp_lib;

namespace vscp_analyzer
{

    class vscp_daemon_iface
    {

        //System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
        //        byte[] ddd = encoding.GetBytes("vers \r\n");


        
        // string tempString = Encoding.ASCII.GetString(c);

        //Encoding.ASCII.GetBytes(





        private string ReadTCP()
        {
            string rc = "";
            int max = 0;
            byte[] data;

            do
            {
                data = tcp.Read(150, 100);
                if(data != null)
                    rc += Encoding.ASCII.GetString(data);

                ++max;
            } while (!rc.Contains("OK"));


            int pos = rc.IndexOf("OK");

            while (!rc.Substring(pos + 2).Contains("\r\n"))
            {
                data = tcp.Read(150, 100);
                if (data != null)
                    rc += Encoding.ASCII.GetString(data);
            }



            rc = rc.Replace("\0","");

            return rc;



        }


        tcpcli tcp = new tcpcli(null);


        public void Close()
        {
            tcp.Close();
            
        }


        public bool Open(string ip, int port)
        {
            bool rc = tcp.OpenServer(ip, port, false);
            if (rc)
               tcp.Read(300, 100);  // dummy read welcome msg

            return rc;
        }


        public bool Login(string user, string pass)
        {

            string pom = "USER " + user + "\r\n";

            tcp.Send(Encoding.ASCII.GetBytes("USER " + user + "\r\n")); //  + "PASS" + pass + "\r\n"));
            string rc = Encoding.ASCII.GetString(tcp.Read(100, 100));

            if (!rc.Contains("+OK"))
                return false;


            tcp.Send(Encoding.ASCII.GetBytes("PASS " + pass + " \r\n")); 
           
            rc = Encoding.ASCII.GetString(tcp.Read(100, 100));
            pom = "PASS " + pass + "\r\n";


            if (!rc.Contains("+OK"))
                return false;



            return true;
        }


        public vscp_message_d[] GetMsg(int count)
        {
            if (!tcp.isConnected | (count == -1))
                return null;

            vscp_message_d[] msg = new vscp_message_d[count];

            for (int i = 0; i < count; i++)
                msg[i] = new vscp_message_d();

            


            tcp.Send(Encoding.ASCII.GetBytes("RETR " + count.ToString() +"\r\n"));


            string rc = ReadTCP();





            //   head  class  type  obid   timestamp                           guid
            //   112,  20,    9,    1,    1884336112, FF:FF:FF:FF:FF:FF:FF:FE:7A:79:05:4C:00:01:00:01,  0, 0, 1
            //     0    1     2     3          4                                     5                  6  7  8 

             if (!rc.Contains("+OK"))
                return null;


            rc = rc.Replace("\r", "");

            string[] data = rc.Split('\n');


            for (int i = 0; i < count; i++)
            {

                if (data[i].Contains("+OK"))
                    break;

                string[] val = data[i].Split(',');

                msg[i].vclass = Convert.ToUInt32(val[1]);
                msg[i].vtype = Convert.ToUInt32(val[2]);
                msg[i].timestamp = Convert.ToUInt32(val[4]) / 1000;
                msg[i].oaddr = Convert.ToUInt32(val[3]);

                uint pom = Convert.ToUInt32(val[0]);

                msg[i].priority = (pom & 0xE) >> 5;

                // TODO if((head & 0xE)


                pom = (uint)val.Length - 6;

                if (pom > 0)
                {
                    msg[i].data_len = pom;

                    msg[i].data = new byte[pom];

                    for (int x = 0; x < pom; x++)
                    {
                        msg[i].data[x] = Convert.ToByte(val[6 + x], 10);
                    }
                }
            }

            return msg;
        }


        public int GetNumMsg()
        {

            if (!tcp.isConnected)
                return -1;


            tcp.Send(Encoding.ASCII.GetBytes("CDTA\r\n"));

            string rc = ReadTCP();

            if (!rc.Contains("+OK"))
                return -1;
            rc = rc.Replace("\r", "");

            string[] pom = rc.Split('\n');

            return System.Convert.ToInt32(pom[0]);
        }


        public bool SendMessage(vscp_message_d msg)
        {

            string data = "SEND 0," + msg.vclass.ToString() + "," + msg.vtype.ToString() + "," + msg.oaddr.ToString() + ",0,-,";


            for (int i = 0; i < msg.data.Length; i++)
            {
                data += msg.data[i].ToString();
                if (i != msg.data_len - 1)
                    data += ",";
            }

            data += "\r\n";

            tcp.Send(Encoding.ASCII.GetBytes(data));

            data = ReadTCP();


            if (!data.Contains("+OK"))
                return false;



            return true;


        }

    }







    public class vscp_message_d
    {
        public uint priority = 0;
        public bool hardcoded = false;
        public bool dont_crc = false;
        public UInt32 timestamp = 0;
        public uint vclass = 0;
        public uint vtype = 0;
        public byte[] data;
        public uint data_len = 0;
        public uint oaddr = 0;

    }

}
