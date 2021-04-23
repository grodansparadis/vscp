// unit_test.cpp
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vscphelper.h>
#include <userlist.h>
#include <gtest/gtest.h>

//-----------------------------------------------------------------------------
TEST(Userlist, vscp_getMeasurementAsFloat) 
{ 
    uint8_t iv1[5] = {0xae, 0x00, 0x00, 0x00, 0x00};
    ASSERT_EQ(0.0f, vscp_getMeasurementAsFloat(iv1, 5));
    
    uint8_t iv2[5] = {0xae, 0x41, 0x83, 0x80, 0x00};
    ASSERT_EQ(16.4375f, vscp_getMeasurementAsFloat(iv2, 5));
    
    uint8_t iv3[5] = {0xae, 0x3E, 0x20, 0x00, 0x00};
    ASSERT_EQ(0.15625f, vscp_getMeasurementAsFloat(iv3, 5));
    
    uint8_t iv4[5] = {0xae,0xC6,0xA7,0xE2,0xA4};
    ASSERT_EQ(-21489.320312f, vscp_getMeasurementAsFloat(iv4, 5));
    ASSERT_NE(-21489.320312, vscp_getMeasurementAsFloat(iv4, 5));
    ASSERT_NE(-1, vscp_getMeasurementAsFloat(iv4, 5));
    ASSERT_NE(0, vscp_getMeasurementAsFloat(iv4, 5));

    uint8_t iv5[3] = {0xae,0x41,0x83};
    ASSERT_EQ(0, vscp_getMeasurementAsFloat(iv5, 3));
}


//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
