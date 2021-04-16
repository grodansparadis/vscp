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
TEST(HelperLib, vscp_convertEventToString)
{
    vscpEvent e;
    e.pdata = new uint8_t[4];

    e.head = 0;
    e.obid = 0;
    memset(e.GUID,0,16);
    //e.timestamp = vscp_makeTimeStamp();
    e.timestamp = 87654321;
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    std::string dt = "1981-02-27T20:20:11Z";
    vscp_parseISOCombined(&tm, dt);
    vscp_setEventDateTime(&e, &tm);
    //vscp_setEventToNow(&e);

    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;

    std::string str;
    bool rv = vscp_convertEventToString(str, &e);
    ASSERT_TRUE(rv);
    //printf("%s\n",str.c_str());
    ASSERT_STREQ(str.c_str(), "0,10,6,0,1981-03-27T20:20:11Z,87654321,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x80,0x02,0x1B,0x22");

    vscp_deleteEvent(&e);    
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_convertEventExToString)
{
    vscpEventEx ex;
  
    ex.head = 0;
    ex.obid = 0;
    memset(ex.GUID,0,16);
    //ex.timestamp = vscp_makeTimeStamp();
    ex.timestamp = 87654321;
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    std::string dt = "1981-02-27T20:20:11Z";
    vscp_parseISOCombined(&tm, dt);
    vscp_setEventExDateTime(&ex, &tm);
    //vscp_setEventToNow(&ex);

    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    ex.sizeData = 4;
    ex.data[0] = 0x80;
    ex.data[1] = 0x02;
    ex.data[2] = 0x1B;
    ex.data[3] = 0x22;

    std::string str;
    bool rv = vscp_convertEventExToString(str, &ex);
    ASSERT_TRUE(rv);
    //printf("%s\n",str.c_str());
    ASSERT_STREQ(str.c_str(), "0,10,6,0,1981-03-27T20:20:11Z,87654321,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00,0x80,0x02,0x1B,0x22");
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
    e.sizeData = 4;
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
TEST(HelperLib, vscp_getDateStringFromEvent)
{
    vscpEvent e;
    e.sizeData = 4;
    e.pdata = new uint8_t[4];

    e.head = 0;
    e.obid = 0;
    memset(e.GUID,0,16);
    e.timestamp = vscp_makeTimeStamp();
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    std::string dt = "1981-02-27T20:20:11Z";
    vscp_parseISOCombined(&tm, dt);
    vscp_setEventDateTime(&e, &tm);
    //vscp_setEventToNow(&e);

    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;

    std::string dt2;
    bool rv = vscp_getDateStringFromEvent(dt2, &e);
    ASSERT_TRUE(rv);
    //printf("%s\n",dt2.c_str());
    ASSERT_STREQ(dt2.c_str(), "1981-03-27T20:20:11Z");

    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_getDateStringFromEventEx)
{
    vscpEventEx ex;
  
    ex.head = 0;
    ex.obid = 0;
    memset(ex.GUID,0,16);
    //ex.timestamp = vscp_makeTimeStamp();
    ex.timestamp = 87654321;
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    std::string dt = "1981-02-27T20:20:11Z";
    vscp_parseISOCombined(&tm, dt);
    vscp_setEventExDateTime(&ex, &tm);
    //vscp_setEventToNow(&ex);

    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    ex.sizeData = 4;
    ex.data[0] = 0x80;
    ex.data[1] = 0x02;
    ex.data[2] = 0x1B;
    ex.data[3] = 0x22;

    std::string dt2;
    bool rv = vscp_getDateStringFromEventEx(dt2, &ex);
    ASSERT_TRUE(rv);
    //printf("%s\n",dt2.c_str());
    ASSERT_STREQ(dt2.c_str(), "1981-03-27T20:20:11Z");

}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_setVscpEventFromString)
{
    vscpEvent e;
    e.sizeData = 4;
    e.pdata = new uint8_t[4];

    e.head = 0;
    e.obid = 0;
    memset(e.GUID,0,16);
    e.timestamp = vscp_makeTimeStamp();
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    std::string dt = "1981-02-27T20:20:11Z";
    vscp_parseISOCombined(&tm, dt);
    vscp_setEventDateTime(&e, &tm);
    //vscp_setEventToNow(&e);

    e.vscp_class = VSCP_CLASS1_MEASUREMENT;
    e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

    // 32-bit float coding = 100
    e.sizeData = 4;
    e.pdata[0] = 0x80;
    e.pdata[1] = 0x02;
    e.pdata[2] = 0x1B;
    e.pdata[3] = 0x22;

    std::string dt2;
    bool rv = vscp_getDateStringFromEvent(dt2, &e);
    ASSERT_TRUE(rv);
    //printf("%s\n",dt2.c_str());
    ASSERT_STREQ(dt2.c_str(), "1981-03-27T20:20:11Z");

    vscp_deleteEvent(&e);
}

//-----------------------------------------------------------------------------
TEST(HelperLib, vscp_hexStr2ByteArray) 
{ 
    const uint8_t orig_key[256] = {
        0x2d, 0xbb, 0x07, 0x9a, 0x38, 0x98, 0x5a, 0xf0, 0x0e, 0xbe, 0xef, 0xe2, 0x2f, 0x9f, 0xfa, 0x0e,
        0x7f, 0x72, 0xdf, 0x06, 0xeb, 0xe4, 0x45, 0x63, 0xed, 0xf4, 0xa1, 0x07, 0x3c, 0xab, 0xc7, 0xd4,
        0x4f, 0xb0, 0xee, 0xc1, 0x27, 0x1c, 0x7d, 0x75, 0x31, 0x61, 0x54, 0xf2, 0xc6, 0xff, 0x80, 0xb8,
        0x62, 0x7b, 0x27, 0xd9, 0xa5, 0xc1, 0xc6, 0xe8, 0x8e, 0x1c, 0xb4, 0xe8, 0xd7, 0xee, 0x4b, 0x71,
        0x1f, 0x51, 0x2b, 0x5b, 0x9e, 0x23, 0xb7, 0xee, 0xb6, 0x60, 0xd1, 0x33, 0xac, 0x32, 0x01, 0xd6,
        0x58, 0x1c, 0xb0, 0x63, 0x9a, 0x93, 0x82, 0x17, 0x11, 0x75, 0xcb, 0x14, 0xdc, 0x82, 0x8c, 0x98,
        0x28, 0x2b, 0xed, 0x75, 0x0a, 0x80, 0x59, 0xe5, 0xcf, 0x86, 0x4b, 0xb5, 0x53, 0x81, 0xaa, 0x9d,
        0xa9, 0x50, 0xb9, 0xe3, 0xce, 0x82, 0x85, 0xe1, 0xea, 0x38, 0xd2, 0x1a, 0xfc, 0x9a, 0x4a, 0xe5,
        0xca, 0x5f, 0x37, 0x52, 0x08, 0xf7, 0x2e, 0x5b, 0x91, 0x13, 0xea, 0x3f, 0x75, 0x70, 0x41, 0x5e,
        0x6e, 0xa0, 0xa6, 0x37, 0xdb, 0x18, 0x48, 0xb3, 0x84, 0x85, 0xb7, 0x10, 0x3e, 0xae, 0x80, 0x53,
        0xda, 0x6a, 0xba, 0xd0, 0x2d, 0x88, 0x20, 0x92, 0x90, 0x21, 0xeb, 0x64, 0x50, 0x3a, 0xc6, 0xfe,
        0x6a, 0x38, 0xf1, 0x40, 0x45, 0xa2, 0x16, 0x44, 0x12, 0xfa, 0xc1, 0x3e, 0x97, 0x07, 0xeb, 0x64,
        0x07, 0xb1, 0xfb, 0x01, 0xfa, 0x77, 0x1d, 0x07, 0xb5, 0x8d, 0x5c, 0x3c, 0x1d, 0x4a, 0x58, 0x4e,
        0x3e, 0xc1, 0xae, 0xbc, 0x44, 0x9e, 0xc1, 0xcb, 0xaf, 0xc0, 0x6c, 0xea, 0xed, 0xa9, 0x75, 0xf7,
        0xc9, 0x36, 0x41, 0x9a, 0x90, 0xc3, 0xf3, 0xbc, 0xc9, 0xf4, 0x41, 0x9e, 0x35, 0xe0, 0x8c, 0x50,
        0xda, 0x92, 0x80, 0xc0, 0x97, 0xd0, 0x7e, 0x7e, 0x77, 0xd5, 0x72, 0x0e, 0xfa, 0x46, 0xd3, 0x2f
    };

    const char *hexstr = 
        "2DBB079A38985AF00EBEEFE22F9FFA0E"
        "7F72DF06EBE44563EDF4A1073CABC7D4"
        "4FB0EEC1271C7D75316154F2C6FF80B8"
        "627B27D9A5C1C6E88E1CB4E8D7EE4B71"
        "1F512B5B9E23B7EEB660D133AC3201D6"
        "581CB0639A9382171175CB14DC828C98"
        "282BED750A8059E5CF864BB55381AA9D"
        "A950B9E3CE8285E1EA38D21AFC9A4AE5"
        "CA5F375208F72E5B9113EA3F7570415E"
        "6EA0A637DB1848B38485B7103EAE8053"
        "DA6ABAD02D8820929021EB64503AC6FE"
        "6A38F14045A2164412FAC13E9707EB64"
        "07B1FB01FA771D07B58D5C3C1D4A584E"
        "3EC1AEBC449EC1CBAFC06CEAEDA975F7"
        "C936419A90C3F3BCC9F4419E35E08C50"
        "DA9280C097D07E7E77D5720EFA46D32F";

    uint8_t workKey[256];
    ASSERT_EQ(256, (int)vscp_hexStr2ByteArray(workKey, 256, hexstr));

    for (int i=0; i<256; i++) {
        ASSERT_EQ(workKey[i], orig_key[i]);
    };
}


//-----------------------------------------------------------------------------
// With a hex string that is to long
TEST(HelperLib, vscp_hexStr2ByteArray2) 
{ 
    const uint8_t orig_key[256] = {
        0x2d, 0xbb, 0x07, 0x9a, 0x38, 0x98, 0x5a, 0xf0, 0x0e, 0xbe, 0xef, 0xe2, 0x2f, 0x9f, 0xfa, 0x0e,
        0x7f, 0x72, 0xdf, 0x06, 0xeb, 0xe4, 0x45, 0x63, 0xed, 0xf4, 0xa1, 0x07, 0x3c, 0xab, 0xc7, 0xd4,
        0x4f, 0xb0, 0xee, 0xc1, 0x27, 0x1c, 0x7d, 0x75, 0x31, 0x61, 0x54, 0xf2, 0xc6, 0xff, 0x80, 0xb8,
        0x62, 0x7b, 0x27, 0xd9, 0xa5, 0xc1, 0xc6, 0xe8, 0x8e, 0x1c, 0xb4, 0xe8, 0xd7, 0xee, 0x4b, 0x71,
        0x1f, 0x51, 0x2b, 0x5b, 0x9e, 0x23, 0xb7, 0xee, 0xb6, 0x60, 0xd1, 0x33, 0xac, 0x32, 0x01, 0xd6,
        0x58, 0x1c, 0xb0, 0x63, 0x9a, 0x93, 0x82, 0x17, 0x11, 0x75, 0xcb, 0x14, 0xdc, 0x82, 0x8c, 0x98,
        0x28, 0x2b, 0xed, 0x75, 0x0a, 0x80, 0x59, 0xe5, 0xcf, 0x86, 0x4b, 0xb5, 0x53, 0x81, 0xaa, 0x9d,
        0xa9, 0x50, 0xb9, 0xe3, 0xce, 0x82, 0x85, 0xe1, 0xea, 0x38, 0xd2, 0x1a, 0xfc, 0x9a, 0x4a, 0xe5,
        0xca, 0x5f, 0x37, 0x52, 0x08, 0xf7, 0x2e, 0x5b, 0x91, 0x13, 0xea, 0x3f, 0x75, 0x70, 0x41, 0x5e,
        0x6e, 0xa0, 0xa6, 0x37, 0xdb, 0x18, 0x48, 0xb3, 0x84, 0x85, 0xb7, 0x10, 0x3e, 0xae, 0x80, 0x53,
        0xda, 0x6a, 0xba, 0xd0, 0x2d, 0x88, 0x20, 0x92, 0x90, 0x21, 0xeb, 0x64, 0x50, 0x3a, 0xc6, 0xfe,
        0x6a, 0x38, 0xf1, 0x40, 0x45, 0xa2, 0x16, 0x44, 0x12, 0xfa, 0xc1, 0x3e, 0x97, 0x07, 0xeb, 0x64,
        0x07, 0xb1, 0xfb, 0x01, 0xfa, 0x77, 0x1d, 0x07, 0xb5, 0x8d, 0x5c, 0x3c, 0x1d, 0x4a, 0x58, 0x4e,
        0x3e, 0xc1, 0xae, 0xbc, 0x44, 0x9e, 0xc1, 0xcb, 0xaf, 0xc0, 0x6c, 0xea, 0xed, 0xa9, 0x75, 0xf7,
        0xc9, 0x36, 0x41, 0x9a, 0x90, 0xc3, 0xf3, 0xbc, 0xc9, 0xf4, 0x41, 0x9e, 0x35, 0xe0, 0x8c, 0x50,
        0xda, 0x92, 0x80, 0xc0, 0x97, 0xd0, 0x7e, 0x7e, 0x77, 0xd5, 0x72, 0x0e, 0xfa, 0x46, 0xd3, 0x2f
    };

    const char *hexstr = 
        "2DBB079A38985AF00EBEEFE22F9FFA0E"
        "7F72DF06EBE44563EDF4A1073CABC7D4"
        "4FB0EEC1271C7D75316154F2C6FF80B8"
        "627B27D9A5C1C6E88E1CB4E8D7EE4B71"
        "1F512B5B9E23B7EEB660D133AC3201D6"
        "581CB0639A9382171175CB14DC828C98"
        "282BED750A8059E5CF864BB55381AA9D"
        "A950B9E3CE8285E1EA38D21AFC9A4AE5"
        "CA5F375208F72E5B9113EA3F7570415E"
        "6EA0A637DB1848B38485B7103EAE8053"
        "DA6ABAD02D8820929021EB64503AC6FE"
        "6A38F14045A2164412FAC13E9707EB64"
        "07B1FB01FA771D07B58D5C3C1D4A584E"
        "3EC1AEBC449EC1CBAFC06CEAEDA975F7"
        "C936419A90C3F3BCC9F4419E35E08C50"
        "DA9280C097D07E7E77D5720EFA46D32F01020304050607";

    uint8_t workKey[256];
    ASSERT_EQ(256, (int)vscp_hexStr2ByteArray(workKey, 256, hexstr));

    for (int i=0; i<256; i++) {
        ASSERT_EQ(workKey[i], orig_key[i]);
    };
}

//-----------------------------------------------------------------------------
// With a hex string that is to short
TEST(HelperLib, vscp_hexStr2ByteArray3) 
{ 
    const uint8_t orig_key[256] = {
        0x2d, 0xbb, 0x07, 0x9a, 0x38, 0x98, 0x5a, 0xf0, 0x0e, 0xbe, 0xef, 0xe2, 0x2f, 0x9f, 0xfa, 0x0e,
        0x7f, 0x72, 0xdf, 0x06, 0xeb, 0xe4, 0x45, 0x63, 0xed, 0xf4, 0xa1, 0x07, 0x3c, 0xab, 0xc7, 0xd4,
        0x4f, 0xb0, 0xee, 0xc1, 0x27, 0x1c, 0x7d, 0x75, 0x31, 0x61, 0x54, 0xf2, 0xc6, 0xff, 0x80, 0xb8,
        0x62, 0x7b, 0x27, 0xd9, 0xa5, 0xc1, 0xc6, 0xe8, 0x8e, 0x1c, 0xb4, 0xe8, 0xd7, 0xee, 0x4b, 0x71,
        0x1f, 0x51, 0x2b, 0x5b, 0x9e, 0x23, 0xb7, 0xee, 0xb6, 0x60, 0xd1, 0x33, 0xac, 0x32, 0x01, 0xd6,
        0x58, 0x1c, 0xb0, 0x63, 0x9a, 0x93, 0x82, 0x17, 0x11, 0x75, 0xcb, 0x14, 0xdc, 0x82, 0x8c, 0x98,
        0x28, 0x2b, 0xed, 0x75, 0x0a, 0x80, 0x59, 0xe5, 0xcf, 0x86, 0x4b, 0xb5, 0x53, 0x81, 0xaa, 0x9d,
        0xa9, 0x50, 0xb9, 0xe3, 0xce, 0x82, 0x85, 0xe1, 0xea, 0x38, 0xd2, 0x1a, 0xfc, 0x9a, 0x4a, 0xe5,
        0xca, 0x5f, 0x37, 0x52, 0x08, 0xf7, 0x2e, 0x5b, 0x91, 0x13, 0xea, 0x3f, 0x75, 0x70, 0x41, 0x5e,
        0x6e, 0xa0, 0xa6, 0x37, 0xdb, 0x18, 0x48, 0xb3, 0x84, 0x85, 0xb7, 0x10, 0x3e, 0xae, 0x80, 0x53,
        0xda, 0x6a, 0xba, 0xd0, 0x2d, 0x88, 0x20, 0x92, 0x90, 0x21, 0xeb, 0x64, 0x50, 0x3a, 0xc6, 0xfe,
        0x6a, 0x38, 0xf1, 0x40, 0x45, 0xa2, 0x16, 0x44, 0x12, 0xfa, 0xc1, 0x3e, 0x97, 0x07, 0xeb, 0x64,
        0x07, 0xb1, 0xfb, 0x01, 0xfa, 0x77, 0x1d, 0x07, 0xb5, 0x8d, 0x5c, 0x3c, 0x1d, 0x4a, 0x58, 0x4e,
        0x3e, 0xc1, 0xae, 0xbc, 0x44, 0x9e, 0xc1, 0xcb, 0xaf, 0xc0, 0x6c, 0xea, 0xed, 0xa9, 0x75, 0xf7,
        0xc9, 0x36, 0x41, 0x9a, 0x90, 0xc3, 0xf3, 0xbc, 0xc9, 0xf4, 0x41, 0x9e, 0x35, 0xe0, 0x8c, 0x50,
        0xda, 0x92, 0x80, 0xc0, 0x97, 0xd0, 0x7e, 0x7e, 0x77, 0xd5, 0x72, 0x0e, 0xfa, 0x46, 0xd3, 0x2f
    };

    const char *hexstr = 
        "2DBB079A38985AF00EBEEFE22F9FFA0E"
        "7F72DF06EBE44563EDF4A1073CABC7D4"
        "4FB0EEC1271C7D75316154F2C6FF80B8"
        "627B27D9A5C1C6E88E1CB4E8D7EE4B71"
        "1F512B5B9E23B7EEB660D133AC3201D6"
        "581CB0639A9382171175CB14DC828C98"
        "282BED750A8059E5CF864BB55381AA9D"
        "A950B9E3CE8285E1EA38D21AFC9A4AE5"
        "CA5F375208F72E5B9113EA3F7570415E"
        "6EA0A637DB1848B38485B7103EAE8053"
        "DA6ABAD02D8820929021EB64503AC6FE"
        "6A38F14045A2164412FAC13E9707EB64"
        "07B1FB01FA771D07B58D5C3C1D4A584E"
        "3EC1AEBC449EC1CBAFC06CEAEDA975F7"
        "C936419A90C3F3BCC9F4419E35E08C50"
        "DA9280C097D07E7E77D5720EFA46D3";

    uint8_t workKey[256];
    ASSERT_NE(256, (int)vscp_hexStr2ByteArray(workKey, 256, hexstr));

    for (int i=0; i<255; i++) {
        ASSERT_EQ(workKey[i], orig_key[i]);
    };
}
    

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
