import sys

onewire_prefix = "FF:FF:FF:FF:FF:FF:FF:FF:"

for line in sys.stdin:
#	sys.stdout.write( line )
	
	arr = line.split(" ")
#	print "ID = " + arr[0][::-1]
#        print "ID = " + arr[0]
	sys.stdout.write(onewire_prefix)
	a1 = arr[0]
	for i in range(7, -1, -1):
		sys.stdout.write( a1[i*2:i*2+2] )
		if ( 0 != i ): 
			sys.stdout.write(":")
	print
#	print a1[4:6] + ":" + a1[2:4] + ":" + a1[0:2]
	print "temperature = " + arr[1]

#	for item in line.split(" "):
#		sys.stdout.write( item + "\n" )	
		
