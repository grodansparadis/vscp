#!/usr/bin/perl -W

# DigiTemp MySQL display script
# -----------------------------
# This script is a wrapper for the digitemp binary
# and when invoked queries the digitemp_metadata table in mysql
# database to get the names of the sensors, making display more useful
# 
# ------------------------------------------------------------------------
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
# -------------------------------------------------------------------------
use DBI;


# Database info
my $db_name     = "stats";
my $db_user     = "dt_logger";
my $db_pass     = "VerySecretPwd34";

# The DigiTemp Configuration file to use
my $digitemp_rcfile = "~/.digitemprc";
my $digitemp_binary = "digitemp";


my $debug = 0;

# Connect to the database
my $dbh = DBI->connect("dbi:mysql:$db_name","$db_user","$db_pass")
          or die "I cannot connect to dbi:mysql:$db_name as $db_user - $DBI::errstr\n";

  #Pre-read all sensor descriptions from metadata table

my $sql="SELECT SerialNumber, name FROM digitemp_metadata ";
print "SQL: $sql\n" if($debug);
$dbp = $dbh->prepare($sql) or die "Can't prepare $sql because: $DBI::errstr";
#$dbh->do($sql) or die "Can't execute statement $sql because: $DBI::errstr";
$dbp->execute();

my $count = 0;
while (@data = $dbp->fetchrow_array()) {
    my $serial = $data[0];
    my $name = $data[1];
    $map{$serial}=$name;
    print "Read $serial = $name" if($debug);
    $count++;
}
$dbp->finish;
$dbh->disconnect;


print "Read from DB (".$count." records).  Waiting for $digitemp_binary \n";



# Gather information from DigiTemp
# Read the output from digitemp
# Output in form SerialNumber<SPACE>Temperature in Fahrenheit
open( DIGITEMP, "$digitemp_binary -q -a -o\"%R %.2F\" -c $digitemp_rcfile |" );

select(STDOUT); $| = 1;
while( <DIGITEMP> )
{
    print "$_\n" if($debug);
    chomp;

    ($serialnumber,$temperature) = split(/ /);

    print "$serialnumber : $temperature"."F ";
    if (defined($map{$serialnumber})) {
        print "(". $map{$serialnumber} . ")";
    } else {
        print "( unnamed )";
    }
    print "\n";

}

close( DIGITEMP );

