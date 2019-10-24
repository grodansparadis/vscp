/*
 * Copyright (c) 2001, Bc. Jiri Kubias, Addat s.r.o, www.addat.cz
 * All rights reserved.


 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
   
 *  * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
 
 * * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

 * * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Xml;

using Peak.Can.Basic;
using TPCANHandle = System.Byte;


namespace vscp_analyzer
{
    public delegate void delSerialMsg(string data);

    public partial class MainForm : Form
    {

        delSerialMsg del;
        string nazev_aplikace = "VSCP Analyzator";

        XmlDocument myXml = new XmlDocument();
        XmlReader reader;
        XmlNode node_vscp;
        XmlNodeList nodeList_utils;
        XmlNodeList nodeList_class;

        vscp_daemon_iface vscp;


        /// <summary>
        /// Saves the handle of a PCAN hardware
        /// </summary>
        private TPCANHandle m_PcanHandle = 0;



        enum CONNECTON_TYPES
        {
            USB = 0,
            PEAK,
            VSCP
        }


        public MainForm()
        {
            InitializeComponent();
        }


        string buffer = "";


        public void recieveCANMessagePEAK(TPCANMsg msg, TPCANTimestamp time)
        {
            string id = msg.ID.ToString(); //can_msg[2].Remove(0, 3);
           // string time = time; //Convert.ToInt32(can_msg[1]).ToString();
            string lengt = msg.LEN.ToString(); //can_msg[3].Remove(0, 2);

            string bytes = "";

            for (int i = 0; i < msg.LEN; i++)
            {
                bytes += Convert.ToString(msg.DATA[i], 16);

                if (i != msg.LEN - 1)
                    bytes += ",";
            }




            update_can_log(id, lengt, bytes, time.millis.ToString());
            update_vscp_mon(Convert.ToInt32(msg.ID), lengt, bytes, time.millis.ToString());

        }
        // this functions recieved data over delagat from RS232 link
        public void recieveCANMessage(string data)
        {
            //todo

            buffer = buffer + data;

            string[] rows = buffer.Split('\n');

            foreach (string line in rows){
                
                if (!line.StartsWith("R:"))
                    continue;

                if (!line.EndsWith(";"))
                {
                    buffer = line;
                    return;
                }
                    
                string[] can_msg = line.Split(' ');
                string bytes = "";


                for(int i = 0; i < can_msg.Length - 6; ++i)
                {
                    bytes += can_msg[5+i] + " ";
                }

                string id = can_msg[2].Remove(0, 3);
                string time = Convert.ToInt32(can_msg[1]).ToString();
                string lengt = can_msg[3].Remove(0, 2);

                update_can_log(id, lengt, bytes, time);
                update_vscp_mon(System.Convert.ToInt32(id, 16), lengt, bytes, time);
            }           
        }


        private void update_can_log(string id, string lengt, string bytes, string time)
        {
            DataGridViewRow row;

            for (int i = 0; i < canLog.Rows.Count; ++i)
            {
                row = canLog.Rows[i];

                string row_pom = row.Cells[1].Value.ToString();

                if (row_pom != id)
                    continue;

                row_pom = row.Cells[2].Value.ToString();

                if (row_pom != lengt)
                    continue;

                row_pom = row.Cells[3].Value.ToString();

                if (row_pom != bytes)
                    continue;
                

                int val = (System.Convert.ToInt32(time) - System.Convert.ToInt32(row.Cells[0].Value));

                if (val != 0)
                    canLog.Rows[i].Cells[4].Value = val.ToString();

                canLog.Rows[i].Cells[0].Value = time;
                return;
            }
                canLog.Rows.Add(time, id, lengt, bytes, "0");       // if not found, store as new message
        }




        /*
         Byte 0 is the data coding byte for all measurement packages. The default unit
        has bits 0,1,2,3 set to 0000 and the first optional unit 0001 and so on. It also
        have a eld for the item ( if more than one sensor is controlled by the node)
        that the value belongs to.
        All events in this class are mirrored in Class=60 (0x3c) as oating point
        values with the defautlt unit.
        12.4.1 Type = 0 (0x00) Unde
         
        
         * */


        string number_reformat(string data)
        {

            string[] pdata = data.Split(',');

            byte[] vals = new byte[pdata.Length];


            for (int i = 0; i < pdata.Length; i++)
                vals[i] = Convert.ToByte(pdata[i], 16);


            int encoding = vals[0] & 0xE0 ;


            switch (encoding)
            {
                case 0x40:                  // FIXME ----------------


                case 0x80:      // Normalized Integer Format

                    double mul = 1;
                    double div = 1;

                    if ((vals[1] & 0x80) == 0)  // nasobime
                        mul = Math.Pow(10.0, (double)(vals[1] & 0x7F));
                    else
                        div = Math.Pow(0.1, (double)(vals[1] & 0x7F));

                    int pom = vals.Length - 2;

                    switch (pom)
                    {
                        case 0:
                            return data;

                        case 1:
                            return ((double)((int)vals[2]) * mul * div).ToString();

                        case 2:
                            return ((double)((Int16)(((int)vals[2] << 8) | ((int)vals[3]))) * mul * div).ToString();

                    }


                        break;

                default:
                    return data;

            }




            return null;
        }




        private void update_vscp_mon(Int32 id, string lenght, string data, string time)
        {

            //int id = System.Convert.ToInt32(id_s,16); 
            int priority_i = (id >> 26) & 0x7;
            int nickname_i = id & 0xFF;
            int class_i = (id >> 16) & 0xFF;
            int type_i = (id >> 8) & 0xFF;
            bool hardmask_b = System.Convert.ToBoolean((id >> 25) & 1);

            string class_s = "";
            string type_s = "";
            class_s = getClassByInt(class_i);
            type_s = getTypeByClassIntTypeInt(class_i, type_i);



            if (class_s == "MEASUREMENT")       // measurement
                switch (type_s)
                {
                    case "TEMPERATURE":         // temperature
                        data = reformatMEasTemp(data);
                        break;

                }



            

            for (int i = 0; i < vscpLog.Rows.Count; ++i)
            {

                DataGridViewRow row = vscpLog.Rows[i];

                if ((row.Cells[1].Value.ToString() == priority_i.ToString()) &&
                    (row.Cells[2].Value.ToString() == hardmask_b.ToString()) &&
                    (row.Cells[3].Value.ToString() == class_s) &&
                    (row.Cells[4].Value.ToString() == type_s) &&
                    (row.Cells[5].Value.ToString() == nickname_i.ToString()) &&
                    (row.Cells[6].Value.ToString() == lenght) &&
                    (row.Cells[7].Value.ToString() == data))
                {
                    int x = (System.Convert.ToInt32(time) - System.Convert.ToInt32(vscpLog.Rows[i].Cells[0].Value));
                    
                    if (x == 0)
                    {
                        vscpLog.Rows[i].Cells[0].Value = time;
                        return;
                    }

                    vscpLog.Rows[i].Cells[8].Value = (System.Convert.ToInt32(time) - System.Convert.ToInt32(vscpLog.Rows[i].Cells[0].Value)).ToString();
                    
                    vscpLog.Rows[i].Cells[0].Value = time;
                    return;
                }


            }

            vscpLog.Rows.Add(time, priority_i.ToString(), hardmask_b, class_s, type_s, nickname_i.ToString(),
                lenght, data, "0");



        }

        private string reformatMEasTemp(string data)
        {

            string[] x = data.Split(',');
            string pom = "";

            int val = Convert.ToInt16(x[0], 16);

            pom = "pos: " +  (val & 0x7).ToString() + " temp: ";


            pom += number_reformat(data);




            val = Convert.ToInt16(x[0],16);
            val >>=  3;
            val &= 0x3;

            switch(val)
            {
                case 1:
                    pom += "°C";
                    break;

                case 2:
                    pom += "°F";
                    break;
            }


            return pom;
        }





        // This is called when new data is recieved in RS232 
        private void serial_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            del = new delSerialMsg(this.recieveCANMessage);
            this.BeginInvoke(del, new object[] { serial.ReadExisting() });  // push the data do main thread
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            
            cbDevice.SelectedIndex = 2;
            tbNickname.Text = "0";
            foreach (string s in System.IO.Ports.SerialPort.GetPortNames())
            {
                cbPort.Items.Add(s);
            }

            if (cbPort.Items.Count > 0)
                cbPort.SelectedIndex = 0;

            btClose.Enabled = false;

            reader = new XmlTextReader("vscp.xml");
            myXml.Load(reader);

            foreach (XmlNode node in myXml.ChildNodes)
            {
                if (node.Name == "VSCP")
                {
                    node_vscp = node;
                }
            }

            foreach (XmlNode node in node_vscp)
            {
                if (node.Name == "utils")
                {
                    nodeList_utils = node.ChildNodes;
                }

                if (node.Name == "class")
                {
                    nodeList_class = node.ChildNodes;
                }
            }

            foreach (XmlNode node in nodeList_class)
            {
                if (node.Name == "#comment")
                    continue;

                cbClass.Items.Add(node.Name.ToUpper());
            }

            if (cbClass.Items.Count > 0)
                cbClass.SelectedIndex = 0;


            foreach (XmlNode node in nodeList_utils)
            {
                if (node.Name == "priority")
                {
                    XmlNodeList nodelist = node.ChildNodes;
                    foreach (XmlNode subnode in nodelist)
                    {
                        cbPriority.Items.Add(subnode.Name.ToUpper());
                    }
                }
            }

            cbLen.SelectedIndex = 0;
            cbPriority.SelectedIndex = 1;

        }

        /// <summary>
        /// Help Function used to get an error as text
        /// </summary>
        /// <param name="error">Error code to be translated</param>
        /// <returns>A text with the translated error</returns>
        private string GetFormatedError(TPCANStatus error)
        {
            StringBuilder strTemp;

            // Creates a buffer big enough for a error-text
            //
            strTemp = new StringBuilder(256);
            // Gets the text using the GetErrorText API function
            // If the function success, the translated error is returned. If it fails,
            // a text describing the current error is returned.
            //
            if (PCANBasic.GetErrorText(error, 0, strTemp) != TPCANStatus.PCAN_ERROR_OK)
                return string.Format("An error occurred. Error-code's text ({0:X}) couldn't be retrieved", error);
            else
                return strTemp.ToString();
        }


        private void btOpen_Click(object sender, EventArgs e)
        {
            switch(cbDevice.SelectedIndex)
            {
                case (int)CONNECTON_TYPES.USB:
                    try
                    {

                        serial.PortName = cbPort.SelectedItem.ToString();
                        serial.BaudRate = 115200;
                        serial.Open();
                        serial.DiscardInBuffer();
                        serial.DiscardOutBuffer();

                        System.Threading.Thread.Sleep(100);
                        serial.DtrEnable = true;
                        System.Threading.Thread.Sleep(100);
                        serial.DtrEnable = false;
                        System.Threading.Thread.Sleep(100);

                        this.Text = nazev_aplikace + " - " + serial.PortName.ToString();

                    }
                    catch (Exception x)
                    {
                        MessageBox.Show(x.Message);
                        return;
                    }
                    break;


                case (int)CONNECTON_TYPES.PEAK:
                    TPCANStatus stsResult;
                    m_PcanHandle = 0x51;

                    // Connects a selected PCAN-Basic channel
                    //
                    stsResult = PCANBasic.Initialize(
                        m_PcanHandle,
                        TPCANBaudrate.PCAN_BAUD_100K,
                        TPCANType.PCAN_TYPE_ISA,
                        (UInt32)100,
                        (UInt16)3);

                    if (stsResult != TPCANStatus.PCAN_ERROR_OK)
                    {
                        MessageBox.Show(GetFormatedError(stsResult));
                        return;
                    }

                    this.Text = nazev_aplikace + " - PEAK";
                    tReader.Start();
                    // Sets the connection status of the main-form
                    //
                    
                    break;

                case (int)CONNECTON_TYPES.VSCP:
                    vscp = new vscp_daemon_iface();
                    if (!vscp.Open(tbIp.Text, 9598))
                    {
                        MessageBox.Show("Server error!");
                        return;
                    }
                    if (!vscp.Login(tbUser.Text, tbPass.Text))
                    {
                        MessageBox.Show("Login or password failure");
                        return;
                    }
                    tReader.Start();
                    break;
                default:
                    break;
                   
            }



            btClose.Enabled = true;
            btOpen.Enabled = false;
            btSend.Enabled = true;
            

            
        }

        private void btClose_Click(object sender, EventArgs e)
        {
            switch (cbDevice.SelectedIndex)
            {
                case (int)CONNECTON_TYPES.USB: serial.Close();
                    break;

                case (int)CONNECTON_TYPES.PEAK:
                    PCANBasic.Uninitialize(m_PcanHandle);
                    m_PcanHandle = 0;
                    break;

                case (int)CONNECTON_TYPES.VSCP:
                    vscp.Close();
                    break;

                default:
                    break;
                    
            }

            tReader.Stop();
            btOpen.Enabled = true;
            btClose.Enabled = false;
            btSend.Enabled = false;
        }



        private void cbDtr_CheckedChanged(object sender, EventArgs e)
        {
            if (serial.IsOpen)
            {
                if (cbDtr.Checked)
                    serial.DtrEnable = true;
                else
                    serial.DtrEnable = false;
            }
        }

        private void cbRTS_CheckedChanged(object sender, EventArgs e)
        {
            if (serial.IsOpen)
            {
                if (cbRTS.Checked)
                    serial.RtsEnable = true;
                else
                    serial.RtsEnable = false;
            }
        }


        

        private void cbClass_SelectedIndexChanged(object sender, EventArgs e)
        {
            string classs = cbClass.SelectedItem.ToString().ToLower();
            cbType.Items.Clear();

            foreach (XmlNode node in nodeList_class)
            {
                if (node.Name == classs)
                {
                    XmlNodeList nodelist = node.ChildNodes;
                    {
                        foreach (XmlNode sub_node in nodelist)
                        {
                            if (sub_node.Name == "type")
                            {
                                XmlNodeList list_type = sub_node.ChildNodes;
                                {
                                    foreach (XmlNode subsub_node in list_type)
                                    {
                                        cbType.Items.Add(subsub_node.Name.ToUpper());
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (cbType.Items.Count > 0)
                cbType.SelectedIndex = 0;
        }


        private string getClassByInt(int val)
        {
            foreach (XmlNode node in nodeList_class)
            {
                XmlNodeList list = node.ChildNodes;

                foreach (XmlNode sub_node in list)
                {
                    if (sub_node.Name == "value")
                    {
                        if (sub_node.InnerText.Replace(" ", "") == val.ToString())
                        {
                            return node.Name.ToUpper();
                        }
                        continue;
                    }

                }
            }

            return "";
        }


        private string getTypeByClassIntTypeInt(int cl, int ty)
        {
            foreach (XmlNode node in nodeList_class)
            {
                XmlNodeList list = node.ChildNodes;

                foreach (XmlNode sub_node in list)
                {
                    if (sub_node.Name == "value")
                    {
                        if (sub_node.InnerText.Replace(" ", "") == cl.ToString())
                        {
                            XmlNodeList nodelist_type;
                            foreach (XmlNode type in list)
                            {
                                if (type.Name == "type")
                                {
                                    nodelist_type = type.ChildNodes;

                                    foreach (XmlNode subsubnode in nodelist_type)
                                    {
                                        if (subsubnode.InnerText.Replace(" ", "") == ty.ToString())
                                            return subsubnode.Name.ToUpper();
                                    }
                                    continue;
                                }
                            }
                        }
                        continue;
                    }

                }
            }
            return "";
        }


        void send_message_serial(vscp_message msg)
        {
            Int32 id = ((msg.priority << 26) | (msg.vclass << 16) | (msg.type << 8) | (msg.nickname));

            if (System.Convert.ToBoolean(msg.hard_mask))
                id |= (1 << 25);

            string final = "T: " + System.Convert.ToString(id, 16) + ":" + msg.data + ";";
            serial.Write(final);

        }


        void send_message_PEAK(vscp_message msg)
        {
            UInt32 id = (UInt32)((msg.priority << 26) | (msg.vclass << 16) | (msg.type << 8) | (msg.nickname));

            if (System.Convert.ToBoolean(msg.hard_mask))
                id |= (1 << 25);

            TPCANMsg CANMsg;
            TPCANStatus stsResult;

            // We create a TPCANMsg message structure 
            //
            CANMsg = new TPCANMsg();
            CANMsg.DATA = new byte[8];

            // We configurate the Message.  The ID (max 0x1FF),
            // Length of the Data, Message Type (Standard in 
            // this example) and die data
            //
            CANMsg.ID = id;
            CANMsg.LEN = (byte) msg.lenght;
            CANMsg.MSGTYPE = TPCANMessageType.PCAN_MESSAGE_EXTENDED;
        
            
            // We get so much data as the Len of the message
            //

            for (int i = 0; i < CANMsg.LEN; i++)
                CANMsg.DATA[i] = Convert.ToByte(msg.data.Substring(i * 2, 1) + msg.data.Substring(i * 2 + 1, 1), 16);
            
            

            // The message is sent to the configured hardware
            //
            stsResult = PCANBasic.Write(m_PcanHandle, ref CANMsg);

            // The message was successfully sent
            //
            if (stsResult != TPCANStatus.PCAN_ERROR_OK)
                MessageBox.Show(GetFormatedError(stsResult));


        }


        public void send_vscp_msg(vscp_message msg)
        {

            

            switch (cbDevice.SelectedIndex)
            {
                case (int)CONNECTON_TYPES.USB:
                    send_message_serial(msg);
                    break;
                case (int)CONNECTON_TYPES.PEAK:
                    send_message_PEAK(msg);
                    break;

                case (int)CONNECTON_TYPES.VSCP:
                    send_message_VSCP(msg);

                    break;
            }


        }

        private void send_message_VSCP(vscp_message msg)
        {


            vscp_message_d mes = new vscp_message_d();



            mes.priority = (uint)msg.priority;
            mes.oaddr = (uint)msg.nickname;
            mes.vclass = (uint)msg.vclass;
            mes.vtype = (uint)msg.type;
            mes.hardcoded = msg.hard_mask;
            mes.data_len = (uint)msg.data.Length;





            mes.data = new byte[msg.data.Length / 2];


            for (int i = 0; i < mes.data.Length; i++)
                mes.data[i] = Convert.ToByte(msg.data.Substring(i * 2, 1) + msg.data.Substring(i * 2 + 1, 1), 16);

            vscp.SendMessage(mes);

        
        }

        public void sendLog_add()
        {
            send_log.Rows.Add(tbNote.Text, cbPriority.SelectedItem.ToString(), cbHM.Checked, cbClass.SelectedItem.ToString(), cbType.SelectedItem.ToString(),
                tbNickname.Text, cbLen.SelectedItem.ToString(), tbData.Text);
        }

        public int getPriorityByName(string name)
        {
            foreach (XmlNode node in nodeList_utils)
            {
                if (node.Name == "priority")
                {
                    XmlNodeList nodelist = node.ChildNodes;
                    foreach (XmlNode subnode in nodelist)
                    {
                        if (subnode.Name == name.ToLower())
                            return  System.Convert.ToInt32(subnode.InnerText);
                    }
                }
            }
            return -1;
        }

        public int getClassByName(string name)
        {
            foreach (XmlNode node in nodeList_class)
            {
                if (node.Name == name.ToLower())
                {
                    XmlNodeList nodelist = node.ChildNodes;
                    foreach (XmlNode subnode in nodelist)
                    {
                        if (subnode.Name == "value")
                            return System.Convert.ToInt32(subnode.InnerText);
                    }
                }
            }
            return -1;
        }

        public int getTypeByClassAndName(string vclass, string type)
        {
            foreach (XmlNode node in nodeList_class)
            {
                if (node.Name == vclass.ToLower())
                {
                    XmlNodeList nodelist = node.ChildNodes;
                    foreach (XmlNode subnode in nodelist)
                    {
                        if (subnode.Name == "type")
                        {
                            XmlNodeList typelist = subnode.ChildNodes;

                            foreach (XmlNode subsubnode in typelist)
                            {
                                if (subsubnode.Name == type.ToLower())
                                {
                                    return System.Convert.ToInt32(subsubnode.InnerText);
                                }
                            }
                        }
                    }
                }
            }
            return -1;
        }


        private string data_reformat(string text, int len)
        {
            string[] data = text.Split(' ');
            string pom = "";


            for (int i = 0; i < len; i++)
            {
                if (data[i].Length < 1)
                {
                    MessageBox.Show("Invalid data for reformat");
                    return "";
                }

                if (data[i].Length < 2)
                {
                    data[i] = "0" + data[i];
                }
                pom += data[i];
            }
            return pom;
        }

        


        private void btSend_Click(object sender, EventArgs e)
        {
            vscp_message msg = new vscp_message();
            sendLog_add();

            msg.hard_mask = cbHM.Checked;
            msg.priority = getPriorityByName(cbPriority.SelectedItem.ToString());
            msg.vclass = getClassByName(cbClass.SelectedItem.ToString());
            msg.type = getTypeByClassAndName(cbClass.SelectedItem.ToString(), cbType.SelectedItem.ToString());
            msg.lenght = System.Convert.ToInt16(cbLen.Text.ToString());
            msg.data = data_reformat(tbData.Text, msg.lenght);
            msg.nickname = System.Convert.ToInt32(tbNickname.Text);
            send_vscp_msg(msg);
        }

        private void dataGridView1_CellContentClick(object sender, DataGridViewCellEventArgs e)
        {

      
            switch (e.ColumnIndex)
            {
                case 8:
                    DataGridViewRow row = send_log.Rows[e.RowIndex];
                    vscp_message msg = new vscp_message();
                    msg.priority = getPriorityByName(row.Cells[1].Value.ToString());
                    msg.hard_mask = Convert.ToBoolean(row.Cells[2].Value);
                    msg.vclass = getClassByName(row.Cells[3].Value.ToString());
                    msg.type = getTypeByClassAndName(row.Cells[3].Value.ToString(), row.Cells[4].Value.ToString());
                    msg.nickname = Convert.ToInt16(row.Cells[5].Value);
                    msg.lenght = Convert.ToInt16(row.Cells[6].Value);
                    msg.data = data_reformat(row.Cells[7].Value.ToString(), msg.lenght);
                    send_vscp_msg(msg);
                    break;

                case 9:
                    send_log.Rows.RemoveAt(e.RowIndex);
                    break;
            }
        }

        private void tsbClear_Click(object sender, EventArgs e)
        {
            vscpLog.Rows.Clear();
            canLog.Rows.Clear();
        }

        private void tsbClearSend_Click(object sender, EventArgs e)
        {
            send_log.Rows.Clear();
        }

        private void toolStripButton1_Click(object sender, EventArgs e)
        {
            SaveFileDialog save = new SaveFileDialog();
            save.Filter = "Xmlf files (*.xml)|*.xml";
            if (save.ShowDialog() == DialogResult.OK)
            {
                XmlDocument xmls = new XmlDocument();
                xmls.LoadXml("<messages> </messages>");


                XmlElement elem = xmls.CreateElement("price");
                XmlText text = xmls.CreateTextNode("19.95");

                for (int i = 0; i < send_log.RowCount; i++)
                {
                    DataGridViewRow row = send_log.Rows[i];
                    for( int j = 0; j < send_log.ColumnCount - 2; j++)
                    {
                        
                        XmlElement el = xmls.CreateElement(send_log.Columns[j].Name);
                       // XmlText te = xmls.CreateTextNode(row.Cells[j].Value.ToString());
                        xmls.DocumentElement.AppendChild(el);
                        xmls.DocumentElement.LastChild.LastChild.AppendChild(text);
                    }
                }

                xmls.DocumentElement.AppendChild(elem);
                xmls.DocumentElement.LastChild.AppendChild(text);

                xmls.Save(save.OpenFile());

            }

        }

        private void cbDevice_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cbDevice.SelectedIndex == (int)CONNECTON_TYPES.USB)
            {
                cbPort.Visible = true;
                cbDtr.Visible = true;
                cbRTS.Visible = true;
            }
            else
            {
                cbPort.Visible = false;
                cbDtr.Visible = false;
                cbRTS.Visible = false;
            }

            if (cbDevice.SelectedIndex == (int)CONNECTON_TYPES.VSCP)
            {
                lUser.Visible = true;
                lPass.Visible = true;
                lIP.Visible = true;
                tbIp.Visible = true;
                tbUser.Visible = true;
                tbPass.Visible = true;
            }
            else
            {
                lUser.Visible = false;
                lPass.Visible = false;
                tbUser.Visible = false;
                tbPass.Visible = false;
                lIP.Visible = false;
                tbIp.Visible = false;
            }

        }





        /// <summary>
        /// Function for reading PCAN-Basic messages
        /// </summary>
        private void ReadMessagesPEAK()
        {
            if (m_PcanHandle == 0)
                return;


            TPCANMsg CANMsg;
            TPCANTimestamp CANTimeStamp;
            TPCANStatus stsResult;

            // We read at least one time the queue looking for messages.
            // If a message is found, we look again trying to find more.
            // If the queue is empty or an error occurr, we get out from
            // the dowhile statement.
            //			
            do
            {
                // We execute the "Read" function of the PCANBasic                
                //
                stsResult = PCANBasic.Read(m_PcanHandle, out CANMsg, out CANTimeStamp);

                // A message was received
                // We process the message(s)
                //
                if (stsResult == TPCANStatus.PCAN_ERROR_OK)
                    recieveCANMessagePEAK(CANMsg, CANTimeStamp);

            } while (btClose.Enabled && (!Convert.ToBoolean(stsResult & TPCANStatus.PCAN_ERROR_QRCVEMPTY)));
        }


        private void tPeak_Tick(object sender, EventArgs e)
        {
            ReadMessagesPEAK();

            ReadMessagesVSCPDaemon();
        }

        private void ReadMessagesVSCPDaemon()
        {
            if (vscp == null)
                return;


            vscp_message_d[] msg = vscp.GetMsg(vscp.GetNumMsg());

            if (msg == null)
                return;

            foreach (vscp_message_d mes in msg)
            {

                string bytes = "";

                for (int i = 0; i < mes.data_len; i++)
                {
                    bytes += Convert.ToString(mes.data[i], 16);

                    if (i != mes.data_len - 1)
                        bytes += ",";
                }


                uint id = ((mes.priority & 0x7) << 26) | (Convert.ToUInt32(mes.hardcoded) << 25) |
                    ((mes.vclass & 0x1FF) << 16) | ((mes.vtype & 0xFF) << 8) | (mes.oaddr & 0xFF); 




                update_can_log(id.ToString(), mes.data_len.ToString(), bytes, mes.timestamp.ToString());
                update_vscp_mon((int)id, mes.data_len.ToString(), bytes, mes.timestamp.ToString());

            }

            if (msg == null)
                return;
        }



        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if(vscp != null)
                vscp.Close();
        }




    }
}
