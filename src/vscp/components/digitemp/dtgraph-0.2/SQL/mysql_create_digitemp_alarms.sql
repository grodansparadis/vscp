
-- Table structure for table 'digitemp_alarms'
--
-- Table is intended to keep track of current alarms
-- and alarm history
--
--

CREATE TABLE digitemp_alarms (
  alarm_id int(11) NOT NULL auto_increment,
  SerialNumber varchar(17) NOT NULL,
  Fahrenheit decimal(4,2) NOT NULL,
  time_raised datetime NOT NULL,
  time_cleared datetime,
  time_updated timestamp(14),
  alarm_type varchar(15) NOT NULL,
  description varchar(255),
  PRIMARY KEY  (alarm_id),
  KEY serial_key (SerialNumber),
  KEY alarm_type_key (alarm_type),
  KEY time_raised_key (time_raised),
  KEY time_cleared_key (time_cleared)
) TYPE=MyISAM;
