// Unit tests for the Level II register functions in register.cpp
// (CLASS2.PROTOCOL read/write register) against a simulated node.

#include <gtest/gtest.h>

#include <guid.h>
#include <mdf.h>
#include <register.h>
#include <vscp-class.h>
#include <vscp-client-base.h>
#include <vscp-type.h>
#include <vscp.h>

#include <algorithm>
#include <cstring>
#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {

const char *NODE_GUID  = "FF:FF:FF:FF:FF:FF:FF:FE:02:76:85:E3:37:76:00:00";
const char *OTHER_GUID = "FF:FF:FF:FF:FF:FF:FF:FE:02:76:85:E3:37:77:00:00";

/*
  A Level II node behind a client: answers CLASS2.PROTOCOL read/write
  register for its GUID with Read Write Responses of at most maxResponse
  registers each (a node may split a response). Read only registers keep
  their value when written.
*/
class FakeLevel2Node : public CVscpClient {
public:
  FakeLevel2Node()
    : m_guid(NODE_GUID)
  {
  }

  std::map<uint32_t, uint8_t> regs; // Missing registers read as zero
  std::set<uint32_t> readOnly;
  uint16_t maxResponse    = 508;
  bool reverseResponses   = false; // Send split responses last part first
  bool silent             = false; // Never answer
  bool answerOtherNode    = false; // Also a response from another node first
  std::vector<vscpEventEx> sent;   // Requests seen

  int connect(void) override { return VSCP_ERROR_SUCCESS; }
  int disconnect(void) override { return VSCP_ERROR_SUCCESS; }
  bool isConnected(void) override { return true; }
  int send(vscpEvent & /*ev*/) override { return VSCP_ERROR_NOT_SUPPORTED; }
  int send(canalMsg & /*msg*/) override { return VSCP_ERROR_NOT_SUPPORTED; }
  int receive(vscpEvent & /*ev*/) override { return VSCP_ERROR_NOT_SUPPORTED; }
  int receiveBlocking(vscpEvent & /*ev*/, long /*timeout*/) override { return VSCP_ERROR_NOT_SUPPORTED; }
  int receiveBlocking(vscpEventEx &ex, long /*timeout*/) override { return receive(ex); }
  int receive(canalMsg & /*msg*/) override { return VSCP_ERROR_NOT_SUPPORTED; }
  int receiveBlocking(canalMsg & /*msg*/, long /*timeout*/) override { return VSCP_ERROR_NOT_SUPPORTED; }
  int setfilter(vscpEventFilter & /*filter*/) override { return VSCP_ERROR_SUCCESS; }
  int getversion(uint8_t *, uint8_t *, uint8_t *, uint8_t *) override { return VSCP_ERROR_SUCCESS; }
  int getinterfaces(std::deque<std::string> & /*iflist*/) override { return VSCP_ERROR_SUCCESS; }
  int getwcyd(uint64_t &wcyd) override
  {
    wcyd = 0;
    return VSCP_ERROR_SUCCESS;
  }
  void setConnectionTimeout(uint32_t /*timeout*/) override {}
  uint32_t getConnectionTimeout(void) override { return 0; }
  void setResponseTimeout(uint32_t /*timeout*/) override {}
  uint32_t getResponseTimeout(void) override { return 0; }
  std::string getConfigAsJson(void) override { return "{}"; }
  bool initFromJson(const std::string & /*config*/) override { return true; }

  int getcount(uint16_t *pcount) override
  {
    *pcount = (uint16_t) m_out.size();
    return VSCP_ERROR_SUCCESS;
  }

  int clear(void) override
  {
    m_out.clear();
    return VSCP_ERROR_SUCCESS;
  }

  int receive(vscpEventEx &ex) override
  {
    if (m_out.empty()) {
      return VSCP_ERROR_RCV_EMPTY;
    }
    ex = m_out.front();
    m_out.pop_front();
    return VSCP_ERROR_SUCCESS;
  }

  int send(vscpEventEx &ex) override
  {
    sent.push_back(ex);
    if (silent || (VSCP_CLASS2_PROTOCOL != ex.vscp_class)) {
      return VSCP_ERROR_SUCCESS;
    }
    if (0 != memcmp(ex.data, m_guid.getGUID(), 16)) {
      return VSCP_ERROR_SUCCESS; // Not to this node
    }
    uint32_t start = ((uint32_t) ex.data[16] << 24) | ((uint32_t) ex.data[17] << 16) |
                     ((uint32_t) ex.data[18] << 8) | ex.data[19];
    uint32_t count;
    if (VSCP2_TYPE_PROTOCOL_READ_REGISTER == ex.vscp_type) {
      count = ((uint32_t) ex.data[20] << 8) | ex.data[21];
    }
    else if (VSCP2_TYPE_PROTOCOL_WRITE_REGISTER == ex.vscp_type) {
      count = ex.sizeData - 20;
      for (uint32_t i = 0; i < count; i++) {
        if (!readOnly.count(start + i)) {
          regs[start + i] = ex.data[20 + i];
        }
      }
    }
    else {
      return VSCP_ERROR_SUCCESS;
    }

    if (answerOtherNode) {
      // Same start register, other node: must be ignored
      m_out.push_back(response(cguid(OTHER_GUID), start, count, 0xEE));
    }
    std::deque<vscpEventEx> parts;
    for (uint32_t done = 0; done < count; done += maxResponse) {
      uint32_t n = std::min<uint32_t>(maxResponse, count - done);
      parts.push_back(response(m_guid, start + done, n));
    }
    if (reverseResponses) {
      std::reverse(parts.begin(), parts.end());
    }
    m_out.insert(m_out.end(), parts.begin(), parts.end());
    return VSCP_ERROR_SUCCESS;
  }

private:
  vscpEventEx response(cguid guid, uint32_t start, uint32_t n, int fill = -1)
  {
    vscpEventEx r;
    memset(&r, 0, sizeof(r));
    r.vscp_class = VSCP_CLASS2_PROTOCOL;
    r.vscp_type  = VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE;
    memcpy(r.GUID, guid.getGUID(), 16);
    r.sizeData = (uint16_t) (4 + n);
    r.data[0]  = (start >> 24) & 0xff;
    r.data[1]  = (start >> 16) & 0xff;
    r.data[2]  = (start >> 8) & 0xff;
    r.data[3]  = start & 0xff;
    for (uint32_t i = 0; i < n; i++) {
      r.data[4 + i] = (fill >= 0) ? (uint8_t) fill : regs[start + i];
    }
    return r;
  }

  cguid m_guid;
  std::deque<vscpEventEx> m_out;
};

uint32_t
request_start(const vscpEventEx &ex)
{
  return ((uint32_t) ex.data[16] << 24) | ((uint32_t) ex.data[17] << 16) | ((uint32_t) ex.data[18] << 8) |
         ex.data[19];
}

} // namespace

TEST(RegisterLevel2, ReadRequestFormat)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  node.regs[0x12345678] = 0x42;

  uint8_t value = 0;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_readLevel2Register(node, guid, 0x12345678, value, 500));
  ASSERT_EQ(0x42, value);

  ASSERT_EQ(1u, node.sent.size());
  const vscpEventEx &ex = node.sent[0];
  ASSERT_EQ(VSCP_CLASS2_PROTOCOL, ex.vscp_class);
  ASSERT_EQ(VSCP2_TYPE_PROTOCOL_READ_REGISTER, ex.vscp_type);
  ASSERT_EQ(22, ex.sizeData);
  ASSERT_EQ(0, memcmp(ex.data, guid.getGUID(), 16));
  ASSERT_EQ(0x12345678u, request_start(ex));
  ASSERT_EQ(0, ex.data[20]);
  ASSERT_EQ(1, ex.data[21]);
}

TEST(RegisterLevel2, ReadBlockSplitIntoRequests)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  for (uint32_t i = 0; i < 300; i++) {
    node.regs[0x1000 + i] = (uint8_t) i;
  }

  std::map<uint32_t, uint8_t> values;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_readLevel2RegisterBlock(node, guid, 0x1000, 300, values, nullptr, 500, 128));
  ASSERT_EQ(3u, node.sent.size()); // 128 + 128 + 44
  ASSERT_EQ(0x1080u, request_start(node.sent[1]));
  ASSERT_EQ(44, node.sent[2].data[21]);
  ASSERT_EQ(300u, values.size());
  for (uint32_t i = 0; i < 300; i++) {
    ASSERT_EQ((uint8_t) i, values[0x1000 + i]) << i;
  }
}

TEST(RegisterLevel2, ReadCollectsSplitResponsesInAnyOrder)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  node.maxResponse      = 7;
  node.reverseResponses = true;
  for (uint32_t i = 0; i < 20; i++) {
    node.regs[i] = (uint8_t) (0xA0 + i);
  }

  std::map<uint32_t, uint8_t> values;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_readLevel2RegisterBlock(node, guid, 0, 20, values, nullptr, 500));
  ASSERT_EQ(1u, node.sent.size());
  ASSERT_EQ(20u, values.size());
  ASSERT_EQ(0xA0, values[0]);
  ASSERT_EQ(0xB3, values[19]);
}

TEST(RegisterLevel2, ResponseFromOtherNodeIgnored)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  node.answerOtherNode = true;
  node.regs[5]         = 0x11;

  uint8_t value = 0;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_readLevel2Register(node, guid, 5, value, 500));
  ASSERT_EQ(0x11, value); // not 0xEE from the other node
}

TEST(RegisterLevel2, ReadStandardRegistersAtTheTop)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  for (uint32_t i = 0; i < 16; i++) {
    node.regs[0xffffffd0 + i] = guid.getGUID()[i];
  }
  node.regs[0xffffff81] = 1; // Conformance version
  node.regs[0xffffff82] = 6;
  const char *url       = "eurosource.se/blinky0.json";
  for (uint32_t i = 0; i < strlen(url); i++) {
    node.regs[0xffffffe0 + i] = (uint8_t) url[i];
  }

  CStandardRegisters stdregs;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, stdregs.initLevel2(node, guid, nullptr, 500));
  ASSERT_EQ(1u, node.sent.size()); // 0xffffff80, 128 registers
  ASSERT_EQ(0xffffff80u, request_start(node.sent[0]));
  ASSERT_EQ(128, node.sent[0].data[21]);
  ASSERT_EQ(1, stdregs.getConformanceVersionMajor());
  ASSERT_EQ(6, stdregs.getConformanceVersionMinor());
  ASSERT_EQ(guid.toString(), stdregs.getGUIDStr());
  ASSERT_NE(std::string::npos, stdregs.getMDF().find("eurosource.se/blinky0.json"));
}

TEST(RegisterLevel2, ReadPastTheLastRegisterRefused)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  std::map<uint32_t, uint8_t> values;
  ASSERT_EQ(VSCP_ERROR_PARAMETER, vscp_readLevel2RegisterBlock(node, guid, 0xffffff80, 129, values));
  ASSERT_EQ(VSCP_ERROR_PARAMETER, vscp_readLevel2RegisterBlock(node, guid, 0, 0, values));
  ASSERT_EQ(0u, node.sent.size());
}

TEST(RegisterLevel2, ReadTimeout)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  node.silent   = true;
  uint8_t value = 0;
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, vscp_readLevel2Register(node, guid, 0, value, 50));
}

TEST(RegisterLevel2, WriteGroupsRegistersNextToEachOther)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);

  std::map<uint32_t, uint8_t> values = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 0x60, 4 }, { 0xffffff84, 5 } };
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_writeLevel2RegisterBlock(node, guid, values, nullptr, 500));
  ASSERT_TRUE(values.empty()); // all read back as written

  ASSERT_EQ(3u, node.sent.size());
  ASSERT_EQ(VSCP2_TYPE_PROTOCOL_WRITE_REGISTER, node.sent[0].vscp_type);
  ASSERT_EQ(0u, request_start(node.sent[0]));
  ASSERT_EQ(23, node.sent[0].sizeData);
  ASSERT_EQ(3, node.sent[0].data[22]);
  ASSERT_EQ(0x60u, request_start(node.sent[1]));
  ASSERT_EQ(0xffffff84u, request_start(node.sent[2]));
  ASSERT_EQ(5, node.regs[0xffffff84]);
}

TEST(RegisterLevel2, WriteSplitAtMaxPerWrite)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  std::map<uint32_t, uint8_t> values;
  for (uint32_t i = 0; i < 250; i++) {
    values[i] = (uint8_t) i;
  }
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_writeLevel2RegisterBlock(node, guid, values, nullptr, 500, 100));
  ASSERT_EQ(3u, node.sent.size());
  ASSERT_EQ(120, node.sent[0].sizeData);
  ASSERT_EQ(100u, request_start(node.sent[1]));
  ASSERT_EQ(70, node.sent[2].sizeData);
  ASSERT_EQ(249, node.regs[249]);
}

TEST(RegisterLevel2, WriteReadOnlyRegisterReported)
{
  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  node.readOnly.insert(2);
  node.regs[2] = 0x77;

  std::map<uint32_t, uint8_t> values = { { 1, 1 }, { 2, 2 }, { 3, 3 } };
  ASSERT_EQ(VSCP_ERROR_WRITE_ERROR, vscp_writeLevel2RegisterBlock(node, guid, values, nullptr, 500));
  ASSERT_EQ(1u, values.size()); // the register that was not written is left
  ASSERT_EQ(1u, values.count(2));

  ASSERT_EQ(VSCP_ERROR_WRITE_ERROR, vscp_writeLevel2Register(node, guid, 2, 9, 500));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_writeLevel2Register(node, guid, 3, 9, 500));
}

TEST(RegisterLevel2, UserRegistersFromMdf)
{
  CMDF mdf;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(std::string(TEST_MDF_JSON_DIR) + "/blinky0.json"));

  FakeLevel2Node node;
  cguid guid(NODE_GUID);
  for (uint32_t i = 0; i < 0x80; i++) {
    node.regs[i] = (uint8_t) (i + 1);
  }

  CUserRegisters userregs;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, userregs.initLevel2(node, guid, mdf, nullptr, 500));

  // Only Level II reads, no register outside the user space
  ASSERT_FALSE(node.sent.empty());
  for (auto const &ex : node.sent) {
    ASSERT_EQ(VSCP_CLASS2_PROTOCOL, ex.vscp_class);
    ASSERT_EQ(VSCP2_TYPE_PROTOCOL_READ_REGISTER, ex.vscp_type);
    ASSERT_LT(request_start(ex), 0xffffff80u);
  }

  // Every register in the MDF is read, with the node's value
  for (auto const preg : *mdf.getRegisterObjList()) {
    for (uint32_t i = 0; i < std::max<uint32_t>(1, preg->getSpan()); i++) {
      uint32_t reg = preg->getOffset() + i;
      ASSERT_EQ(node.regs[reg], userregs.getReg(reg, (uint16_t) preg->getPage())) << reg;
    }
  }
  CMDF_DecisionMatrix *pdm = mdf.getDM();
  if (pdm->getRowCount()) {
    uint32_t last = pdm->getStartOffset() + pdm->getRowCount() * pdm->getRowSize() - 1;
    ASSERT_EQ(node.regs[last], userregs.getReg(last, pdm->getStartPage()));
  }
  ASSERT_FALSE(userregs.hasChanges());

  // A changed register can be written back at its offset
  ASSERT_TRUE(userregs.putReg(1, 0, 0x5A));
  ASSERT_TRUE(userregs.isChanged(1, 0));
}
