#
# Perl5 script for testing simple example

use perl_cci;

# Open the CANAL channel
$devid = perl_cci::cci_CanalOpen("", 0);
if (  $devid ) {
	print "Interface is Open.\n";
}
else {
	print "Unable to open interface.\n";
}

# Create new CANAL message
$canalMsg = perl_cci::new_canalMsg;

# Add some information to the message
perl_cci::setCanalMsgId( $canalMsg, $x );
perl_cci::setCanalMsgSizeData( $canalMsg, 4 );
perl_cci::setCanalMsgData( $canalMsg, 1, 11 );
perl_cci::setCanalMsgData( $canalMsg, 2, 22 );
perl_cci::setCanalMsgData( $canalMsg, 3, 33 );
perl_cci::setCanalMsgData( $canalMsg, 4, 44 );

print "------------------------------------------------------------------\n";
print "		Test perl module for the CANAL daemon interface\n";
print "		=================================================\n";
print "		q - Quits program.\n";
print "		s - sends a dummy CAN message.\n";
print "		c - Checks the number of messages avaibale in the inqueue.\n";
print "		r - reads a CAN message from the queue if on is available.\n";
print "------------------------------------------------------------------\n";

do {
	# Get a line of input
	chomp( $line = <STDIN> );
	if (( $line eq "s" ) or ( $line eq "S" )) {
		# Send dummy message
		$rv = perl_cci::cci_CanalSend( $devid, $canalMsg );
		print $rv;	
	}
	elsif (( $line eq "c" ) or ( $line eq "C" )) {
		# Check for messages
		$rv = perl_cci::cci_CanalDataAvailable( $devid );
		print "There are $rv messages available\n";
	}
	elsif (( $line eq "r" ) or ( $line eq "R" )) {
		# Receive a message
		$newcanalmsg = perl_cci::new_canalMsg();
		$rv = perl_cci::cci_CanalReceive( $devid, $newcanalmsg );
		if ( $rv ) {
			print "One message received\n";
			$flag = perl_cci::getCanalMsgFlags( $newcanalmsg );
			$sizeData = perl_cci::getCanalMsgSizeData( $newcanalmsg );
			$id = perl_cci::getCanalMsgId( $newcanalmsg );
			print "flag = $flag\m"; 
			if ( perl_cci::isCanalMsgExtendedId( $newcanalmsg ) ) {
				print "Extended id = $id \n"; 
			}
			else {
				print "Standard id = $id \n";
			}
			print "Number of databytes = $sizeData\n"; 	
			for ( $i=0; $i< $sizeData; $i++ ) {
				print perl_cci::getCanalMsgData( $newcanalmsg, $i );
				print " ";
			}
			print "\n";
		}
	}
} until ( $line eq "q" ) or ( $line eq "Q" );

# Close the channel
$rv = perl_cci::cci_CanalClose( $devid );

