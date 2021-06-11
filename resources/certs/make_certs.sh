#!/bin/sh
#using "pass" for every password

echo "Generating client certificate ..."

openssl genrsa -des3 -out client.key 2048
openssl req -new -key client.key -out client.csr

cp client.key client.key.orig

openssl rsa -in client.key.orig -out client.key

openssl x509 -req -days 3650 -in client.csr -signkey client.key -out client.crt

cp client.crt client.pem
cat client.key >> client.pem

openssl pkcs12 -export -inkey client.key -in client.pem -name ClientName -out client.pfx


echo "Generating first server certificate ..."

openssl genrsa -des3 -out server.key 2048
openssl req -new -key server.key -out server.csr

cp server.key server.key.orig

openssl rsa -in server.key.orig -out server.key

openssl x509 -req -days 3650 -in server.csr -signkey server.key -out server.crt

cp server.crt server.pem
cat server.key >> server.pem

openssl pkcs12 -export -inkey server.key -in server.pem -name ServerName -out server.pfx

echo "First server certificate hash for Public-Key-Pins header:"

openssl x509 -pubkey < server.crt | openssl pkey -pubin -outform der | openssl dgst -sha256 -binary | base64 > server.pin

cat server.pin

echo "Generating backup server certificate ..."

openssl genrsa -des3 -out server_bkup.key 2048
openssl req -new -key server_bkup.key -out server_bkup.csr

cp server_bkup.key server_bkup.key.orig

openssl rsa -in server_bkup.key.orig -out server_bkup.key

openssl x509 -req -days 3650 -in server_bkup.csr -signkey server_bkup.key -out server_bkup.crt

cp server_bkup.crt server_bkup.pem
cat server_bkup.key >> server_bkup.pem

openssl pkcs12 -export -inkey server_bkup.key -in server_bkup.pem -name ServerName -out server_bkup.pfx

echo "Backup server certificate hash for Public-Key-Pins header:"

openssl x509 -pubkey < server_bkup.crt | openssl pkey -pubin -outform der | openssl dgst -sha256 -binary | base64 > server_bkup.pin

cat server_bkup.pin

echo "Generating tcpip client certificate ..."

openssl genrsa -des3 -out tcpip_client.key 2048
openssl req -new -key tcpip_client.key -out tcpip_client.csr

cp tcpip_client.key tcpip_client.key.orig

openssl rsa -in tcpip_client.key.orig -out tcpip_client.key

openssl x509 -req -days 3650 -in tcpip_client.csr -signkey tcpip_client.key -out tcpip_client.crt

cp tcpip_client.crt tcpip_client.pem
cat tcpip_client.key >> tcpip_client.pem

openssl pkcs12 -export -inkey tcpip_client.key -in tcpip_client.pem -name ClientName -out tcpip_client.pfx

echo "Generating tcpip server certificate ..."

openssl genrsa -des3 -out tcpip_server.key 2048
openssl req -new -key tcpip_server.key -out tcpip_server.csr

cp tcpip_server.key tcpip_server.key.orig

openssl rsa -in tcpip_server.key.orig -out tcpip_server.key

openssl x509 -req -days 3650 -in tcpip_server.csr -signkey tcpip_server.key -out tcpip_server.crt

cp tcpip_server.crt tcpip_server.pem
cat tcpip_server.key >> tcpip_server.pem

openssl pkcs12 -export -inkey tcpip_server.key -in tcpip_server.pem -name ServerName -out tcpip_server.pfx

echo "tcpip server certificate hash for Public-Key-Pins header:"

openssl x509 -pubkey < tcpip_server.crt | openssl pkey -pubin -outform der | openssl dgst -sha256 -binary | base64 > tcpip_server.pin

cat tcpip_server.pin