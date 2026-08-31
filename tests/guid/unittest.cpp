///////////////////////////////////////////////////////////////////////////////
// unittest.cpp
//
// Unit tests for cguid class
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, the VSCP project
// <info@vscp.org>
//

#include <gtest/gtest.h>
#include <string>
#include <cstring>

#include "guid.h"

//-----------------------------------------------------------------------------
// Constructor tests
//-----------------------------------------------------------------------------

TEST(Cguid, DefaultConstructor) 
{
    cguid guid;
    
    // Default constructor should create a NULL GUID
    EXPECT_TRUE(guid.isNULL());
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0, guid.getAt(i));
    }
}

TEST(Cguid, ArrayConstructor) 
{
    uint8_t data[16] = {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
                        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
    
    cguid guid(data);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(data[i], guid.getAt(i));
    }
}

TEST(Cguid, CopyConstructor) 
{
    uint8_t data[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    
    cguid guid1(data);
    cguid guid2(guid1);
    
    EXPECT_TRUE(guid1 == guid2);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(guid1.getAt(i), guid2.getAt(i));
    }
}

TEST(Cguid, StringConstructor) 
{
    std::string strGUID = "FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00";
    cguid guid(strGUID);
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xEE, guid.getAt(1));
    EXPECT_EQ(0xDD, guid.getAt(2));
    EXPECT_EQ(0xCC, guid.getAt(3));
    EXPECT_EQ(0xBB, guid.getAt(4));
    EXPECT_EQ(0xAA, guid.getAt(5));
    EXPECT_EQ(0x99, guid.getAt(6));
    EXPECT_EQ(0x88, guid.getAt(7));
    EXPECT_EQ(0x77, guid.getAt(8));
    EXPECT_EQ(0x66, guid.getAt(9));
    EXPECT_EQ(0x55, guid.getAt(10));
    EXPECT_EQ(0x44, guid.getAt(11));
    EXPECT_EQ(0x33, guid.getAt(12));
    EXPECT_EQ(0x22, guid.getAt(13));
    EXPECT_EQ(0x11, guid.getAt(14));
    EXPECT_EQ(0x00, guid.getAt(15));
}

//-----------------------------------------------------------------------------
// Operator tests
//-----------------------------------------------------------------------------

TEST(Cguid, AssignmentOperator) 
{
    uint8_t data[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    
    cguid guid1(data);
    cguid guid2;
    
    EXPECT_TRUE(guid2.isNULL());
    
    guid2 = guid1;
    
    EXPECT_TRUE(guid1 == guid2);
}

TEST(Cguid, SelfAssignment) 
{
    uint8_t data[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    
    cguid guid(data);
    cguid &ref = guid;
    
    guid = ref;  // Self-assignment
    
    // Should still be equal to original data
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(data[i], guid.getAt(i));
    }
}

TEST(Cguid, EqualityOperator) 
{
    cguid guid1("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    cguid guid2("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    cguid guid3("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
    
    EXPECT_TRUE(guid1 == guid2);
    EXPECT_FALSE(guid1 == guid3);
}

TEST(Cguid, InequalityOperator) 
{
    cguid guid1("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    cguid guid2("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    cguid guid3("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01");
    
    EXPECT_FALSE(guid1 != guid2);
    EXPECT_TRUE(guid1 != guid3);
}

TEST(Cguid, IndexOperatorRead) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    EXPECT_EQ(0xFF, guid[0]);
    EXPECT_EQ(0xEE, guid[1]);
    EXPECT_EQ(0x00, guid[15]);
}

TEST(Cguid, IndexOperatorWrite) 
{
    cguid guid;
    
    guid[0] = 0xAB;
    guid[15] = 0xCD;
    
    EXPECT_EQ(0xAB, guid[0]);
    EXPECT_EQ(0xCD, guid[15]);
}

TEST(Cguid, IndexOperatorMasked) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    // Index is masked with 0x0F, so index 16 should wrap to 0
    EXPECT_EQ(guid[0], guid[16]);
    EXPECT_EQ(guid[1], guid[17]);
}

//-----------------------------------------------------------------------------
// Clear and isNULL tests
//-----------------------------------------------------------------------------

TEST(Cguid, Clear) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    EXPECT_FALSE(guid.isNULL());
    
    guid.clear();
    
    EXPECT_TRUE(guid.isNULL());
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0, guid.getAt(i));
    }
}

TEST(Cguid, IsNULL) 
{
    cguid guid1;
    cguid guid2("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
    cguid guid3("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01");
    
    EXPECT_TRUE(guid1.isNULL());
    EXPECT_TRUE(guid2.isNULL());
    EXPECT_FALSE(guid3.isNULL());
}

//-----------------------------------------------------------------------------
// String conversion tests
//-----------------------------------------------------------------------------

TEST(Cguid, GetFromString) 
{
    cguid guid;
    
    guid.getFromString("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0x00, guid.getAt(15));
}

TEST(Cguid, GetFromStringCharPtr) 
{
    cguid guid;
    
    guid.getFromString("AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99");
    
    EXPECT_EQ(0xAA, guid.getAt(0));
    EXPECT_EQ(0x99, guid.getAt(15));
}

TEST(Cguid, GetFromStringEmpty) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    // Empty string should clear the GUID
    guid.getFromString("");
    
    EXPECT_TRUE(guid.isNULL());
}

TEST(Cguid, GetFromStringDash) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    // Dash string should clear the GUID
    guid.getFromString("-");
    
    EXPECT_TRUE(guid.isNULL());
}

TEST(Cguid, ToStringRef) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    std::string str;
    
    guid.toString(str);
    
    EXPECT_EQ("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00", str);
}

TEST(Cguid, ToStringReturn) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    std::string str = guid.toString();
    
    EXPECT_EQ("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00", str);
}

TEST(Cguid, GetAsString) 
{
    cguid guid("01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F:10");
    
    std::string str = guid.getAsString();
    
    EXPECT_EQ("01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F:10", str);
}

TEST(Cguid, ToStringCompactRef) 
{
    cguid guid("FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:11:22:33:44:55:66");
    std::string str;
    
    guid.toStringCompact(str);
    
    // Leading FF bytes become "::"
    EXPECT_EQ("::11:22:33:44:55:66", str);
}

TEST(Cguid, ToStringCompactReturn) 
{
    cguid guid("FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:11:22:33:44:55:66");
    
    std::string str = guid.toStringCompact();
    
    EXPECT_EQ("::11:22:33:44:55:66", str);
}

TEST(Cguid, ToStringCompactAllFF) 
{
    cguid guid("::");  // All 0xFF
    
    std::string str = guid.toStringCompact();
    
    EXPECT_EQ("::", str);
}

TEST(Cguid, ToStringCompactNoFF) 
{
    cguid guid("01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F:10");
    
    std::string str = guid.toStringCompact();
    
    // No leading FF, should be standard format
    EXPECT_EQ("01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F:10", str);
}

TEST(Cguid, ToStringUUIDRef) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    std::string str;
    
    guid.toStringUUID(str);
    
    // UUID format: 8-4-4-4-12
    EXPECT_EQ("FFEEDDCC-BBAA-9988-7766-554433221100", str);
}

TEST(Cguid, ToStringUUIDReturn) 
{
    cguid guid("00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF");
    
    std::string str = guid.toStringUUID();
    
    EXPECT_EQ("00112233-4455-6677-8899-AABBCCDDEEFF", str);
}

//-----------------------------------------------------------------------------
// Array operations tests
//-----------------------------------------------------------------------------

TEST(Cguid, GetFromArray) 
{
    uint8_t data[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
                        0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    
    cguid guid;
    guid.getFromArray(data);
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(data[i], guid.getAt(i));
    }
}

TEST(Cguid, Reverse)
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    uint8_t output[16] = {0};
    
    // Reverse local array
    guid.reverse();
    
    // Reverse: index 0 gets guid[15], index 15 gets guid[0]
    EXPECT_EQ(0x00, guid.getAt(0));
    EXPECT_EQ(0x11, guid.getAt(1));
    EXPECT_EQ(0xFF, guid.getAt(15));
}

TEST(Cguid, WriteGUID) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    uint8_t output[16] = {0};
    
    guid.writeGUID(output);
    
    EXPECT_EQ(0xFF, output[0]);
    EXPECT_EQ(0xEE, output[1]);
    EXPECT_EQ(0x00, output[15]);
}



TEST(Cguid, WriteGUIDReverse) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    uint8_t output[16] = {0};
    
    // Reverse output array
    guid.writeGUID_reverse(output);
    
    // Reverse: index 0 gets guid[15], index 15 gets guid[0]
    EXPECT_EQ(0x00, output[0]);
    EXPECT_EQ(0x11, output[1]);
    EXPECT_EQ(0xFF, output[15]);
}

TEST(Cguid, GetGUIDPointer) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    const uint8_t *ptr = guid.getGUID();
    
    EXPECT_EQ(0xFF, ptr[0]);
    EXPECT_EQ(0xEE, ptr[1]);
    EXPECT_EQ(0x00, ptr[15]);
}

TEST(Cguid, GetGUIDByPosition) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    EXPECT_EQ(0xFF, guid.getGUID(0));
    EXPECT_EQ(0xEE, guid.getGUID(1));
    EXPECT_EQ(0x00, guid.getGUID(15));
}

//-----------------------------------------------------------------------------
// Byte access tests
//-----------------------------------------------------------------------------

TEST(Cguid, GetAt) 
{
    cguid guid("01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F:10");
    
    EXPECT_EQ(0x01, guid.getAt(0));
    EXPECT_EQ(0x10, guid.getAt(15));
}

TEST(Cguid, SetAt) 
{
    cguid guid;
    
    guid.setAt(0, 0xAA);
    guid.setAt(15, 0xBB);
    
    EXPECT_EQ(0xAA, guid.getAt(0));
    EXPECT_EQ(0xBB, guid.getAt(15));
}

TEST(Cguid, SetAtMasked) 
{
    cguid guid;
    
    // Position is masked with 0x0F
    guid.setAt(16, 0xCC);  // Should set position 0
    
    EXPECT_EQ(0xCC, guid.getAt(0));
}

TEST(Cguid, GetSetLSB) 
{
    cguid guid;
    
    guid.setLSB(0xAB);
    
    EXPECT_EQ(0xAB, guid.getLSB());
    EXPECT_EQ(0xAB, guid.getAt(15));
}

TEST(Cguid, GetSetMSB) 
{
    cguid guid;
    
    guid.setMSB(0xCD);
    
    EXPECT_EQ(0xCD, guid.getMSB());
    EXPECT_EQ(0xCD, guid.getAt(0));
}

//-----------------------------------------------------------------------------
// Client ID tests
//-----------------------------------------------------------------------------

TEST(Cguid, SetClientID) 
{
    cguid guid;
    
    guid.setClientID(0x1234);
    
    EXPECT_EQ(0x12, guid.getAt(12));
    EXPECT_EQ(0x34, guid.getAt(13));
}

TEST(Cguid, GetClientID) 
{
    cguid guid;
    
    guid.setAt(12, 0xAB);
    guid.setAt(13, 0xCD);
    
    EXPECT_EQ(0xABCD, guid.getClientID());
}

TEST(Cguid, ClientIDRoundTrip) 
{
    cguid guid;
    
    guid.setClientID(0xFFEE);
    
    EXPECT_EQ(0xFFEE, guid.getClientID());
}

//-----------------------------------------------------------------------------
// Nickname ID tests
//-----------------------------------------------------------------------------

TEST(Cguid, SetNicknameID) 
{
    cguid guid;
    
    guid.setNicknameID(0x5678);
    
    EXPECT_EQ(0x56, guid.getAt(14));
    EXPECT_EQ(0x78, guid.getAt(15));
}

TEST(Cguid, GetNicknameID) 
{
    cguid guid;
    
    guid.setAt(14, 0xDE);
    guid.setAt(15, 0xAD);
    
    EXPECT_EQ(0xDEAD, guid.getNicknameID());
}

TEST(Cguid, GetNickname) 
{
    cguid guid;
    
    guid.setLSB(0x42);
    
    EXPECT_EQ(0x42, guid.getNickname());
}

TEST(Cguid, NicknameIDRoundTrip) 
{
    cguid guid;
    
    guid.setNicknameID(0xBEEF);
    
    EXPECT_EQ(0xBEEF, guid.getNicknameID());
}

//-----------------------------------------------------------------------------
// Comparison tests
//-----------------------------------------------------------------------------

TEST(Cguid, IsSameGUID) 
{
    cguid guid("FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00");
    
    uint8_t same[16] = {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
                        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};
    
    uint8_t different[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    
    EXPECT_TRUE(guid.isSameGUID(same));
    EXPECT_FALSE(guid.isSameGUID(different));
}

TEST(Cguid, IsSameGUIDNull) 
{
    cguid guid;
    
    EXPECT_FALSE(guid.isSameGUID(nullptr));
}

//-----------------------------------------------------------------------------
// Edge cases
//-----------------------------------------------------------------------------

TEST(Cguid, LowercaseHexInput) 
{
    cguid guid("ff:ee:dd:cc:bb:aa:99:88:77:66:55:44:33:22:11:00");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xEE, guid.getAt(1));
}

TEST(Cguid, MixedCaseHexInput) 
{
    cguid guid("Ff:eE:Dd:cC:Bb:aA:99:88:77:66:55:44:33:22:11:00");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xEE, guid.getAt(1));
}

TEST(Cguid, PartialString) 
{
    cguid guid;  // Start with cleared GUID
    guid.getFromString("FF:EE:DD");
    
    // Should parse first 3 bytes
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xEE, guid.getAt(1));
    EXPECT_EQ(0xDD, guid.getAt(2));
    // Rest remains 0 from clear()
    for (int i = 3; i < 16; i++) {
        EXPECT_EQ(0x00, guid.getAt(i));
    }
}

//-----------------------------------------------------------------------------
// Extended GUID format tests (vscp-guid-parser formats)
//-----------------------------------------------------------------------------

TEST(Cguid, UUIDFormat) 
{
    // UUID format: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    cguid guid;
    guid.getFromString("FFEEDDCC-BBAA-9988-7766-554433221100");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xEE, guid.getAt(1));
    EXPECT_EQ(0xDD, guid.getAt(2));
    EXPECT_EQ(0xCC, guid.getAt(3));
    EXPECT_EQ(0xBB, guid.getAt(4));
    EXPECT_EQ(0xAA, guid.getAt(5));
    EXPECT_EQ(0x99, guid.getAt(6));
    EXPECT_EQ(0x88, guid.getAt(7));
    EXPECT_EQ(0x77, guid.getAt(8));
    EXPECT_EQ(0x66, guid.getAt(9));
    EXPECT_EQ(0x55, guid.getAt(10));
    EXPECT_EQ(0x44, guid.getAt(11));
    EXPECT_EQ(0x33, guid.getAt(12));
    EXPECT_EQ(0x22, guid.getAt(13));
    EXPECT_EQ(0x11, guid.getAt(14));
    EXPECT_EQ(0x00, guid.getAt(15));
}

TEST(Cguid, DoubleColonAllFF) 
{
    // "::" alone means all bytes are 0xFF
    cguid guid;
    guid.getFromString("::");
    
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(0xFF, guid.getAt(i)) << "Byte " << i << " should be 0xFF";
    }
}

TEST(Cguid, DoubleColonLeading) 
{
    // "::01:02:03" means leading bytes are 0xFF
    cguid guid;
    guid.getFromString("::01:02:03");
    
    // Bytes 0-12 should be 0xFF
    for (int i = 0; i < 13; i++) {
        EXPECT_EQ(0xFF, guid.getAt(i)) << "Byte " << i << " should be 0xFF";
    }
    // Last 3 bytes
    EXPECT_EQ(0x01, guid.getAt(13));
    EXPECT_EQ(0x02, guid.getAt(14));
    EXPECT_EQ(0x03, guid.getAt(15));
}

TEST(Cguid, DoubleColonTrailing) 
{
    // Trailing "::" is NOT supported by vscp-guid-parser
    // The parser only handles leading "::xx" format
    // With "01:02:03::", it parses the first 3 bytes, then stops
    cguid guid;
    guid.getFromString("01:02:03::");
    
    // First 3 bytes parsed
    EXPECT_EQ(0x01, guid.getAt(0));
    EXPECT_EQ(0x02, guid.getAt(1));
    EXPECT_EQ(0x03, guid.getAt(2));
    // Remaining bytes stay 0x00 (not expanded to 0xFF)
    for (int i = 3; i < 16; i++) {
        EXPECT_EQ(0x00, guid.getAt(i)) << "Byte " << i << " should be 0x00";
    }
}

TEST(Cguid, DoubleColonMiddle) 
{
    // Middle "::" is NOT supported by vscp-guid-parser
    // It only handles leading "::xx" format
    // With "01:02::FE:FF", parsing stops after "::"
    cguid guid;
    guid.getFromString("01:02::FE:FF");
    
    // First 2 bytes parsed
    EXPECT_EQ(0x01, guid.getAt(0));
    EXPECT_EQ(0x02, guid.getAt(1));
    // Remaining bytes stay 0x00 (middle :: doesn't work)
    for (int i = 2; i < 16; i++) {
        EXPECT_EQ(0x00, guid.getAt(i)) << "Byte " << i << " should be 0x00";
    }
}

TEST(Cguid, HyphenColonLeadingZeros) 
{
    // "-:01:02:03" means leading bytes are 0x00
    cguid guid("FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF");
    guid.getFromString("-:01:02:03");
    
    // Bytes 0-12 should be 0x00
    for (int i = 0; i < 13; i++) {
        EXPECT_EQ(0x00, guid.getAt(i)) << "Byte " << i << " should be 0x00";
    }
    // Last 3 bytes
    EXPECT_EQ(0x01, guid.getAt(13));
    EXPECT_EQ(0x02, guid.getAt(14));
    EXPECT_EQ(0x03, guid.getAt(15));
}

TEST(Cguid, HyphenColonTrailingZeros) 
{
    // "01:02:03:-" means trailing bytes are 0x00
    cguid guid("FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF");
    guid.getFromString("01:02:03:-");
    
    // First 3 bytes
    EXPECT_EQ(0x01, guid.getAt(0));
    EXPECT_EQ(0x02, guid.getAt(1));
    EXPECT_EQ(0x03, guid.getAt(2));
    // Bytes 3-15 should be 0x00
    for (int i = 3; i < 16; i++) {
        EXPECT_EQ(0x00, guid.getAt(i)) << "Byte " << i << " should be 0x00";
    }
}

TEST(Cguid, HyphenColonMiddleZeros) 
{
    // Middle "-:" is NOT supported by vscp-guid-parser
    // It only handles leading "-:xx" format
    // With "01:02:-:FE:FF", parsing stops at "-"
    cguid guid("FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF");
    guid.getFromString("01:02:-:FE:FF");
    
    // First 2 bytes parsed
    EXPECT_EQ(0x01, guid.getAt(0));
    EXPECT_EQ(0x02, guid.getAt(1));
    // Remaining bytes stay 0x00 (middle -: doesn't work)
    for (int i = 2; i < 16; i++) {
        EXPECT_EQ(0x00, guid.getAt(i)) << "Byte " << i << " should be 0x00";
    }
}

TEST(Cguid, BracesFormat) 
{
    // GUID with braces
    cguid guid;
    guid.getFromString("{::0102:03aa:44:01:30}");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xFF, guid.getAt(1));
    EXPECT_EQ(0xFF, guid.getAt(2));
    EXPECT_EQ(0xFF, guid.getAt(3));
    EXPECT_EQ(0xFF, guid.getAt(4));
    EXPECT_EQ(0xFF, guid.getAt(5));
    EXPECT_EQ(0xFF, guid.getAt(6));
    EXPECT_EQ(0xFF, guid.getAt(7));
    EXPECT_EQ(0xFF, guid.getAt(8));
    EXPECT_EQ(0x01, guid.getAt(9));
    EXPECT_EQ(0x02, guid.getAt(10));
    EXPECT_EQ(0x03, guid.getAt(11));
    EXPECT_EQ(0xaa, guid.getAt(12));
    EXPECT_EQ(0x44, guid.getAt(13));
    EXPECT_EQ(0x01, guid.getAt(14));
    EXPECT_EQ(0x30, guid.getAt(15));
}

TEST(Cguid, BracesFormat2) 
{
    // GUID with braces
    cguid guid;
    guid.getFromString("{::0102-03aa-44-01-30}");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xFF, guid.getAt(1));
    EXPECT_EQ(0xFF, guid.getAt(2));
    EXPECT_EQ(0xFF, guid.getAt(3));
    EXPECT_EQ(0xFF, guid.getAt(4));
    EXPECT_EQ(0xFF, guid.getAt(5));
    EXPECT_EQ(0xFF, guid.getAt(6));
    EXPECT_EQ(0xFF, guid.getAt(7));
    EXPECT_EQ(0xFF, guid.getAt(8));
    EXPECT_EQ(0x01, guid.getAt(9));
    EXPECT_EQ(0x02, guid.getAt(10));
    EXPECT_EQ(0x03, guid.getAt(11));
    EXPECT_EQ(0xaa, guid.getAt(12));
    EXPECT_EQ(0x44, guid.getAt(13));
    EXPECT_EQ(0x01, guid.getAt(14));
    EXPECT_EQ(0x30, guid.getAt(15));
}

TEST(Cguid, UnBracesFormat) 
{
    // GUID with braces
    cguid guid;
    guid.getFromString("::0102:03aa:44:01:30");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xFF, guid.getAt(1));
    EXPECT_EQ(0xFF, guid.getAt(2));
    EXPECT_EQ(0xFF, guid.getAt(3));
    EXPECT_EQ(0xFF, guid.getAt(4));
    EXPECT_EQ(0xFF, guid.getAt(5));
    EXPECT_EQ(0xFF, guid.getAt(6));
    EXPECT_EQ(0xFF, guid.getAt(7));
    EXPECT_EQ(0xFF, guid.getAt(8));
    EXPECT_EQ(0x01, guid.getAt(9));
    EXPECT_EQ(0x02, guid.getAt(10));
    EXPECT_EQ(0x03, guid.getAt(11));
    EXPECT_EQ(0xaa, guid.getAt(12));
    EXPECT_EQ(0x44, guid.getAt(13));
    EXPECT_EQ(0x01, guid.getAt(14));
    EXPECT_EQ(0x30, guid.getAt(15));
}

TEST(Cguid, BracesUUIDFormat) 
{
    // UUID format with braces
    cguid guid;
    guid.getFromString("{FFEEDDCC-BBAA-9988-7766-554433221100}");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xEE, guid.getAt(1));
    EXPECT_EQ(0xDD, guid.getAt(2));
    EXPECT_EQ(0xCC, guid.getAt(3));
    EXPECT_EQ(0xBB, guid.getAt(4));
    EXPECT_EQ(0xAA, guid.getAt(5));
    EXPECT_EQ(0x99, guid.getAt(6));
    EXPECT_EQ(0x88, guid.getAt(7));
    EXPECT_EQ(0x77, guid.getAt(8));
    EXPECT_EQ(0x66, guid.getAt(9));
    EXPECT_EQ(0x55, guid.getAt(10));
    EXPECT_EQ(0x44, guid.getAt(11));
    EXPECT_EQ(0x33, guid.getAt(12));
    EXPECT_EQ(0x22, guid.getAt(13));
    EXPECT_EQ(0x11, guid.getAt(14));
    EXPECT_EQ(0x00, guid.getAt(15));
}

TEST(Cguid, VSCP_UUIDFormat1)
{
    // UUID format with braces
    cguid guid;
    guid.getFromString("{FFEEDDCC-BBAA-9988-7766-55443322-1100}");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xEE, guid.getAt(1));
    EXPECT_EQ(0xDD, guid.getAt(2));
    EXPECT_EQ(0xCC, guid.getAt(3));
    EXPECT_EQ(0xBB, guid.getAt(4));
    EXPECT_EQ(0xAA, guid.getAt(5));
    EXPECT_EQ(0x99, guid.getAt(6));
    EXPECT_EQ(0x88, guid.getAt(7));
    EXPECT_EQ(0x77, guid.getAt(8));
    EXPECT_EQ(0x66, guid.getAt(9));
    EXPECT_EQ(0x55, guid.getAt(10));
    EXPECT_EQ(0x44, guid.getAt(11));
    EXPECT_EQ(0x33, guid.getAt(12));
    EXPECT_EQ(0x22, guid.getAt(13));
    EXPECT_EQ(0x11, guid.getAt(14));
    EXPECT_EQ(0x00, guid.getAt(15));
}

TEST(Cguid, VSCP_UUIDFormatStrangeGrouping)
{
    // UUID format with braces
    cguid guid;
    guid.getFromString("{FF-EE-DD-CC-BBAA-9988-7766-5544-3322-1100}");
    
    EXPECT_EQ(0xFF, guid.getAt(0));
    EXPECT_EQ(0xEE, guid.getAt(1));
    EXPECT_EQ(0xDD, guid.getAt(2));
    EXPECT_EQ(0xCC, guid.getAt(3));
    EXPECT_EQ(0xBB, guid.getAt(4));
    EXPECT_EQ(0xAA, guid.getAt(5));
    EXPECT_EQ(0x99, guid.getAt(6));
    EXPECT_EQ(0x88, guid.getAt(7));
    EXPECT_EQ(0x77, guid.getAt(8));
    EXPECT_EQ(0x66, guid.getAt(9));
    EXPECT_EQ(0x55, guid.getAt(10));
    EXPECT_EQ(0x44, guid.getAt(11));
    EXPECT_EQ(0x33, guid.getAt(12));
    EXPECT_EQ(0x22, guid.getAt(13));
    EXPECT_EQ(0x11, guid.getAt(14));
    EXPECT_EQ(0x00, guid.getAt(15));
}

TEST(Cguid, FreelyGroupedHexFormat)
{
    cguid guid;
    guid.getFromString("FFEEDD:CCBBAA99-8877,665544332211:00");

    const uint8_t expected[16] = { 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
                                   0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00 };
    for (int index = 0; index < 16; index++) {
        EXPECT_EQ(expected[index], guid.getAt(index));
    }
}

TEST(Cguid, MultiByteHexValues) 
{
    // Multi-byte hex values: "::0102:03aa:bb0c"
    // This should expand to individual bytes
    cguid guid;
    guid.getFromString("::0102:03aa:bb0c");
    
    // Leading bytes should be 0xFF (due to ::)
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(0xFF, guid.getAt(i)) << "Byte " << i << " should be 0xFF";
    }
    // Multi-byte values split into bytes
    EXPECT_EQ(0x01, guid.getAt(10));
    EXPECT_EQ(0x02, guid.getAt(11));
    EXPECT_EQ(0x03, guid.getAt(12));
    EXPECT_EQ(0xAA, guid.getAt(13));
    EXPECT_EQ(0xBB, guid.getAt(14));
    EXPECT_EQ(0x0C, guid.getAt(15));
}

TEST(Cguid, Compact) 
{

    cguid guid;
    guid.getFromString("0102030405060708090A0B0C0D0E0F10");
    
    // Multi-byte values split into bytes
    EXPECT_EQ(0x01, guid.getAt(0));
    EXPECT_EQ(0x02, guid.getAt(1));
    EXPECT_EQ(0x03, guid.getAt(2));
    EXPECT_EQ(0x04, guid.getAt(3));
    EXPECT_EQ(0x05, guid.getAt(4));
    EXPECT_EQ(0x06, guid.getAt(5));
    EXPECT_EQ(0x07, guid.getAt(6));
    EXPECT_EQ(0x08, guid.getAt(7));
    EXPECT_EQ(0x09, guid.getAt(8));
    EXPECT_EQ(0x0A, guid.getAt(9));
    EXPECT_EQ(0x0B, guid.getAt(10));
    EXPECT_EQ(0x0C, guid.getAt(11));
    EXPECT_EQ(0x0D, guid.getAt(12));
    EXPECT_EQ(0x0E, guid.getAt(13));
    EXPECT_EQ(0x0F, guid.getAt(14));
    EXPECT_EQ(0x10, guid.getAt(15));
}


TEST(Cguid, CompactMinimized) 
{
    // Does not ned to be all digits
    cguid guid;
    guid.getFromString("112233");
    
    // Multi-byte values split into bytes
    EXPECT_EQ(0x11, guid.getAt(0));
    EXPECT_EQ(0x22, guid.getAt(1));
    EXPECT_EQ(0x33, guid.getAt(2));
    EXPECT_EQ(0x00, guid.getAt(3));
    EXPECT_EQ(0x00, guid.getAt(4));
    EXPECT_EQ(0x00, guid.getAt(5));
    EXPECT_EQ(0x00, guid.getAt(6));
    EXPECT_EQ(0x00, guid.getAt(7));
    EXPECT_EQ(0x00, guid.getAt(8));
    EXPECT_EQ(0x00, guid.getAt(9));
    EXPECT_EQ(0x00, guid.getAt(10));
    EXPECT_EQ(0x00, guid.getAt(11));
    EXPECT_EQ(0x00, guid.getAt(12));
    EXPECT_EQ(0x00, guid.getAt(13));
    EXPECT_EQ(0x00, guid.getAt(14));
    EXPECT_EQ(0x00, guid.getAt(15));
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
