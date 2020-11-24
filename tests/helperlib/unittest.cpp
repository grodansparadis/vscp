// unit_test.cpp
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vscphelper.h>
#include <gtest/gtest.h>

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_getMeasurementAsFloat) 
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
TEST(HelperLib, vscp_getMeasurementAsDouble) 
{
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
    
    vscp_deleteEvent(&e);

    // 32-bit float coding = 100
    e.sizeData = 3;
    e.pdata = new uint8_t[3];
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x85;
    e.pdata[2] = 0x8d;
    ASSERT_TRUE(vscp_getMeasurementAsDouble(&val,&e));
    ASSERT_EQ(-0.00115, val );
    
    vscp_deleteEvent(&e);

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata = new uint8_t[4];
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x81;
    e.pdata[2] = 0x01;
    e.pdata[3] = 0x07;
    ASSERT_TRUE(vscp_getMeasurementAsDouble(&val,&e));
    ASSERT_EQ(26.3, val );
    
    vscp_deleteEvent(&e);

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
    
    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertEventToJSON) 
{
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
    
    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertEventExToJSON) 
{    
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

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

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertEventToXML) 
{
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
    
    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertEventExToXML) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

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

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertEventToHTML) 
{
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
    
    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertEventExToHTML) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

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

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_makeFloatMeasurementEvent) 
{
    vscpEvent e;
    e.pdata = NULL;

    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    float f = 23.456;
    bool rv = vscp_makeFloatMeasurementEvent( &e,
                                                f,
                                                1,
                                                3 );
    ASSERT_TRUE(rv);
    ASSERT_TRUE(e.vscp_class == VSCP_CLASS1_MEASUREMENT);
    //printf("%d",e.sizeData);
    ASSERT_TRUE(5 == e.sizeData);
    //printf("%d %d %d %d %d ",e.pdata[0],e.pdata[1],e.pdata[2],e.pdata[3],e.pdata[4]);
    ASSERT_TRUE(171 == e.pdata[0]);
    ASSERT_TRUE(227 == e.pdata[1]);
    ASSERT_TRUE(165 == e.pdata[2]);
    ASSERT_TRUE(187 == e.pdata[3]);
    ASSERT_TRUE(65 == e.pdata[4]);

    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_makeFloatMeasurementEventEx) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

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

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_makeStringMeasurementEvent) 
{
    vscpEvent e;
    e.pdata = NULL;

    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    float f = 23.456;
    bool rv = vscp_makeStringMeasurementEvent( &e,
                                                f,
                                                1,
                                                3 );
    ASSERT_TRUE(rv);
    ASSERT_TRUE(e.vscp_class == VSCP_CLASS1_MEASUREMENT);
    //printf("%d\n",e.sizeData);
    ASSERT_TRUE(8 == e.sizeData);
    // printf("%d %d %d %d %d %d %d %d\n", 
    //         e.pdata[0], e.pdata[1], e.pdata[2], e.pdata[3], 
    //         e.pdata[4], e.pdata[5], e.pdata[6], e.pdata[7]);
    ASSERT_TRUE(75 == e.pdata[0]);
    ASSERT_TRUE(50 == e.pdata[1]);
    ASSERT_TRUE(51 == e.pdata[2]);
    ASSERT_TRUE(46 == e.pdata[3]);
    ASSERT_TRUE(52 == e.pdata[4]);
    ASSERT_TRUE(53 == e.pdata[5]);
    ASSERT_TRUE(53 == e.pdata[6]);
    ASSERT_TRUE(57 == e.pdata[7]);

    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_makeStringMeasurementEventEx) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    float f = 23.456;
    bool rv = vscp_makeStringMeasurementEventEx( &ex,
                                                f,
                                                1,
                                                3 );
    ASSERT_TRUE(rv);
    ASSERT_TRUE(ex.vscp_class == VSCP_CLASS1_MEASUREMENT);
    //printf("%d\n",ex.sizeData);
    ASSERT_TRUE(8 == ex.sizeData);
    // printf("%d %d %d %d %d %d %d %d\n", 
    //         ex.data[0], ex.data[1], ex.data[2], ex.data[3], 
    //         ex.data[4], ex.data[5], ex.data[6], ex.data[7]);
    ASSERT_TRUE(75 == ex.data[0]);
    ASSERT_TRUE(50 == ex.data[1]);
    ASSERT_TRUE(51 == ex.data[2]);
    ASSERT_TRUE(46 == ex.data[3]);
    ASSERT_TRUE(52 == ex.data[4]);
    ASSERT_TRUE(53 == ex.data[5]);
    ASSERT_TRUE(53 == ex.data[6]);
    ASSERT_TRUE(57 == ex.data[7]);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_makeLevel2FloatMeasurementEvent)
{
    vscpEvent e;
    e.pdata = NULL;

    //e.vscp_class = VSCP2_CLASS_MEASUREMENT;
    double d = 23.456;
    bool rv = vscp_makeLevel2FloatMeasurementEvent( &e,
                                                    VSCP_TYPE_MEASUREMENT_TEMPERATURE,
                                                    d,
                                                    1,  
                                                    3,
                                                    33,
                                                    44 );
    ASSERT_TRUE(rv);
    ASSERT_TRUE(e.vscp_class == VSCP_CLASS2_MEASUREMENT_FLOAT);
    ASSERT_TRUE(e.vscp_type == VSCP_TYPE_MEASUREMENT_TEMPERATURE);
    // printf("%d\n",e.sizeData);
    ASSERT_TRUE(12 == e.sizeData);
    // printf("%d %d %d %d %d %d %d %d %d %d %d %d\n", 
    //         e.pdata[0], e.pdata[1], e.pdata[2], e.pdata[3], 
    //         e.pdata[4], e.pdata[5], e.pdata[6], e.pdata[7],
    //         e.pdata[8], e.pdata[9], e.pdata[10], e.pdata[11]);
    ASSERT_TRUE(3 == e.pdata[0]);
    ASSERT_TRUE(33 == e.pdata[1]);
    ASSERT_TRUE(44 == e.pdata[2]);
    ASSERT_TRUE(1 == e.pdata[3]);
    ASSERT_TRUE(219 == e.pdata[4]);
    ASSERT_TRUE(249 == e.pdata[5]);
    ASSERT_TRUE(126 == e.pdata[6]);
    ASSERT_TRUE(106 == e.pdata[7]);
    ASSERT_TRUE(188 == e.pdata[8]);
    ASSERT_TRUE(116 == e.pdata[9]);
    ASSERT_TRUE(55 == e.pdata[10]);
    ASSERT_TRUE(64 == e.pdata[11]);

    vscp_deleteEvent(&e);    
}


//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_makeLevel2FloatMeasurementEventEx) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    double d = 23.456;
    bool rv = vscp_makeLevel2FloatMeasurementEventEx( &ex,
                                                        VSCP_TYPE_MEASUREMENT_TEMPERATURE,
                                                        d,
                                                        1,  
                                                        3,
                                                        33,
                                                        44 );
    ASSERT_TRUE(rv);
    ASSERT_TRUE(ex.vscp_class == VSCP_CLASS2_MEASUREMENT_FLOAT);
    ASSERT_TRUE(ex.vscp_type == VSCP_TYPE_MEASUREMENT_TEMPERATURE);
    // printf("%d\n",ex.sizeData);
    ASSERT_TRUE(12 == ex.sizeData);
    // printf("%d %d %d %d %d %d %d %d %d %d %d %d\n", 
    //         ex.data[0], ex.data[1], ex.data[2], ex.data[3], 
    //         ex.data[4], ex.data[5], ex.data[6], ex.data[7],
    //         ex.data[8], ex.data[9], ex.data[10], ex.data[11]);
    ASSERT_TRUE(3 == ex.data[0]);
    ASSERT_TRUE(33 == ex.data[1]);
    ASSERT_TRUE(44 == ex.data[2]);
    ASSERT_TRUE(1 == ex.data[3]);
    ASSERT_TRUE(219 == ex.data[4]);
    ASSERT_TRUE(249 == ex.data[5]);
    ASSERT_TRUE(126 == ex.data[6]);
    ASSERT_TRUE(106 == ex.data[7]);
    ASSERT_TRUE(188 == ex.data[8]);
    ASSERT_TRUE(116 == ex.data[9]);
    ASSERT_TRUE(55 == ex.data[10]);
    ASSERT_TRUE(64 == ex.data[11]);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_makeLevel2StringMeasurementEvent) 
{
    vscpEvent e;
    e.pdata = NULL;

    double d = 23.456;
    bool rv = vscp_makeLevel2StringMeasurementEvent( &e,
                                                        VSCP_TYPE_MEASUREMENT_TEMPERATURE,
                                                        d,
                                                        0,   /* unit */ 
                                                        1,   /* sensorindex */
                                                        2,   /* zone*/
                                                        3 ); /* subzone */
    ASSERT_TRUE(rv);
    ASSERT_TRUE(e.vscp_class == VSCP_CLASS2_MEASUREMENT_STR);
    ASSERT_TRUE(e.vscp_type == VSCP_TYPE_MEASUREMENT_TEMPERATURE);
    //printf("%d",ex.sizeData);
    ASSERT_TRUE(14 == e.sizeData);
    //printf(" %d %d %d %d %d ",ex.data[10],ex.data[11],ex.data[12],ex.data[13],ex.data[13]);
    ASSERT_TRUE(1 == e.pdata[0]);
    ASSERT_TRUE(2 == e.pdata[1]);
    ASSERT_TRUE(3 == e.pdata[2]);
    ASSERT_TRUE(0 == e.pdata[3]);
    ASSERT_TRUE(50 == e.pdata[4]);
    ASSERT_TRUE(51 == e.pdata[5]);
    ASSERT_TRUE(46 == e.pdata[6]);
    ASSERT_TRUE(52 == e.pdata[7]);
    ASSERT_TRUE(53 == e.pdata[8]);
    ASSERT_TRUE(54 == e.pdata[9]);
    ASSERT_TRUE(48 == e.pdata[10]);
    ASSERT_TRUE(48 == e.pdata[11]);
    ASSERT_TRUE(48 == e.pdata[12]);
    ASSERT_TRUE(0 == e.pdata[13]);

    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_makeLevel2StringMeasurementEventEx) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

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
    ASSERT_TRUE(ex.vscp_type == VSCP_TYPE_MEASUREMENT_TEMPERATURE);
    //printf("%d",ex.sizeData);
    ASSERT_TRUE(14 == ex.sizeData);
    //printf(" %d %d %d %d %d ",ex.data[10],ex.data[11],ex.data[12],ex.data[13],ex.data[13]);
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

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertLevel1MeasuremenToLevel2Double) 
{
    vscpEvent e;
    e.pdata = new uint8_t[4];

    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;

    bool rv = vscp_convertLevel1MeasuremenToLevel2Double( &e ); 
    ASSERT_TRUE(rv);
    ASSERT_TRUE(e.vscp_class == VSCP_CLASS2_MEASUREMENT_FLOAT);
    //printf("%d",ex.sizeData);
    ASSERT_TRUE(12 == e.sizeData);
    // printf(" %d %d %d %d %d ",ex.data[0],ex.data[1],ex.data[2],ex.data[3],ex.data[4]);
    // printf(" %d %d %d %d %d ",ex.data[5],ex.data[6],ex.data[7],ex.data[8],ex.data[9]);
    // printf(" %d %d ",ex.data[10],ex.data[11]);
    ASSERT_TRUE(0 == e.pdata[0]);
    ASSERT_TRUE(0 == e.pdata[1]);
    ASSERT_TRUE(0 == e.pdata[2]);
    ASSERT_TRUE(0 == e.pdata[3]);
    ASSERT_TRUE(0 == e.pdata[4]);
    ASSERT_TRUE(0 == e.pdata[5]);
    ASSERT_TRUE(0 == e.pdata[6]);
    ASSERT_TRUE(128 == e.pdata[7]);
    ASSERT_TRUE(66 == e.pdata[8]);
    ASSERT_TRUE(38 == e.pdata[9]);
    ASSERT_TRUE(210 == e.pdata[10]);
    ASSERT_TRUE(67 == e.pdata[11]);

    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertLevel1MeasuremenToLevel2DoubleEx) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
    // 32-bit float coding = 100
    ex.sizeData = 4;
    ex.data[0] = 0x80;
    ex.data[1] = 0x02;
    ex.data[2] = 0x1B;
    ex.data[3] = 0x22;

    bool rv = vscp_convertLevel1MeasuremenToLevel2DoubleEx( &ex ); 
    ASSERT_TRUE(rv);
    ASSERT_TRUE(ex.vscp_class == VSCP_CLASS2_MEASUREMENT_FLOAT);
    //printf("%d",ex.sizeData);
    ASSERT_TRUE(12 == ex.sizeData);
    // printf(" %d %d %d %d %d ",ex.data[0],ex.data[1],ex.data[2],ex.data[3],ex.data[4]);
    // printf(" %d %d %d %d %d ",ex.data[5],ex.data[6],ex.data[7],ex.data[8],ex.data[9]);
    // printf(" %d %d ",ex.data[10],ex.data[11]);
    ASSERT_TRUE(0 == ex.data[0]);
    ASSERT_TRUE(0 == ex.data[1]);
    ASSERT_TRUE(0 == ex.data[2]);
    ASSERT_TRUE(0 == ex.data[3]);
    ASSERT_TRUE(0 == ex.data[4]);
    ASSERT_TRUE(0 == ex.data[5]);
    ASSERT_TRUE(0 == ex.data[6]);
    ASSERT_TRUE(128 == ex.data[7]);
    ASSERT_TRUE(66 == ex.data[8]);
    ASSERT_TRUE(38 == ex.data[9]);
    ASSERT_TRUE(210 == ex.data[10]);
    ASSERT_TRUE(67 == ex.data[11]);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertLevel1MeasuremenToLevel2String) 
{
    vscpEvent e;
    e.pdata = new uint8_t[4];

    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;

    bool rv = vscp_convertLevel1MeasuremenToLevel2String( &e ); 
    ASSERT_TRUE(rv);
    ASSERT_TRUE(e.vscp_class == VSCP_CLASS2_MEASUREMENT_STR);
    // printf("%d\n",e.sizeData);
    ASSERT_TRUE(17 == e.sizeData);
    // printf(" %d %d %d %d %d ",e.pdata[0],e.pdata[1],e.pdata[2],e.pdata[3],e.pdata[4]);
    // printf(" %d %d %d %d %d ",e.pdata[5],e.pdata[6],e.pdata[7],e.pdata[8],e.pdata[9]);
    // printf(" %d %d %d %d %d ",e.pdata[10],e.pdata[11],e.pdata[12],e.pdata[13],e.pdata[14]);
    // printf(" %d %d \n",e.pdata[15],e.pdata[16]);
    ASSERT_TRUE(0 == e.pdata[0]);
    ASSERT_TRUE(0 == e.pdata[1]);
    ASSERT_TRUE(0 == e.pdata[2]);
    ASSERT_TRUE(0 == e.pdata[3]);
    ASSERT_TRUE(54 == e.pdata[4]);
    ASSERT_TRUE(57 == e.pdata[5]);
    ASSERT_TRUE(52 == e.pdata[6]);
    ASSERT_TRUE(54 == e.pdata[7]);
    ASSERT_TRUE(48 == e.pdata[8]);
    ASSERT_TRUE(48 == e.pdata[9]);
    ASSERT_TRUE(46 == e.pdata[10]);
    ASSERT_TRUE(48 == e.pdata[11]);
    ASSERT_TRUE(48 == e.pdata[12]);
    ASSERT_TRUE(48 == e.pdata[13]);
    ASSERT_TRUE(48 == e.pdata[14]);
    ASSERT_TRUE(48 == e.pdata[15]);
    ASSERT_TRUE(48 == e.pdata[16]);

    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertLevel1MeasuremenToLevel2StringEx) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
    // 32-bit float coding = 100
    ex.sizeData = 4;
    ex.data[0] = 0x80;
    ex.data[1] = 0x02;
    ex.data[2] = 0x1B;
    ex.data[3] = 0x22;

    bool rv = vscp_convertLevel1MeasuremenToLevel2StringEx( &ex ); 
    ASSERT_TRUE(rv);
    ASSERT_TRUE(ex.vscp_class == VSCP_CLASS2_MEASUREMENT_STR);
    // printf("%d\n",ex.sizeData);
    ASSERT_TRUE(17 == ex.sizeData);
    // printf(" %d %d %d %d %d ",ex.data[0],ex.data[1],ex.data[2],ex.data[3],ex.data[4]);
    // printf(" %d %d %d %d %d ",ex.data[5],ex.data[6],ex.data[7],ex.data[8],ex.data[9]);
    // printf(" %d %d %d %d %d ",ex.data[10],ex.data[11],ex.data[12],ex.data[13],ex.data[14]);
    // printf(" %d %d \n",ex.data[15],ex.data[16]);
    ASSERT_TRUE(0 == ex.data[0]);
    ASSERT_TRUE(0 == ex.data[1]);
    ASSERT_TRUE(0 == ex.data[2]);
    ASSERT_TRUE(0 == ex.data[3]);
    ASSERT_TRUE(54 == ex.data[4]);
    ASSERT_TRUE(57 == ex.data[5]);
    ASSERT_TRUE(52 == ex.data[6]);
    ASSERT_TRUE(54 == ex.data[7]);
    ASSERT_TRUE(48 == ex.data[8]);
    ASSERT_TRUE(48 == ex.data[9]);
    ASSERT_TRUE(46 == ex.data[10]);
    ASSERT_TRUE(48 == ex.data[11]);
    ASSERT_TRUE(48 == ex.data[12]);
    ASSERT_TRUE(48 == ex.data[13]);
    ASSERT_TRUE(48 == ex.data[14]);
    ASSERT_TRUE(48 == ex.data[15]);
    ASSERT_TRUE(48 == ex.data[16]);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertStringToEvent) 
{
    vscpEvent e;
    e.pdata = new uint8_t[4];

    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;

    std::string str = "0,10,6,0,2020-11-24T17:28:11Z,4216090068,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x0B,0x16,0x21,0x2C";
    bool rv = vscp_convertStringToEvent( &e, str ); 
    ASSERT_TRUE(rv);
    ASSERT_TRUE(e.vscp_class == VSCP_CLASS1_MEASUREMENT);
    ASSERT_TRUE(e.vscp_type == VSCP_TYPE_MEASUREMENT_TEMPERATURE);
    //printf("%d",ex.sizeData);
    ASSERT_TRUE(4 == e.sizeData);
    //printf("Hour: %d\n",e.hour);
    ASSERT_TRUE(2020 == e.year);
    ASSERT_TRUE(11 == e.month);
    ASSERT_TRUE(24 == e.day);
    ASSERT_TRUE(17 == e.hour);
    ASSERT_TRUE(28 == e.minute);
    ASSERT_TRUE(11 == e.second);
    ASSERT_TRUE(4216090068 == e.timestamp);
    ASSERT_TRUE(11 == e.pdata[0]);
    ASSERT_TRUE(22 == e.pdata[1]);
    ASSERT_TRUE(33 == e.pdata[2]);
    ASSERT_TRUE(44 == e.pdata[3]);

    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertStringToEventEx) 
{
    vscpEventEx ex;
    memset(&ex, 0, sizeof(vscpEventEx));

    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
    // 32-bit float coding = 100
    ex.sizeData = 4;
    ex.data[0] = 0x80;
    ex.data[1] = 0x02;
    ex.data[2] = 0x1B;
    ex.data[3] = 0x22;

    std::string str = "0,10,6,0,2020-11-24T17:28:11Z,4216090068,FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00,0x0B,0x16,0x21,0x2C";
    bool rv = vscp_convertStringToEventEx( &ex, str ); 
    ASSERT_TRUE(rv);
    ASSERT_TRUE(ex.vscp_class == VSCP_CLASS1_MEASUREMENT);
    ASSERT_TRUE(ex.vscp_type == VSCP_TYPE_MEASUREMENT_TEMPERATURE);
    //printf("%d",ex.sizeData);
    ASSERT_TRUE(4 == ex.sizeData);
    //printf("Hour: %d\n",ex.hour);
    ASSERT_TRUE(2020 == ex.year);
    ASSERT_TRUE(11 == ex.month);
    ASSERT_TRUE(24 == ex.day);
    ASSERT_TRUE(17 == ex.hour);
    ASSERT_TRUE(28 == ex.minute);
    ASSERT_TRUE(11 == ex.second);
    ASSERT_TRUE(4216090068 == ex.timestamp);
    ASSERT_TRUE(11 == ex.data[0]);
    ASSERT_TRUE(22 == ex.data[1]);
    ASSERT_TRUE(33 == ex.data[2]);
    ASSERT_TRUE(44 == ex.data[3]);
}

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
