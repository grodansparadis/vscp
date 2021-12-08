// unit_test for mdfparser

#include <gtest/gtest.h>
#include <math.h>
#include <mdf.h>
#include <stdio.h>
#include <stdlib.h>

#include <vscphelper.h>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <string>

//-----------------------------------------------------------------------------

TEST(parseMDF, Invalid_Path)
{
  CMDF mdf;

  std::string path = "xml/non.existent.xml";
  ASSERT_EQ(VSCP_ERROR_INVALID_PATH, mdf.parseMDF(path));
}

//-----------------------------------------------------------------------------

TEST(parseMDF, Invalid_Tag)
{
  CMDF mdf;

  std::string path = "xml/invalid-tag.xml";
  ASSERT_EQ(VSCP_ERROR_PARSING, mdf.parseMDF(path));
}

//-----------------------------------------------------------------------------

TEST(parseMDF, Simple_XML_A)
{
  CMDF mdf;

  std::string path = "xml/simpleA.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  // Check # descriptions
  ASSERT_EQ(8, mdf.getModuleDescriptionSize());

  // Check # info URL's
  ASSERT_EQ(8, mdf.getModuleHelpUrlCount());

  // Check name
  ASSERT_TRUE(mdf.getModuleName() == "Simple test");

  // Check description
  ASSERT_TRUE(mdf.getModuleDescription("en") == "This is an english description");
  ASSERT_TRUE(mdf.getModuleDescription("es") == "Esta es una descripción en inglés");
  ASSERT_TRUE(mdf.getModuleDescription("pt") == "Esta é uma descrição em inglês");
  ASSERT_TRUE(mdf.getModuleDescription("zh") == "这是英文说明");
  ASSERT_TRUE(mdf.getModuleDescription("se") == "Det här är en engelsk beskrivning");
  ASSERT_TRUE(mdf.getModuleDescription("lt") == "Tai yra angliškas aprašymas");
  ASSERT_TRUE(mdf.getModuleDescription("de") == "Dies ist eine englische Beschreibung");
  ASSERT_TRUE(mdf.getModuleDescription("eo") == "Ĉi tio estas angla priskribo");

  // Check info URL
  ASSERT_TRUE(mdf.getModuleHelpUrl("en") == "https://www.english.en");
  ASSERT_TRUE(mdf.getModuleHelpUrl("es") == "https://www.spanish.es");
  ASSERT_TRUE(mdf.getModuleHelpUrl("pt") == "https://www.portuguese.pt");
  ASSERT_TRUE(mdf.getModuleHelpUrl("zh") == "https://www.chineese.zh");
  ASSERT_TRUE(mdf.getModuleHelpUrl("se") == "https://www.swedish.se");
  ASSERT_TRUE(mdf.getModuleHelpUrl("lt") == "https://www.lithuanian.lt");
  ASSERT_TRUE(mdf.getModuleHelpUrl("de") == "https://www.german.de");
  ASSERT_TRUE(mdf.getModuleHelpUrl("eo") == "https://www.esperanto.eo");

  ASSERT_EQ(8, mdf.getModuleBufferSize());
}

//-----------------------------------------------------------------------------

TEST(parseMDF, Simple_XML_B)
{
  CMDF mdf;

  std::string path = "xml/simpleB.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  // Check # descriptions
  ASSERT_EQ(1, mdf.getModuleDescriptionSize());

  // Check # info URL's
  ASSERT_EQ(1, mdf.getModuleHelpUrlCount());

  // Check name
  ASSERT_TRUE(mdf.getModuleName() == "Simple B test");

  // Check description
  ASSERT_TRUE(mdf.getModuleDescription("en") == "This is an english BBB description");

  // Check description
  ASSERT_TRUE(mdf.getModuleHelpUrl("en") == "http://www.grodansparadis.com/kelvin1w/index.html");

  ASSERT_EQ(128, mdf.getModuleBufferSize());

  // Check Manufacturer name
  ASSERT_TRUE(mdf.getManufacturerName() == "Grodans Paradis AB");
}

//-----------------------------------------------------------------------------

TEST(parseMDF, Simple_Picture_Old_Format)
{
  CMDF mdf;
  CMDF_Picture *pPicture;

  std::string path = "xml/simple_picture_old_format.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  ASSERT_EQ(2, mdf.getPictureCount());

  uint16_t index = 0;
  ASSERT_TRUE(nullptr != mdf.getPictureObj());
  ASSERT_TRUE(nullptr != mdf.getPictureObj(0));
  ASSERT_TRUE(nullptr != mdf.getPictureObj(1));
  ASSERT_TRUE(nullptr == mdf.getPictureObj(2));
  ASSERT_TRUE(mdf.getPictureObj() == mdf.getPictureObj(0));

  // Get first picture url
  pPicture = mdf.getPictureObj();
  ASSERT_TRUE("http://www.somewhere.com/images/pict1.jpg" == pPicture->getUrl());

  ASSERT_TRUE("jpg" == pPicture->getFormat());

  ASSERT_TRUE("This is a picture description in English" == pPicture->getDescription("en"));
  ASSERT_TRUE("Det här är en svensk beskrivning av bild 1" == pPicture->getDescription("se"));

  // Get first picture url again
  pPicture = mdf.getPictureObj(0);
  ASSERT_TRUE("http://www.somewhere.com/images/pict1.jpg" == pPicture->getUrl());

  ASSERT_TRUE("jpg" == pPicture->getFormat());

  ASSERT_TRUE("This is a picture description in English" == pPicture->getDescription("en"));
  ASSERT_TRUE("Det här är en svensk beskrivning av bild 1" == pPicture->getDescription("se"));

  // Get second picture url
  pPicture = mdf.getPictureObj(1);
  ASSERT_TRUE("http://www.somewhere.com/images/pict2.png" == pPicture->getUrl());

  ASSERT_TRUE("png" == pPicture->getFormat());

  ASSERT_TRUE("This is a picture 2 description in English" == pPicture->getDescription("en"));
  ASSERT_TRUE("Det här är en svensk beskrivning av bild 2" == pPicture->getDescription("se"));
}

//-----------------------------------------------------------------------------

TEST(parseMDF, Simple_Picture_Standard_Format)
{
  CMDF mdf;
  CMDF_Picture *pPicture;

  std::string path = "xml/simple_picture_standard_format.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  ASSERT_EQ(2, mdf.getPictureCount());

  uint16_t index = 0;
  ASSERT_TRUE(nullptr != mdf.getPictureObj());
  ASSERT_TRUE(nullptr != mdf.getPictureObj(0));
  ASSERT_TRUE(nullptr != mdf.getPictureObj(1));
  ASSERT_TRUE(nullptr == mdf.getPictureObj(2));
  ASSERT_TRUE(mdf.getPictureObj() == mdf.getPictureObj(0));

  // Get first picture url
  pPicture = mdf.getPictureObj();
  ASSERT_TRUE("http://www.somewhere.com/images/stdpict1.jpg" == pPicture->getUrl());

  ASSERT_TRUE("jpg" == pPicture->getFormat());

  ASSERT_TRUE("This is a picture description in English" == pPicture->getDescription("en"));
  ASSERT_TRUE("Det här är en svensk beskrivning av bild 1" == pPicture->getDescription("se"));

  // Get first picture url again
  pPicture = mdf.getPictureObj(0);
  ASSERT_TRUE("http://www.somewhere.com/images/stdpict1.jpg" == pPicture->getUrl());

  ASSERT_TRUE("jpg" == pPicture->getFormat());

  ASSERT_TRUE("This is a picture description in English" == pPicture->getDescription("en"));
  ASSERT_TRUE("Det här är en svensk beskrivning av bild 1" == pPicture->getDescription("se"));

  // Get second picture url
  pPicture = mdf.getPictureObj(1);
  ASSERT_TRUE("http://www.somewhere.com/images/stdpict2.png" == pPicture->getUrl());

  ASSERT_TRUE("png" == pPicture->getFormat());

  ASSERT_TRUE("This is a picture 2 description in English" == pPicture->getDescription("en"));
  ASSERT_TRUE("Det här är en svensk beskrivning av bild 2" == pPicture->getDescription("se"));
}

//-----------------------------------------------------------------------------

TEST(parseMDF, Simple_Firmware_Standard_Format)
{
  CMDF mdf;
  CMDF_Firmware *pFirmware;

  std::string path = "xml/simple_firmware_standard_format.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  ASSERT_EQ(6, mdf.getFirmwareCount());

  ASSERT_TRUE(nullptr != mdf.getFirmwareObj());
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(0));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(1));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(2));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(3));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(4));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(5));
  ASSERT_TRUE(nullptr == mdf.getFirmwareObj(6));
  ASSERT_TRUE(mdf.getFirmwareObj() == mdf.getFirmwareObj(0));

  // Get first firmare url
  pFirmware = mdf.getFirmwareObj();
  ASSERT_TRUE("pic18f2580" == pFirmware->getTarget());
  ASSERT_TRUE("https://github.com/grodansparadis/can4vscp_paris/releases/download/v1.1.6/"
              "paris_relay_pic18f2580_1_1_6_relocated.hex" == pFirmware->getUrl());
  ASSERT_TRUE(11 == pFirmware->getTargetCode());
  ASSERT_TRUE("intelhex8" == pFirmware->getFormat());
  ASSERT_TRUE("2020-05-15" == pFirmware->getDate());
  ASSERT_TRUE(8192 == pFirmware->getSize());
  ASSERT_TRUE(1 == pFirmware->getVersionMajor());
  ASSERT_TRUE(1 == pFirmware->getVersionMinor());
  ASSERT_TRUE(6 == pFirmware->getVersionPatch());
  ASSERT_TRUE("0x595f44fec1e92a71d3e9e77456ba80d1" == pFirmware->getMd5());

  ASSERT_TRUE("Firmware for the CAN4VSCP Paris relay module with PIC18f2580 processor." ==
              pFirmware->getDescription("en"));
  ASSERT_TRUE("Firmware för CAN4VSCP Paris relay modul med PIC18f2580 processor." == pFirmware->getDescription("se"));

  // Get first firmare url
  pFirmware = mdf.getFirmwareObj(3);
  ASSERT_TRUE("esp32c3" == pFirmware->getTarget());
  ASSERT_TRUE("https://github.com/grodansparadis/can4vscp_paris/releases/download/v1.1.1/paris_relay_1_1_1.hex" ==
              pFirmware->getUrl());
  ASSERT_TRUE(44 == pFirmware->getTargetCode());
  ASSERT_TRUE("intelhex16" == pFirmware->getFormat());
  ASSERT_TRUE("2021-11-02" == pFirmware->getDate());
  ASSERT_TRUE(0 == pFirmware->getSize());
  ASSERT_TRUE(99 == pFirmware->getVersionMajor());
  ASSERT_TRUE(8 == pFirmware->getVersionMinor());
  ASSERT_TRUE(17 == pFirmware->getVersionPatch());
  ASSERT_TRUE("595f44fec1e92a71d3e9e77456ba80d1" == pFirmware->getMd5());

  ASSERT_TRUE("Description in English." == pFirmware->getDescription("en"));
  ASSERT_TRUE("Beskrivning på Svenska." == pFirmware->getDescription("se"));
}

//-----------------------------------------------------------------------------

TEST(parseMDF, Simple_Firmware_Old_Format)
{
  CMDF mdf;
  CMDF_Firmware *pFirmware;

  std::string path = "xml/simple_firmware_old_format.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  ASSERT_EQ(6, mdf.getFirmwareCount());

  ASSERT_TRUE(nullptr != mdf.getFirmwareObj());
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(0));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(1));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(2));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(3));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(4));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(5));
  ASSERT_TRUE(nullptr == mdf.getFirmwareObj(6));
  ASSERT_TRUE(mdf.getFirmwareObj() == mdf.getFirmwareObj(0));

  // Get first firmare url
  pFirmware = mdf.getFirmwareObj(0);
  ASSERT_TRUE("pic18f2580" == pFirmware->getTarget());
  ASSERT_TRUE("https://github.com/grodansparadis/can4vscp_paris/releases/download/v1.1.6/"
              "paris_relay_pic18f2580_1_1_6_relocated.hex" == pFirmware->getUrl());
  ASSERT_TRUE(11 == pFirmware->getTargetCode());
  ASSERT_TRUE("intelhex8" == pFirmware->getFormat());
  ASSERT_TRUE("2020-05-15" == pFirmware->getDate());
  ASSERT_TRUE(8192 == pFirmware->getSize());
  ASSERT_TRUE(1 == pFirmware->getVersionMajor());
  ASSERT_TRUE(1 == pFirmware->getVersionMinor());
  ASSERT_TRUE(6 == pFirmware->getVersionPatch());
  ASSERT_TRUE("0x595f44fec1e92a71d3e9e77456ba80d1" == pFirmware->getMd5());

  ASSERT_TRUE("Firmware for the CAN4VSCP Paris relay module with PIC18f2580 processor." ==
              pFirmware->getDescription("en"));
  ASSERT_TRUE("Firmware för CAN4VSCP Paris relay modul med PIC18f2580 processor." == pFirmware->getDescription("se"));

  // Get first firmare url
  pFirmware = mdf.getFirmwareObj(3);
  ASSERT_TRUE("esp32c3" == pFirmware->getTarget());
  ASSERT_TRUE("https://github.com/grodansparadis/can4vscp_paris/releases/download/v1.1.1/paris_relay_1_1_1.hex" ==
              pFirmware->getUrl());
  ASSERT_TRUE(44 == pFirmware->getTargetCode());
  ASSERT_TRUE("intelhex16" == pFirmware->getFormat());
  ASSERT_TRUE("2021-11-02" == pFirmware->getDate());
  ASSERT_TRUE(0 == pFirmware->getSize());
  ASSERT_TRUE(99 == pFirmware->getVersionMajor());
  ASSERT_TRUE(8 == pFirmware->getVersionMinor());
  ASSERT_TRUE(17 == pFirmware->getVersionPatch());
  ASSERT_TRUE("595f44fec1e92a71d3e9e77456ba80d1" == pFirmware->getMd5());

  ASSERT_TRUE("Description in English." == pFirmware->getDescription("en"));
  ASSERT_TRUE("Beskrivning på Svenska." == pFirmware->getDescription("se"));
}

//-----------------------------------------------------------------------------

TEST(parseMDF, Simple_Manual_Standard_Format)
{
  CMDF mdf;
  CMDF_Manual *pManual;

  std::string path = "xml/simple_manual_standard_format.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  ASSERT_EQ(2, mdf.getManualCount());

  ASSERT_TRUE(nullptr != mdf.getManualObj());
  ASSERT_TRUE(nullptr != mdf.getManualObj(0));

  pManual = mdf.getManualObj(0);
  ASSERT_TRUE("https://www.grodansparadis.com/paris/manual1.pdf" == pManual->getUrl());
  ASSERT_TRUE("en" == pManual->getLanguage());
  ASSERT_TRUE("pdf" == pManual->getFormat());

  pManual = mdf.getManualObj(1);
  ASSERT_TRUE("https://www.grodansparadis.com/paris/manual2" == pManual->getUrl());
  ASSERT_TRUE("xx" == pManual->getLanguage());
  ASSERT_TRUE("html" == pManual->getFormat());
}

//-----------------------------------------------------------------------------

TEST(parseMDF, Simple_Manual_Old_Format)
{
  CMDF mdf;
  CMDF_Manual *pManual;

  std::string path = "xml/simple_manual_old_format.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  ASSERT_EQ(2, mdf.getManualCount());

  ASSERT_TRUE(nullptr != mdf.getManualObj());
  ASSERT_TRUE(nullptr != mdf.getManualObj(0));

  pManual = mdf.getManualObj(0);
  ASSERT_TRUE("https://www.grodansparadis.com/paris/manual1.pdf" == pManual->getUrl());
  ASSERT_TRUE("en" == pManual->getLanguage());
  ASSERT_TRUE("pdf" == pManual->getFormat());

  pManual = mdf.getManualObj(1);
  ASSERT_TRUE("https://www.grodansparadis.com/paris/manual2" == pManual->getUrl());
  ASSERT_TRUE("xx" == pManual->getLanguage());
  ASSERT_TRUE("html" == pManual->getFormat());
}

//-----------------------------------------------------------------------------

TEST(parseMDF, XML_BOOTLOADER)
{
  std::string path;
  CMDF mdf;

  path = "xml/boot.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  ASSERT_EQ(1, mdf.getBootLoaderObj()->getAlgorithm());
  ASSERT_EQ(8, mdf.getBootLoaderObj()->getBlockSize());
  ASSERT_EQ(0x2000, mdf.getBootLoaderObj()->getBlockCount());
}

//-----------------------------------------------------------------------------

TEST(parseMDF, REALXML)
{
  std::string path;
  CMDF mdf;

  path = "xml/paris_010.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/1wire_1.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/avr128_02.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));
  path = "xml/latch_jm1.mdf";

  path = "xml/ntc10KA_2.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/paris_010.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/sht_001.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/temp_at90can32.mdf";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/accra_1.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/beijing_1.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/mesp12.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/ntc10KA_3.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/raweth_a.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/simpleA.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/vilnius_1.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/arduino01.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/beijing_2.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/mesp17.mdf";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/odessa001.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/smart_001.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/avr128_01.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/ntc10KA_1.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/paris_001.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/rfid2000.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  path = "xml/smart2_001.xml";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));
}

///////////////////////////////////////////////////////////////////////////////
//                                JSON
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------

TEST(parseMDF, JSON_SIMPLE_A)
{
  std::string path;
  CMDF mdf;

  path = "json/simple_a.json";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  // Check name
  ASSERT_TRUE(mdf.getModuleName() == "Simple A test");

  // Check description
  ASSERT_TRUE(mdf.getModuleDescription("en") == "This is an english description");
  ASSERT_TRUE(mdf.getModuleDescription("es") == "Esta es una descripción en inglés");
  ASSERT_TRUE(mdf.getModuleDescription("pt") == "Esta é uma descrição em inglês");
  ASSERT_TRUE(mdf.getModuleDescription("zh") == "这是英文说明");
  ASSERT_TRUE(mdf.getModuleDescription("se") == "Det här är en engelsk beskrivning");
  ASSERT_TRUE(mdf.getModuleDescription("lt") == "Tai yra angliškas aprašymas");
  ASSERT_TRUE(mdf.getModuleDescription("de") == "Dies ist eine englische Beschreibung");
  ASSERT_TRUE(mdf.getModuleDescription("eo") == "Ĉi tio estas angla priskribo");

  // Check info URL
  ASSERT_EQ(8, mdf.getModuleHelpUrlCount());
  ASSERT_TRUE(mdf.getModuleHelpUrl("en") == "https://www.english.en");
  ASSERT_TRUE(mdf.getModuleHelpUrl("es") == "https://www.spanish.es");
  ASSERT_TRUE(mdf.getModuleHelpUrl("pt") == "https://www.portuguese.pt");
  ASSERT_TRUE(mdf.getModuleHelpUrl("zh") == "https://www.chineese.zh");
  ASSERT_TRUE(mdf.getModuleHelpUrl("se") == "https://www.swedish.se");
  ASSERT_TRUE(mdf.getModuleHelpUrl("lt") == "https://www.lithuanian.lt");
  ASSERT_TRUE(mdf.getModuleHelpUrl("de") == "https://www.german.de");
  ASSERT_TRUE(mdf.getModuleHelpUrl("eo") == "https://www.esperanto.eo");

  ASSERT_EQ(8, mdf.getModuleBufferSize());
}

//-----------------------------------------------------------------------------

TEST(parseMDF, JSON_SIMPLE_B)
{
  std::string path;
  CMDF mdf;

  path = "json/simple_b.json";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  // Check name
  ASSERT_TRUE(mdf.getModuleName() == "Simple B test");

  ASSERT_TRUE(mdf.getModuleDescription("en") == "This is an english BBB description");

  ASSERT_TRUE(mdf.getModuleHelpUrl("en") == "https://www.BBBenglishBBB.en");

  ASSERT_EQ(64, mdf.getModuleBufferSize());

  // * * * Picture * * *

  CMDF_Picture *pPicture;

  ASSERT_EQ(2, mdf.getPictureCount());

  uint16_t index = 0;
  ASSERT_TRUE(nullptr != mdf.getPictureObj());
  ASSERT_TRUE(nullptr != mdf.getPictureObj(0));
  ASSERT_TRUE(nullptr != mdf.getPictureObj(1));
  ASSERT_TRUE(nullptr == mdf.getPictureObj(2));
  ASSERT_TRUE(mdf.getPictureObj() == mdf.getPictureObj(0));

  // Get first picture url
  pPicture = mdf.getPictureObj();
  ASSERT_TRUE("http://www.grodansparadis.com/logo.png" == pPicture->getUrl());

  ASSERT_TRUE("png" == pPicture->getFormat());

  ASSERT_TRUE("This is a picture description in English." == pPicture->getDescription("en"));
  ASSERT_TRUE("Det här är en svensk beskrivning av bild 1." == pPicture->getDescription("se"));

  pPicture = mdf.getPictureObj(0);
  ASSERT_TRUE("http://www.grodansparadis.com/logo.png" == pPicture->getUrl());

  ASSERT_TRUE("png" == pPicture->getFormat());

  ASSERT_TRUE("This is a picture description in English." == pPicture->getDescription("en"));
  ASSERT_TRUE("Det här är en svensk beskrivning av bild 1." == pPicture->getDescription("se"));

  // Get first picture url again
  pPicture = mdf.getPictureObj(1);
  ASSERT_TRUE("http://www.somewhere.com/images/pict2.png" == pPicture->getUrl());

  ASSERT_TRUE("png" == pPicture->getFormat());

  ASSERT_TRUE("This is a picture 2 description in English." == pPicture->getDescription("en"));
  ASSERT_TRUE("Det här är en svensk beskrivning av bild 2." == pPicture->getDescription("se"));

  // * * * Firmware * * *

  CMDF_Firmware *pFirmware;

  ASSERT_EQ(8, mdf.getFirmwareCount());

  ASSERT_TRUE(nullptr != mdf.getFirmwareObj());
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(0));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(1));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(2));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(3));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(4));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(5));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(6));
  ASSERT_TRUE(nullptr != mdf.getFirmwareObj(7));
  ASSERT_TRUE(nullptr == mdf.getFirmwareObj(8));
  ASSERT_TRUE(mdf.getFirmwareObj() == mdf.getFirmwareObj(0));

  // Get first firmare url
  pFirmware = mdf.getFirmwareObj(0);
  ASSERT_TRUE("pic18f2580" == pFirmware->getTarget());
  ASSERT_TRUE("https://xxx.yy/1.hex" == pFirmware->getUrl());
  ASSERT_TRUE(11 == pFirmware->getTargetCode());
  ASSERT_TRUE("intelhex8" == pFirmware->getFormat());
  ASSERT_TRUE("2020-05-15" == pFirmware->getDate());
  ASSERT_TRUE(8192 == pFirmware->getSize());
  ASSERT_TRUE(1 == pFirmware->getVersionMajor());
  ASSERT_TRUE(1 == pFirmware->getVersionMinor());
  ASSERT_TRUE(6 == pFirmware->getVersionPatch());
  ASSERT_TRUE("0x595f44fec1e92a71d3e9e77456ba80d1" == pFirmware->getMd5());

  ASSERT_TRUE("English 1." == pFirmware->getDescription("en"));
  ASSERT_TRUE("Svenska 1." == pFirmware->getDescription("se"));

  // Get first firmare url
  pFirmware = mdf.getFirmwareObj(3);
  ASSERT_TRUE("esp32c3" == pFirmware->getTarget());
  ASSERT_TRUE("https://xxx.yy/4.hex" == pFirmware->getUrl());
  ASSERT_TRUE(0x33 == pFirmware->getTargetCode());
  ASSERT_TRUE("xxx123" == pFirmware->getFormat());
  ASSERT_TRUE("2021-11-02" == pFirmware->getDate());
  ASSERT_TRUE(0x2000 == pFirmware->getSize());
  ASSERT_TRUE(0x99 == pFirmware->getVersionMajor());
  ASSERT_TRUE(0x08 == pFirmware->getVersionMinor());
  ASSERT_TRUE(0x17 == pFirmware->getVersionPatch());
  ASSERT_TRUE("43c191bf6d6c3f263a8cd0efd4a058ab" == pFirmware->getMd5());

  ASSERT_TRUE("English 3." == pFirmware->getDescription("en"));

  // * * * Manual * * *

  CMDF_Manual *pManual;

  ASSERT_EQ(2, mdf.getManualCount());

  ASSERT_TRUE(nullptr != mdf.getManualObj());
  ASSERT_TRUE(nullptr != mdf.getManualObj(0));

  pManual = mdf.getManualObj(0);
  ASSERT_TRUE("https://www.grodansparadis.com/paris/manual1.pdf" == pManual->getUrl());
  ASSERT_TRUE("en" == pManual->getLanguage());
  ASSERT_TRUE("pdf" == pManual->getFormat());

  pManual = mdf.getManualObj(1);
  ASSERT_TRUE("https://www.grodansparadis.com/paris/manual2" == pManual->getUrl());
  ASSERT_TRUE("xx" == pManual->getLanguage());
  ASSERT_TRUE("html" == pManual->getFormat());

  // * * * Boot * * *

  ASSERT_EQ(1, mdf.getBootLoaderObj()->getAlgorithm());
  ASSERT_EQ(8, mdf.getBootLoaderObj()->getBlockSize());
  ASSERT_EQ(4096, mdf.getBootLoaderObj()->getBlockCount());
}

//-----------------------------------------------------------------------------

TEST(parseMDF, JSON_SIMPLE_Registers)
{
  std::string path;
  CMDF mdf;
  CMDF_Register *preg;
  std::deque<CMDF_ValueListValue *> *pValueList;
  std::deque<CMDF_Bit *> *pBitList;
  CMDF_Bit *pBit;

  path = "json/simple_registers.json";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  // Check name
  ASSERT_TRUE(mdf.getModuleName() == "Simple registers");

  // Check module version
  ASSERT_TRUE(mdf.getModuleVersion() == "Special version");

  // Check model
  ASSERT_TRUE(mdf.getModuleModel() == "Simple reg model");

  // Check module date
  ASSERT_TRUE(mdf.getModuleChangeDate() == "2029-11-02");

  // Check description
  ASSERT_TRUE(mdf.getModuleDescription("en") == "This is a simple reg description");

  // Check help url
  ASSERT_TRUE(mdf.getModuleHelpUrl("en") == "https://www.simplereg.en");

  // Check buffer size
  ASSERT_EQ(16, mdf.getModuleBufferSize());

  // Check number of registers that is defined
  ASSERT_EQ(3, mdf.getRegisterCount());

  // Check number of registers that is defined on page == 0
  ASSERT_EQ(2, mdf.getRegisterCount(2));

  // Check number of registers that is defined on page = 99
  ASSERT_EQ(1, mdf.getRegisterCount(99));

  // Invalid register offset
  preg = mdf.getRegister(33, 0);
  ASSERT_EQ(nullptr, preg);

  // *************************************************************************
  // 02:22
  // *************************************************************************

  // Check register att offset 0x22, page=2
  preg = mdf.getRegister(0x22, 2);

  // Should return a valid pointer
  ASSERT_TRUE(nullptr != preg);

  // Check register offset
  ASSERT_EQ(0x22, preg->getOffset());

  // Check register page
  ASSERT_EQ(2, preg->getPage());

  // Check register span
  ASSERT_EQ(1, preg->getSpan());

  // Check register width
  ASSERT_EQ(8, preg->getWidth());

  // Check register min
  ASSERT_EQ(2, preg->getMin());

  // Check register max
  ASSERT_EQ(128, preg->getMax());

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_READ_WRITE, preg->getAccess());

  // Check register default value
  ASSERT_EQ(99, vscp_readStringValue(preg->getDefault()));

  // Check name
  ASSERT_TRUE(preg->getName() == "Register example 1");

  // Check description
  ASSERT_TRUE(preg->getDescription("en") == "Just a byte register with color settings");
  ASSERT_TRUE(preg->getDescription("se") == "Ett vanligt register med färginställningar");

  // Check info URL
  ASSERT_TRUE(preg->getInfoURL("en") == "https://one.com");
  ASSERT_TRUE(preg->getInfoURL("se") == "https://two.com");

  // * * * VSCP Works * * *

  // Get current value - Should be default value
  ASSERT_EQ(99, preg->getValue());

  // Get row position
  ASSERT_EQ(11, preg->getRowPosition());

  // Get foreground color
  ASSERT_EQ(0x001200, preg->getForegroundColor());

  // Get background color
  ASSERT_EQ(0xfff3d4, preg->getBackgroundColor());

  // Empty value list
  pValueList = preg->getListValues();
  ASSERT_TRUE(nullptr != pValueList);

  // Check number of values
  ASSERT_EQ(0, pValueList->size());

  // *************************************************************************
  //  02:31   -- Testing value list
  // *************************************************************************

  // Check register att offset 0x22, page=2
  preg = mdf.getRegister(0x31, 2);

  // Should return a valid pointer
  ASSERT_TRUE(nullptr != preg);

  // Check register offset
  ASSERT_EQ(0x31, preg->getOffset());

  // Check register page
  ASSERT_EQ(2, preg->getPage());

  // Check register span
  ASSERT_EQ(1, preg->getSpan());

  // Check register width
  ASSERT_EQ(4, preg->getWidth());

  // Check register min
  ASSERT_EQ(0, preg->getMin());

  // Check register max
  ASSERT_EQ(15, preg->getMax());

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_READ_ONLY, preg->getAccess());

  // Check register default value
  ASSERT_EQ(253, vscp_readStringValue(preg->getDefault()));

  // Check name
  ASSERT_TRUE(preg->getName() == "Register example 2");

  // Check description
  ASSERT_TRUE(preg->getDescription("en") == "Register with value list");
  ASSERT_TRUE(preg->getDescription("se") == "Beskrivning med värde lista");
  ASSERT_TRUE(preg->getDescription("xx") == "");

  // Check info URL
  ASSERT_TRUE(preg->getInfoURL("gb") == "English help");
  ASSERT_TRUE(preg->getInfoURL("se") == "Svensk hjälp");
  ASSERT_TRUE(preg->getInfoURL("lt") == "Lietuvos padeda");
  ASSERT_TRUE(preg->getInfoURL("xx") == "");

  pValueList = preg->getListValues();
  ASSERT_TRUE(nullptr != pValueList);

  // Check number of values in list
  ASSERT_EQ(3, pValueList->size());

  // Check value list
  CMDF_ValueListValue *pValue;
  pValue = pValueList->at(0);
  ASSERT_TRUE(nullptr != pValue);
  ASSERT_EQ(0, vscp_readStringValue(pValue->getValue()));
  ASSERT_EQ("Off", pValue->getName());
  ASSERT_EQ("The device is off", pValue->getDescription("en"));
  ASSERT_EQ("Enheten är av", pValue->getDescription("se"));
  ASSERT_EQ("", pValue->getDescription("xx"));

  // Check info URL
  ASSERT_TRUE(pValue->getInfoURL("gb") == "English help url");
  ASSERT_TRUE(pValue->getInfoURL("se") == "Svensk hjälp url");
  ASSERT_TRUE(pValue->getInfoURL("lt") == "Lietuvos padeda url");
  ASSERT_TRUE(pValue->getInfoURL("xx") == "");

  // * * * VSCP Works * * *

  // Get current value - Should be default value
  ASSERT_EQ(253, preg->getValue());

  // Get row position
  ASSERT_EQ(0xaa, preg->getRowPosition());

  // Get foreground color
  ASSERT_EQ(0x100000, preg->getForegroundColor());

  // Get background color
  ASSERT_EQ(0x00f3d4, preg->getBackgroundColor());

  // *************************************************************************
  //  63:02   -- Testing bit list
  // *************************************************************************

  // Check register att offset 0x02, page=99
  preg = mdf.getRegister(2, 99);

  // Should return a valid pointer
  ASSERT_TRUE(nullptr != preg);

  // Check register offset
  ASSERT_EQ(2, preg->getOffset());

  // Check register page
  ASSERT_EQ(99, preg->getPage());

  // Check register span
  ASSERT_EQ(3, preg->getSpan());

  // Check register width
  ASSERT_EQ(8, preg->getWidth());

  // Check register min
  ASSERT_EQ(0, preg->getMin());

  // Check register max
  ASSERT_EQ(255, preg->getMax());

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_WRITE_ONLY, preg->getAccess());

  // Check register default value
  ASSERT_EQ(0x55, vscp_readStringValue(preg->getDefault()));

  // Check name
  ASSERT_TRUE(preg->getName() == "Register example 3");

  // ******  Bitarray 0 ******

  // Check bit list
  pBitList = preg->getListBits();
  ASSERT_TRUE(nullptr != pBitList);

  // Check item count
  ASSERT_EQ(2, pBitList->size());

  // Check bit definitions
  pBit = pBitList->at(0);
  ASSERT_TRUE(nullptr != pBit);

  // Check start for bit array
  ASSERT_EQ(0, pBit->getPos());

  // Check width for bit array
  ASSERT_EQ(3, pBit->getWidth());

  // Check width for bit array
  ASSERT_EQ(4, pBit->getDefault());

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_READ_WRITE, pBit->getAccess());

  // Check name
  ASSERT_TRUE(pBit->getName() == "English bit or bitfield name 0");

  // Check description
  ASSERT_EQ("English description bitfield 0", pBit->getDescription("gb"));
  ASSERT_EQ("Svensk beskrivning bitfield 0", pBit->getDescription("se"));
  ASSERT_EQ("Lietuvos aprašymas bitfield 0", pBit->getDescription("lt"));
  ASSERT_EQ("", pBit->getDescription("xx"));

  // Check info URL
  ASSERT_TRUE(pBit->getInfoURL("gb") == "English help bitfield 0");
  ASSERT_TRUE(pBit->getInfoURL("se") == "Svensk hjälp bitfield 0");
  ASSERT_TRUE(pBit->getInfoURL("lt") == "Lietuvos padeda bitfield 0");
  ASSERT_TRUE(pBit->getInfoURL("xx") == "");

  // ******  Bitarray 1 ******

  // Check bit definitions
  pBit = pBitList->at(1);
  ASSERT_TRUE(nullptr != pBit);

  // Check start for bit array
  ASSERT_EQ(3, pBit->getPos());

  // Check width for bit array
  ASSERT_EQ(2, pBit->getWidth());

  // Check width for bit array
  ASSERT_EQ(0, pBit->getDefault());

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_READ_ONLY, pBit->getAccess());

  // Check name
  ASSERT_TRUE(pBit->getName() == "English bit or bitfield name 1");

  // Check description
  ASSERT_EQ("English description bitfield 1", pBit->getDescription("gb"));
  ASSERT_EQ("Svensk beskrivning bitfield 1", pBit->getDescription("se"));
  ASSERT_EQ("Lietuvos aprašymas bitfield 1", pBit->getDescription("lt"));
  ASSERT_EQ("", pBit->getDescription("xx"));

  // Check info URL
  ASSERT_TRUE(pBit->getInfoURL("gb") == "English help bitfield 1");
  ASSERT_TRUE(pBit->getInfoURL("se") == "Svensk hjälp bitfield 1");
  ASSERT_TRUE(pBit->getInfoURL("lt") == "Lietuvos padeda bitfield 1");
  ASSERT_TRUE(pBit->getInfoURL("xx") == "");
}

//-----------------------------------------------------------------------------

TEST(parseMDF, JSON_SIMPLE_Remotevar)
{
  std::string path;
  CMDF mdf;
  CMDF_RemoteVariable *prvar;
  std::deque<CMDF_ValueListValue *> *pValueList;
  std::deque<CMDF_Bit *> *pBitList;
  CMDF_Bit *pBit;

  path = "json/simple_remotevar.json";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  // Check name
  ASSERT_TRUE(mdf.getModuleName() == "Simple remotevar");

  // Check module version
  ASSERT_TRUE(mdf.getModuleVersion() == "var2022");

  // Check model
  ASSERT_TRUE(mdf.getModuleModel() == "Simple remotevar module");

  // Check module date
  ASSERT_TRUE(mdf.getModuleChangeDate() == "1956-11-02");

  // Check description
  ASSERT_TRUE(mdf.getModuleDescription("en") == "This is an english BBB description");

  // Check help url
  ASSERT_TRUE(mdf.getModuleHelpUrl("en") == "https://www.BBBenglishBBB.en");

  // Check buffer size
  ASSERT_EQ(32, mdf.getModuleBufferSize());

  // Check number of registers that is defined
  ASSERT_EQ(3, mdf.getRemoteVariableCount());

  // Invalid remote variable
  prvar = mdf.getRemoteVariable("Unexisting_remotevar");
  ASSERT_EQ(nullptr, prvar);

  // ***** Remote variable 1 *****

  // Test remote variable
  prvar = mdf.getRemoteVariable("Remote variable 1");
  ASSERT_NE(nullptr, prvar);

  // Check name
  ASSERT_TRUE(prvar->getName() == "Remote variable 1");

  // Check type "short" == int16_t
  ASSERT_EQ(remote_variable_type_int16_t, prvar->getType());

  // Check default value
  ASSERT_EQ(8, vscp_readStringValue(prvar->getDefault()));

  // Check default value
  ASSERT_EQ(8, vscp_readStringValue(prvar->getDefault()));

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_READ_WRITE, prvar->getAccess());

  // Check page
  ASSERT_EQ(2, prvar->getPage());

  // Check offset
  ASSERT_EQ(18, prvar->getOffset());

  // Check VSCP Work grid position
  ASSERT_EQ(99, prvar->getRowPosition());

  // Get foreground color
  ASSERT_EQ(0x123456, prvar->getForegroundColor());

  // Get background color
  ASSERT_EQ(0xCCFFFF, prvar->getBackgroundColor());

  // Check description
  ASSERT_EQ("English description remotevar 1", prvar->getDescription("en"));
  ASSERT_EQ("Svensk beskrivning remotevar 1", prvar->getDescription("se"));
  ASSERT_EQ("Lietuvos padeda remotevar 1", prvar->getDescription("lt"));
  ASSERT_EQ("", prvar->getDescription("xx"));

  // Check info URL
  ASSERT_TRUE(prvar->getInfoURL("gb") == "English help remotevar 1");
  ASSERT_TRUE(prvar->getInfoURL("se") == "Svensk hjälp remotevar 1");
  ASSERT_TRUE(prvar->getInfoURL("lt") == "Lietuvos padeda remotevar 1");
  ASSERT_TRUE(prvar->getInfoURL("xx") == "");

  // Check complete type set/get
  prvar->setType(remote_variable_type_unknown);
  ASSERT_EQ(remote_variable_type_unknown, prvar->getType());

  prvar->setType(remote_variable_type_string);
  ASSERT_EQ(remote_variable_type_string, prvar->getType());

  prvar->setType(remote_variable_type_boolean);
  ASSERT_EQ(remote_variable_type_boolean, prvar->getType());

  prvar->setType(remote_variable_type_int8_t);
  ASSERT_EQ(remote_variable_type_int8_t, prvar->getType());

  prvar->setType(remote_variable_type_uint8_t);
  ASSERT_EQ(remote_variable_type_uint8_t, prvar->getType());

  prvar->setType(remote_variable_type_int16_t);
  ASSERT_EQ(remote_variable_type_int16_t, prvar->getType());

  prvar->setType(remote_variable_type_uint16_t);
  ASSERT_EQ(remote_variable_type_uint16_t, prvar->getType());

  prvar->setType(remote_variable_type_int32_t);
  ASSERT_EQ(remote_variable_type_int32_t, prvar->getType());

  prvar->setType(remote_variable_type_uint32_t);
  ASSERT_EQ(remote_variable_type_uint32_t, prvar->getType());

  prvar->setType(remote_variable_type_int64_t);
  ASSERT_EQ(remote_variable_type_int64_t, prvar->getType());

  prvar->setType(remote_variable_type_uint64_t);
  ASSERT_EQ(remote_variable_type_uint64_t, prvar->getType());

  prvar->setType(remote_variable_type_float);
  ASSERT_EQ(remote_variable_type_float, prvar->getType());

  prvar->setType(remote_variable_type_double);
  ASSERT_EQ(remote_variable_type_double, prvar->getType());

  prvar->setType(remote_variable_type_date);
  ASSERT_EQ(remote_variable_type_date, prvar->getType());

  prvar->setType(remote_variable_type_time);
  ASSERT_EQ(remote_variable_type_time, prvar->getType());

  // ***** Remote variable 2 *****

  // Test remote variable
  prvar = mdf.getRemoteVariable("Remote variable 2");
  ASSERT_NE(nullptr, prvar);

  // Check name
  ASSERT_TRUE(prvar->getName() == "Remote variable 2");

  // Check type "byte" == uint8_t
  ASSERT_EQ(remote_variable_type_uint8_t, prvar->getType());

  // Check default value
  ASSERT_EQ(0x22, vscp_readStringValue(prvar->getDefault()));

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_READ_ONLY, prvar->getAccess());

  // Check page
  ASSERT_EQ(0x11, prvar->getPage());

  // Check offset
  ASSERT_EQ(12, prvar->getOffset());

  // Check VSCP Work grid position
  ASSERT_EQ(4, prvar->getRowPosition());

  // Get foreground color
  ASSERT_EQ(0x888888, prvar->getForegroundColor());

  // Get background color
  ASSERT_EQ(0x777777, prvar->getBackgroundColor());

  // Check description
  ASSERT_EQ("English description remotevar 2", prvar->getDescription("en"));
  ASSERT_EQ("Svensk beskrivning remotevar 2", prvar->getDescription("se"));
  ASSERT_EQ("Lietuvos padeda remotevar 2", prvar->getDescription("lt"));
  ASSERT_EQ("", prvar->getDescription("xx"));

  // Check info URL
  ASSERT_TRUE(prvar->getInfoURL("en") == "English help remotevar 2");
  ASSERT_TRUE(prvar->getInfoURL("se") == "Svensk hjälp remotevar 2");
  ASSERT_TRUE(prvar->getInfoURL("lt") == "Lietuvos padeda remotevar 2");
  ASSERT_TRUE(prvar->getInfoURL("xx") == "");

  pValueList = prvar->getListValues();
  ASSERT_TRUE(nullptr != pValueList);

  // Check number of values in list
  ASSERT_EQ(3, pValueList->size());

  // Check value list
  CMDF_ValueListValue *pValue;
  pValue = pValueList->at(0);
  ASSERT_TRUE(nullptr != pValue);
  ASSERT_EQ(0, vscp_readStringValue(pValue->getValue()));
  ASSERT_EQ("Low", pValue->getName());
  ASSERT_EQ("Low speed", pValue->getDescription("en"));
  ASSERT_EQ("Låg hastighet", pValue->getDescription("se"));
  ASSERT_EQ("??????????", pValue->getDescription("lt"));
  ASSERT_EQ("", pValue->getDescription("xx"));

  // Check info URL
  ASSERT_TRUE(pValue->getInfoURL("gb") == "English help 1 vl2");
  ASSERT_TRUE(pValue->getInfoURL("se") == "Svensk hjälp 1 vl2");
  ASSERT_TRUE(pValue->getInfoURL("lt") == "Lietuvos padeda 1 vl2");
  ASSERT_TRUE(pValue->getInfoURL("xx") == "");

  pValue = pValueList->at(1);
  ASSERT_TRUE(nullptr != pValue);
  ASSERT_EQ(1, vscp_readStringValue(pValue->getValue()));
  ASSERT_EQ("Medium", pValue->getName());

  pValue = pValueList->at(2);
  ASSERT_TRUE(nullptr != pValue);
  ASSERT_EQ(3, vscp_readStringValue(pValue->getValue()));
  ASSERT_EQ("High", pValue->getName());

  // ***** Remote variable 3 *****

  // Test remote variable
  prvar = mdf.getRemoteVariable("Remote variable 3");
  ASSERT_NE(nullptr, prvar);

  // Check name
  ASSERT_TRUE(prvar->getName() == "Remote variable 3");

  // Check type "byte" == uint8_t
  ASSERT_EQ(remote_variable_type_uint32_t, prvar->getType());

  // Check default value
  ASSERT_EQ(0, vscp_readStringValue(prvar->getDefault()));

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_READ_WRITE, prvar->getAccess());

  // Check page
  ASSERT_EQ(9, prvar->getPage());

  // Check offset
  ASSERT_EQ(98, prvar->getOffset());

  // Check VSCP Work grid position
  ASSERT_EQ(0x44, prvar->getRowPosition());

  // Get foreground color
  ASSERT_EQ(0xAAAAAA, prvar->getForegroundColor());

  // Get background color
  ASSERT_EQ(0x999999, prvar->getBackgroundColor());

  // ******  Bitarray 0 ******

  // Check bit list
  pBitList = prvar->getListBits();
  ASSERT_TRUE(nullptr != pBitList);

  // Check item count
  ASSERT_EQ(2, pBitList->size());

  // Check bit definitions
  pBit = pBitList->at(0);
  ASSERT_TRUE(nullptr != pBit);

  // Check start for bit array
  ASSERT_EQ(0, pBit->getPos());

  // Check width for bit array
  ASSERT_EQ(3, pBit->getWidth());

  // Check width for bit array
  ASSERT_EQ(4, pBit->getDefault());

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_READ_WRITE, pBit->getAccess());

  // Check name
  ASSERT_TRUE(pBit->getName() == "Bitfield name 0");

  // Check description
  ASSERT_EQ("English description bitfield 0", pBit->getDescription("gb"));
  ASSERT_EQ("Svensk beskrivning bitfield 0", pBit->getDescription("se"));
  ASSERT_EQ("Lietuvos aprašymas bitfield 0", pBit->getDescription("lt"));
  ASSERT_EQ("", pBit->getDescription("xx"));

  // Check info URL
  ASSERT_TRUE(pBit->getInfoURL("gb") == "English help bitfield 0");
  ASSERT_TRUE(pBit->getInfoURL("se") == "Svensk hjälp bitfield 0");
  ASSERT_TRUE(pBit->getInfoURL("lt") == "Lietuvos padeda bitfield 0");
  ASSERT_TRUE(pBit->getInfoURL("xx") == "");

  // ******  Bitarray 1 ******

  // Check bit definitions
  pBit = pBitList->at(1);
  ASSERT_TRUE(nullptr != pBit);

  // Check start for bit array
  ASSERT_EQ(3, pBit->getPos());

  // Check width for bit array
  ASSERT_EQ(2, pBit->getWidth());

  // Check width for bit array
  ASSERT_EQ(0, pBit->getDefault());

  // Get access rights
  ASSERT_EQ(MDF_REG_ACCESS_READ_ONLY, pBit->getAccess());

  // Check name
  ASSERT_TRUE(pBit->getName() == "Bitfield name 1");

  // Check description
  ASSERT_EQ("English description bitfield 1", pBit->getDescription("gb"));
  ASSERT_EQ("Svensk beskrivning bitfield 1", pBit->getDescription("se"));
  ASSERT_EQ("Lietuvos aprašymas bitfield 1", pBit->getDescription("lt"));
  ASSERT_EQ("", pBit->getDescription("xx"));

  // Check info URL
  ASSERT_TRUE(pBit->getInfoURL("gb") == "English help bitfield 1");
  ASSERT_TRUE(pBit->getInfoURL("se") == "Svensk hjälp bitfield 1");
  ASSERT_TRUE(pBit->getInfoURL("lt") == "Lietuvos padeda bitfield 1");
  ASSERT_TRUE(pBit->getInfoURL("xx") == "");

  // Value list

  pValueList = pBit->getListValues();
  ASSERT_TRUE(nullptr != pValueList);

  // Check number of values in list
  ASSERT_EQ(3, pValueList->size());

  // Check value list
  pValue = pValueList->at(0);
  ASSERT_TRUE(nullptr != pValue);
  ASSERT_EQ(0, vscp_readStringValue(pValue->getValue()));
  ASSERT_EQ("Low", pValue->getName());
  ASSERT_EQ("Low speed", pValue->getDescription("en"));
  ASSERT_EQ("Låg hastighet", pValue->getDescription("se"));
  ASSERT_EQ("", pValue->getDescription("xx"));

  // Check info URL
  ASSERT_TRUE(pValue->getInfoURL("gb") == "English help 1 vl2");
  ASSERT_TRUE(pValue->getInfoURL("se") == "Svensk hjälp 1 vl2");
  ASSERT_TRUE(pValue->getInfoURL("lt") == "Lietuvos padeda 1 vl2");
  ASSERT_TRUE(pValue->getInfoURL("xx") == "");

  pValue = pValueList->at(1);
  ASSERT_TRUE(nullptr != pValue);
  ASSERT_EQ(1, vscp_readStringValue(pValue->getValue()));
  ASSERT_EQ("Medium", pValue->getName());
}

//-----------------------------------------------------------------------------

TEST(parseMDF, JSON_SIMPLE_DMatrix)
{
  std::string path;
  CMDF mdf;

  path = "json/simple_dm.json";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  // Check name
  ASSERT_TRUE(mdf.getModuleName() == "Simple DM");

  // Get decision matrix
  CMDF_DecisionMatrix *pdm = mdf.getDM();
  ASSERT_NE(nullptr, pdm);

  // Verify level
  ASSERT_EQ(1, pdm->getLevel());

  // Verify start page
  ASSERT_EQ(16, pdm->getStartPage());

  // Verify offset
  ASSERT_EQ(88, pdm->getStartOffset());

  // Verify number of rows
  ASSERT_EQ(10, pdm->getRowCount());

  // Verify size of rows
  ASSERT_EQ(8, pdm->getRowSize());

  // Verify indexed
  ASSERT_EQ(false, pdm->isIndexed());

  {

    // Get action list
    std::deque<CMDF_Action *> *pActionList = pdm->getActionList();

    // Verify pointer
    ASSERT_NE(nullptr, pActionList);

    // Verify number of actions
    ASSERT_EQ(2, pActionList->size());

    // * * Param 1 * *

    {

      // Get action
      CMDF_Action *paction = pActionList->front();

      // Verify pointer
      ASSERT_NE(nullptr, paction);

      // Verify action code
      ASSERT_EQ(1, paction->getCode());

      // Verify action name
      ASSERT_EQ("action 1", paction->getName());

      ASSERT_EQ(paction->getDescription("en"), "English description dm action 1");
      ASSERT_EQ(paction->getDescription("se"), "Svensk beskrivning dm action 1");
      ASSERT_EQ(paction->getDescription("lt"), "Lietuvos padeda dm action 1");
      ASSERT_EQ(paction->getDescription("xx"), "");

      // Check info URL
      ASSERT_TRUE(paction->getInfoURL("en") == "English help dm action 1");
      ASSERT_TRUE(paction->getInfoURL("se") == "Svensk hjälp dm action 1");
      ASSERT_TRUE(paction->getInfoURL("lt") == "Lietuvos padeda dm action 1");
      ASSERT_TRUE(paction->getInfoURL("xx") == "");

      // Get action parameter list
      std::deque<CMDF_ActionParameter *> *pActionParameterList = paction->getListActionParameter();

      // Verify pointer
      ASSERT_NE(nullptr, pActionParameterList);

      // Verify number of action parameters
      ASSERT_EQ(2, pActionParameterList->size());

      {
        // Get action
        CMDF_ActionParameter *pactionparam = pActionParameterList->front();

        // Verify pointer
        ASSERT_NE(nullptr, pactionparam);

        ASSERT_EQ(pactionparam->getName(), "parm 1");

        ASSERT_EQ(pactionparam->getDescription("en"), "English description dm param 1");
        ASSERT_EQ(pactionparam->getDescription("se"), "Svensk beskrivning dm param 1");
        ASSERT_EQ(pactionparam->getDescription("lt"), "Lietuvos padeda dm param 1");
        ASSERT_EQ(pactionparam->getDescription("xx"), "");

        // Check info URL
        ASSERT_TRUE(pactionparam->getInfoURL("en") == "English help dm param 1");
        ASSERT_TRUE(pactionparam->getInfoURL("se") == "Svensk hjälp dm param 1");
        ASSERT_TRUE(pactionparam->getInfoURL("lt") == "Lietuvos padeda dm param 1");
        ASSERT_TRUE(pactionparam->getInfoURL("xx") == "");

        // ******  Bitarray 0 ******

        std::deque<CMDF_Bit *> *pBitList;
        CMDF_Bit *pBit;

        // Check bit list
        pBitList = pactionparam->getListBits();
        ASSERT_TRUE(nullptr != pBitList);

        // Check item count
        ASSERT_EQ(2, pBitList->size());

        // Check bit definitions
        pBit = pBitList->at(0);
        ASSERT_TRUE(nullptr != pBit);

        // Check start for bit array
        ASSERT_EQ(0, pBit->getPos());

        // Check width for bit array
        ASSERT_EQ(3, pBit->getWidth());

        // Check width for bit array
        ASSERT_EQ(7, pBit->getDefault());

        // Get access rights
        ASSERT_EQ(MDF_REG_ACCESS_READ_WRITE, pBit->getAccess());

        // Check name
        ASSERT_TRUE(pBit->getName() == "Bitfield name 0");
      }

      // * * Param 2 * *

      {
        // Get action
        CMDF_ActionParameter *pactionparam = pActionParameterList->at(1);

        // Verify pointer
        ASSERT_NE(nullptr, pactionparam);

        ASSERT_EQ(pactionparam->getName(), "parm 2");

        ASSERT_EQ(pactionparam->getDescription("en"), "English description dm param 2");
        ASSERT_EQ(pactionparam->getDescription("se"), "Svensk beskrivning dm param 2");
        ASSERT_EQ(pactionparam->getDescription("lt"), "Lietuvos padeda dm param 2");
        ASSERT_EQ(pactionparam->getDescription("xx"), "");

        // Check info URL
        ASSERT_TRUE(pactionparam->getInfoURL("en") == "English help dm param 2");
        ASSERT_TRUE(pactionparam->getInfoURL("se") == "Svensk hjälp dm param 2");
        ASSERT_TRUE(pactionparam->getInfoURL("lt") == "Lietuvos padeda dm param 2");
        ASSERT_TRUE(pactionparam->getInfoURL("xx") == "");
      }
    }
  }
}

//-----------------------------------------------------------------------------

TEST(parseMDF, JSON_Events)
{
  std::string path;
  CMDF mdf;

  path = "json/simple_events.json";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  // Check name
  ASSERT_EQ(mdf.getModuleName(), "Simple Events");

  // Get events list
  std::deque<CMDF_Event *> *pEventList = mdf.getEventList();
  ASSERT_NE(nullptr, pEventList);

  // Validate event count
  ASSERT_EQ(1, pEventList->size());

  {
    CMDF_Event *pevent = pEventList->front();
    ASSERT_NE(nullptr, pevent);

    // Verify name
    ASSERT_EQ(pevent->getName(), "test event 1");

    ASSERT_EQ(pevent->getDescription("en"), "English event description 3");
    ASSERT_EQ(pevent->getDescription("se"), "Svensk event beskrivning 3");
    ASSERT_EQ(pevent->getDescription("lt"), "Lietuvos event padeda 3");
    ASSERT_EQ(pevent->getDescription("xx"), "");

    // Check info URL
    ASSERT_TRUE(pevent->getInfoURL("en") == "English event help 3");
    ASSERT_TRUE(pevent->getInfoURL("se") == "Svensk event hjälp 3");
    ASSERT_TRUE(pevent->getInfoURL("lt") == "Lietuvos event padeda 3");
    ASSERT_TRUE(pevent->getInfoURL("xx") == "");

    // Verify class
    ASSERT_EQ(pevent->getClass(), 10);

    // Verify type
    ASSERT_EQ(pevent->getType(), 6);

    // Verify priority
    ASSERT_EQ(pevent->getPriority(), 7);

    // Verify direction
    ASSERT_EQ(pevent->getDirection(), MDF_EVENT_DIR_IN);

    std::deque<CMDF_EventData *> *pEventDataList = pevent->getListEventData();
    ASSERT_NE(nullptr, pEventDataList);

    // Validate event data count
    ASSERT_EQ(1, pEventDataList->size());

    {
      CMDF_EventData *peventdata = pEventDataList->front();
      ASSERT_NE(nullptr, peventdata);

      // Verify name
      ASSERT_EQ(peventdata->getName(), "test event data 1");

      CMDF_EventData *pEventData = pEventDataList->front();
      ASSERT_NE(nullptr, pEventDataList);

      ASSERT_EQ(pEventData->getDescription("en"), "English event data description 3");
      ASSERT_EQ(pEventData->getDescription("se"), "Svensk event data beskrivning 3");
      ASSERT_EQ(pEventData->getDescription("lt"), "Lietuvos event data padeda 3");
      ASSERT_EQ(pEventData->getDescription("xx"), "");

      // Check info URL
      ASSERT_TRUE(pEventData->getInfoURL("en") == "English event data help 3");
      ASSERT_TRUE(pEventData->getInfoURL("se") == "Svensk event data hjälp 3");
      ASSERT_TRUE(pEventData->getInfoURL("lt") == "Lietuvos event data padeda 3");
      ASSERT_TRUE(pEventData->getInfoURL("xx") == "");

      {
        std::deque<CMDF_Bit *> *pBitList;
        CMDF_Bit *pBit;
        std::deque<CMDF_ValueListValue *> *pValueList;

        // ******  Bitarray 0 ******

        // Check bit list
        pBitList = pEventData->getListBits();
        ASSERT_TRUE(nullptr != pBitList);

        // Check item count
        ASSERT_EQ(2, pBitList->size());

        // Check bit definitions
        pBit = pBitList->at(0);
        ASSERT_TRUE(nullptr != pBit);

        // Check start for bit array
        ASSERT_EQ(0, pBit->getPos());

        // Check width for bit array
        ASSERT_EQ(3, pBit->getWidth());

        // Check width for bit array
        ASSERT_EQ(4, pBit->getDefault());

        // Get access rights
        ASSERT_EQ(MDF_REG_ACCESS_READ_WRITE, pBit->getAccess());

        // Check name
        ASSERT_TRUE(pBit->getName() == "Bitfield name 0");

        // Check description
        ASSERT_EQ("English description bit 0", pBit->getDescription("en"));
        ASSERT_EQ("Svensk beskrivning bit 0", pBit->getDescription("se"));
        ASSERT_EQ("Lietuvos aprašymas bit 0", pBit->getDescription("lt"));
        ASSERT_EQ("", pBit->getDescription("xx"));

        // Check info URL
        ASSERT_TRUE(pBit->getInfoURL("en") == "English help bit 0");
        ASSERT_TRUE(pBit->getInfoURL("se") == "Svensk hjälp bit 0");
        ASSERT_TRUE(pBit->getInfoURL("lt") == "Lietuvos padeda bit 0");
        ASSERT_TRUE(pBit->getInfoURL("xx") == "");

        // ******  Bitarray 1 ******

        // Check bit definitions
        pBit = pBitList->at(1);
        ASSERT_TRUE(nullptr != pBit);

        // Check start for bit array
        ASSERT_EQ(3, pBit->getPos());

        // Check width for bit array
        ASSERT_EQ(2, pBit->getWidth());

        // Check width for bit array
        ASSERT_EQ(0, pBit->getDefault());

        // Get access rights
        ASSERT_EQ(MDF_REG_ACCESS_READ_ONLY, pBit->getAccess());

        // Check name
        ASSERT_TRUE(pBit->getName() == "Bitfield name 1");

        // Check description
        ASSERT_EQ("English description bit 1", pBit->getDescription("en"));
        ASSERT_EQ("Svensk beskrivning bit 1", pBit->getDescription("se"));
        ASSERT_EQ("Lietuvos aprašymas bit 1", pBit->getDescription("lt"));
        ASSERT_EQ("", pBit->getDescription("xx"));

        // Check info URL
        ASSERT_TRUE(pBit->getInfoURL("en") == "English help bit 1");
        ASSERT_TRUE(pBit->getInfoURL("se") == "Svensk hjälp bit 1");
        ASSERT_TRUE(pBit->getInfoURL("lt") == "Lietuvos padeda bit 1");
        ASSERT_TRUE(pBit->getInfoURL("xx") == "");

        // Value list

        pValueList = pBit->getListValues();
        ASSERT_TRUE(nullptr != pValueList);

        // Check number of values in list
        ASSERT_EQ(3, pValueList->size());

        CMDF_ValueListValue *pValue;

        // Check value list
        pValue = pValueList->at(0);
        ASSERT_TRUE(nullptr != pValue);
        ASSERT_EQ(0, vscp_readStringValue(pValue->getValue()));
        ASSERT_EQ("Low", pValue->getName());
        ASSERT_EQ("Low speed", pValue->getDescription("en"));
        ASSERT_EQ("Låg hastighet", pValue->getDescription("se"));
        ASSERT_EQ("", pValue->getDescription("xx"));

        // Check info URL
        ASSERT_TRUE(pValue->getInfoURL("en") == "English help 1 vl2");
        ASSERT_TRUE(pValue->getInfoURL("se") == "Svensk hjälp 1 vl2");
        ASSERT_TRUE(pValue->getInfoURL("lt") == "Lietuvos padeda 1 vl2");
        ASSERT_TRUE(pValue->getInfoURL("xx") == "");

        pValue = pValueList->at(1);
        ASSERT_TRUE(nullptr != pValue);
        ASSERT_EQ(1, vscp_readStringValue(pValue->getValue()));
        ASSERT_EQ("Medium", pValue->getName());
      }
    }
  }
}

//-----------------------------------------------------------------------------

TEST(parseMDF, JSON_Alarm)
{
  std::deque<CMDF_Bit *> *pAlarmList;
  std::string path;
  CMDF mdf;

  path = "json/simple_alarm.json";
  ASSERT_EQ(VSCP_ERROR_SUCCESS, mdf.parseMDF(path));

  // Check name
  ASSERT_EQ(mdf.getModuleName(), "Simple Alarm");

  // Check bit list
  pAlarmList = mdf.getAlarmList();
  ASSERT_TRUE(nullptr != pAlarmList);

  // Check item count
  ASSERT_EQ(8, pAlarmList->size());

  for (uint8_t i = 0; i < 7; i++) {
    CMDF_Bit *pBit;

    // Check bit definitions
    pBit = pAlarmList->at(i);
    ASSERT_TRUE(nullptr != pBit);

    // Check start for bit array
    ASSERT_EQ(i, pBit->getPos());
  }
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
  // Init pool
  spdlog::init_thread_pool(8192, 1);

  auto console = spdlog::stdout_color_mt("console");

  // console->set_level(spdlog::level::off);
  console->set_level(spdlog::level::err);
  // console->set_level(spdlog::level::trace);

  console->set_pattern("[mdfparser: %c] [%^%l%$] %v");
  spdlog::set_default_logger(console);

  console->info("mdfparser: Starting tmdpparser test...");

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
