///////////////////////////////////////////////////////////////////////////////
// vscp.java:
//
// This file is part is part of VSCP, Very Simple Control Protocol
// http://www.vscp.org)
//
// Copyright (C) 2000-2011 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package org.me.vscp1;

import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.OutputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.Socket;
import android.util.Log;

/**
 *
 * @author Ake Hedman, eurosource (C) 2011 akhe@eurosource.se
 */
public class vscp {

    private Socket m_socket;
    private InetAddress m_serverAddr;
    BufferedReader m_input;
    PrintWriter m_output;

    ///////////////////////////////////////////////////////////////////////////
    // checkReturnValue
    //

    boolean checkReturnValue()
    {
        String buf = "";
        boolean rv = false;
        
        while ( true ) {
            try {
                buf += m_input.readLine();
                if ( ( 0 != buf.length() ) && buf.contains( "+OK" ) ) {
                    rv = true;
                    break;
                }
                else if ( ( 0 != buf.length() ) && buf.contains( "-OK" ) ) {
                    rv = false;
                    break;
                }
            }
            catch( Exception e ) {
                Log.e("TCP", "VSCP Client (checkReturnValue): Error" + e.getMessage(), e);
                return false;
            }
            
        }

        return rv;
    }

    ///////////////////////////////////////////////////////////////////////////
    // doCmdOpen
    //

    public boolean doCmdOpen( String host,
                                int port,
                                String username,
                                String password,
                                int timeout )
    {
        try {
            m_serverAddr = InetAddress.getByName( host );
            Log.d("TCP", "VSCP Client: Connecting...");
            m_socket = new Socket( m_serverAddr, port );
            m_socket.setSoTimeout( timeout );

            try {

                // Create input channel
                m_input = new BufferedReader( new InputStreamReader( m_socket.getInputStream() ) );

                // outgoing stream redirect to socket
                OutputStream out = m_socket.getOutputStream();
                PrintWriter m_output = new PrintWriter( out );

                // If we are connected to the server we should get a
                // welcome message that ens with a row with "+OK Success".
                // Check for it
                if ( !checkReturnValue() ) {
                    Log.d("TCP", "VSCP Client: Unable to connect.");
                    m_socket.close();
                    return false;
                }

                // Issue username
                Log.d("TCP", "VSCP Client: Username" );
                m_output.println( "USER " + username );
                m_output.flush();
                if ( !checkReturnValue() ) {
                    Log.d("TCP", "VSCP Client: Username failure.");
                    m_socket.close();
                    return false;
                }

                // Issue password
                Log.d("TCP", "VSCP Client: Password" );
                m_output.println( "PASS " + password );
                m_output.flush();
                if ( !checkReturnValue() ) {
                    Log.d("TCP", "VSCP Client: Password failed.");
                    m_socket.close();
                    return false;
                }

                Log.d("TCP", "VSCP Client: Password 2" );

                
                
            }
            catch( Exception e ) {
                Log.e("TCP", "VSCP Client (doCmdOpen): Error " + e.getMessage(), e);
                m_socket.close();
                return false;
            }

        }
        catch (Exception e) {
            Log.e( "TCP", "VSCP Client (doCmdOpen2): Error " + e.getMessage(), e );
            return false;
        }

        return true;

    }

    ///////////////////////////////////////////////////////////////////////////
    // doCmdOpen
    //

    public boolean doCmdOpen( String host,
                                int port,
                                String username,
                                String password )
    {
        return doCmdOpen( host,
                            port,
                            username,
                            password,
                            12000 );
    }

    ///////////////////////////////////////////////////////////////////////////
    // doCmdClose
    //

    public boolean doCmdClose()
    {
        try {

            // Issue quit
            Log.d("TCP", "VSCP Client: Close" );
            m_output.println( "QUIT" );
            m_output.flush();
            if ( !checkReturnValue() ) {
                Log.d("TCP", "VSCP Client: QUIT failure.");
                m_socket.close();
                return false;
            }

            m_socket.close();

            return true;

        }
        catch (Exception e) {
            Log.e( "TCP", "VSCP Client (doCmdClose): Error " + e.getMessage(), e );
            return false;
        }
        
    }

    ///////////////////////////////////////////////////////////////////////////////
    // doCmdNOOP
    //

    public boolean doCmdNOOP()
    {
        try {
            // Issue quit
            Log.d("TCP", "VSCP Client: NOOP" );
            m_output.println( "NOOP" );
            m_output.flush();

            Log.d("TCP", "VSCP Client: NOOP 2" );

            if ( !checkReturnValue() ) {
                Log.d("TCP", "VSCP Client: NOOP failure.");
                return false;
            }

            return true;
            
        }
        catch (Exception e) {
            Log.e( "TCP", "VSCP Client (doCmdNOOP): Error " + e.getMessage() , e );
            return false;
        }

    }

    ///////////////////////////////////////////////////////////////////////////////
    // doCmdSend
    //

    boolean doCmdSend( vscpEvent event )
    {
        String cmd;

        cmd = "SEND ";
        cmd += event.m_head + ",";
        cmd += event.m_vscp_class + ",";
        cmd += event.m_vscp_type + ",";
        cmd += event.m_obid + ",";
        cmd += event.m_timestamp + ",";

        return true;
    }
}


