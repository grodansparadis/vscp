//
// log2mysql.js
// =============
// Copyright (C) 2014 Grodans Paradis AB
// 
// This script log temperature to a remote mysql database
// http://www.thegeekstuff.com/2014/01/mysql-nodejs-intro/
// npm install -g mysql

var mysql =  require('mysql');

var connection =  mysql.createConnection({
  	host : process.argv[2],
    port: process.argv[3],
  	user : process.argv[4],
  	password: process.argv[5],
    database : process.argv[6],
});
  
connection.connect();
  
//var queryString = 'SELECT * FROM temperature';
var queryString = "INSERT INTO temperature (GUID,Date,Value) VALUES ('" +
                        process.argv[7] + 
                        "','" +
                        process.argv[8] + 
                        "'," +
                        process.argv[9] + 
                        ");";
 
connection.query(queryString, function(err, rows, fields) {
    if (err) throw err;
 
    console.log( rows );
});
   
connection.end(function(err){
    // Do something after the connection is gracefully terminated.
    console.log('Done!');
});