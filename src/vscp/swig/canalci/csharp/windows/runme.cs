// This example illustrates how C++ classes for CANAL  can be used from C# using SWIG.
// The C# class gets mapped onto the C++ class and behaves as if it is a C# class.

using System;

public class runme
{
    static void Main() 
    {
        // ----- Object creation -----

		Console.WriteLine( "Creating some objects:" );

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

		Console.WriteLine( "----------------------------------------------------------------------------");
		Console.WriteLine( " Test python module for the CANAL daemon interface");
		Console.WriteLine( " =================================================");
		Console.WriteLine( " q - Quits program.");
		Console.WriteLine( " s - sends a CAN message.");
		Console.WriteLine( " c - Checks the number of messages avaibale in the inqueue.");
		Console.WriteLine( " r - reads a CAN message from the queue if on is available.");
		Console.WriteLine( "----------------------------------------------------------------------------");

		string ln;

		while ( true ) 
		{
			ln = Console.ReadLine();

			if ( ( 0 == ln.CompareTo("q")) || ( 0 == ln.CompareTo("Q")) ) 
			{
				break;
			}
			else if (( 0 == ln.CompareTo("s")) || ( 0 == ln.CompareTo("S")) ) 
			{
				// Send a message
				canalobj.doCmdSend( canalmsg );
				Console.WriteLine("Message sent");
			}
			else if (( 0 == ln.CompareTo("c")) || ( 0 == ln.CompareTo("C")) ) 
			{
				// Check for messages
				Console.WriteLine( "There are " + canalobj.doCmdDataAvailable() + " messages available" );
			}
			else if (( 0 == ln.CompareTo("r")) || ( 0 == ln.CompareTo("R")) ) 
			{
				canalMsg newcanalmsg = canalobj.new_canalMsg();
				if ( canalobj.doCmdReceive( newcanalmsg ) ) {
					Console.WriteLine("---------------------");
					Console.WriteLine( "One message received" );
					Console.WriteLine( "flag = " +  canalobj.getCanalMsgFlags( newcanalmsg ) );
					if ( canalobj.isCanalMsgExtendedId( newcanalmsg ) ) {
						Console.WriteLine( "Extended id " +  canalobj.getCanalMsgId( newcanalmsg ) ); 
					}
					else {
						Console.WriteLine( "Standard id " + canalobj.getCanalMsgId( newcanalmsg ) );
					}
					Console.WriteLine( "Number of databytes " + canalobj.getCanalMsgSizeData( newcanalmsg ) ); 	
					for ( int i=0; i<canalobj.getCanalMsgSizeData( newcanalmsg ); i++ ) {
						Console.WriteLine( canalobj.getCanalMsgData( newcanalmsg, i ) + " " );
					}
					Console.WriteLine("\n---------------------");
				}
			}
		}

		// Close the interface
		canalobj.doCmdClose();

        
        Console.WriteLine( "Goodbye" );
    }
}
