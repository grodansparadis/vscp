#!/bin/sh
#################################################
# alarmNotify.sh
#  This is a script to aid in setting/unsetting
#   Alarm conditions
#
#   Once set, the alarm action is carried out only
#   once until it is unset.
#
#   Please check that configuration is acceptable
#
#   Usage:
#   alarmNotify.sh MODE NAME 
#
#   exit status will be 2 if the alarm has been set
#   for the first time,
#   or 0 if it's a repeat
#   exit status will be 1 if there was an error
#
#   eg: 
#   alarmNotify.sh SET alarmX 
#
#################################################

####### CONFIGURATION START #######

#Note that temp space is better off being 
#on a shared memory space to keep 
#from writing and deleting files all the time

TEMP_SPACE=/dev/shm
ALARM_PREFIX=.alarm_notify_

####### CONFIGURATION END #########

usage() {
    echo "Usage:"
    echo
    echo "$0 ALARM_ACTION ALARM_NAME "
    echo "ALARM action is either SET or UNSET"
    echo "Note that this script is only intended to be used by other scripts";
    exit 1
}

ALARM_ACTION=$1
ALARM_NAME=$2
ALARM_TEXT=$3

if [ -z "$ALARM_ACTION" ] ; then
    echo "Alarm action must be specified"
    usage
fi

if [ -z "$ALARM_NAME" ] 
then
    echo "Alarm name must be specified!"
    usage
fi


    
FILENAME="$TEMP_SPACE/$ALARM_PREFIX$ALARM_NAME"
    #echo "FILENAME = $FILENAME"

case "$ALARM_ACTION" in
    SET|set)
    echo -n "Setting alarm $ALARM_NAME"
    if [ -f $FILENAME ] ; then
        echo " ..Already set"
    else
        echo " ..Setting"
        date > $FILENAME
        exit 2
    fi
    ACTION_TAKEN=1
    ;;
    UNSET|unset)
    echo -n "Unsetting alarm $ALARM_NAME"
    rm -f $FILENAME
    ACTION_TAKEN=1
    echo " ... done"
    ;;
esac

if [ -z $ACTION_TAKEN ] ; then
    echo "UNKNOWN ACTION $ALARM_ACTION"
fi

exit 0
