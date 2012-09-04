
// http://msdn.microsoft.com/en-us/magazine/cc164123.aspx
// http://www.dotnetheaven.com/Uploadfile/mahesh/pr1202172006014101AM/pr12.aspx
// http://www.ftdichip.com/Projects/CodeExamples.htm
// http://msdn.microsoft.com/en-us/library/ef4c3t39%28VS.80%29.aspx
// http://transnum.blogspot.com/2008/12/gcc-pragma-pack-bug.html


using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Runtime.InteropServices;

namespace cSharp_testVSCPDll
{
    public partial class Form1 : Form
    {
        [DllImport("vscp.dll", 
            EntryPoint = "vscp_setInterfaceTcp", 
            ExactSpelling=false,
            CharSet = CharSet.Auto)] 
        static extern void vscp_setInterfaceTcp( String pHost, 
                                                    UInt16 port,
                                                    String pUsername,
                                                    String pPassword );

        [DllImport("vscp.dll",
            EntryPoint = "vscp_setInterfaceDll",
            ExactSpelling = false,
            CharSet = CharSet.Auto)]
        static extern void vscp_setInterfacedll( String pName,
                                                    String pPath,
                                                    String pParameters,
                                                    UInt32 flags,
                                                    UInt32 filter,
                                                    UInt32 mask);

        [DllImport("vscp.dll",
            EntryPoint = "vscp_doCmdOpen",
            ExactSpelling = false,
            CharSet = CharSet.Auto)]
        static extern long vscp_doCmdOpen( String pInterface,
                                                UInt32 flags);

        [DllImport("vscp.dll",
            EntryPoint = "vscp_doCmdClose",
            ExactSpelling = false,
            CharSet = CharSet.Auto)]
        static extern int vscp_doCmdClose();

        [DllImport("vscp.dll",
            EntryPoint = "vscp_doCmdNoop",
            ExactSpelling = false,
            CharSet = CharSet.Auto)]
        static extern int vscp_doCmdNoop();

        [DllImport("vscp.dll",
            EntryPoint = "vscp_doCmdGetLevel",
            ExactSpelling = false,
            CharSet = CharSet.Auto)]
        static extern UInt32 vscp_doCmdGetLevel();

        [DllImport("vscp.dll",
             EntryPoint = "vscp_doCmdSendCanal",
             ExactSpelling = false,
             CharSet = CharSet.Auto)]
        static extern int vscp_doCmdSendCanal(canalMsg* pMsg);

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }
    }
}
