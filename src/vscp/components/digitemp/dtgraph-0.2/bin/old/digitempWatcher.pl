#!/usr/bin/perl -W

# DigiTemp MySQL watcher script
# Copyright 2003 by Akom
# All Rights Reserved
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
#
##################################################
################# What this does #################
# Looks into the tables defined below,
# and determines if any alarm conditions have been met
# If so, it raises alarm(s) using alarmTracker.sh
# and if that's the first time for that alarm,
# executes the alarm command specified below
# Be sure to configure this variable
#
# 
#
#CREATE table digitemp (
#   dtKey int(11) NOT NULL auto_increment,
#   time timestamp NOT NULL,
#   SerialNumber varchar(17) NOT NULL,
#   Fahrenheit decimal(3,2) NOT NULL,
#   PRIMARY KEY (dtKey),
#   KEY serial_key (SerialNumber),
#   KEY time_key (time)
# );
#--
#-- Table structure for table 'digitemp_metadata'
#--
#
#CREATE TABLE digitemp_metadata (
# SerialNumber varchar(17) NOT NULL default '',
# name varchar(15) NOT NULL default '',
# description varchar(255) default NULL,
# min float(4,3) default NULL,
# max float(4,3) default NULL,
# alarm tinyint(1) unsigned NOT NULL default '0',
# maxchange float(4,3) unsigned default NULL,
# maxchange_interval int(10) unsigned NOT NULL default '3600',
# maxchange_alarm tinyint(1) unsigned NOT NULL default '0',
#  color varchar(15) NOT NULL default 'black',
#  PRIMARY KEY  (SerialNumber)
#) TYPE=MyISAM;
#                      
#
# 
# GRANT SELECT,INSERT ON digitemp.* TO dt_logger@localhost
# IDENTIFIED BY 'VerySecretPwd34';
# GRANT SELECT,INSERT ON digitemp_metadata.* TO dt_logger@localhost
# IDENTIFIED BY 'VerySecretPwd34';
#
# -------------------------------------------------------------------------
use DBI;


# Database info
my $db_name     = "stats";
my $db_user     = "dt_logger";
my $db_pass     = "VerySecretPwd34";

# The DigiTemp Configuration file to use
my $digitemp_rcfile = "~/.digitemprc";
my $digitemp_binary = "digitemp";
my $alarm_notify_script = "alarmTracker.sh";

# Alarm command to pipe error
# string to (once for all alarms (concatenated))
my $alarm_cmd = "mail -s 'digitemp problem' akom";

#Prefix of alarm name to pass to alarmTracker.sh
#postixed with sensor SerialNumber
my $alarm_name_prefix = "digitemp";


my $debug = 0;

# Connect to the database
my $dbh = DBI->connect("dbi:mysql:$db_name","$db_user","$db_pass")
          or die "I cannot connect to dbi:mysql:$db_name as $db_user - $DBI::errstr\n";

  #Pre-read all sensor descriptions from metadata table

my $sql="SELECT SerialNumber, name, min, max , alarm FROM digitemp_metadata ";
print "SQL: $sql\n" if($debug);
$dbp = $dbh->prepare($sql) or die "Can't prepare $sql because: $DBI::errstr";
#$dbh->do($sql) or die "Can't execute statement $sql because: $DBI::errstr";
$dbp->execute() or die "Can't execute statement $sql because of : $DBI::errstr";


while (@data = $dbp->fetchrow_array()) {
    my $serial = $data[0];
    my $name = $data[1];
    my $min = $data[2];
    my $max = $data[3];
    my $alarm = $data[4];
    $map{$serial}=$name;
    #Just a little safety below
    if ($alarm == 1 && $min != $max) {
        $minmap{$serial}=$min;
        $maxmap{$serial}=$max;
    }
    print "Read $serial = $name" if($debug);
}
$dbp->finish;

print "Completed read from DB\n";

#ALso run this:
# select ((max(Fahrenheit) - min(Fahrenheit)) - maxchange)  from digitemp a, digitemp_metadata where a.SerialNumber = '10362657000800E8' and time > FROM_UNIXTIME(UNIX_TIMESTAMP() - maxchange_interval);

# Gather information from DigiTemp
# Read the output from digitemp
# Output in form SerialNumber<SPACE>Temperature in Fahrenheit
open( DIGITEMP, "$digitemp_binary -q -a -o\"%R %.2F\" -c $digitemp_rcfile |" );

select(STDOUT); $| = 1;

my $alarm_text;

#Setup prepared statement for maxchange_alarm checks
$sql = "SELECT max(Fahrenheit) , min(Fahrenheit) , coalesce(maxchange) , coalesce(maxchange_interval) from digitemp a, digitemp_metadata b where maxchange_alarm = 1 and a.SerialNumber = ? and a.SerialNumber = b.SerialNumber and time > FROM_UNIXTIME(UNIX_TIMESTAMP() - maxchange_interval) group by maxchange"; 

print "SQL: $sql\n" if($debug);
$dbp = $dbh->prepare($sql) or die "Can't prepare $sql because: $DBI::errstr";

while( <DIGITEMP> )
{
    print "$_\n" if($debug);
    chomp;
    
    my $min;
    my $max;
    my $name;
    ($serialnumber,$temperature) = split(/ /);
    
    if (defined $minmap{$serialnumber}) {
        $min = $minmap{$serialnumber};
        $max = $maxmap{$serialnumber};
        #print "set max to $max \t";
    }
    if (defined $map{$serialnumber}) {
        $name = $map{$serialnumber};
    } else {
        $name = "unnamed";
    }


    print "$serialnumber: $temperature"." F\t";
    print "($name)\t";

    #Run maxchange query
    #To see if alarms should be raised
    #based on maximum change per 
    #pre-specified interval

    $dbp->execute($serialnumber);
    while (@data = $dbp->fetchrow_array()) {
        my $realMax = $data[0];
        my $realMin = $data[1];
        my $maxchange = $data[2];
        my $maxchange_interval = $data[3];
        
        my $realChange = $realMax - $realMin;
        my $maxchangeExcess = ($realMax - $realMin) - $maxchange; 
        print " [MaxchangeExcess = $maxchangeExcess] " if ($debug);
        if ($maxchangeExcess > 0) {
            print " Excess is positive! for $name" if ($debug);
            #Handle alarm here
            if(&setAlarm($alarm_name_prefix.$serialnumber."maxchange") == 2) {
                $alarm_text .= "Sensor $name is changing too fast - $realChange in $maxchange_interval seconds is too much by $maxchangeExcess ";
            }
        } else {
            #clear alarm
            &unSetAlarm($alarm_name_prefix.$serialnumber."maxchange");
        }
    }


    #Check if max/min have been exceeded
    #and raise alarm(s) appropriately

    if ((defined($min) && $min > $temperature) ||
        (defined($max) && $max < $temperature)) {

        print "\tALARM - min is $min";
        my $this_alarm_text = "$name is at $temperature, not in $min-$max ";
        if(&setAlarm($alarm_name_prefix.$serialnumber) == 2) {
            $alarm_text .= $this_alarm_text;
        }
    } else {
        #clear alarm
        &unSetAlarm($alarm_name_prefix.$serialnumber);
    }
    print "\n";

}

###### DONE with DB #######
$dbh->disconnect;

#In the end, if alarm_text is defined at all
#then some issue has been raised
#so run the action then 

if (defined($alarm_text)) {
    print "An alarm has been raised: $alarm_text\n";

    #Run action
    print "Should run action";
    `echo $alarm_text | $alarm_cmd`;   
}


close( DIGITEMP );

################ END MAIN CODE ##################
#################################################
#################################################

##################################################
#   Internal helpers
#   
##################################################
sub setAlarm() {
    my $alarmName = shift;

    @args = ("sh", "$alarm_notify_script", "SET", "$alarmName");
    $exit_value  = system(@args) >> 8;
    return $exit_value;
}

sub unSetAlarm() {
    my $alarmName = shift;

    @args = ("sh", "$alarm_notify_script", "UNSET", "$alarmName");
    $exit_value  = system(@args) >> 8;
    return $exit_value;
}





