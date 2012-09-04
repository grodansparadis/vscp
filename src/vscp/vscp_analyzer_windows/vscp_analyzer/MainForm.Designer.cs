namespace vscp_analyzer
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.serial = new System.IO.Ports.SerialPort(this.components);
            this.cbPort = new System.Windows.Forms.ComboBox();
            this.btClose = new System.Windows.Forms.Button();
            this.btOpen = new System.Windows.Forms.Button();
            this.canLog = new System.Windows.Forms.DataGridView();
            this.time = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.id = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.len = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.data = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.Period = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cbRTS = new System.Windows.Forms.CheckBox();
            this.cbDtr = new System.Windows.Forms.CheckBox();
            this.vscpLog = new System.Windows.Forms.DataGridView();
            this.dataGridViewTextBoxColumn1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.prio = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.mask = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.Class = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.type = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.nick = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.dataGridViewTextBoxColumn5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.richTextBox1 = new System.Windows.Forms.RichTextBox();
            this.cbPriority = new System.Windows.Forms.ComboBox();
            this.cbClass = new System.Windows.Forms.ComboBox();
            this.cbHM = new System.Windows.Forms.CheckBox();
            this.cbType = new System.Windows.Forms.ComboBox();
            this.cbLen = new System.Windows.Forms.ComboBox();
            this.tbData = new System.Windows.Forms.TextBox();
            this.send_log = new System.Windows.Forms.DataGridView();
            this.tbNickname = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.btSend = new System.Windows.Forms.Button();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.toolStripButton1 = new System.Windows.Forms.ToolStripButton();
            this.toolStripButton2 = new System.Windows.Forms.ToolStripButton();
            this.tsbClear = new System.Windows.Forms.ToolStripButton();
            this.tsbClearSend = new System.Windows.Forms.ToolStripButton();
            this.tbNote = new System.Windows.Forms.TextBox();
            this.label8 = new System.Windows.Forms.Label();
            this.cbDevice = new System.Windows.Forms.ComboBox();
            this.tReader = new System.Windows.Forms.Timer(this.components);
            this.tbUser = new System.Windows.Forms.TextBox();
            this.lUser = new System.Windows.Forms.Label();
            this.lPass = new System.Windows.Forms.Label();
            this.tbPass = new System.Windows.Forms.TextBox();
            this.lIP = new System.Windows.Forms.Label();
            this.tbIp = new System.Windows.Forms.TextBox();
            this.SNote = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SPri = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SHM = new System.Windows.Forms.DataGridViewCheckBoxColumn();
            this.SClass = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SType = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SNick = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SLen = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.SData = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.resend = new System.Windows.Forms.DataGridViewButtonColumn();
            this.remove = new System.Windows.Forms.DataGridViewButtonColumn();
            ((System.ComponentModel.ISupportInitialize)(this.canLog)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.vscpLog)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.send_log)).BeginInit();
            this.toolStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // serial
            // 
            this.serial.BaudRate = 115200;
            this.serial.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(this.serial_DataReceived);
            // 
            // cbPort
            // 
            this.cbPort.FormattingEnabled = true;
            this.cbPort.Location = new System.Drawing.Point(234, 40);
            this.cbPort.Name = "cbPort";
            this.cbPort.Size = new System.Drawing.Size(75, 21);
            this.cbPort.TabIndex = 10;
            // 
            // btClose
            // 
            this.btClose.Location = new System.Drawing.Point(153, 40);
            this.btClose.Name = "btClose";
            this.btClose.Size = new System.Drawing.Size(75, 23);
            this.btClose.TabIndex = 9;
            this.btClose.Text = "Close";
            this.btClose.UseVisualStyleBackColor = true;
            this.btClose.Click += new System.EventHandler(this.btClose_Click);
            // 
            // btOpen
            // 
            this.btOpen.Location = new System.Drawing.Point(72, 40);
            this.btOpen.Name = "btOpen";
            this.btOpen.Size = new System.Drawing.Size(75, 23);
            this.btOpen.TabIndex = 8;
            this.btOpen.Text = "Open";
            this.btOpen.UseVisualStyleBackColor = true;
            this.btOpen.Click += new System.EventHandler(this.btOpen_Click);
            // 
            // canLog
            // 
            this.canLog.AllowUserToAddRows = false;
            this.canLog.AllowUserToDeleteRows = false;
            this.canLog.AllowUserToResizeColumns = false;
            this.canLog.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            this.canLog.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.canLog.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.time,
            this.id,
            this.len,
            this.data,
            this.Period});
            this.canLog.Location = new System.Drawing.Point(12, 375);
            this.canLog.Name = "canLog";
            this.canLog.ReadOnly = true;
            this.canLog.RowHeadersVisible = false;
            this.canLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.canLog.Size = new System.Drawing.Size(474, 228);
            this.canLog.TabIndex = 11;
            // 
            // time
            // 
            this.time.HeaderText = "Time";
            this.time.Name = "time";
            this.time.ReadOnly = true;
            // 
            // id
            // 
            this.id.HeaderText = "ID";
            this.id.Name = "id";
            this.id.ReadOnly = true;
            // 
            // len
            // 
            this.len.HeaderText = "Lenght";
            this.len.Name = "len";
            this.len.ReadOnly = true;
            this.len.Width = 50;
            // 
            // data
            // 
            this.data.HeaderText = "Data";
            this.data.Name = "data";
            this.data.ReadOnly = true;
            this.data.Width = 150;
            // 
            // Period
            // 
            this.Period.HeaderText = "Period";
            this.Period.Name = "Period";
            this.Period.ReadOnly = true;
            this.Period.Width = 50;
            // 
            // cbRTS
            // 
            this.cbRTS.AutoSize = true;
            this.cbRTS.Location = new System.Drawing.Point(370, 42);
            this.cbRTS.Name = "cbRTS";
            this.cbRTS.Size = new System.Drawing.Size(48, 17);
            this.cbRTS.TabIndex = 15;
            this.cbRTS.Text = "RTS";
            this.cbRTS.UseVisualStyleBackColor = true;
            this.cbRTS.CheckedChanged += new System.EventHandler(this.cbRTS_CheckedChanged);
            // 
            // cbDtr
            // 
            this.cbDtr.AutoSize = true;
            this.cbDtr.Location = new System.Drawing.Point(315, 42);
            this.cbDtr.Name = "cbDtr";
            this.cbDtr.Size = new System.Drawing.Size(49, 17);
            this.cbDtr.TabIndex = 14;
            this.cbDtr.Text = "DTR";
            this.cbDtr.UseVisualStyleBackColor = true;
            this.cbDtr.CheckedChanged += new System.EventHandler(this.cbDtr_CheckedChanged);
            // 
            // vscpLog
            // 
            this.vscpLog.AllowUserToAddRows = false;
            this.vscpLog.AllowUserToDeleteRows = false;
            this.vscpLog.AllowUserToResizeColumns = false;
            this.vscpLog.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)));
            this.vscpLog.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.vscpLog.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.dataGridViewTextBoxColumn1,
            this.prio,
            this.mask,
            this.Class,
            this.type,
            this.nick,
            this.dataGridViewTextBoxColumn3,
            this.dataGridViewTextBoxColumn4,
            this.dataGridViewTextBoxColumn5});
            this.vscpLog.Location = new System.Drawing.Point(492, 375);
            this.vscpLog.Name = "vscpLog";
            this.vscpLog.ReadOnly = true;
            this.vscpLog.RowHeadersVisible = false;
            this.vscpLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.vscpLog.Size = new System.Drawing.Size(651, 228);
            this.vscpLog.TabIndex = 16;
            // 
            // dataGridViewTextBoxColumn1
            // 
            this.dataGridViewTextBoxColumn1.HeaderText = "Time";
            this.dataGridViewTextBoxColumn1.Name = "dataGridViewTextBoxColumn1";
            this.dataGridViewTextBoxColumn1.ReadOnly = true;
            this.dataGridViewTextBoxColumn1.Width = 80;
            // 
            // prio
            // 
            this.prio.HeaderText = "Pri";
            this.prio.Name = "prio";
            this.prio.ReadOnly = true;
            this.prio.Width = 40;
            // 
            // mask
            // 
            this.mask.HeaderText = "HM";
            this.mask.Name = "mask";
            this.mask.ReadOnly = true;
            this.mask.Width = 30;
            // 
            // Class
            // 
            this.Class.HeaderText = "Class";
            this.Class.Name = "Class";
            this.Class.ReadOnly = true;
            this.Class.Width = 110;
            // 
            // type
            // 
            this.type.HeaderText = "Type";
            this.type.Name = "type";
            this.type.ReadOnly = true;
            this.type.Width = 120;
            // 
            // nick
            // 
            this.nick.HeaderText = "nick";
            this.nick.Name = "nick";
            this.nick.ReadOnly = true;
            this.nick.Width = 30;
            // 
            // dataGridViewTextBoxColumn3
            // 
            this.dataGridViewTextBoxColumn3.HeaderText = "Len";
            this.dataGridViewTextBoxColumn3.Name = "dataGridViewTextBoxColumn3";
            this.dataGridViewTextBoxColumn3.ReadOnly = true;
            this.dataGridViewTextBoxColumn3.Width = 30;
            // 
            // dataGridViewTextBoxColumn4
            // 
            this.dataGridViewTextBoxColumn4.HeaderText = "Data";
            this.dataGridViewTextBoxColumn4.Name = "dataGridViewTextBoxColumn4";
            this.dataGridViewTextBoxColumn4.ReadOnly = true;
            this.dataGridViewTextBoxColumn4.Width = 140;
            // 
            // dataGridViewTextBoxColumn5
            // 
            this.dataGridViewTextBoxColumn5.HeaderText = "Period";
            this.dataGridViewTextBoxColumn5.Name = "dataGridViewTextBoxColumn5";
            this.dataGridViewTextBoxColumn5.ReadOnly = true;
            this.dataGridViewTextBoxColumn5.Width = 50;
            // 
            // richTextBox1
            // 
            this.richTextBox1.Location = new System.Drawing.Point(906, 164);
            this.richTextBox1.Name = "richTextBox1";
            this.richTextBox1.Size = new System.Drawing.Size(237, 175);
            this.richTextBox1.TabIndex = 17;
            this.richTextBox1.Text = "";
            // 
            // cbPriority
            // 
            this.cbPriority.FormattingEnabled = true;
            this.cbPriority.Location = new System.Drawing.Point(174, 88);
            this.cbPriority.Name = "cbPriority";
            this.cbPriority.Size = new System.Drawing.Size(121, 21);
            this.cbPriority.TabIndex = 19;
            // 
            // cbClass
            // 
            this.cbClass.FormattingEnabled = true;
            this.cbClass.Location = new System.Drawing.Point(350, 88);
            this.cbClass.Name = "cbClass";
            this.cbClass.Size = new System.Drawing.Size(143, 21);
            this.cbClass.TabIndex = 21;
            this.cbClass.SelectedIndexChanged += new System.EventHandler(this.cbClass_SelectedIndexChanged);
            // 
            // cbHM
            // 
            this.cbHM.AutoSize = true;
            this.cbHM.Location = new System.Drawing.Point(315, 92);
            this.cbHM.Name = "cbHM";
            this.cbHM.Size = new System.Drawing.Size(15, 14);
            this.cbHM.TabIndex = 20;
            this.cbHM.UseVisualStyleBackColor = true;
            // 
            // cbType
            // 
            this.cbType.FormattingEnabled = true;
            this.cbType.Location = new System.Drawing.Point(499, 88);
            this.cbType.Name = "cbType";
            this.cbType.Size = new System.Drawing.Size(188, 21);
            this.cbType.TabIndex = 22;
            // 
            // cbLen
            // 
            this.cbLen.FormattingEnabled = true;
            this.cbLen.Items.AddRange(new object[] {
            "0",
            "1",
            "2",
            "3",
            "4",
            "5",
            "6",
            "7",
            "8"});
            this.cbLen.Location = new System.Drawing.Point(742, 89);
            this.cbLen.Name = "cbLen";
            this.cbLen.Size = new System.Drawing.Size(53, 21);
            this.cbLen.TabIndex = 23;
            // 
            // tbData
            // 
            this.tbData.Location = new System.Drawing.Point(801, 89);
            this.tbData.Name = "tbData";
            this.tbData.Size = new System.Drawing.Size(150, 20);
            this.tbData.TabIndex = 25;
            // 
            // send_log
            // 
            this.send_log.AllowUserToAddRows = false;
            this.send_log.AllowUserToDeleteRows = false;
            this.send_log.AllowUserToResizeColumns = false;
            this.send_log.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.send_log.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.SNote,
            this.SPri,
            this.SHM,
            this.SClass,
            this.SType,
            this.SNick,
            this.SLen,
            this.SData,
            this.resend,
            this.remove});
            this.send_log.Location = new System.Drawing.Point(12, 125);
            this.send_log.Name = "send_log";
            this.send_log.RowHeadersVisible = false;
            this.send_log.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.send_log.Size = new System.Drawing.Size(761, 228);
            this.send_log.TabIndex = 26;
            this.send_log.CellContentClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.dataGridView1_CellContentClick);
            // 
            // tbNickname
            // 
            this.tbNickname.Location = new System.Drawing.Point(693, 89);
            this.tbNickname.Name = "tbNickname";
            this.tbNickname.Size = new System.Drawing.Size(43, 20);
            this.tbNickname.TabIndex = 27;
            this.tbNickname.Text = "1";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(174, 72);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(38, 13);
            this.label1.TabIndex = 28;
            this.label1.Text = "Priority";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(298, 72);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(45, 13);
            this.label2.TabIndex = 29;
            this.label2.Text = "Hard M.";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(347, 72);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(32, 13);
            this.label3.TabIndex = 30;
            this.label3.Text = "Class";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(496, 72);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(31, 13);
            this.label4.TabIndex = 31;
            this.label4.Text = "Type";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(690, 72);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(29, 13);
            this.label5.TabIndex = 32;
            this.label5.Text = "Nick";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(739, 72);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(40, 13);
            this.label6.TabIndex = 33;
            this.label6.Text = "Lenght";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(798, 72);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(30, 13);
            this.label7.TabIndex = 34;
            this.label7.Text = "Data";
            // 
            // btSend
            // 
            this.btSend.Location = new System.Drawing.Point(957, 86);
            this.btSend.Name = "btSend";
            this.btSend.Size = new System.Drawing.Size(75, 23);
            this.btSend.TabIndex = 35;
            this.btSend.Text = "Send";
            this.btSend.UseVisualStyleBackColor = true;
            this.btSend.Click += new System.EventHandler(this.btSend_Click);
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButton1,
            this.toolStripButton2,
            this.tsbClear,
            this.tsbClearSend});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(1155, 25);
            this.toolStrip1.TabIndex = 36;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // toolStripButton1
            // 
            this.toolStripButton1.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.toolStripButton1.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButton1.Image")));
            this.toolStripButton1.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButton1.Name = "toolStripButton1";
            this.toolStripButton1.Size = new System.Drawing.Size(70, 22);
            this.toolStripButton1.Text = "Save preset";
            this.toolStripButton1.Click += new System.EventHandler(this.toolStripButton1_Click);
            // 
            // toolStripButton2
            // 
            this.toolStripButton2.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.toolStripButton2.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButton2.Image")));
            this.toolStripButton2.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButton2.Name = "toolStripButton2";
            this.toolStripButton2.Size = new System.Drawing.Size(72, 22);
            this.toolStripButton2.Text = "Load preset";
            // 
            // tsbClear
            // 
            this.tsbClear.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.tsbClear.Image = ((System.Drawing.Image)(resources.GetObject("tsbClear.Image")));
            this.tsbClear.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbClear.Name = "tsbClear";
            this.tsbClear.Size = new System.Drawing.Size(103, 22);
            this.tsbClear.Text = "Clear recieve logs";
            this.tsbClear.Click += new System.EventHandler(this.tsbClear_Click);
            // 
            // tsbClearSend
            // 
            this.tsbClearSend.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Text;
            this.tsbClearSend.Image = ((System.Drawing.Image)(resources.GetObject("tsbClearSend.Image")));
            this.tsbClearSend.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbClearSend.Name = "tsbClearSend";
            this.tsbClearSend.Size = new System.Drawing.Size(86, 22);
            this.tsbClearSend.Text = "Clear send log";
            this.tsbClearSend.Click += new System.EventHandler(this.tsbClearSend_Click);
            // 
            // tbNote
            // 
            this.tbNote.Location = new System.Drawing.Point(12, 89);
            this.tbNote.Name = "tbNote";
            this.tbNote.Size = new System.Drawing.Size(156, 20);
            this.tbNote.TabIndex = 37;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(12, 73);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(30, 13);
            this.label8.TabIndex = 38;
            this.label8.Text = "Note";
            // 
            // cbDevice
            // 
            this.cbDevice.FormattingEnabled = true;
            this.cbDevice.Items.AddRange(new object[] {
            "USB",
            "PEAK",
            "VSCP"});
            this.cbDevice.Location = new System.Drawing.Point(12, 40);
            this.cbDevice.Name = "cbDevice";
            this.cbDevice.Size = new System.Drawing.Size(54, 21);
            this.cbDevice.TabIndex = 39;
            this.cbDevice.SelectedIndexChanged += new System.EventHandler(this.cbDevice_SelectedIndexChanged);
            // 
            // tReader
            // 
            this.tReader.Interval = 50;
            this.tReader.Tick += new System.EventHandler(this.tPeak_Tick);
            // 
            // tbUser
            // 
            this.tbUser.Location = new System.Drawing.Point(283, 40);
            this.tbUser.Name = "tbUser";
            this.tbUser.Size = new System.Drawing.Size(81, 20);
            this.tbUser.TabIndex = 40;
            this.tbUser.Text = "admin";
            // 
            // lUser
            // 
            this.lUser.AutoSize = true;
            this.lUser.Location = new System.Drawing.Point(248, 43);
            this.lUser.Name = "lUser";
            this.lUser.Size = new System.Drawing.Size(29, 13);
            this.lUser.TabIndex = 41;
            this.lUser.Text = "User";
            // 
            // lPass
            // 
            this.lPass.AutoSize = true;
            this.lPass.Location = new System.Drawing.Point(369, 43);
            this.lPass.Name = "lPass";
            this.lPass.Size = new System.Drawing.Size(53, 13);
            this.lPass.TabIndex = 43;
            this.lPass.Text = "Password";
            // 
            // tbPass
            // 
            this.tbPass.Location = new System.Drawing.Point(428, 40);
            this.tbPass.Name = "tbPass";
            this.tbPass.Size = new System.Drawing.Size(81, 20);
            this.tbPass.TabIndex = 42;
            this.tbPass.Text = "123";
            // 
            // lIP
            // 
            this.lIP.AutoSize = true;
            this.lIP.Location = new System.Drawing.Point(515, 43);
            this.lIP.Name = "lIP";
            this.lIP.Size = new System.Drawing.Size(20, 13);
            this.lIP.TabIndex = 45;
            this.lIP.Text = "IP:";
            // 
            // tbIp
            // 
            this.tbIp.Location = new System.Drawing.Point(541, 41);
            this.tbIp.Name = "tbIp";
            this.tbIp.Size = new System.Drawing.Size(81, 20);
            this.tbIp.TabIndex = 44;
            this.tbIp.Text = "127.0.0.1";
            // 
            // SNote
            // 
            this.SNote.HeaderText = "Note";
            this.SNote.Name = "SNote";
            this.SNote.Width = 150;
            // 
            // SPri
            // 
            this.SPri.HeaderText = "Pri";
            this.SPri.Name = "SPri";
            this.SPri.ReadOnly = true;
            this.SPri.Width = 40;
            // 
            // SHM
            // 
            this.SHM.HeaderText = "HM";
            this.SHM.Name = "SHM";
            this.SHM.ReadOnly = true;
            this.SHM.Width = 30;
            // 
            // SClass
            // 
            this.SClass.HeaderText = "Class";
            this.SClass.Name = "SClass";
            this.SClass.ReadOnly = true;
            this.SClass.Width = 110;
            // 
            // SType
            // 
            this.SType.HeaderText = "Type";
            this.SType.Name = "SType";
            this.SType.ReadOnly = true;
            this.SType.Width = 120;
            // 
            // SNick
            // 
            this.SNick.HeaderText = "Nick";
            this.SNick.Name = "SNick";
            this.SNick.ReadOnly = true;
            this.SNick.Width = 30;
            // 
            // SLen
            // 
            this.SLen.HeaderText = "Len";
            this.SLen.Name = "SLen";
            this.SLen.ReadOnly = true;
            this.SLen.Width = 30;
            // 
            // SData
            // 
            this.SData.HeaderText = "Data";
            this.SData.Name = "SData";
            this.SData.ReadOnly = true;
            this.SData.Width = 140;
            // 
            // resend
            // 
            this.resend.HeaderText = "Resend";
            this.resend.Name = "resend";
            this.resend.Width = 50;
            // 
            // remove
            // 
            this.remove.HeaderText = "Remove";
            this.remove.Name = "remove";
            this.remove.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.remove.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Automatic;
            this.remove.Width = 50;
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1155, 615);
            this.Controls.Add(this.lIP);
            this.Controls.Add(this.tbIp);
            this.Controls.Add(this.lPass);
            this.Controls.Add(this.tbPass);
            this.Controls.Add(this.lUser);
            this.Controls.Add(this.tbUser);
            this.Controls.Add(this.cbDevice);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.tbNote);
            this.Controls.Add(this.toolStrip1);
            this.Controls.Add(this.btSend);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.tbNickname);
            this.Controls.Add(this.send_log);
            this.Controls.Add(this.tbData);
            this.Controls.Add(this.cbLen);
            this.Controls.Add(this.cbType);
            this.Controls.Add(this.cbHM);
            this.Controls.Add(this.cbClass);
            this.Controls.Add(this.cbPriority);
            this.Controls.Add(this.richTextBox1);
            this.Controls.Add(this.vscpLog);
            this.Controls.Add(this.cbRTS);
            this.Controls.Add(this.cbDtr);
            this.Controls.Add(this.canLog);
            this.Controls.Add(this.cbPort);
            this.Controls.Add(this.btClose);
            this.Controls.Add(this.btOpen);
            this.Name = "MainForm";
            this.Text = "VSCP Analyzator";
            this.Load += new System.EventHandler(this.MainForm_Load);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.canLog)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.vscpLog)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.send_log)).EndInit();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.IO.Ports.SerialPort serial;
        private System.Windows.Forms.ComboBox cbPort;
        private System.Windows.Forms.Button btClose;
        private System.Windows.Forms.Button btOpen;
        private System.Windows.Forms.DataGridView canLog;
        private System.Windows.Forms.CheckBox cbRTS;
        private System.Windows.Forms.CheckBox cbDtr;
        private System.Windows.Forms.DataGridView vscpLog;
        private System.Windows.Forms.RichTextBox richTextBox1;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn1;
        private System.Windows.Forms.DataGridViewTextBoxColumn prio;
        private System.Windows.Forms.DataGridViewCheckBoxColumn mask;
        private System.Windows.Forms.DataGridViewTextBoxColumn Class;
        private System.Windows.Forms.DataGridViewTextBoxColumn type;
        private System.Windows.Forms.DataGridViewTextBoxColumn nick;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn3;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn4;
        private System.Windows.Forms.DataGridViewTextBoxColumn dataGridViewTextBoxColumn5;
        private System.Windows.Forms.ComboBox cbPriority;
        private System.Windows.Forms.ComboBox cbClass;
        private System.Windows.Forms.CheckBox cbHM;
        private System.Windows.Forms.ComboBox cbType;
        private System.Windows.Forms.ComboBox cbLen;
        private System.Windows.Forms.TextBox tbData;
        private System.Windows.Forms.DataGridView send_log;
        private System.Windows.Forms.TextBox tbNickname;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Button btSend;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton toolStripButton1;
        private System.Windows.Forms.ToolStripButton toolStripButton2;
        private System.Windows.Forms.ToolStripButton tsbClear;
        private System.Windows.Forms.TextBox tbNote;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.ToolStripButton tsbClearSend;
        private System.Windows.Forms.DataGridViewTextBoxColumn time;
        private System.Windows.Forms.DataGridViewTextBoxColumn id;
        private System.Windows.Forms.DataGridViewTextBoxColumn len;
        private System.Windows.Forms.DataGridViewTextBoxColumn data;
        private System.Windows.Forms.DataGridViewTextBoxColumn Period;
        private System.Windows.Forms.ComboBox cbDevice;
        private System.Windows.Forms.Timer tReader;
        private System.Windows.Forms.TextBox tbUser;
        private System.Windows.Forms.Label lUser;
        private System.Windows.Forms.Label lPass;
        private System.Windows.Forms.TextBox tbPass;
        private System.Windows.Forms.Label lIP;
        private System.Windows.Forms.TextBox tbIp;
        private System.Windows.Forms.DataGridViewTextBoxColumn SNote;
        private System.Windows.Forms.DataGridViewTextBoxColumn SPri;
        private System.Windows.Forms.DataGridViewCheckBoxColumn SHM;
        private System.Windows.Forms.DataGridViewTextBoxColumn SClass;
        private System.Windows.Forms.DataGridViewTextBoxColumn SType;
        private System.Windows.Forms.DataGridViewTextBoxColumn SNick;
        private System.Windows.Forms.DataGridViewTextBoxColumn SLen;
        private System.Windows.Forms.DataGridViewTextBoxColumn SData;
        private System.Windows.Forms.DataGridViewButtonColumn resend;
        private System.Windows.Forms.DataGridViewButtonColumn remove;
    }
}

