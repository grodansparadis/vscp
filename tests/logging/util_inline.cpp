#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <gtest/gtest.h>
#include <vscphelper.h>
#include <vscphelperlogging.h>

namespace {

namespace fs = std::filesystem;

TEST(UtilLogging, InlineAndUtilityFunctionsWorkWithoutCore)
{
    const fs::path directory = fs::temp_directory_path() /
        ("vscp-util-logging-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    ASSERT_TRUE(fs::create_directory(directory));
    struct Cleanup {
        fs::path directory;
        ~Cleanup()
        {
            EXPECT_EQ(VSCP_ERROR_SUCCESS, vscphelper_disableLogging());
            fs::remove_all(directory);
        }
    } cleanup{directory};
    const std::string path = (directory / "util.log").string();
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_configureLogging(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_DEBUG, path.c_str(), 1024 * 1024, 2));

    std::string value = "MIXED";
    vscp_makeLower(value);
    EXPECT_EQ("mixed", value);
    EXPECT_EQ(16, vscp_readStringValue("0x10"));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());
    std::ifstream stream(path);
    const std::string output{std::istreambuf_iterator<char>(stream), {}};
    EXPECT_NE(std::string::npos, output.find("[debug] VSCP helper: vscp_makeLower: entered"));
    EXPECT_NE(std::string::npos, output.find("[debug] VSCP helper: vscp_readStringValue: entered"));
}

} // namespace
