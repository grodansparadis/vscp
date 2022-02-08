// unit_test for mdfparser

#include <gtest/gtest.h>
#include <math.h>
#include <mdf.h>
#include <register.h>
#include <stdio.h>
#include <stdlib.h>
#include <vscp_client_base.h>
#include <vscp_client_canal.h>
#include <vscp_client_mqtt.h>
#include <vscp_client_socketcan.h>
#include <vscp_client_tcp.h>
#include <vscp_client_udp.h>
#include <vscp_client_ws1.h>
#include <vscp_client_ws2.h>

#include <vscphelper.h>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <set>
#include <string>

//-----------------------------------------------------------------------------

TEST(Register, Test_API_ReadLevel1Register_Interface)
{
  int rv;
  std::set<uint8_t> found_nodes;

  // Internal device pi5
  vscpClientTcp client;
  rv = client.init("tcp://192.168.1.32:9598", "admin", "secret");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.connect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // CAN4VSCP interface
  cguid guidInterface("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:00");

  // Node 1 Beijing I/O
  cguid guidNode("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:01");

  uint8_t value;
  rv = vscp_readLevel1Register(client, guidNode, guidInterface, 0, 0xf7, value, 1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
  ASSERT_EQ(0x6e, value);

  rv = vscp_readLevel1Register(client, guidNode, guidInterface, 0, 0xf8, value, 1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
  ASSERT_EQ(0x67, value);

  rv = vscp_readLevel1Register(client, guidNode, guidInterface, 0, 0xf9, value, 1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
  ASSERT_EQ(0x5f, value);

  rv = vscp_readLevel1Register(client, guidNode, guidInterface, 0, 0xfa, value, 1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
  ASSERT_EQ(0x32, value);

  rv = vscp_readLevel1Register(client, guidNode, guidInterface, 0, 0xfb, value, 1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
  ASSERT_EQ(0x2e, value);

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

}

//-----------------------------------------------------------------------------

TEST(Register, Test_API_WriteLevel1Register_Interface)
{
  int rv;
  std::set<uint8_t> found_nodes;

  // Internal device pi5
  vscpClientTcp client;
  rv = client.init("tcp://192.168.1.32:9598", "admin", "secret");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.connect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // CAN4VSCP interface
  cguid guidInterface("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:00");

  // Node 1 Beijing I/O
  cguid guidNode("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:01");

  // Write User ID 0
  rv = vscp_writeLevel1Register(client, guidNode, guidInterface, 0, 0x84, 0x55, 1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // Write User ID 0
  rv = vscp_writeLevel1Register(client, guidNode, guidInterface, 0, 0x84, 0xAA, 1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // Write User ID 0
  rv = vscp_writeLevel1Register(client, guidNode, guidInterface, 0, 0x84, 0x00, 1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

}


//-----------------------------------------------------------------------------


TEST(Register, Test_Local_Find_Nodes_Interface)
{
  int rv;
  std::set<uint8_t> nodes;

  // Internal device pi5
  vscpClientTcp client;
  rv = client.init("tcp://192.168.1.32:9598", "admin", "secret");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.connect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // CAN4VSCP interface
  cguid guid("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:00");
  // std::cout << "GUID " << guid.getAsString() << std::endl;

  vscpEventEx ex;
  memset(&ex, 0, sizeof(vscpEventEx));
  ex.vscp_class = VSCP_CLASS1_PROTOCOL + 512;
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_WHO_IS_THERE;
  memcpy(ex.data, guid.getGUID(), 16); // Use GUID of interface
  memset(ex.GUID, 0, 16);              // Use GUID of interface
  ex.sizeData = 17;

  ex.data[16] = 0xff; // all devices

  rv = client.send(ex);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  uint32_t startTime = vscp_getMsTimeStamp();

  while (true) {
    uint16_t cnt;
    client.getcount(&cnt);
    if (cnt) {
      rv = client.receive(ex);
    }
    if (VSCP_ERROR_SUCCESS == rv) {
      // std::cout << std::hex << "Class:" << ex.vscp_class << " type:" << ex.vscp_type << " Byte 0: "  <<
      // (int)ex.data[0] << std::endl;
      if (ex.vscp_class == VSCP_CLASS1_PROTOCOL && ex.vscp_type == VSCP_TYPE_PROTOCOL_WHO_IS_THERE_RESPONSE) {
        // std::cout << std::dec << "   Found: " << (int)ex.GUID[15] << " timestamp: " <<
        // (vscp_getMsTimeStamp()-startTime) << std::endl;
        nodes.insert(ex.GUID[15]);
      }
    }

    if ((vscp_getMsTimeStamp() - startTime) > 500) {
      // std::cout << std::dec << " End timestamp: " << (vscp_getMsTimeStamp()-startTime) << std::endl;
      break;
    }
  }

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // std::cout << std::dec << "   Found: " << nodes.size() << " nodes" << std::endl;
  // for (auto nodeid : nodes) {
  //   std::cout << "   nodeid: " << (int)nodeid << std::endl;
  // }
  ASSERT_EQ(6, nodes.size());
}

//-----------------------------------------------------------------------------

TEST(Register, Test_API_Find_Nodes_Interface)
{
  int rv;
  std::set<uint8_t> nodes;

  // Internal device pi5
  vscpClientTcp client;
  rv = client.init("tcp://192.168.1.32:9598", "admin", "secret");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.connect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  cguid guid("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:00");
  rv = vscp_scanForDevices(client, guid, nodes, 1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // std::cout << std::dec << "   Found: " << nodes.size() << " nodes" << std::endl;
  // for (auto nodeid : nodes) {
  //   std::cout << "   nodeid: " << (int)nodeid << std::endl;
  // }
  ASSERT_EQ(6, nodes.size());
}

//-----------------------------------------------------------------------------

TEST(Register, Test_Local_Slow_Find_Nodes_Interface)
{
  int rv;
  std::set<uint8_t> nodes;

  // Internal device pi5
  vscpClientTcp client;
  rv = client.init("tcp://192.168.1.32:9598", "admin", "secret");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.connect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // CAN4VSCP interface
  cguid guid("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:00");
  // std::cout << "GUID " << guid.getAsString() << std::endl;

  vscpEventEx ex;
  for (int idx = 0; idx < 256; idx++) {
    memset(&ex, 0, sizeof(vscpEventEx));
    ex.vscp_class = VSCP_CLASS1_PROTOCOL + 512;
    ex.vscp_type  = VSCP_TYPE_PROTOCOL_READ_REGISTER;
    memcpy(ex.data, guid.getGUID(), 16); // Use GUID of interface
    memset(ex.GUID, 0, 16);              // Use GUID of interface
    ex.sizeData = 18;

    ex.data[16] = idx;  // nodeid
    ex.data[17] = 0xd0; // register: First byte of GUID

    rv = client.send(ex);
    ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
    usleep(10000);

    // std::cout << std::hex << "index:" << idx << std::endl;
  }

  uint32_t startTime = vscp_getMsTimeStamp();

  while (true) {
    rv = -1;
    uint16_t cnt;

    client.getcount(&cnt);
    if (cnt) {
      rv = client.receive(ex);
    }
    if (VSCP_ERROR_SUCCESS == rv) {
      // std::cout << std::hex << "Class:" << ex.vscp_class << " type:" << ex.vscp_type << " Byte 0: "  <<
      // (int)ex.data[0] << std::endl;
      if (ex.vscp_class == VSCP_CLASS1_PROTOCOL && ex.vscp_type == VSCP_TYPE_PROTOCOL_RW_RESPONSE) {
        // std::cout << std::dec << "   Found: " << (int)ex.GUID[15] << " timestamp: " <<
        // (vscp_getMsTimeStamp()-startTime) << std::endl;
        nodes.insert(ex.GUID[15]);
      }
    }

    if ((vscp_getMsTimeStamp() - startTime) > 2000) {
      // std::cout << std::dec << " End timestamp: " << (vscp_getMsTimeStamp()-startTime) << std::endl;
      break;
    }
  }

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // std::cout << std::dec << "   Found: " << nodes.size() << " nodes" << std::endl;
  // for (auto nodeid : nodes) {
  //   std::cout << "   nodeid: " << (int)nodeid << std::endl;
  // }
  ASSERT_EQ(6, nodes.size());
}

//-----------------------------------------------------------------------------

TEST(Register, Test_API_Slow_Find_Nodes_Interface)
{
  int rv;
  std::set<uint8_t> found_nodes;

  // Internal device pi5
  vscpClientTcp client;
  rv = client.init("tcp://192.168.1.32:9598", "admin", "secret");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.connect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // CAN4VSCP interface
  cguid guid("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:00");
  // std::cout << "GUID " << guid.getAsString() << std::endl;

  rv = vscp_scanSlowForDevices(client, guid, found_nodes, 0, 255, 10000, 5000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  ASSERT_EQ(6, found_nodes.size());

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // std::cout << std::dec << "   Found: " << nodes.size() << " nodes" << std::endl;
  // for (auto nodeid : nodes) {
  //   std::cout << "   nodeid: " << (int)nodeid << std::endl;
  // }
  ASSERT_EQ(6, found_nodes.size());
}



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------




int
main(int argc, char **argv)
{
  // Init pool
  spdlog::init_thread_pool(8192, 1);

  auto console = spdlog::stdout_color_mt("console");

  // console->set_level(spdlog::level::off);
  console->set_level(spdlog::level::err);
  // console->set_level(spdlog::level::trace);

  console->set_pattern("[mdfparser: %c] [%^%l%$] %v");
  spdlog::set_default_logger(console);

  console->info("mdfparser: Starting tmdpparser test...");

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
