#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>

#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include <vscphelper.h>
#include <vscphelperlogging.h>
#include <vscpremotetcpif.h>

namespace {

namespace fs = std::filesystem;

class CoreLogging : public testing::Test {
protected:
    void SetUp() override
    {
        directory = fs::temp_directory_path() /
                    ("vscp-core-logging-" + std::to_string(
                        std::chrono::steady_clock::now().time_since_epoch().count()));
        ASSERT_TRUE(fs::create_directory(directory));
        path = (directory / "core.log").string();
    }

    void TearDown() override
    {
        EXPECT_EQ(VSCP_ERROR_SUCCESS, vscphelper_disableLogging());
        fs::remove_all(directory);
    }

    std::string read() const
    {
        std::ifstream stream(path);
        return std::string(std::istreambuf_iterator<char>(stream), {});
    }

    fs::path directory;
    std::string path;
};

TEST_F(CoreLogging, RoutesHelperAndTransportDiagnostics)
{
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_configureLogging(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_ERROR, path.c_str(), 1024 * 1024, 2));
    VSCPHLP_LOG_DEBUG("filtered {}", 123);
    VSCPHLP_LOG_ERROR("literal {{}} {}", 7);

    vscpEventEx event{};
    std::string invalidJson = "{";
    EXPECT_FALSE(vscp_convertJSONToEventEx(&event, invalidJson));

    VscpRemoteTcpIf client;
    client.setTLSOptions(true, (directory / "missing.pem").string(),
                         "", "", "", "private-key-password");
    EXPECT_EQ(VSCP_ERROR_CONNECTION,
              client.doCmdOpen("stcp://127.0.0.1:1", "private-user", "private-password"));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());

    const std::string output = read();
    EXPECT_NE(std::string::npos, output.find("literal {} 7"));
    EXPECT_NE(std::string::npos, output.find("VSCP helper: Parse error"));
    EXPECT_NE(std::string::npos, output.find("VSCP TCP: cannot read TLS file:"));
    EXPECT_EQ(std::string::npos, output.find("filtered 123"));
    EXPECT_EQ(std::string::npos, output.find("private-user"));
    EXPECT_EQ(std::string::npos, output.find("private-password"));
    EXPECT_EQ(std::string::npos, output.find("private-key-password"));
}

TEST_F(CoreLogging, DoesNotChangeHostDefaultLogger)
{
    struct RestoreDefault {
        std::shared_ptr<spdlog::logger> original = spdlog::default_logger();
        ~RestoreDefault() { spdlog::set_default_logger(original); }
    } restore;

    std::ostringstream hostOutput;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(hostOutput);
    auto host = std::make_shared<spdlog::logger>("host-default", sink);
    host->set_level(spdlog::level::trace);
    spdlog::set_default_logger(host);

    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_configureLogging(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_ERROR, path.c_str(), 4096, 1));
    VSCPHLP_LOG_ERROR("VSCP diagnostic");
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());
    EXPECT_NE(std::string::npos, read().find("VSCP diagnostic"));
    EXPECT_EQ(host, spdlog::default_logger());
    EXPECT_EQ(spdlog::level::trace, host->level());
    EXPECT_TRUE(hostOutput.str().empty());
    EXPECT_EQ(nullptr, spdlog::get("vscp-helper"));
}

TEST_F(CoreLogging, InvalidConfigurationRetainsLogger)
{
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_configureLogging(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_ERROR, path.c_str(), 4096, 1));
    EXPECT_EQ(VSCP_ERROR_PARAMETER, vscphelper_configureLogging(
        VSCP_HELPER_LOG_LEVEL_OFF, 7, path.c_str(), 4096, 1));
    EXPECT_NE('\0', *vscphelper_getLastLoggingError());
    VSCPHLP_LOG_ERROR("retained");
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());
    EXPECT_NE(std::string::npos, read().find("retained"));
    EXPECT_EQ('\0', *vscphelper_getLastLoggingError());
}

} // namespace
