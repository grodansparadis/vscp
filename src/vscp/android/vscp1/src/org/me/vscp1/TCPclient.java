/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.me.vscp1;

import android.app.Activity;
import android.os.Bundle;
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
 * @author ahedman1
 */
public class TCPclient implements Runnable {

    public void run() {

         try {
             
            InetAddress serverAddr = InetAddress.getByName( "192.168.2.139" );
            Log.d("TCP", "VSCP Client: Connecting...");
            Socket socket = new Socket( serverAddr, 9598 );
            socket.setSoTimeout( 3000 );

            try {   

                BufferedReader input =
                        new BufferedReader( new InputStreamReader( socket.getInputStream() ) );
                
                //read line(s)
                String st = input.readLine();
                Log.d("TCP", "VSCP Client: rcv = " + st );

                st = input.readLine();
                Log.d("TCP", "VSCP Client: rcv = " + st );

                st = input.readLine();
                Log.d("TCP", "VSCP Client: rcv = " + st );

                st = input.readLine();
                Log.d("TCP", "VSCP Client: rcv = " + st );

                st = input.readLine();

                //outgoing stream redirect to socket
                OutputStream out = socket.getOutputStream();
                PrintWriter output = new PrintWriter( out );

                //output.print("\r\n");
                //st = input.readLine();

                Log.d("TCP", "VSCP Client: Username" );
                output.println("user admin");
                output.flush();
                st = input.readLine();
                Log.d("TCP", "VSCP Client: rcv = " + st );

                Log.d("TCP", "VSCP Client: Password" );
                output.println("pass secret");
                output.flush();
                st = input.readLine();
                Log.d("TCP", "VSCP Client: rcv = " + st );

                Log.d("TCP", "VSCP Client: Sent.");


                Log.d("TCP", "VSCP Client: Done.");

            }
            catch( Exception e ) {
                Log.e("TCP", "VSCP Client: Error", e);
            }
            finally {
                socket.close();
            }
        }
        catch (Exception e) {
            Log.e("TCP", "VSCP Client: Error", e);
        }
    }
}
