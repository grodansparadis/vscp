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

  for (int i=0; i<60; i++) {
    vscpEventEx ex;
    vscp_setEventExToNow(&ex);
    for ( int i=0;i<16;i++) {
      ex.GUID[i] = i*11;
    }
    ex.vscp_class = 10;
    ex.vscp_type = 6;
    ex.sizeData = 3;
    ex.data[0] = 0x11;
    ex.data[1] = 0x22;
    ex.data[3] = 0x33;
    client.send(ex);
    sleep(1);
  }

  if (VSCP_ERROR_SUCCESS != client.disconnect()) {
    spdlog::debug("Failed to init MQTT.");
  }

  return 0;
}
