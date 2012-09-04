using System;
using System.Collections.Generic;
using System.Text;

using System.Net.Sockets;

namespace tcp_lib
{
        public delegate void delAddMsg(string msg);
        public delegate void delProcessMsg(byte[] msg, int count);
        public delegate void delConnectedMsg(string who);

        class tcpcli
        {

            /// <summary>
            /// Vraci status pripojeni.
            /// </summary>
            public bool isConnected = false;

            private TcpClient tcp;
            private NetworkStream ns;
            private byte[] m_byBuff = new byte[256];    // Recieved data buffer

            private delProcessMsg del;


            /// <summary>
            /// Vytvori novou intanci objektu
            /// </summary>
            /// <param name="x">Delegat na lokalni funkci kam se maji dorucovat data</param>
            public tcpcli(delProcessMsg x)
            {
                del = x;//new delProcessMsg(x.rec_data);
            }


            /// <summary>
            /// Otevre pripojeni se zadanym IP a portem serveru
            /// </summary>
            /// <param name="server">Ip adresa serveru napr 192.168.0.10</param>
            /// <param name="port">Port serveru</param>
            /// <param name="asyncRead">Povoleni asynchnoniho cteni </param>
            /// <returns>Vraci true pokud sepripoji</returns>
            public bool OpenServer(string server, int port, bool asyncRead)
            {
                try
                {
                    tcp = new TcpClient(server, port);
                    ns = tcp.GetStream();
                    isConnected = tcp.Connected;


                    if (asyncRead)
                    {
                        AsyncCallback recieveData = new AsyncCallback(OnRecievedData);
                        ns.BeginRead(m_byBuff, 0, m_byBuff.Length, recieveData, null);
                    }
                }
                catch (Exception ex)
                {
                    printOutDbg("\nSetup Recieve Callback failed!\n" + ex.Message);
                    return false;
                }
                return true;
            }



            /// <summary>
            /// Uzavre TCP/IP spojeni
            /// </summary>
            public void Close()
            {
                ns.Close();
                tcp.Close();
            }

            /// <summary>
            /// Vysle data po TCPip
            /// </summary>
            /// <param name="data">Data k vyslani</param>
            public void Send(byte[] data)
            {
                ns.Write(data, 0, data.Length);
            }



            public byte[] ReadAll(int timeout)
            {
                ns.ReadTimeout = timeout;
                byte[] data = new byte[ns.Length];

                ns.Read(data, 0, data.Length);
              
                return data;

            }

            public byte[] Read(int len, int timeout)
            {
                int pom = len;
                if (len < 9)
                    pom = 9;

                byte[] data = new byte[pom];
                ns.ReadTimeout = timeout;

                try
                {
                    ns.Read(data, 0, len);
                    if (ns.DataAvailable && (len < 9))
                        ns.Read(data, len, 9 - len);
                }
                catch (Exception)
                {
                    return null;
                }
                return data;
            }


            /// <summary>
            /// Asynchronni udalost, ktera se vyvola pri precteni dat
            /// </summary>
            /// <param name="ar"></param>
            private void OnRecievedData(IAsyncResult ar)
            {
                // Socket was the passed in object

                Socket sock = (Socket)ar.AsyncState;

                // Check if we got any data

                try
                {
                    int nBytesRec = sock.EndReceive(ar);
                    if (nBytesRec > 0)
                    {
                        // Wrote the data to the List

                        string sRecieved = Encoding.ASCII.GetString(m_byBuff,
                                                                     0, nBytesRec);

                        // WARNING : The following line is NOT thread safe. Invoke is

                        // m_lbRecievedData.Items.Add( sRecieved );

                        //form.Invoke(form.m_AddMessage, new string[] { sRecieved });
                        object[] paramList = new object[2];
                        paramList[0] = m_byBuff;
                        paramList[1] = nBytesRec;
                        //   form.Invoke(del, paramList);  //---------------------------------
                        // If the connection is still usable restablish the callback

                        SetupRecieveCallback(sock);
                    }
                    else
                    {
                        // If no data was recieved then the connection is probably dead
                        isConnected = false;
                        Console.WriteLine("Client {0}, disconnected",
                                           sock.RemoteEndPoint);
                        sock.Shutdown(SocketShutdown.Both);
                        sock.Close();
                    }
                }
                catch (Exception ex)
                {
                    printOutDbg("\n" + sock.RemoteEndPoint.ToString() + ": Remote servr lost!\n" + ex.Message);
                    isConnected = false;
                }
            }

            /// <summary>
            /// Priradi socketu udalost na prijem dat
            /// </summary>
            /// <param name="sock"></param>
            private void SetupRecieveCallback(Socket sock)
            {
                try
                {
                    AsyncCallback recieveData = new AsyncCallback(OnRecievedData);
                    sock.BeginReceive(m_byBuff, 0, m_byBuff.Length,
                                       SocketFlags.None, recieveData, sock);
                }
                catch (Exception ex)
                {
                    printOutDbg("\n" + sock.RemoteEndPoint.ToString() + ": nSetup Recieve Callback failed!\n" + ex.Message);
                }
            }


            /// <summary>
            /// Vypis debugu na konzoli
            /// </summary>
            /// <param name="test"></param>
            private void printOutDbg(string test)
            {
                if (true)
                {
                    Console.WriteLine(test);
                }
            }



        }
    
}
