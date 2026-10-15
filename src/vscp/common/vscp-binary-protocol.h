// vscp-binary-protocol.h: interface for the vscp binary protocol.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright (C) 2000-2026 Ake Hedman,
// Ake Hedman, the VSCP project, <info@vscp.org>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#ifndef VSCP_TCPIP_BINARYPROTOCOL_H
#define VSCP_TCPIP_BINARYPROTOCOL_H

#include <crc.h>
#include <vscphelper.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

namespace vscp_binary {

constexpr size_t maxFrameSize = 4096;

// TCP framing header: [syncByte][version:3 | length 12..8][length 7..0].
constexpr uint8_t syncByte = 0x55;
constexpr uint8_t framingVersion = 1;
constexpr size_t headerSize = 3;

// An incomplete frame older than this is treated as a false frame start.
constexpr uint64_t partialFrameTimeoutMs = 2000;

/*!
    @brief Read a 16-bit big-endian value from a byte array.

    @param data Pointer to the byte array.
    @return The 16-bit value.
*/
inline uint16_t read16(const uint8_t* data)
{
    return (static_cast<uint16_t>(data[0]) << 8) | data[1];
}

/*!
    @brief Append a multi-byte value to a byte array in big-endian order.

    @param data The byte array to append to.
    @param value The value to append.
    @param bytes The number of bytes to append.
*/
inline void append(std::vector<uint8_t>& data, uint64_t value, size_t bytes)
{
    for (size_t i = bytes; i > 0; --i) {
        data.push_back(static_cast<uint8_t>(value >> ((i - 1) * 8)));
    }
}

/*!
    @brief Set the CRC for a frame.

    @param frame The frame to set the CRC for.
*/
inline void setCrc(std::vector<uint8_t>& frame)
{
    // The installed command/reply helpers include the unwritten CRC bytes.
    // Calculate over the content only, as the event helpers do.
    crc value = crcFast(frame.data() + 1, static_cast<int>(frame.size() - 3));
    frame[frame.size() - 2] = static_cast<uint8_t>(value >> 8);
    frame.back() = static_cast<uint8_t>(value);
}

/*!
    @brief Validate the CRC of a frame.

    @param data Pointer to the frame data.
    @param size Size of the frame data.
    @return True if the CRC is valid, false otherwise.
*/
inline bool validCrc(const uint8_t* data, size_t size)
{
    return size >= 3 &&
           crcFast(data + 1, static_cast<int>(size - 3)) ==
             read16(data + size - 2);
}

/*!
    @brief Check whether a frame length suits the encryption in its lead byte.

    @param lead First byte of the VSCP-GBP frame.
    @param length Length of the VSCP-GBP frame.
    @return True if the lead byte and length can start a valid frame.
*/
inline bool plausibleFrame(uint8_t lead, size_t length)
{
    uint8_t type = lead >> 4;
    uint8_t encryption = lead & 0x0f;
    if ((type > 1 && type < 14) || encryption > VSCP_ENCRYPTION_AES256 ||
        length < 5 || length > maxFrameSize) {
        return false;
    }
    return !encryption || (length >= 33 && (length - 17) % 16 == 0);
}

/*!
    @brief Parse a TCP framing header.

    @param data Pointer to at least headerSize + 1 bytes.
    @param length Receives the VSCP-GBP frame length.
    @return True if the header and frame lead byte look valid.
*/
inline bool parseHeader(const uint8_t* data, size_t& length)
{
    uint16_t word = read16(data + 1);
    length = word & 0x1fff;
    return data[0] == syncByte && (word >> 13) == framingVersion &&
           plausibleFrame(data[headerSize], length);
}

/*!
    @brief Get the cipher for a given encryption type.

    @param encryption The encryption type.
    @return The corresponding EVP_CIPHER, or nullptr if unsupported.
*/
inline const EVP_CIPHER* cipher(uint8_t encryption)
{
    switch (encryption) {
        case 1: return EVP_aes_128_cbc();
        case 2: return EVP_aes_192_cbc();
        case 3: return EVP_aes_256_cbc();
        default: return nullptr;
    }
}

/*!
    @brief Encode a frame with optional encryption.

    @param frame The frame to encode.
    @param encryption The encryption type.
    @param key The encryption key.
    @param wire The output byte array containing the encoded frame.
    @return True if encoding was successful, false otherwise.
*/
inline bool encode(std::vector<uint8_t> frame,
                   uint8_t encryption,
                   const uint8_t* key,
                   std::vector<uint8_t>& wire)
{
    if (frame.size() < 5 || frame.size() > maxFrameSize - 34 ||
        encryption > VSCP_ENCRYPTION_AES256) {
        return false;
    }
    frame[0] = (frame[0] & 0xf0) | encryption;
    if (encryption) {
        uint8_t iv[16];
        if (!key || RAND_bytes(iv, sizeof(iv)) != 1) return false;
        std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>
          context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
        std::vector<uint8_t> encrypted(frame.size() + 32, 0);
        encrypted[0] = frame[0];
        int written = 0, final = 0;
        // PKCS#7 is deliberate: zero padding cannot delimit variable commands.
        if (!context ||
            EVP_EncryptInit_ex(context.get(), cipher(encryption), nullptr, key, iv) != 1 ||
            EVP_EncryptUpdate(context.get(), encrypted.data() + 1, &written,
                               frame.data() + 1,
                               static_cast<int>(frame.size() - 1)) != 1 ||
            EVP_EncryptFinal_ex(context.get(), encrypted.data() + 1 + written,
                                &final) != 1) {
            return false;
        }
        encrypted.resize(1 + written + final);
        encrypted.insert(encrypted.end(), iv, iv + sizeof(iv));
        frame = std::move(encrypted);
    }
    wire.clear();
    wire.push_back(syncByte);
    append(wire, (static_cast<uint16_t>(framingVersion) << 13) | frame.size(), 2);
    wire.insert(wire.end(), frame.begin(), frame.end());
    return true;
}

/*!
    @brief Decode a frame with optional decryption.

    @param wire The encoded frame data.
    @param key The decryption key.
    @param frame The output byte array containing the decoded frame.
    @return True if decoding was successful, false otherwise.
*/
inline bool decode(std::vector<uint8_t> wire,
                   const uint8_t* key,
                   std::vector<uint8_t>& frame)
{
    if (wire.empty() || !plausibleFrame(wire[0], wire.size())) {
        return false;
    }
    uint8_t encryption = wire[0] & 0x0f;
    if (!encryption) {
        frame = std::move(wire);
        return validCrc(frame.data(), frame.size());
    }
    if (!key) return false;
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>
      context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    frame.assign(wire.size(), 0);
    frame[0] = wire[0];
    int written = 0, final = 0;
    if (!context ||
        EVP_DecryptInit_ex(context.get(), cipher(encryption), nullptr,
                            key, wire.data() + wire.size() - 16) != 1 ||
        EVP_DecryptUpdate(context.get(), frame.data() + 1, &written,
                           wire.data() + 1,
                           static_cast<int>(wire.size() - 17)) != 1 ||
        EVP_DecryptFinal_ex(context.get(), frame.data() + 1 + written,
                            &final) != 1) {
        return false;
    }
    frame.resize(1 + written + final);
    return frame.size() >= 5 && validCrc(frame.data(), frame.size());
}

} // namespace vscp_binary

#endif
