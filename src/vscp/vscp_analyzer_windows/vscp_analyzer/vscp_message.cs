using System;
using System.Collections.Generic;
using System.Text;

namespace vscp_analyzer
{
    public class vscp_message
    {
        public int priority;
        public int type;
        public int vclass;
        public bool hard_mask;
        public int lenght;
        public string data;
        public int nickname;
    }
}
