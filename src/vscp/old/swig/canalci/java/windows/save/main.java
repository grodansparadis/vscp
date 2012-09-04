
public class main {

	static {
		try {
			System.loadLibrary("java_cci");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Native code library failed to load. See the chapter on Dynamic Linking Problems in the SWIG Java documentation for help.\n" + e);
			System.exit(1);
		}
	}

	public static void main( String argv[] ) {
		
		int devid;
		int rv;
	
		// open tha CANAL interface
		devid = java_cci.cci_CanalOpen( "", 0);
		if ( devid != 0 ) {
			System.out.println("Canal import library opened.");
		}
		else {
			System.out.println("Failed to open Canal import library.");
		}
		
		// Create new CANAL message
		//$canalMsg = perl_cci::new_canalMsg;
		canalMsg msg = new canalMsg();
		//long msg = java_cci.new_canalMsg;

		// Add some information to the message
		java_cci.setCanalMsgId( msg, 123);
		java_cci.setCanalMsgSizeData( msg, 4 );
		java_cci.setCanalMsgData( msg, 1, 11 );
		java_cci.setCanalMsgData( msg, 2, 22 );
		java_cci.setCanalMsgData( msg, 3, 33 );
		java_cci.setCanalMsgData( msg, 4, 44 );
	
		System.out.println("------------------------------------------------------------------\n");
		System.out.println("		Test perl module for the CANAL daemon interface\n");
		System.out.println("		=================================================\n");
		System.out.println("		q - Quits program.\n");
		System.out.println("		s - sends a dummy CAN message.\n");
		System.out.println("		c - Checks the number of messages avaibale in the inqueue.\n");
		System.out.println("		r - reads a CAN message from the queue if on is available.\n");
		System.out.println("------------------------------------------------------------------\n");
	
		// Close the interface
		rv = java_cci.cci_CanalClose( devid );
	
	}
}
