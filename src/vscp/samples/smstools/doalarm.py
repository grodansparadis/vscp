#!//usr/bin/python
# Copyright 2018 Ake Hedman, Grodans Paradis AB
# for the VSCP project (https://www.vscp.org)
#
# Arguments: #tones zone subzone time date
# where #tones is number of voicecall tone repeats.


import sys
import uuid

filename_voice = str(uuid.uuid4())
filename_sms = str(uuid.uuid4())

# Dir that holds outgoing SMS messages
OUTDIR="/var/spool/sms/outgoing"

# Voicel recipients (comma separated list, empty for non)
VOICE_RECEIVERS="4673232323232323,4612332323232"

# Set to true for flash SMS
bflash = False

# SMS recipients (comma eparated list, empty for non )
#SMS_RECEIVERS="46732232323232,46762323232323"
SMS_RECEIVERS=""

SMS_TEXT="An alarm condition has occured!"

# -----------------------------------------------------------------------------

voicetone="5 "
for x in range(0, int( sys.argv[1] ) ):
    voicetone = voicetone + "1,"

voiceRcvList = VOICE_RECEIVERS.split(",")

cnt=0
if VOICE_RECEIVERS != "" :
    for receiver in voiceRcvList:
        print( OUTDIR + "/" + filename_voice + str(cnt) )
        with open( OUTDIR + "/" + filename_voice + str(cnt), "w") as text_file:
            text_file.write( "To: {0}\n".format(receiver) )
            text_file.write( "Voicecall: yes\n\n" )
            text_file.write( voicetone )
        cnt = cnt + 1

SMSRcvList = SMS_RECEIVERS.split(",")

if SMS_RECEIVERS != "" :
    for receiver in SMSRcvList:
        print( OUTDIR + "/" + filename_voice + str(cnt) )
        with open( OUTDIR + "/" + filename_voice + str(cnt), "w") as text_file:
            text_file.write( "To: {0}\n".format(receiver) )
            if bflash:
                text_file.write( "Flash: yes\n" )
            text_file.write( "\n\n" )
            text_file.write( SMS_TEXT+"\n" )
            text_file.write( "Zone={0} SubZone={1} Time={2} Date={3}\n".format(sys.argv[1],sys.argv[2],sys.argv[3],sys.argv[4] ) )
            text_file.write( "{0}".format(sys.argv[5]) )        
        cnt = cnt + 1

