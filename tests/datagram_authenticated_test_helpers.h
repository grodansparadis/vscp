#ifndef VSCP_DATAGRAM_AUTHENTICATED_TEST_HELPERS_H
#define VSCP_DATAGRAM_AUTHENTICATED_TEST_HELPERS_H

#include "datagram_test_helpers.h"

#include <vscp-binary-protocol.h>
#include <openssl/hmac.h>

#include <array>
#include <atomic>
#include <memory>

inline const char *
testDatagramAesKey()
{
  return "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";
}

inline const char *
testDatagramHmacKey()
{
  return "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f";
}

inline std::array<uint8_t, 32>
testDatagramKey(const char *hex)
{
  std::array<uint8_t, 32> key{};
  EXPECT_EQ(key.size(), vscp_hexStr2ByteArray(key.data(), key.size(), hex));
  return key;
}

inline nlohmann::json
authenticatedDatagramConfig(uint8_t encryption)
{
  const size_t keyLength = 16 + 8 * (encryption - VSCP_ENCRYPTION_AES128);
  return {{"encryption", encryption},
          {"key", std::string(testDatagramAesKey()).substr(0, keyLength * 2)},
          {"hmac-key", testDatagramHmacKey()}};
}

template<typename Client>
void
verifyAuthenticatedDatagramOptions(const std::string &address)
{
  using json = nlohmann::json;
  Client missing;
  EXPECT_EQ("", json::parse(missing.getConfigAsJson())["hmac-key"]);
  EXPECT_FALSE(missing.initFromJson(json{{"hmac-key", testDatagramHmacKey()}}.dump()));
  EXPECT_FALSE(missing.initFromJson(json{{"encryption", VSCP_ENCRYPTION_AES128},
                                         {"hmac-key", testDatagramHmacKey()}}.dump()));

  for (uint8_t encryption = VSCP_ENCRYPTION_AES128; encryption <= VSCP_ENCRYPTION_AES256; ++encryption) {
    SCOPED_TRACE(encryption);
    Client client;
    json config = authenticatedDatagramConfig(encryption);
    config["ip"] = address;
    ASSERT_TRUE(client.initFromJson(config.dump()));
    const json accepted = json::parse(client.getConfigAsJson());
    const json acceptedInput = client.m_j_config;
    EXPECT_EQ(testDatagramHmacKey(), accepted["hmac-key"]);

    Client restored;
    ASSERT_TRUE(restored.initFromJson(accepted.dump()));
    EXPECT_EQ(accepted, json::parse(restored.getConfigAsJson()));

    const size_t keyLength = 16 + 8 * (encryption - VSCP_ENCRYPTION_AES128);
    const std::string reusedKey = std::string(testDatagramAesKey()).substr(0, keyLength * 2) +
                                  std::string(64 - keyLength * 2, 'f');
    const std::vector<json> invalid = {
      json{{"hmac-key", 1}},
      json{{"hmac-key", nullptr}},
      json{{"hmac-key", std::string(63, 'a')}},
      json{{"hmac-key", std::string(64, 'z')}},
      json{{"hmac-key", reusedKey}},
      json{{"encryption", VSCP_ENCRYPTION_NONE}},
      json{{"encryption", encryption == VSCP_ENCRYPTION_AES256 ? VSCP_ENCRYPTION_AES128 : encryption + 1}},
      json{{"key", "abcd"}},
      json{{"class-filter", 7}, {"hmac-key", "bad"}}
    };
    for (const json &option : invalid) {
      EXPECT_FALSE(client.initFromJson(option.dump())) << option;
      EXPECT_EQ(accepted, json::parse(client.getConfigAsJson()));
      EXPECT_EQ(acceptedInput, client.m_j_config);
    }

    ASSERT_TRUE(client.initFromJson(R"({"class-filter":7})"));
    EXPECT_EQ(testDatagramHmacKey(), json::parse(client.getConfigAsJson())["hmac-key"]);
    client.m_encryptType = VSCP_ENCRYPTION_NONE;
    EXPECT_EQ(VSCP_ERROR_PARAMETER, client.connect());
    EXPECT_FALSE(client.initFromJson("{}"));
    client.m_encryptType = encryption;
    client.m_key[0] ^= 0x40;
    EXPECT_EQ(VSCP_ERROR_PARAMETER, client.connect());
    EXPECT_FALSE(client.initFromJson("{}"));
    client.m_key[0] ^= 0x40;
    ASSERT_TRUE(client.initFromJson(R"({"hmac-key":"","encryption":0})"));
    EXPECT_EQ("", json::parse(client.getConfigAsJson())["hmac-key"]);

    Client incremental;
    ASSERT_TRUE(incremental.initFromJson(json{{"encryption", encryption}, {"key", config["key"]}}.dump()));
    incremental.m_key[0] ^= 0x40;
    EXPECT_FALSE(incremental.initFromJson(json{{"hmac-key", testDatagramHmacKey()}}.dump()));
    incremental.m_key[0] ^= 0x40;
    ASSERT_TRUE(incremental.initFromJson(json{{"hmac-key", testDatagramHmacKey()}}.dump()));
    EXPECT_EQ(testDatagramHmacKey(), json::parse(incremental.getConfigAsJson())["hmac-key"]);

    Client direct;
    direct.m_encryptType = encryption;
    EXPECT_FALSE(direct.initFromJson(json{{"key", config["key"]}, {"hmac-key", testDatagramHmacKey()}}.dump()));
    ASSERT_TRUE(direct.initFromJson(config.dump()));
  }
}

class AuthenticatedDatagramSender {
public:
  AuthenticatedDatagramSender(uint16_t port, bool multicast)
    : m_socket(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)), m_target{}
  {
    if (m_socket == invalidTestSocket) {
      return;
    }
    m_target.sin_family = AF_INET;
#ifdef __APPLE__
    m_target.sin_len = sizeof(m_target);
#endif
    m_target.sin_port = htons(port);
    const char *address = multicast ? "239.255.23.158" : "127.0.0.1";
    if (inet_pton(AF_INET, address, &m_target.sin_addr) != 1) {
      closeTestSocket(m_socket);
      m_socket = invalidTestSocket;
      return;
    }
    if (multicast) {
      in_addr local{};
      local.s_addr = htonl(INADDR_LOOPBACK);
      const unsigned char loop = 1;
      if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_IF,
                     reinterpret_cast<const char *>(&local), sizeof(local)) != 0 ||
          setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP,
                     reinterpret_cast<const char *>(&loop), sizeof(loop)) != 0) {
        closeTestSocket(m_socket);
        m_socket = invalidTestSocket;
      }
    }
  }

  ~AuthenticatedDatagramSender()
  {
    if (m_socket != invalidTestSocket) {
      closeTestSocket(m_socket);
    }
  }

  bool valid() const { return m_socket != invalidTestSocket; }

  bool send(const std::vector<uint8_t> &wire) const
  {
    const int sent = static_cast<int>(sendto(m_socket, reinterpret_cast<const char *>(wire.data()),
                                              static_cast<int>(wire.size()), 0,
                                              reinterpret_cast<const sockaddr *>(&m_target), sizeof(m_target)));
    return sent == static_cast<int>(wire.size());
  }

private:
  TestSocket m_socket;
  sockaddr_in m_target;
};

inline bool
retagDatagram(std::vector<uint8_t> &wire, const std::array<uint8_t, 32> &key)
{
  if (wire.size() < 16) {
    return false;
  }
  uint8_t digest[EVP_MAX_MD_SIZE];
  unsigned int length = 0;
  if (!HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()), wire.data(), wire.size() - 16,
            digest, &length) || length != 32) {
    return false;
  }
  std::memcpy(wire.data() + wire.size() - 16, digest, 16);
  return true;
}

template<typename Client>
void
verifyAuthenticatedDatagramReceive(Client &client, uint16_t port, uint8_t encryption, bool multicast)
{
  const auto aes = testDatagramKey(testDatagramAesKey());
  const auto hmac = testDatagramKey(testDatagramHmacKey());
  auto callbacks = std::make_shared<std::atomic<int>>(0);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setCallbackEv([callbacks](vscpEvent &, void *) { ++*callbacks; }));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  EXPECT_FALSE(client.initFromJson(R"({"hmac-key":""})"));
  AuthenticatedDatagramSender sender(port, multicast);
  ASSERT_TRUE(sender.valid());

  uint8_t data[] = {0x12, 0x34, 0x56};
  vscpEvent event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  event.timestamp_ns = 123456789;
  event.vscp_class = 123;
  event.vscp_type = 45;
  event.pdata = data;
  event.sizeData = sizeof(data);
  std::vector<uint8_t> plain(vscp_getFrameSizeFromEvent(&event));
  ASSERT_TRUE(vscp_writeEventToFrame(plain.data(), plain.size(), 0, &event));

  std::vector<uint8_t> wire;
  ASSERT_TRUE(vscp_binary::encode(plain, encryption, aes.data(), wire,
                                  vscp_binary::authenticatedVersion, hmac.data()));
  std::vector<std::vector<uint8_t>> invalid;
  invalid.push_back({0x55});
  invalid.push_back(plain);
  invalid.emplace_back(wire.begin() + vscp_binary::headerSize, wire.end());
  invalid.push_back(wire);
  invalid.back()[0] ^= 1;
  invalid.push_back(wire);
  invalid.back()[1] &= 0x1f;
  ASSERT_TRUE(retagDatagram(invalid.back(), hmac));
  invalid.push_back(wire);
  invalid.back()[1] = (invalid.back()[1] & 0x1f) | 0x40;
  ASSERT_TRUE(retagDatagram(invalid.back(), hmac));
  invalid.push_back(wire);
  invalid.back()[2] ^= 1;
  invalid.push_back(wire);
  invalid.back().push_back(0);
  invalid.push_back(wire);
  invalid.back().pop_back();
  invalid.push_back(wire);
  invalid.back()[3] = 0xe0 | encryption;
  ASSERT_TRUE(retagDatagram(invalid.back(), hmac));
  invalid.push_back(wire);
  invalid.back()[3] &= 0xf0;
  ASSERT_TRUE(retagDatagram(invalid.back(), hmac));
  invalid.push_back(wire);
  invalid.back()[4] ^= 1;
  invalid.push_back(wire);
  invalid.back()[invalid.back().size() - 32] ^= 1;
  invalid.push_back(wire);
  invalid.back().back() ^= 1;

  const size_t ciphertextLength = wire.size() - vscp_binary::headerSize - 1 - 32;
  ASSERT_GE(ciphertextLength, 32u);
  invalid.push_back(wire);
  invalid.back()[4 + ciphertextLength - 17] ^= 0x10;
  ASSERT_TRUE(retagDatagram(invalid.back(), hmac));

  std::vector<uint8_t> modified = plain;
  modified.back() ^= 1;
  invalid.emplace_back();
  ASSERT_TRUE(vscp_binary::encode(modified, encryption, aes.data(), invalid.back(),
                                  vscp_binary::authenticatedVersion, hmac.data()));
  modified = plain;
  modified.insert(modified.end(), {0x99, 0, 0});
  vscp_binary::setCrc(modified);
  invalid.emplace_back();
  ASSERT_TRUE(vscp_binary::encode(modified, encryption, aes.data(), invalid.back(),
                                  vscp_binary::authenticatedVersion, hmac.data()));

  std::vector<uint8_t> tooMuchData(VSCP_LEVEL2_MAXDATA + 1, 0x42);
  vscpEvent oversized = event;
  oversized.pdata = tooMuchData.data();
  oversized.sizeData = static_cast<uint16_t>(tooMuchData.size());
  modified.resize(vscp_getFrameSizeFromEvent(&oversized));
  ASSERT_TRUE(vscp_writeEventToFrame(modified.data(), modified.size(), 0, &oversized));
  invalid.emplace_back();
  ASSERT_TRUE(vscp_binary::encode(modified, encryption, aes.data(), invalid.back(),
                                  vscp_binary::authenticatedVersion, hmac.data()));

  auto wrongAes = aes;
  wrongAes[0] ^= 1;
  invalid.emplace_back();
  ASSERT_TRUE(vscp_binary::encode(plain, encryption, wrongAes.data(), invalid.back(),
                                  vscp_binary::authenticatedVersion, hmac.data()));
  auto wrongHmac = hmac;
  wrongHmac[0] ^= 1;
  invalid.emplace_back();
  ASSERT_TRUE(vscp_binary::encode(plain, encryption, aes.data(), invalid.back(),
                                  vscp_binary::authenticatedVersion, wrongHmac.data()));
  const uint8_t otherEncryption = encryption == VSCP_ENCRYPTION_AES128 ?
                                  VSCP_ENCRYPTION_AES192 : VSCP_ENCRYPTION_AES128;
  invalid.emplace_back();
  ASSERT_TRUE(vscp_binary::encode(plain, otherEncryption, aes.data(), invalid.back(),
                                  vscp_binary::authenticatedVersion, hmac.data()));
  invalid.emplace_back(plain.size() + 32);
  const size_t legacyLength = vscp_encryptFrame(invalid.back().data(), plain.data(), plain.size(),
                                                 aes.data(), nullptr, encryption);
  ASSERT_NE(0u, legacyLength);
  invalid.back().resize(legacyLength);

  for (const auto &packet : invalid) {
    ASSERT_TRUE(sender.send(packet));
  }
  ASSERT_TRUE(sender.send(wire));
  vscpEventEx received{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(received, 3000));
  EXPECT_EQ(event.vscp_class, received.vscp_class);
  EXPECT_EQ(event.vscp_type, received.vscp_type);
  ASSERT_EQ(sizeof(data), received.sizeData);
  EXPECT_EQ(0, std::memcmp(data, received.data, sizeof(data)));
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBlocking(received, 150));
  EXPECT_EQ(1, callbacks->load());

  vscpEventFilter filter{};
  filter.filter_class = event.vscp_class + 1;
  filter.mask_class = 0xffff;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setfilter(filter));
  ASSERT_TRUE(sender.send(wire));
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBlocking(received, 150));
  EXPECT_EQ(1, callbacks->load());
  EXPECT_TRUE(client.isConnected());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());

  vscp_clearVSCPFilter(&filter);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setfilter(filter));
  ASSERT_TRUE(client.initFromJson(R"({"hmac-key":""})"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  ASSERT_TRUE(sender.send(wire));
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBlocking(received, 150));
  ASSERT_TRUE(sender.send(plain));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(received, 2000));
  EXPECT_EQ(event.vscp_class, received.vscp_class);
  ASSERT_TRUE(sender.send(invalid.back()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(received, 2000));
  EXPECT_EQ(event.vscp_class, received.vscp_class);
  ASSERT_TRUE(sender.send(plain));
  ASSERT_TRUE(waitForDatagramCount(client, 1));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());

  const nlohmann::json legacyConfig = nlohmann::json::parse(client.getConfigAsJson());
  EXPECT_FALSE(client.initFromJson(nlohmann::json{{"hmac-key", testDatagramHmacKey()}}.dump()));
  EXPECT_EQ(legacyConfig, nlohmann::json::parse(client.getConfigAsJson()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.clear());
  ASSERT_TRUE(client.initFromJson(nlohmann::json{{"hmac-key", testDatagramHmacKey()}}.dump()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  ASSERT_TRUE(sender.send(wire));
  vscpEvent receivedEvent{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(receivedEvent, 2000));
  EXPECT_EQ(event.vscp_class, receivedEvent.vscp_class);
  EXPECT_EQ(event.sizeData, receivedEvent.sizeData);
  if (receivedEvent.sizeData == sizeof(data) && receivedEvent.pdata) {
    EXPECT_EQ(0, std::memcmp(data, receivedEvent.pdata, sizeof(data)));
  }
  vscp_deleteEvent(&receivedEvent);

  ASSERT_TRUE(sender.send(wire));
  ASSERT_TRUE(waitForDatagramCount(client, 1));
  canalMsg receivedCanal{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receive(receivedCanal));
  EXPECT_EQ((event.vscp_class << 16) | (event.vscp_type << 8), receivedCanal.id);
  ASSERT_EQ(sizeof(data), receivedCanal.sizeData);
  EXPECT_EQ(0, std::memcmp(data, receivedCanal.data, sizeof(data)));
  ASSERT_TRUE(sender.send(wire));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(receivedCanal, 2000));
  EXPECT_EQ(7, callbacks->load());

  ASSERT_TRUE(sender.send(wire));
  ASSERT_TRUE(waitForDatagramCount(client, 1));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
  const nlohmann::json authenticatedConfig = nlohmann::json::parse(client.getConfigAsJson());
  client.m_key[0] ^= 0x40;
  EXPECT_EQ(VSCP_ERROR_PARAMETER, client.connect());
  EXPECT_FALSE(client.initFromJson("{}"));
  client.m_key[0] ^= 0x40;
  EXPECT_FALSE(client.initFromJson(R"({"hmac-key":""})"));
  EXPECT_FALSE(client.initFromJson(nlohmann::json{{"hmac-key", std::string(64, 'a')}}.dump()));
  const size_t aesHexLength = 2 * (16 + 8 * (encryption - VSCP_ENCRYPTION_AES128));
  EXPECT_FALSE(client.initFromJson(nlohmann::json{{"key", std::string(aesHexLength, 'b')}}.dump()));
  EXPECT_EQ(authenticatedConfig, nlohmann::json::parse(client.getConfigAsJson()));
  uint16_t count = 0;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.getcount(&count));
  EXPECT_EQ(1, count);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.clear());
  EXPECT_TRUE(client.initFromJson(R"({"hmac-key":""})"));
}

#ifdef __linux__
class AuthenticatedDatagramWireReceiver {
public:
  AuthenticatedDatagramWireReceiver(uint16_t port, bool multicast)
    : m_socket(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
  {
    if (m_socket == invalidTestSocket) {
      return;
    }
    const int reuse = 1;
    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = htonl(multicast ? INADDR_ANY : INADDR_LOOPBACK);
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0 ||
        bind(m_socket, reinterpret_cast<const sockaddr *>(&local), sizeof(local)) != 0) {
      closeTestSocket(m_socket);
      m_socket = invalidTestSocket;
      return;
    }
    if (multicast) {
      ip_mreq membership{};
      inet_pton(AF_INET, "239.255.23.158", &membership.imr_multiaddr);
      membership.imr_interface.s_addr = htonl(INADDR_LOOPBACK);
      if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &membership, sizeof(membership)) != 0) {
        closeTestSocket(m_socket);
        m_socket = invalidTestSocket;
      }
    }
  }

  ~AuthenticatedDatagramWireReceiver()
  {
    if (m_socket != invalidTestSocket) {
      closeTestSocket(m_socket);
    }
  }

  bool valid() const { return m_socket != invalidTestSocket; }

  std::vector<uint8_t> receive() const
  {
    fd_set readers;
    FD_ZERO(&readers);
    FD_SET(m_socket, &readers);
    timeval timeout{2, 0};
    if (select(m_socket + 1, &readers, nullptr, nullptr, &timeout) != 1) {
      return {};
    }
    std::vector<uint8_t> wire(1024);
    const int received = static_cast<int>(recv(m_socket, wire.data(), wire.size(), 0));
    if (received <= 0) {
      return {};
    }
    wire.resize(static_cast<size_t>(received));
    return wire;
  }

private:
  TestSocket m_socket;
};

inline void
verifyAuthenticatedDatagramWire(const std::vector<uint8_t> &wire, uint8_t encryption, const vscpEvent &event)
{
  const auto aes = testDatagramKey(testDatagramAesKey());
  const auto hmac = testDatagramKey(testDatagramHmacKey());
  const size_t ciphertextLength = ((37 + event.sizeData) / 16 + 1) * 16;
  ASSERT_EQ(ciphertextLength + 36, wire.size());
  EXPECT_EQ(0x55, wire[0]);
  const size_t innerLength = ciphertextLength + 33;
  EXPECT_EQ(0x20 | (innerLength >> 8), wire[1]);
  EXPECT_EQ(innerLength & 0xff, wire[2]);
  EXPECT_EQ((VSCP_MULTICAST_TYPE_EVENT1 << 4) | encryption, wire[3]);

  uint8_t digest[EVP_MAX_MD_SIZE];
  unsigned int digestLength = 0;
  ASSERT_NE(nullptr, HMAC(EVP_sha256(), hmac.data(), static_cast<int>(hmac.size()), wire.data(),
                           wire.size() - 16, digest, &digestLength));
  ASSERT_EQ(32u, digestLength);
  EXPECT_EQ(0, std::memcmp(digest, wire.data() + wire.size() - 16, 16));

  std::vector<uint8_t> plain(innerLength);
  const size_t length = vscp_decryptFrameV1(plain.data(), plain.size(), wire.data() + 3, innerLength,
                                             aes.data(), hmac.data());
  ASSERT_EQ(38 + event.sizeData, length);
  vscpEventEx decoded{};
  ASSERT_TRUE(vscp_getEventExFromFrame(&decoded, plain.data(), length));
  EXPECT_EQ(event.vscp_class, decoded.vscp_class);
  EXPECT_EQ(event.vscp_type, decoded.vscp_type);
  ASSERT_EQ(event.sizeData, decoded.sizeData);
  if (event.sizeData) {
    EXPECT_EQ(0, std::memcmp(event.pdata, decoded.data, event.sizeData));
  }
}

template<typename Client>
void
verifyAuthenticatedDatagramSend(Client &client, AuthenticatedDatagramWireReceiver &receiver, uint8_t encryption)
{
  std::vector<uint8_t> data(VSCP_LEVEL2_MAXDATA);
  for (size_t i = 0; i < data.size(); ++i) {
    data[i] = static_cast<uint8_t>(i);
  }
  vscpEvent event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  event.timestamp_ns = 123456789;
  event.vscp_class = 321;
  event.vscp_type = 43;
  event.pdata = data.data();
  for (uint16_t size : {0, 10, 11, 12, VSCP_LEVEL2_MAXDATA}) {
    event.sizeData = size;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
    const auto wire = receiver.receive();
    verifyAuthenticatedDatagramWire(wire, encryption, event);
  }
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  const auto first = receiver.receive();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  const auto second = receiver.receive();
  verifyAuthenticatedDatagramWire(first, encryption, event);
  verifyAuthenticatedDatagramWire(second, encryption, event);
  ASSERT_EQ(first.size(), second.size());
  ASSERT_GE(first.size(), 32u);
  EXPECT_NE(0, std::memcmp(first.data() + first.size() - 32, second.data() + second.size() - 32, 16));

  vscpEventEx extended{};
  extended.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  extended.vscp_class = 55;
  extended.vscp_type = 4;
  extended.sizeData = 1;
  extended.data[0] = 0x99;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(extended));
  vscpEvent expected{};
  expected.vscp_class = extended.vscp_class;
  expected.vscp_type = extended.vscp_type;
  expected.sizeData = extended.sizeData;
  expected.pdata = extended.data;
  verifyAuthenticatedDatagramWire(receiver.receive(), encryption, expected);

  canalMsg message{};
  message.id = (10U << 16) | (6U << 8) | 42U;
  message.sizeData = 1;
  message.data[0] = 0x71;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(message));
  uint8_t guid[16]{};
  vscpEvent converted{};
  ASSERT_TRUE(vscp_convertCanalToEvent(&converted, &message, guid));
  verifyAuthenticatedDatagramWire(receiver.receive(), encryption, converted);
  vscp_deleteEvent(&converted);
}
#endif

#endif
