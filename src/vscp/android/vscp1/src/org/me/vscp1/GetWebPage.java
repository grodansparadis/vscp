/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.me.vscp1;

import android.app.Activity;
import android.os.Bundle;

// used for interacting with user interface
import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;
import android.view.View;

// used for passing data
import android.os.Handler;
import android.os.Message;

// used for connectivity
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.URL;
import java.net.URLConnection;

import java.net.InetAddress;
import java.net.Socket;

// debug
import android.util.Log;

public class GetWebPage extends Activity {

    /** Called when the activity is first created. */
    Handler h;

    @Override
    public void onCreate( Bundle savedInstanceState ) {

        super.onCreate( savedInstanceState );
        setContentView( R.layout.main );

        final EditText eText = ( EditText ) findViewById( R.id.address );
        final TextView tView = ( TextView ) findViewById( R.id.pagetext );

        this.h = new Handler() {

            @Override
            public void handleMessage( Message msg ) {

                // process incoming messages here
                switch ( msg.what ) {
                    case 0:
                    	tView.append( (String) msg.obj );
                    	break;
                }

                super.handleMessage( msg );
            }
        };

        final Button button = (Button) findViewById( R.id.ButtonGo );

        button.setOnClickListener( new Button.OnClickListener() {
            
            public void onClick( View v ) {
                
                try {

                    tView.setText("Button clicked - page is loading.");
                    tView.invalidate();

                    InetAddress serverAddr = InetAddress.getByName( "192.168.2.139" );
                    vscp vscp_client = new vscp();

                    if ( vscp_client.doCmdOpen("192.168.2.139", 9598, "admin", "secret" ) ) {
                        tView.setText("Session opened.");
                        tView.invalidate();
                        Log.d("TCP", "Success");
                    }

                    vscp_client.doCmdNOOP();

                    vscp_client.doCmdClose();
                    
                    /*
                    // Perform action on click
                    URL url = new URL( eText.getText().toString() );
                    URLConnection conn = url.openConnection();

                    // Get the response
                    BufferedReader rd = new BufferedReader( new
                            InputStreamReader( conn.getInputStream() ) );
                    String line = "";
                    while ( ( line = rd.readLine() ) != null ) {
                        Message lmsg;
                        lmsg = new Message();
                        lmsg.obj = line;
                        lmsg.what = 0;
                        GetWebPage.this.h.sendMessage( lmsg );
                    }

                     */
                }
                catch ( Exception e ) {
                    tView.setText("Exception caught! " + e.getMessage() );;
                }
            }
        });
    }
}

