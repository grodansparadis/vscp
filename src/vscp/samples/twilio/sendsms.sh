#!/bin/sh
# $1 user-key.
# $2 api-key.
# $3 from number. 
# $4 to number.
# $5 message.
curl -X POST 'https://api.twilio.com/2010-04-01/Accounts/$1/Messages.xml' \
--data-urlencode 'To=$4'  \
--data-urlencode 'From=$3'  \
--data-urlencode 'Body=$5' \
-u $1:$2
