#!/usr/bin/python

# Use as a VSCP action to update meaurement values to Xively
# See http://www.vscp.org/wiki/doku.php/howto/how_to_xively?do=edit&rev=0
# Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>

import sys
import xively

if len(sys.argv) < 5:
	print 'Xively Data updater'
	print '==================='
	print 'Error: To few paramers.'
	print 'Should be: api-key feed-id datapoint-name datapoint-value'
else:	

	# Connect to the API using your API key.
	client = xively.XivelyAPIClient(sys.argv[1])
 
	# Create a new Feed object.
	feed = client.feeds.get(sys.argv[2])

	# Let's give it one datastream with id 'temperature'
	datastream = feed.datastreams.get(sys.argv[3])

	datastream.current_value = sys.argv[4]
	
	# We only want to change current_value.
	datastream.update(fields=['current_value'])

