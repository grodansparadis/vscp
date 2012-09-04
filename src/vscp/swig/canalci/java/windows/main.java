// This file illustrates how to interface the Java Canal client interface

 import java.io.DataInputStream;
 import java.lang.String;
 import java.io.IOException;

public class main {
	static {
		try {
			System.loadLibrary("jcanalci");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("jcanalci native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n" + e);
			System.exit(1);
		}
	}

	public static void main(String argv[]) 
	{
		DataInputStream dis = new DataInputStream( System.in );
		String ln = null;
		
		// ----- Object creation -----
	
		CCanalSuperWrapper canalobj = new CCanalSuperWrapper();
		canalMsg canalmsg = new canalMsg();
	
		canalobj.setCanalMsgId( canalmsg, 65 );
		canalobj.setCanalMsgExtendedId( canalmsg );
		canalobj.setCanalMsgData( canalmsg, 0, 0 );
		canalobj.setCanalMsgData( canalmsg, 1, 1 );
		canalobj.setCanalMsgData( canalmsg, 2, 2 );
		canalobj.setCanalMsgData( canalmsg, 3, 3 );
		canalobj.setCanalMsgSizeData( canalmsg, 4 );
	
		// Open the interface
		canalobj.doCmdOpen();

		// Send a message
		canalobj.doCmdSend( canalmsg );
		
		System.out.println("----------------------------------------------------------------------------");
		System.out.println(" Test python module for the CANAL daemon interface");
		System.out.println(" =================================================");
		System.out.println(" q - Quits program.");
		System.out.println(" s - sends a CAN message.");
		System.out.println(" c - Checks the number of messages avaibale in the inqueue.");
		System.out.println(" r - reads a CAN message from the queue if on is available.");
		System.out.println("----------------------------------------------------------------------------");


		boolean bRun = true;
		while ( bRun ) 
		{
		
			try {
				ln = dis.readLine();
			}
			catch ( IOException ioe ) 
			{
				System.out.println( ioe.toString() );
				System.exit( 1 );	
			}
			
			if ( ( ln.compareTo( "q" ) == 0 ) || ( ln.compareTo( "Q" ) == 0 ) ) {
				bRun = false;
			}
			else if ( ( ln.compareTo( "s" ) == 0 ) || ( ln.compareTo( "S" ) == 0 ) ) {
				canalobj.doCmdSend( canalmsg );
				System.out.println("Message sent");
			} 
			else if ( ( ln.compareTo( "c" ) == 0 ) || ( ln.compareTo( "C" ) == 0 ) ) {
				System.out.println( "There are " + canalobj.doCmdDataAvailable() + " messages available" );
			}
			else if ( ( ln.compareTo( "r" ) == 0 ) || ( ln.compareTo( "R" ) == 0 ) ) {
				canalMsg newcanalmsg = canalobj.new_canalMsg();
				if ( canalobj.doCmdReceive( newcanalmsg ) ) {
					System.out.println("---------------------");
					System.out.println( "One message received" );
					System.out.println( "flag = " +  canalobj.getCanalMsgFlags( newcanalmsg ) );
					if ( canalobj.isCanalMsgExtendedId( newcanalmsg ) ) {
						System.out.println( "Extended id " +  canalobj.getCanalMsgId( newcanalmsg ) ); 
					}
					else {
						System.out.println( "Standard id " + canalobj.getCanalMsgId( newcanalmsg ) );
					}
					System.out.println( "Number of databytes " + canalobj.getCanalMsgSizeData( newcanalmsg ) ); 	
					for ( int i=0; i<canalobj.getCanalMsgSizeData( newcanalmsg ); i++ ) {
						System.out.print( canalobj.getCanalMsgData( newcanalmsg, i ) + " " );
					}
					System.out.println("\n---------------------");
				}
			}
		}
		
		// Close the interface
		canalobj.doCmdClose();
		System.out.println("Goodbye");
	}
}
