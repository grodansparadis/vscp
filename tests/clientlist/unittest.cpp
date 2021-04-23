// unit_test.cpp
//
// Tests for the userlist and useritem classes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <clientlist.h>
#include <gtest/gtest.h>

//-----------------------------------------------------------------------------
TEST(ClientList, getter_setters) 
{ 
    CUserItem item;

    item.setUserName("KalleMero");
    ASSERT_EQ("KalleMero_", item.getUserName());
    
    // uint8_t iv4[5] = {0xae,0xC6,0xA7,0xE2,0xA4};
    // ASSERT_EQ(-21489.320312f, vscp_getMeasurementAsFloat(iv4, 5));
    // ASSERT_NE(-21489.320312, vscp_getMeasurementAsFloat(iv4, 5));
    // ASSERT_NE(-1, vscp_getMeasurementAsFloat(iv4, 5));
    // ASSERT_NE(0, vscp_getMeasurementAsFloat(iv4, 5));
}

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
