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
import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.URL;
import java.net.URLConnection;

// Loging
import android.util.Log;



/**
 *
 * @author ahedman1
 */

/*
public class MainActivity extends Activity {


    @Override
    public void onCreate( Bundle icicle ) {
        
        super.onCreate( icicle );
        setContentView( R.layout.main );

       
    }



        Thread cThread = new Thread( new TCPclient() );
        cThread.start();
    }

}

*/