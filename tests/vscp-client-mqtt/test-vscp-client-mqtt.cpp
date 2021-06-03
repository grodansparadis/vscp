// uint64_t wcyd;
// std::string strLine = "00-00-00-00-00-00-9E-1F";

// wcyd = 0;
// size_t pos;
// for ( int i=0; i<8; i++) {
//     wcyd = wcyd << 8;
//     printf("%s\n",strLine.c_str());
//     wcyd += std::stoul(strLine,&pos,16);
//     if (7 != i) {
//         strLine = strLine.substr(pos+1);  // Take away first part of str "zz:"
//     }
// }
// printf( "%" PRIx64 "\n",wcyd);

#define _XOPEN_SOURCE 700
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <streambuf>
#include <string>

#include "spdlog/spdlog.h"
#include "vscp_client_mqtt.h"

int
main()
{
  spdlog::set_level(spdlog::level::trace); // Set global log level to debug

  vscpClientMqtt client;

  //   const char *jsoncfg = "{"
  //                         "\"bind\" : \"interface\","
  //                         "\"host\" : \"tcp://192.168.1.7:1883\","
  //                         "\"port\" : 1883,"
  //                         "\"mqtt-options\" : {"
  //                         "\"tcp-nodelay\" : true,"
  //                         "\"protocol-version\": 500,"
  //                         "\"receive-maximum\": 20,"
  //                         "\"send-maximum\": 20,"
  //                         "\"ssl-ctx-with-defaults\": 0,"
  //                         "\"tls-ocsp-required\": 0,"
  //                         "\"tls-use-os-certs\" : 0"
  //                         "},"
  //                         "\"user\" : \"vscp\","
  //                         "\"password\": \"secret\","
  //                         "\"clientid\" : \"mosq-vscp-daemon-000001\","
  //                         "\"publish-format\" : \"json\","
  //                         "\"subscribe-format\" : \"auto\","
  //                         "\"qos\" : 1,"
  //                         "\"bcleansession\" : false,"
  //                         "\"bretain\" : false,"
  //                         "\"keepalive\" : 60,"
  //                         "\"reconnect\" : {"
  //                         "\"delay\" : 2,"
  //                         "\"delay-max\" : 10,"
  //                         "\"exponential-backoff\" : false"
  //                         "},"
  //                         "\"tls\" : {"
  //                         "\"cafile\" : \"\","
  //                         "\"capath\" : \"\","
  //                         "\"certfile\" : \"\","
  //                         "\"keyfile\" : \"\","
  //                         "\"pwkeyfile\" : \"\""
  //                         "},"
  //                         "\"will\": {"
  //                         "\"topic\": \"topic\","
  //                         "\"qos\": 0,"
  //                         "\"retain\": true,"
  //                         "\"payload\": \"string/json/xml\""
  //                         "},"
  //                         "\"subscribe\": ["
  //                         "\"subscribe/topic/A\","
  //                         "\"subscribe/topic/B\""
  //                         "],"
  //                         "\"publish\": ["
  //                         "\"publish/topic/A\","
  //                         "\"publish/topic/B\""
  //                         "],"
  //                         "\"v5\" : {"
  //                         "\"user-properties\": {"
  //                         "\"prop1\" : \"value\","
  //                         "\"prop2\" : \"value\""
  //                         "},"
  //                         "\"subscribe-options\": ["
  //                         "\"NO_LOCAL\","
  //                         "\"RETAIN_AS_PUBLISHED\","
  //                         "\"SEND_RETAIN_ALWAYS\","
  //                         "\"SEND_RETAIN_NEW\","
  //                         "\"SEND_RETAIN_NEVER\""
  //                         "]"
  //                         "}"
  //                         "}";

  long n;
  char *buf;

  n = pathconf(".", _PC_PATH_MAX);
  assert(n != -1);
  buf = (char *) malloc(n * sizeof(*buf));
  assert(buf);
  if (getcwd(buf, n) == NULL) {
    perror("getcwd");
    exit(EXIT_FAILURE);
  }
  else {
    printf("%s\n", buf);
  }
  free(buf);

  std::ifstream f1("default_311.json");
  std::string jsoncfg((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());

  if (!client.initFromJson(jsoncfg)) {
    spdlog::debug("Failed to init from JSON.");
  }

  if (VSCP_ERROR_SUCCESS != client.connect()) {
    spdlog::debug("Failed to conect to MQTT broker.");
  }

  sleep(60);

  if (VSCP_ERROR_SUCCESS != client.disconnect()) {
    spdlog::debug("Failed to init MQTT.");
  }

  return 0;
}
