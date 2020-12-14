#!/bin/sh
# Wrapper to check for custom config in $SNAP_USER_COMMON or $SNAP_COMMON and
# use it otherwise fall back to the included basic config which will at least
# allow vscpd to run and do something.
# This script will also copy the full example config in to SNAP_USER_COMMON or
# SNAP_COMMON so that people can refer to it.
#
# The decision about whether to use SNAP_USER_COMMON or SNAP_COMMON is taken
# based on the user that runs the command. If the user is root, it is assumed
# that vscpd is being run as a system daemon, and SNAP_COMMON will be used.
# If a non-root user runs the command, then SNAP_USER_COMMON will be used.

case "$SNAP_USER_COMMON" in
	*/root/snap/vscpd/common*) COMMON=$SNAP_COMMON ;;
	*)                         COMMON=$SNAP_USER_COMMON ;;
esac

CONFIG_FILE="$SNAP/default_config.conf"
CUSTOM_CONFIG="$COMMON/vscpd.xml"


# Copy the example config if it doesn't exist
if [ ! -e "$COMMON/vscpd_example.xml" ]
then
  echo "Copying example config to $COMMON/vscpd_example.conf"
  echo "You can create a custom config by creating a file called $CUSTOM_CONFIG"
  cp $SNAP/vscpd.json $COMMON/vscpd_example.conf
fi


# Does the custom config exist?  If so use it.
if [ -e "$CUSTOM_CONFIG" ]
then
  echo "Found config in $CUSTOM_CONFIG"
  CONFIG_FILE=$CUSTOM_CONFIG
else
  echo "Using default config from $CONFIG_FILE"
fi

# Launch the snap
$SNAP/usr/sbin/vscpd -c $CONFIG_FILE $@
