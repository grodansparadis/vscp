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

TEST(HelperLib, vscphlp_convertEventToJSON) {
    vscpEvent e;
    std::string result;
    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata = new uint8_t[4];
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;
    ASSERT_TRUE(vscp_convertEventToJSON(result, &e));
    //printf("%s\n",result.c_str());
    ASSERT_TRUE(NULL != strstr(result.c_str(), "vscpClass\": 10,") );
    ASSERT_TRUE(NULL != strstr(result.c_str(), "vscpType\": 6,") );
    delete [] e.pdata;
}

TEST(HelperLib, vscphlp_convertEventExToJSON) {
    vscpEventEx ex;
    std::string result;
    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    ex.sizeData = 4;
    ex.data[0] = 0x80;
    ex.data[1] = 0x02;
    ex.data[2] = 0x1B;
    ex.data[3] = 0x22;
    ASSERT_TRUE(vscp_convertEventExToJSON(result, &ex));
    //printf("%s\n",result.c_str());
    ASSERT_TRUE(NULL != strstr(result.c_str(), "vscpClass\": 10,") );
    ASSERT_TRUE(NULL != strstr(result.c_str(), "vscpType\": 6,") );
}

TEST(HelperLib, vscphlp_convertEventToXML) {
    vscpEvent e;
    std::string result;
    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata = new uint8_t[4];
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;
    ASSERT_TRUE(vscp_convertEventToXML(result, &e));
    //printf("%s\n", result.c_str());
    ASSERT_TRUE(NULL != strstr(result.c_str(), "vscpClass=\"10\"") );
    ASSERT_TRUE(NULL != strstr(result.c_str(), "vscpType=\"6\"") );
    delete [] e.pdata;
}

TEST(HelperLib, vscphlp_convertEventExToXML) {
    vscpEventEx ex;
    std::string result;
    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    ex.sizeData = 4;
    ex.data[0] = 0x80;
    ex.data[1] = 0x02;
    ex.data[2] = 0x1B;
    ex.data[3] = 0x22;
    ASSERT_TRUE(vscp_convertEventExToXML(result, &ex));
    //printf("%s\n", result.c_str());
    ASSERT_TRUE(NULL != strstr(result.c_str(), "vscpClass=\"10\"") );
    ASSERT_TRUE(NULL != strstr(result.c_str(), "vscpType=\"6\"") );
}

TEST(HelperLib, vscphlp_convertEventToHTML) {
    vscpEvent e;
    std::string result;
    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata = new uint8_t[4];
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;
    ASSERT_TRUE(vscp_convertEventToHTML(result, &e));
    //printf("|%s|\n", result.c_str());
    ASSERT_TRUE(NULL != strstr(result.c_str(), "<h2>VSCP Event</h2> <p>Class: 10 <br>Type: 6 <br></p><p>Data count: 4<br>Data: 0x80,0x02,0x1B,0x22<br></p><p>From GUID: ") );
    delete [] e.pdata;
}

TEST(HelperLib, vscphlp_convertEventExToHTML) {
    vscpEventEx ex;
    std::string result;
    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    ex.sizeData = 4;
    ex.data[0] = 0x80;
    ex.data[1] = 0x02;
    ex.data[2] = 0x1B;
    ex.data[3] = 0x22;
    ASSERT_TRUE(vscp_convertEventExToHTML(result, &ex));
    //printf("|%s|\n", result.c_str());
    ASSERT_TRUE(NULL != strstr(result.c_str(), "<h2>VSCP Event</h2> <p>Class: 10 <br>Type: 6 <br></p><p>Data count: 4<br>Data: 0x80,0x02,0x1B,0x22<br></p><p>From GUID: ") );
}

TEST(HelperLib, vscp_makeFloatMeasurementEventEx) {
    vscpEventEx ex;

    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    float f = 23.456;
    bool rv = vscp_makeFloatMeasurementEventEx( &ex,
                                                f,
                                                1,
                                                3 );
    ASSERT_TRUE(rv);
    ASSERT_TRUE(ex.vscp_class == VSCP_CLASS1_MEASUREMENT);
    //printf("%d",ex.sizeData);
    ASSERT_TRUE(5 == ex.sizeData);
    //printf("%d %d %d %d %d ",ex.data[0],ex.data[1],ex.data[2],ex.data[3],ex.data[4]);
    ASSERT_TRUE(171 == ex.data[0]);
    ASSERT_TRUE(227 == ex.data[1]);
    ASSERT_TRUE(165 == ex.data[2]);
    ASSERT_TRUE(187 == ex.data[3]);
    ASSERT_TRUE(65 == ex.data[4]);
}

TEST(HelperLib, vscp_makeLevel2StringMeasurementEventEx) {
    vscpEventEx ex;

    double d = 23.456;
    bool rv = vscp_makeLevel2StringMeasurementEventEx( &ex,
                                                        VSCP_TYPE_MEASUREMENT_TEMPERATURE,
                                                        d,
                                                        0,   /* unit */ 
                                                        1,   /* sensorindex */
                                                        2,   /* zone*/
                                                        3 ); /* subzone */
    ASSERT_TRUE(rv);
    ASSERT_TRUE(ex.vscp_class == VSCP_CLASS2_MEASUREMENT_STR);
    printf("%d",ex.sizeData);
    ASSERT_TRUE(14 == ex.sizeData);
    printf(" %d %d %d %d %d ",ex.data[10],ex.data[11],ex.data[12],ex.data[13],ex.data[13]);
    ASSERT_TRUE(1 == ex.data[0]);
    ASSERT_TRUE(2 == ex.data[1]);
    ASSERT_TRUE(3 == ex.data[2]);
    ASSERT_TRUE(0 == ex.data[3]);
    ASSERT_TRUE(50 == ex.data[4]);
    ASSERT_TRUE(51 == ex.data[5]);
    ASSERT_TRUE(46 == ex.data[6]);
    ASSERT_TRUE(52 == ex.data[7]);
    ASSERT_TRUE(53 == ex.data[8]);
    ASSERT_TRUE(54 == ex.data[9]);
    ASSERT_TRUE(48 == ex.data[10]);
    ASSERT_TRUE(48 == ex.data[11]);
    ASSERT_TRUE(48 == ex.data[12]);
    ASSERT_TRUE(0 == ex.data[13]);
}

TEST(HelperLib, vscp_convertLevel1MeasuremenToLevel2DoubleEx) {
    vscpEventEx ex;

    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_class = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
    // 32-bit float coding = 100
    ex.sizeData = 4;
    ex.data[0] = 0x80;
    ex.data[1] = 0x02;
    ex.data[2] = 0x1B;
    ex.data[3] = 0x22;

    bool rv = vscp_convertLevel1MeasuremenToLevel2DoubleEx( &ex ); 
    ASSERT_TRUE(rv);
    ASSERT_TRUE(ex.vscp_class == VSCP_CLASS2_MEASUREMENT_FLOAT);
    printf("%d",ex.sizeData);
    ASSERT_TRUE(14 == ex.sizeData);
    // printf(" %d %d %d %d %d ",ex.data[10],ex.data[11],ex.data[12],ex.data[13],ex.data[13]);
    // ASSERT_TRUE(1 == ex.data[0]);
    // ASSERT_TRUE(2 == ex.data[1]);
    // ASSERT_TRUE(3 == ex.data[2]);
    // ASSERT_TRUE(0 == ex.data[3]);
    // ASSERT_TRUE(50 == ex.data[4]);
    // ASSERT_TRUE(51 == ex.data[5]);
    // ASSERT_TRUE(46 == ex.data[6]);
    // ASSERT_TRUE(52 == ex.data[7]);
    // ASSERT_TRUE(53 == ex.data[8]);
    // ASSERT_TRUE(54 == ex.data[9]);
    // ASSERT_TRUE(48 == ex.data[10]);
    // ASSERT_TRUE(48 == ex.data[11]);
    // ASSERT_TRUE(48 == ex.data[12]);
    // ASSERT_TRUE(0 == ex.data[13]);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
