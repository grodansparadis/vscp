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


TEST(Register, Test_API_ReadLevel1RegisterBlock_Interface)
{
  int rv;
  std::map<uint8_t, uint8_t> regs;

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

  rv = vscp_readLevel1RegisterBlock(client, guidNode, guidInterface, 0, 0xd0, 16, regs, 5000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
  ASSERT_EQ(regs.size(), 16);
  ASSERT_EQ(regs[0xd0], 1);
  ASSERT_EQ(regs[0xd1], 0);
  ASSERT_EQ(regs[0xd2], 0);
  ASSERT_EQ(regs[0xd3], 0);
  ASSERT_EQ(regs[0xd4], 0);
  ASSERT_EQ(regs[0xd5], 0);
  ASSERT_EQ(regs[0xd6], 0);
  ASSERT_EQ(regs[0xd7], 0);
  ASSERT_EQ(regs[0xd8], 0);
  ASSERT_EQ(regs[0xd9], 0);
  ASSERT_EQ(regs[0xda], 0);
  ASSERT_EQ(regs[0xdb], 0);
  ASSERT_EQ(regs[0xdc], 6);
  ASSERT_EQ(regs[0xdd], 0);
  ASSERT_EQ(regs[0xde], 0);
  ASSERT_EQ(regs[0xdf], 15);

  regs.clear();
  rv = vscp_readLevel1RegisterBlock(client, guidNode, guidInterface, 0, 0xd0, 2, regs, 5000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
  ASSERT_EQ(regs.size(), 2);
  ASSERT_EQ(regs[0xd0], 1);
  ASSERT_EQ(regs[0xd1], 0);

  regs.clear();
  rv = vscp_readLevel1RegisterBlock(client, guidNode, guidInterface, 0, 0xd0, 12, regs, 5000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
  ASSERT_EQ(regs.size(), 12);
  ASSERT_EQ(regs[0xd0], 1);
  ASSERT_EQ(regs[0xd1], 0);
  ASSERT_EQ(regs[0xd2], 0);
  ASSERT_EQ(regs[0xd3], 0);
  ASSERT_EQ(regs[0xd4], 0);
  ASSERT_EQ(regs[0xd5], 0);
  ASSERT_EQ(regs[0xd6], 0);
  ASSERT_EQ(regs[0xd7], 0);
  ASSERT_EQ(regs[0xd8], 0);
  ASSERT_EQ(regs[0xd9], 0);
  ASSERT_EQ(regs[0xda], 0);
  ASSERT_EQ(regs[0xdb], 0);

  regs.clear();
  rv = vscp_readLevel1RegisterBlock(client, guidNode, guidInterface, 0, 0x80, 128, regs, 5000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
  ASSERT_EQ(regs.size(), 0x80);
  ASSERT_EQ(regs[0xd0], 1);
  ASSERT_EQ(regs[0xd1], 0);
  ASSERT_EQ(regs[0xd2], 0);
  ASSERT_EQ(regs[0xd3], 0);
  ASSERT_EQ(regs[0xd4], 0);
  ASSERT_EQ(regs[0xd5], 0);
  ASSERT_EQ(regs[0xd6], 0);
  ASSERT_EQ(regs[0xd7], 0);
  ASSERT_EQ(regs[0xd8], 0);
  ASSERT_EQ(regs[0xd9], 0);
  ASSERT_EQ(regs[0xda], 0);
  ASSERT_EQ(regs[0xdb], 0);
  ASSERT_EQ(regs[0xdc], 6);
  ASSERT_EQ(regs[0xdd], 0);
  ASSERT_EQ(regs[0xde], 0);
  ASSERT_EQ(regs[0xdf], 0x0f);

  ASSERT_EQ(regs[0xe0], 0x77);
  ASSERT_EQ(regs[0xe1], 0x77);
  ASSERT_EQ(regs[0xe2], 0x77);
  ASSERT_EQ(regs[0xe3], 0x2e);
  ASSERT_EQ(regs[0xe4], 0x65);
  ASSERT_EQ(regs[0xe5], 0x75);
  ASSERT_EQ(regs[0xe6], 0x72);
  ASSERT_EQ(regs[0xe7], 0x6f);
  ASSERT_EQ(regs[0xe8], 0x73);
  ASSERT_EQ(regs[0xe9], 0x6f);
  ASSERT_EQ(regs[0xea], 0x75);
  ASSERT_EQ(regs[0xeb], 0x72);
  ASSERT_EQ(regs[0xec], 0x63);
  ASSERT_EQ(regs[0xed], 0x65);
  ASSERT_EQ(regs[0xee], 0x2e);
  ASSERT_EQ(regs[0xef], 0x73);

  ASSERT_EQ(regs[0xf0], 0x65);
  ASSERT_EQ(regs[0xf1], 0x2f);
  ASSERT_EQ(regs[0xf2], 0x62);
  ASSERT_EQ(regs[0xf3], 0x65);
  ASSERT_EQ(regs[0xf4], 0x69);
  ASSERT_EQ(regs[0xf5], 0x6a);
  ASSERT_EQ(regs[0xf6], 0x69);
  ASSERT_EQ(regs[0xf7], 0x6e);
  ASSERT_EQ(regs[0xf8], 0x67);
  ASSERT_EQ(regs[0xf9], 0x5f);
  ASSERT_EQ(regs[0xfa], 0x32);
  ASSERT_EQ(regs[0xfb], 0x2e);
  ASSERT_EQ(regs[0xfc], 0x78);
  ASSERT_EQ(regs[0xfd], 0x6d);
  ASSERT_EQ(regs[0xfe], 0x6c);
  ASSERT_EQ(regs[0xff], 0x00);

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

}


//-----------------------------------------------------------------------------


TEST(Register, Test_API_writeLevel1RegisterBlock_Interface)
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

  // Node GUID
  cguid guidNode("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:01");

  // Regvalues - Use DM on page 2 for test
  std::map<uint8_t, uint8_t> regvalue;
  regvalue[0x19] = 0x01;
  regvalue[0x1A] = 0x02;
  regvalue[0x1B] = 0x03;

  regvalue[0x1D] = 0x04;
  regvalue[0x1E] = 0x05;

  regvalue[0x20] = 0x06;

  rv = vscp_writeLevel1RegisterBlock( client,
                                      guidNode,
                                      guidInterface,
                                      2, // page
                                      regvalue,
                                      1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);                                    

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
}


//-----------------------------------------------------------------------------


TEST(Register, Test_Class_CStandardRegisters)
{
  int rv;
  std::set<uint8_t> nodes;

  // Internal device pi5
  vscpClientTcp client;
  rv = client.init("tcp://192.168.1.32:9598", "admin", "secret");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.connect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  cguid guidNode("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:01");

  // CAN4VSCP interface
  cguid guidInterface("FF:FF:FF:FF:FF:FF:FF:F5:01:00:00:00:00:00:00:00");
  // std::cout << "GUID " << guid.getAsString() << std::endl;

  CStandardRegisters stdregs;
  rv = stdregs.init(client, guidNode, guidInterface);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  // Check alarm byte
  ASSERT_EQ(0, stdregs.getAlarm());

  // Check confirmance version
  ASSERT_EQ(1, stdregs.getConfirmanceVersionMajor());
  ASSERT_EQ(6, stdregs.getConfirmanceVersionMinor());

  // User id bytes can vary in content so no check here

  // Check manufacturer id
  ASSERT_EQ(0x000f0b16, stdregs.getManufacturerDeviceID());
  ASSERT_EQ(0x01000000, stdregs.getManufacturerSubDeviceID());

  // Check nickname
  ASSERT_EQ(1, stdregs.getNickname());

  // Check firmware version
  ASSERT_EQ(1, stdregs.getFirmwareMajorVersion());
  ASSERT_EQ(1, stdregs.getFirmwareMinorVersion());
  ASSERT_EQ(6, stdregs.getFirmwareSubMinorVersion());

  // Get bootloader algorithm
  ASSERT_EQ(1, stdregs.getBootloaderAlgorithm());

  // Buffer size
  ASSERT_EQ(8, stdregs.getBufferSize());

  // Number of register pages
  ASSERT_EQ(3, stdregs.getNumberOfRegisterPages());

  // Standard device family code
  ASSERT_EQ(0, stdregs.getStandardDeviceFamilyCode());

  // Standard device family type
  ASSERT_EQ(0, stdregs.getStandardDeviceFamilyType());

  // Firmware device code
  ASSERT_EQ(0, stdregs.getFirmwareDeviceCode());

  // Get GUID
  cguid guid;
  stdregs.getGUID(guid);
  ASSERT_EQ("01:00:00:00:00:00:00:00:00:00:00:00:06:00:00:0F", guid.getAsString());

 
  std::string str = stdregs.getMDF();
  //std::cout << "MDF: " << str << std::endl;
  ASSERT_EQ("www.eurosource.se/beijing_2.xml", str);

  rv = client.disconnect();
  
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

    if ((vscp_getMsTimeStamp() - startTime) > 10000) {
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

  rv = vscp_scanSlowForDevices(client, guid, found_nodes, 0, 255, 20000, 5000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  ASSERT_EQ(6, found_nodes.size());

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

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
