BEGIN TRANSACTION;
CREATE TABLE `vscp_unit` (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	`link_to_vscp_type`	INTEGER,
	`code`	INTEGER,
	`unit`	TEXT,
	`description`	INTEGER
);
CREATE TABLE `vscp_type` (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
	`link_to_class`	INTEGER,
	`name`	TEXT,
	`description`	TEXT
);
CREATE TABLE `vscp_data_description` (
	`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT UNIQUE,
	`link_to_vscp_type`	INTEGER,
	`offset`	INTEGER,
	`description`	TEXT
);
CREATE TABLE `vscp_class` (
	`Id`	INTEGER NOT NULL UNIQUE,
	`Name`	TEXT,
	`Description`	INTEGER,
	PRIMARY KEY(Id)
);
COMMIT;
