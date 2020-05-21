// unit_test.cpp
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vscphelper.h>
#include <gtest/gtest.h>

TEST(HelperLib, vscp_getMeasurementAsFloat) { 
    uint8_t iv1[5] = {0xae,0x00,0x00,0x00,0x00};
    ASSERT_EQ(0.0f, vscp_getMeasurementAsFloat(iv1,5));
    
    uint8_t iv2[5] = {0xae,0x41,0x83,0x80,0x00};
    ASSERT_EQ(16.4375f, vscp_getMeasurementAsFloat(iv2,5));
    
    uint8_t iv3[5] = {0xae,0x3E,0x20,0x00,0x00};
    ASSERT_EQ(0.15625f, vscp_getMeasurementAsFloat(iv3,5));
    
    uint8_t iv4[5] = {0xae,0xC6,0xA7,0xE2,0xA4};
    ASSERT_EQ(-21489.320312f, vscp_getMeasurementAsFloat(iv4,5));
    ASSERT_NE(-21489.320312, vscp_getMeasurementAsFloat(iv4,5));
    ASSERT_NE(-1, vscp_getMeasurementAsFloat(iv4,5));
    ASSERT_NE(0, vscp_getMeasurementAsFloat(iv4,5));

    uint8_t iv5[3] = {0xae,0x41,0x83};
    ASSERT_EQ(0, vscp_getMeasurementAsFloat(iv5,3));

}

TEST(HelperLib, vscp_getMeasurementAsDouble) {
    vscpEvent e;
    double val;
    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata = new uint8_t[4];
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;
    ASSERT_TRUE(vscp_getMeasurementAsDouble(&val,&e));
    ASSERT_EQ(694600, val );

    // 32-bit float coding = 100
    e.sizeData = 3;
    e.pdata = new uint8_t[3];
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x85;
    e.pdata[2] = 0x8d;
    ASSERT_TRUE(vscp_getMeasurementAsDouble(&val,&e));
    ASSERT_EQ(-0.00115, val );
    delete [] e.pdata;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata = new uint8_t[4];
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x81;
    e.pdata[2] = 0x01;
    e.pdata[3] = 0x07;
    ASSERT_TRUE(vscp_getMeasurementAsDouble(&val,&e));
    ASSERT_EQ(26.3, val );
    delete [] e.pdata;

    // 32-bit float coding = 101 Unit=01 Sensor=7
    e.sizeData = 5;
    e.pdata = new uint8_t[5];
    e.pdata[0] = 0xAE;
    e.pdata[1] = 0x41;
    e.pdata[2] = 0x83;
    e.pdata[3] = 0x80;
    e.pdata[4] = 0x00;
    ASSERT_TRUE(vscp_getMeasurementAsDouble(&val,&e));
    ASSERT_EQ(16.4375, val );
    delete [] e.pdata;
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}