
#include <vscphelper.h>
 
int
main(void)
{
    int i,j,k;          // General iterators  
    uint32_t sum;       // General sum
    uint8_t guid[16];   // General GUID
    vscpEvent e;        // General event
    e.pdata = NULL;
    e.sizeData = 0;
    vscpEventEx ex;     // General eventex
    ex.sizeData = 0;

    // ------------------------------------------------------------------------
    // Testing vscp_readStringValue
    // ------------------------------------------------------------------------
    printf(" * Testing vscp2_readStringValue\n");

	std::string test = "54321";
    int32_t t1 =  vscp2_readStringValue( test );
    if ( t1 != 54321 ) {
        printf("[vscp2_readStringValue] Value is not 102!\n");
        exit( -1 );
    }

    int32_t t2 =  vscp2_readStringValue( std::string("102") );
    if ( t2 != 102 ) {
        printf("[vscp2_readStringValue] Value is not 102!\n");
        exit( -1 );
    }

    int32_t t3 =  vscp2_readStringValue( "12345" );
    if ( t3 != 12345 ) {
        printf("[vscp2_readStringValue] Value is not 12345!\n");
        exit( -1 );
    }

    int32_t t4 =  vscp2_readStringValue( "0xffff" );
    if ( t4 != 65535 ) {
        printf("[vscp2_readStringValue] Value is not 65535!\n");
        exit( -1 );
    }

    int32_t t5 =  vscp2_readStringValue( "0o77" );
    if ( t5 != 63 ) {
        printf("[vscp2_readStringValue] Value is not 63!\n");
        exit( -1 );
    }

    int32_t t6 =  vscp2_readStringValue( "0b1010" );
    if ( t6 != 10 ) {
        printf("[vscp2_readStringValue] Value is not 10!\n");
        exit( -1 );
    }

    int32_t t7 =  vscp2_readStringValue( "0b189" );
    if ( t7 != 1 ) {
        printf("[vscp2_readStringValue] Value is not 1!\n");
        exit( -1 );
    }

    int32_t t8 =  vscp2_readStringValue( "4294967295" );
    if ( t8 != ULONG_MAX ) {
        printf("[vscp2_readStringValue] Value is not 4294967295 (ULONG_MAX)!\n");
        exit( -1 );
    }
    
    // ------------------------------------------------------------------------
    // Testing vscp_getGuidFromStringToArray
    // ------------------------------------------------------------------------
    printf(" * Testing vscp2_getGuidFromStringToArray\n");

    memset( guid, 0, sizeof(guid) );
    
    std::string strGUID1 = "00:01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F";
    if ( !vscp2_getGuidFromStringToArray( guid, strGUID1 ) ) {
        printf("[vscp2_getGuidFromStringToArray] Could not read GUID from string!\n");
        exit( -1 );
    }

    sum = 0;
    for ( i=0; i<16; i++ ) {
        sum += guid[i];
    }
    if ( 120 != sum ) {
        printf("[vscp2_getGuidFromStringToArray] GUID read wrongly!\n");
        exit( -1 );
    }

    // ------------------------------------------------------------------------
    // Testing vscp2_readFilterFromString
    // ------------------------------------------------------------------------
    printf(" * Testing vscp2_readFilterFromString\n");

    vscpEventFilter filter;
    std::string strfilter = "3,0x0201,0x0006,ff:ff:ff:ff:ff:ff:fe:01:00:00:00:00:01:00:00:20";
    if ( !vscp2_readFilterFromString( &filter, strfilter ) ) {
        printf("[vscp2_readFilterFromString] Failed!\n");
        exit( -1 );                                
    }

    if ( ( filter.filter_priority != 3 ) ||
         ( filter.filter_class != 513 ) ||
         ( filter.filter_type != 6 ) ) {
        printf("[vscp2_readFilterFromString] Did not read in string data correctly!\n");
        exit( -1 );
    }

    if ( ( filter.filter_GUID[0] != 255 ) ||
         ( filter.filter_GUID[1] != 255 ) ||
         ( filter.filter_GUID[2] != 255 ) ||
         ( filter.filter_GUID[3] != 255 ) ||
         ( filter.filter_GUID[4] != 255 ) ||
         ( filter.filter_GUID[5] != 255 ) ||
         ( filter.filter_GUID[6] != 254 ) ||
         ( filter.filter_GUID[7] != 1 ) ||
         ( filter.filter_GUID[8] != 0 ) ||
         ( filter.filter_GUID[9] != 0 ) ||
         ( filter.filter_GUID[10] != 0 ) ||
         ( filter.filter_GUID[11] != 0 ) ||
         ( filter.filter_GUID[12] != 1 ) ||
         ( filter.filter_GUID[13] != 0 ) ||
         ( filter.filter_GUID[14] != 0 ) ||
         ( filter.filter_GUID[15] != 32 ) ) {
        printf("[vscp2_readFilterFromString] Did not read GUID correctly!\n");
        exit( -1 );
    }

    // ------------------------------------------------------------------------
    // Testing vscp2_readMaskFromString
    // ------------------------------------------------------------------------
    printf(" * Testing vscp2_readMaskFromString\n");

    std::string strmask = "7,0x0101,0x0076,ff:ff:ff:33:ff:ff:fe:01:00:00:00:00:01:00:00:20";
    if ( !vscp2_readMaskFromString( &filter, strmask ) ) {
        printf("[vscp2_readMaskFromString] Failed!\n");
        exit( -1 );                                
    }

    if ( ( filter.mask_priority != 7 ) ||
         ( filter.mask_class != 257 ) ||
         ( filter.mask_type != 118 ) ) {
        printf("[vscp2_readMaskFromString] Did not read in string data correctly!\n");
        exit( -1 );
    }

    if ( ( filter.mask_GUID[0] != 255 ) ||
         ( filter.mask_GUID[1] != 255 ) ||
         ( filter.mask_GUID[2] != 255 ) ||
         ( filter.mask_GUID[3] != 51 ) ||
         ( filter.mask_GUID[4] != 255 ) ||
         ( filter.mask_GUID[5] != 255 ) ||
         ( filter.mask_GUID[6] != 254 ) ||
         ( filter.mask_GUID[7] != 1 ) ||
         ( filter.mask_GUID[8] != 0 ) ||
         ( filter.mask_GUID[9] != 0 ) ||
         ( filter.mask_GUID[10] != 0 ) ||
         ( filter.mask_GUID[11] != 0 ) ||
         ( filter.mask_GUID[12] != 1 ) ||
         ( filter.mask_GUID[13] != 0 ) ||
         ( filter.mask_GUID[14] != 0 ) ||
         ( filter.mask_GUID[15] != 32 ) ) {
        printf("[vscp2_readMaskFromString] Did not read GUID correctly!\n");
        exit( -1 );
    }

    // ------------------------------------------------------------------------
    // Testing vscp2_setVscpEventDataFromString
    // ------------------------------------------------------------------------
    printf(" * Testing vscp2_setVscpEventDataFromString\n");

    vscpEvent *pEvent = &e;
    pEvent->sizeData = 0;
    pEvent->pdata = NULL;
    std::string strData = "1,2,30,4,66";
    if ( !vscp2_setVscpEventDataFromString( pEvent, strData ) ) {
        printf("[vscp2_setVscpEventDataFromString] Failed!\n");
        exit( -1 );
    }

    if ( ( pEvent->sizeData != 5 ) ||
            ( pEvent->pdata == NULL ) || 
            ( pEvent->pdata[0] != 1 ) ||
            ( pEvent->pdata[1] != 2 ) ||
            ( pEvent->pdata[2] != 30 ) ||
            ( pEvent->pdata[3] != 4 ) ||
            ( pEvent->pdata[4] != 66 ) ) {
        printf("[vscp2_setVscpEventDataFromString] Did not read GUID correctly!\n");
        exit( -1 );
    }

    delete pEvent->pdata;
    pEvent->sizeData = 0;
    pEvent->pdata = NULL;

    // ------------------------------------------------------------------------
    // Testing vscp2_setVscpEventExDataFromString
    // ------------------------------------------------------------------------
    printf(" * Testing vscp2_setVscpEventExDataFromString\n");

    ex.sizeData = 0;
    
    strData = "1,2,30,4,66";
    if ( !vscp2_setVscpEventExDataFromString( &ex, strData ) ) {
        printf("[vscp2_setVscpEventExDataFromString] Failed!\n");
        exit( -1 );
    }

    if ( ( ex.sizeData != 5 ) ||
            ( ex.data[0] != 1 ) ||
            ( ex.data[1] != 2 ) ||
            ( ex.data[2] != 30 ) ||
            ( ex.data[3] != 4 ) ||
            ( ex.data[4] != 66 ) ) {
        printf("[vscp2_setVscpEventExDataFromString] Did not read GUID correctly!\n");
        exit( -1 );
    }

    ex.sizeData = 0;

    // ------------------------------------------------------------------------
    // Testing vscp2_base64_decode
    // ------------------------------------------------------------------------
    printf(" * Testing vscp2_base64_decode\n");
    std::string base64val("VGhpcyBpcyBhIHNpbXBsZSB0ZXN0");
    if ( !vscp2_base64_decode( base64val ) ) {
        printf("[vscp2_base64_decode] Failed to decode!\n");
        exit( -1 );
    }

    if ( 0 != strcmp( "This is a simple test", base64val.c_str() ) ) {
        printf("[vscp2_base64_decode] Decoding value not correct!\n");
        exit( -1 );
    }

    // ------------------------------------------------------------------------
    // Testing vscp2_base64_encode
    // ------------------------------------------------------------------------
    printf(" * Testing vscp2_base64_encode\n");
    std::string strBase64Encode = "This is a simple test";
    if ( !vscp2_base64_encode( strBase64Encode ) ) {
        printf("[vscp2_base64_encode] Failed to encode!\n");
        exit( -1 );
    }

    if ( 0 != strcmp( strBase64Encode.c_str(), "VGhpcyBpcyBhIHNpbXBsZSB0ZXN0" ) ) {
        printf("[vscp2_base64_decode] Encoding value not correct!\n");
        exit( -1 );
    }

    // ------------------------------------------------------------------------
    // Testing vscp2_decodeBase64IfNeeded
    // ------------------------------------------------------------------------
    printf(" * Testing vscp2_decodeBase64IfNeeded\n");
    std::string strResult;
    base64val = "VGhpcyBpcyBhIHNpbXBsZSB0ZXN0";
    if ( !vscp2_decodeBase64IfNeeded( base64val, strResult ) ) {
        printf("[vscp2_decodeBase64IfNeeded] Failed!\n");
        exit( -1 );
    }

    if ( 0 != strcmp( strResult.c_str(), "VGhpcyBpcyBhIHNpbXBsZSB0ZXN0" ) ) {
        printf("[vscp2_decodeBase64IfNeeded] Encoding value not correct!\n");
        exit( -1 );
    }

    base64val = "BASE64:VGhpcyBpcyBhIHNpbXBsZSB0ZXN0";
    if ( !vscp2_decodeBase64IfNeeded( base64val, strResult ) ) {
        printf("[vscp2_decodeBase64IfNeeded] Failed!\n");
        exit( -1 );
    }

    if ( 0 != strcmp( strResult.c_str(), "This is a simple test" ) ) {
        printf("[vscp2_decodeBase64IfNeeded] Encoding value not correct!\n");
        exit( -1 );
    }

    return 0;
}