#ifndef VSCP_DATAGRAM_AUTHENTICATED_TEST_HELPERS_H
#define VSCP_DATAGRAM_AUTHENTICATED_TEST_HELPERS_H

#include "datagram_test_helpers.h"

#include <vscp-binary-protocol.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

inline nlohmann::json
authenticatedDatagramConfig(uint8_t encryption)
{
  static const std::string aesKey =
    "0102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F20";
  return nlohmann::json{{"encryption", encryption},
                        {"key", aesKey.substr(0, (encryption + 1) * 16)},
                        {"hmac-key", "F0E0D0C0B0A090807060504030201000FFEEDDCCBBAA99887766554433221100"}};
}

inline bool
authenticatedDatagramKeys(const nlohmann::json &config, uint8_t encryption, uint8_t *aesKey, uint8_t *hmacKey)
{
  const uint8_t keyLength = encryption == VSCP_ENCRYPTION_AES128 ? 16 :
                            encryption == VSCP_ENCRYPTION_AES192 ? 24 :
                            encryption == VSCP_ENCRYPTION_AES256 ? 32 : 0;
  if (!keyLength || !config.contains("key") || !config["key"].is_string() ||
      !config.contains("hmac-key") || !config["hmac-key"].is_string()) {
    return false;
  }
  const std::string aesKeyHex = config["key"].get<std::string>();
  const std::string hmacKeyHex = config["hmac-key"].get<std::string>();
  return aesKeyHex.size() == keyLength * 2 && hmacKeyHex.size() == 64 &&
         vscp_hexStr2ByteArray(aesKey, keyLength, aesKeyHex.c_str()) == keyLength &&
         vscp_hexStr2ByteArray(hmacKey, 32, hmacKeyHex.c_str()) == 32;
}

template<typename Client>
inline void
verifyAuthenticatedDatagramOptions(const std::string &address)
{
  Client client;
  auto config = authenticatedDatagramConfig(VSCP_ENCRYPTION_AES256);
  config["ip"] = address;
  ASSERT_TRUE(client.initFromJson(config.dump()));
  const auto accepted = nlohmann::json::parse(client.getConfigAsJson());
  std::string normalizedHmacKey = config["hmac-key"].get<std::string>();
  std::transform(normalizedHmacKey.begin(), normalizedHmacKey.end(), normalizedHmacKey.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  EXPECT_EQ(normalizedHmacKey, accepted["hmac-key"]);
  EXPECT_EQ(config["encryption"], accepted["encryption"]);

  auto invalidHmacLength = config;
  invalidHmacLength["hmac-key"] = "1234";
  EXPECT_FALSE(client.initFromJson(invalidHmacLength.dump()));

  auto invalidHmacHex = config;
  invalidHmacHex["hmac-key"] = std::string(64, 'z');
  EXPECT_FALSE(client.initFromJson(invalidHmacHex.dump()));

  auto reusedKey = config;
  reusedKey["hmac-key"] = config["key"];
  EXPECT_FALSE(client.initFromJson(reusedKey.dump()));
  EXPECT_EQ(accepted, nlohmann::json::parse(client.getConfigAsJson()));

  Client missingEncryption;
  EXPECT_FALSE(missingEncryption.initFromJson(
    nlohmann::json{{"ip", address}, {"hmac-key", config["hmac-key"]}}.dump()));
}

inline bool
sendAuthenticatedDatagram(uint16_t port, const std::vector<uint8_t> &wire, bool multicast)
{
  TestSocket socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socket == invalidTestSocket) {
    return false;
  }
  sockaddr_in target{};
  target.sin_family = AF_INET;
  target.sin_port = htons(port);
  const char *address = multicast ? "239.255.23.158" : "127.0.0.1";
  bool ready = inet_pton(AF_INET, address, &target.sin_addr) == 1;
  if (multicast) {
    in_addr interfaceAddress{};
    interfaceAddress.s_addr = htonl(INADDR_LOOPBACK);
    const unsigned char loop = 1;
    ready = ready &&
            setsockopt(socket, IPPROTO_IP, IP_MULTICAST_IF,
                       reinterpret_cast<const char *>(&interfaceAddress), sizeof(interfaceAddress)) == 0 &&
            setsockopt(socket, IPPROTO_IP, IP_MULTICAST_LOOP,
                       reinterpret_cast<const char *>(&loop), sizeof(loop)) == 0;
  }
  const int sent = ready ?
    static_cast<int>(sendto(socket, reinterpret_cast<const char *>(wire.data()),
                            static_cast<int>(wire.size()), 0,
                            reinterpret_cast<const sockaddr *>(&target), sizeof(target))) :
    -1;
  closeTestSocket(socket);
  return sent == static_cast<int>(wire.size());
}

template<typename Client>
inline void
verifyAuthenticatedDatagramReceive(Client &client, uint16_t port, uint8_t encryption, bool multicast)
{
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());

  vscpEvent event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  event.vscp_class = 42;
  event.vscp_type = 8;
  uint8_t payload[] = {0xa5, 0x5a};
  event.sizeData = sizeof(payload);
  event.pdata = payload;
  uint8_t frame[1024]{};
  ASSERT_TRUE(vscp_writeEventToFrame(frame, sizeof(frame), 0, &event));

  uint8_t aesKey[32]{};
  uint8_t hmacKey[32]{};
  ASSERT_TRUE(authenticatedDatagramKeys(nlohmann::json::parse(client.getConfigAsJson()),
                                        encryption, aesKey, hmacKey));
  std::vector<uint8_t> wire;
  ASSERT_TRUE(vscp_binary::encode(std::vector<uint8_t>(frame, frame + vscp_getFrameSizeFromEvent(&event)),
                                  encryption, aesKey, wire,
                                  vscp_binary::authenticatedVersion, hmacKey));
  std::vector<uint8_t> tampered = wire;
  tampered.back() ^= 1;
  ASSERT_TRUE(sendAuthenticatedDatagram(port, tampered, multicast));
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  uint16_t count = 0;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.getcount(&count));
  EXPECT_EQ(0, count);

  ASSERT_TRUE(sendAuthenticatedDatagram(port, wire, multicast));
  vscpEventEx received{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(received, 2000));
  EXPECT_EQ(event.vscp_class, received.vscp_class);
  EXPECT_EQ(event.vscp_type, received.vscp_type);
  ASSERT_EQ(event.sizeData, received.sizeData);
  EXPECT_EQ(0, std::memcmp(payload, received.data, sizeof(payload)));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
}

class AuthenticatedDatagramWireReceiver
{
public:
  AuthenticatedDatagramWireReceiver(uint16_t port, bool multicast)
  {
    m_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket == invalidTestSocket) {
      return;
    }

    int reuse = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char *>(&reuse), sizeof(reuse)) != 0) {
      closeTestSocket(m_socket);
      m_socket = invalidTestSocket;
      return;
    }
    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(multicast ? INADDR_ANY : INADDR_LOOPBACK);
    local.sin_port = htons(port);
    if (bind(m_socket, reinterpret_cast<const sockaddr *>(&local), sizeof(local)) != 0) {
      closeTestSocket(m_socket);
      m_socket = invalidTestSocket;
      return;
    }

    if (multicast) {
      ip_mreq membership{};
      inet_pton(AF_INET, "239.255.23.158", &membership.imr_multiaddr);
      membership.imr_interface.s_addr = htonl(INADDR_LOOPBACK);
      if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     reinterpret_cast<const char *>(&membership), sizeof(membership)) != 0) {
        closeTestSocket(m_socket);
        m_socket = invalidTestSocket;
        return;
      }
    }

#ifdef WIN32
    const DWORD timeout = 2000;
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO,
               reinterpret_cast<const char *>(&timeout), sizeof(timeout));
#else
    timeval timeout{2, 0};
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif
  }

  ~AuthenticatedDatagramWireReceiver()
  {
    if (m_socket != invalidTestSocket) {
      closeTestSocket(m_socket);
    }
  }

  bool valid() const
  {
    return m_socket != invalidTestSocket;
  }

  bool receive(std::vector<uint8_t> &wire)
  {
    uint8_t buffer[1024]{};
    const int received = static_cast<int>(recv(m_socket, reinterpret_cast<char *>(buffer),
                                               sizeof(buffer), 0));
    if (received <= 0) {
      return false;
    }
    wire.assign(buffer, buffer + received);
    return true;
  }

private:
  TestSocket m_socket = invalidTestSocket;
};

template<typename Client>
inline void
verifyAuthenticatedDatagramSend(Client &client, AuthenticatedDatagramWireReceiver &receiver,
                                uint8_t encryption)
{
  vscpEventEx event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  event.vscp_class = 42;
  event.vscp_type = 8;
  uint8_t payload[] = {0xa5, 0x5a};
  event.sizeData = sizeof(payload);
  std::memcpy(event.data, payload, sizeof(payload));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));

  std::vector<uint8_t> wire;
  ASSERT_TRUE(receiver.receive(wire));
  ASSERT_GE(wire.size(), vscp_binary::headerSize + 1);
  size_t frameLength = 0;
  uint8_t version = 0;
  ASSERT_TRUE(vscp_binary::parseHeader(wire.data(), frameLength, version));
  EXPECT_EQ(vscp_binary::authenticatedVersion, version);
  ASSERT_EQ(wire.size() - vscp_binary::headerSize, frameLength);

  uint8_t aesKey[32]{};
  uint8_t hmacKey[32]{};
  ASSERT_TRUE(authenticatedDatagramKeys(nlohmann::json::parse(client.getConfigAsJson()),
                                        encryption, aesKey, hmacKey));
  std::vector<uint8_t> frame;
  ASSERT_TRUE(vscp_binary::decode(
    std::vector<uint8_t>(wire.begin() + vscp_binary::headerSize, wire.end()),
    aesKey, frame, version, hmacKey));
  ASSERT_EQ(encryption, frame[0] & 0x0f);

  vscpEvent decoded{};
  ASSERT_TRUE(vscp_getEventFromFrame(&decoded, frame.data(), frame.size()));
  EXPECT_EQ(event.vscp_class, decoded.vscp_class);
  EXPECT_EQ(event.vscp_type, decoded.vscp_type);
  ASSERT_EQ(event.sizeData, decoded.sizeData);
  EXPECT_EQ(0, std::memcmp(payload, decoded.pdata, sizeof(payload)));
  vscp_deleteEvent(&decoded);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
}

#endif
