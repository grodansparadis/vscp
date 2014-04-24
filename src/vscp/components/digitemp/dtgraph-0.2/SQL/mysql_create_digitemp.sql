
-- Table structure for table 'digitemp'
--
-- Note that this is identical to the 
-- structure proposed by the digitemp_mysql.pl
-- which comes with digitemp distribution
-- So if you already have it, don't use this

CREATE TABLE digitemp (
  dtKey int(11) NOT NULL auto_increment,
  time timestamp(14) NOT NULL,
  SerialNumber varchar(17) NOT NULL default '',
  Fahrenheit decimal(4,2) NOT NULL default '0.00',
  PRIMARY KEY  (dtKey),
  KEY serial_key (SerialNumber),
  KEY time_key (time)
) TYPE=MyISAM;
