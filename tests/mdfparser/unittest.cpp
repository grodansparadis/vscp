// unit_test for mdfparser

#include <gtest/gtest.h>
#include <math.h>
#include <mdf.h>
#include <stdio.h>
#include <stdlib.h>

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

  // Check # names
  ASSERT_EQ(1, mdf.getModuleNameSize());

  // Check # descriptions
  ASSERT_EQ(8, mdf.getModuleDescriptionSize());

  // Check # info URL's
  ASSERT_EQ(8, mdf.getModuleInfoUrlSize());
  
  // Check name
  ASSERT_TRUE(mdf.getModuleName("en") == "Simple test");

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
  ASSERT_TRUE(mdf.getModuleInfoUrl("en") == "https://www.english.en");
  ASSERT_TRUE(mdf.getModuleInfoUrl("es") == "https://www.spanish.es");
  ASSERT_TRUE(mdf.getModuleInfoUrl("pt") == "https://www.portuguese.pt");
  ASSERT_TRUE(mdf.getModuleInfoUrl("zh") == "https://www.chineese.zh");
  ASSERT_TRUE(mdf.getModuleInfoUrl("se") == "https://www.swedish.se");
  ASSERT_TRUE(mdf.getModuleInfoUrl("lt") == "https://www.lithuanian.lt");
  ASSERT_TRUE(mdf.getModuleInfoUrl("de") == "https://www.german.de");
  ASSERT_TRUE(mdf.getModuleInfoUrl("eo") == "https://www.esperanto.eo");
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

//-----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
  // Init pool
  spdlog::init_thread_pool(8192, 1);

  auto console = spdlog::stdout_color_mt("console");
  
  //console->set_level(spdlog::level::off);
  console->set_level(spdlog::level::err);
  //console->set_level(spdlog::level::trace);

  console->set_pattern("[mdfparser: %c] [%^%l%$] %v");
  spdlog::set_default_logger(console);

  console->info("mdfparser: Starting tmdpparser test...");

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
