# file: runme.py

# This file illustrates the use of the CANAL CAN routines

import python_cci 

# ----- Object creation -----

print '----------------------------------------------------------------------------'

canalobj = python_cci.CCanalSuperWrapper()

devitem = canalobj.new_devItem()
canalobj.setdevItemId( devitem, 1 )
canalobj.setdevItemName( devitem, 'Test' )
print canalobj.getdevItemName( devitem )

canalmsg = canalobj.new_canalMsg()
canalobj.setCanalMsgId( canalmsg, 65 )
canalobj.setCanalMsgExtendedId( canalmsg )
canalobj.setCanalMsgData( canalmsg, 0, 0 )
canalobj.setCanalMsgData( canalmsg, 1, 1 )
canalobj.setCanalMsgData( canalmsg, 2, 2 )
canalobj.setCanalMsgData( canalmsg, 3, 3 )
canalobj.setCanalMsgSizeData( canalmsg, 4 )

print canalobj.getCanalMsgId( canalmsg )

# Open the interface
canalobj.doCmdOpen()

#Send a message
canalobj.doCmdSend( canalmsg )

print '----------------------------------------------------------------------------'
print '''\
		Test python module for the CANAL daemon interface
		=================================================
		q - Quits program.
		s - sends a CAN message.
		c - Checks the number of messages avaibale in the inqueue.
		r - reads a CAN message from the queue if on is available.
'''
print '----------------------------------------------------------------------------'

while True:
	input = raw_input('Select Operation : ')
	if (input == 'q') or (input == 'Q'):
		break
	elif (input == 's') or (input == 'S'):
		canalobj.doCmdSend( canalmsg )
		print 'Message sent'
	elif (input == 'c') or (input == 'C'):
		print 'There are %d messages available' % canalobj.doCmdDataAvailable()
	elif (input == 'r') or (input == 'R'):
			newcanalmsg = canalobj.new_canalMsg()
			if canalobj.doCmdReceive( newcanalmsg ):
				print 'One message received'
				print 'flag = %d' % canalobj.getCanalMsgFlags( newcanalmsg )
				if canalobj.isCanalMsgExtendedId( newcanalmsg ):
					print 'Extended id = %d' % canalobj.getCanalMsgId( newcanalmsg )
				else:
					print 'Standard id = %d' % canalobj.getCanalMsgId( newcanalmsg )
				print 'Number of databytes = %d' % canalobj.getCanalMsgSizeData( newcanalmsg )	
				for i in range(0, canalobj.getCanalMsgSizeData( newcanalmsg ) ):
					print canalobj.getCanalMsgData( newcanalmsg, i )

# Close the interface
canalobj.doCmdClose()
print "Goodbye"




