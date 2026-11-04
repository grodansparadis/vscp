#include <cerrno>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <gtest/gtest.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include <vscpremotetcpif.h>
#include <vscphelper.h>
#include <vscphelperlogging.h>

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
    std::string invalidJson = "{\"private-event-payload\":";
    EXPECT_FALSE(vscp_convertJSONToEventEx(&event, invalidJson));
    vscpEvent legacyEvent{};
    vscp_convertJSONToEvent(&legacyEvent, invalidJson);

    VscpRemoteTcpIf client;
    client.setTLSOptions(true, (directory / "missing.pem").string(),
                         "", "", "", "private-key-password");
    EXPECT_EQ(VSCP_ERROR_CONNECTION,
              client.doCmdOpen("stcp://127.0.0.1:1", "private-user", "private-password"));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());

    const std::string output = read();
    EXPECT_NE(std::string::npos, output.find("literal {} 7"));
    EXPECT_NE(std::string::npos, output.find("VSCP helper: vscp_convertJSONToEventEx failed"));
    EXPECT_NE(std::string::npos, output.find("VSCP helper: vscp_convertJSONToEvent failed"));
    EXPECT_NE(std::string::npos, output.find("VSCP TCP: cannot read TLS file:"));
    EXPECT_EQ(std::string::npos, output.find("filtered 123"));
    EXPECT_EQ(std::string::npos, output.find("private-user"));
    EXPECT_EQ(std::string::npos, output.find("private-password"));
    EXPECT_EQ(std::string::npos, output.find("private-key-password"));
    EXPECT_EQ(std::string::npos, output.find("private-event-payload"));
}

TEST_F(CoreLogging, TracesFailuresButNotNormalPredicateResults)
{
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_configureLogging(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_DEBUG, path.c_str(), 1024 * 1024, 2));

    std::string value = "MIXED";
    errno = EDOM;
    vscp_makeLower(value);
    int savedError = errno;
    EXPECT_EQ("mixed", value);
    EXPECT_EQ(EDOM, savedError);

    uint8_t first[16] = {1};
    uint8_t second[16] = {2};
    EXPECT_FALSE(vscp_startsWith("event", "other"));
    EXPECT_FALSE(vscp_isGUIDEmpty(first));
    EXPECT_FALSE(vscp_isSameGUID(first, second));
    vscpEvent event{};
    vscpEventFilter filter{};
    filter.filter_class = 1;
    filter.mask_class = 0xffff;
    EXPECT_FALSE(vscp_isMeasurement(&event));
    EXPECT_FALSE(vscp_doLevel2Filter(&event, &filter));

    errno = ERANGE;
    std::string json;
    bool converted = vscp_convertEventToJSON(json, nullptr);
    savedError = errno;
    EXPECT_FALSE(converted);
    EXPECT_EQ(ERANGE, savedError);
    EXPECT_EQ(VSCP_ERROR_PARAMETER, vscp_writeCommandToFrame(nullptr, 0, 0, nullptr, 0));
    EXPECT_EQ(0, vscp_readStringValue("0"));
    EXPECT_EQ(0, vscp_readStringValue("private-invalid-number"));
    uint8_t zeroValue[] = {0, 0};
    EXPECT_EQ(0, vscp_getDataCodingInteger(zeroValue, sizeof(zeroValue)));
    EXPECT_EQ(0, vscp_getDataCodingInteger(nullptr, sizeof(zeroValue)));
    uint32_t network = 0;
    uint32_t mask = 0;
    EXPECT_GT(vscp_parse_ipv4_addr("127.0.0.1", &network, &mask), 0);
    EXPECT_EQ(0, vscp_parse_ipv4_addr("private-invalid-address", &network, &mask));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());

    const std::string output = read();
    EXPECT_NE(std::string::npos, output.find("[debug] VSCP helper: vscp_makeLower: entered"));
    EXPECT_NE(std::string::npos, output.find("[error] VSCP helper: vscp_convertEventToJSON failed"));
    EXPECT_NE(std::string::npos, output.find("[error] VSCP helper: vscp_writeCommandToFrame failed"));
    EXPECT_NE(std::string::npos, output.find("[error] VSCP helper: vscp_readStringValue failed"));
    const std::string integerFailure = "[error] VSCP helper: vscp_getDataCodingInteger failed";
    const size_t integerFailurePos = output.find(integerFailure);
    ASSERT_NE(std::string::npos, integerFailurePos);
    EXPECT_EQ(std::string::npos, output.find(integerFailure, integerFailurePos + 1));
    EXPECT_NE(std::string::npos, output.find("[error] VSCP helper: vscp_parse_ipv4_addr failed"));
    EXPECT_EQ(std::string::npos, output.find("vscp_startsWith failed"));
    EXPECT_EQ(std::string::npos, output.find("vscp_isGUIDEmpty failed"));
    EXPECT_EQ(std::string::npos, output.find("vscp_isSameGUID failed"));
    EXPECT_EQ(std::string::npos, output.find("vscp_isMeasurement failed"));
    EXPECT_EQ(std::string::npos, output.find("vscp_doLevel2Filter failed"));
    EXPECT_EQ(std::string::npos, output.find("private-invalid-number"));
    EXPECT_EQ(std::string::npos, output.find("private-invalid-address"));
}

TEST_F(CoreLogging, DisabledLoggerSuppressesDebugAndErrors)
{
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_configureLogging(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_DEBUG, path.c_str(), 1024 * 1024, 2));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_disableLogging());
    std::string value = "MIXED";
    vscp_makeLower(value);
    std::string json;
    EXPECT_FALSE(vscp_convertEventToJSON(json, nullptr));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());
    EXPECT_TRUE(read().empty());
}

#ifdef _WIN32
TEST_F(CoreLogging, FunctionDiagnosticsPreserveWindowsErrorState)
{
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_configureLogging(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_DEBUG, path.c_str(), 1024 * 1024, 2));
    WSASetLastError(WSAEACCES);
    DWORD windowsError = GetLastError();
    vscphelper_logFunctionEntry("helper", "error_state_test");
    int savedSocketError = WSAGetLastError();
    DWORD savedWindowsError = GetLastError();
    EXPECT_EQ(WSAEACCES, savedSocketError);
    EXPECT_EQ(windowsError, savedWindowsError);

    WSASetLastError(WSAEINVAL);
    windowsError = GetLastError();
    vscphelper_logFunctionFailure("helper", "error_state_test", 1);
    savedSocketError = WSAGetLastError();
    savedWindowsError = GetLastError();
    EXPECT_EQ(WSAEINVAL, savedSocketError);
    EXPECT_EQ(windowsError, savedWindowsError);
}
#endif

TEST_F(CoreLogging, SemaphoreTimeoutIsDebugAndPreservesErrorState)
{
#if defined(__APPLE__)
    GTEST_SKIP() << "Unnamed POSIX semaphores are unavailable on macOS";
#else
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_configureLogging(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_DEBUG, path.c_str(), 1024 * 1024, 2));
#ifdef _WIN32
    HANDLE semaphore = CreateSemaphoreA(nullptr, 0, 1, nullptr);
    ASSERT_NE(nullptr, semaphore);
    int result = vscp_sem_wait(&semaphore, 10);
    EXPECT_EQ(ETIMEDOUT, result);
    EXPECT_TRUE(CloseHandle(semaphore));
#else
    sem_t semaphore;
    ASSERT_EQ(0, sem_init(&semaphore, 0, 0));
    errno = 0;
    int result = vscp_sem_wait(&semaphore, 10);
    int savedError = errno;
    EXPECT_EQ(-1, result);
    EXPECT_EQ(ETIMEDOUT, savedError);
    EXPECT_EQ(0, sem_destroy(&semaphore));
#endif
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());
    const std::string output = read();
    EXPECT_NE(std::string::npos,
              output.find("[debug] VSCP helper: vscp_sem_wait returned " + std::to_string(ETIMEDOUT)));
    EXPECT_EQ(std::string::npos, output.find("[error] VSCP helper: vscp_sem_wait"));
#endif
}

#ifndef _WIN32
TEST_F(CoreLogging, UtilAndCoreExportsShareLoggingConfiguration)
{
    struct CloseModule {
        void operator()(void *module) const { dlclose(module); }
    };
    using Module = std::unique_ptr<void, CloseModule>;
    Module util(dlopen(VSCP_TEST_UTIL_LIBRARY, RTLD_NOW | RTLD_LOCAL));
    ASSERT_NE(nullptr, util) << dlerror();
    Module core(dlopen(VSCP_TEST_CORE_LIBRARY, RTLD_NOW | RTLD_LOCAL));
    ASSERT_NE(nullptr, core) << dlerror();

    auto utilConfigure = reinterpret_cast<decltype(&vscphelper_configureLogging)>(
        dlsym(util.get(), "vscphelper_configureLogging"));
    auto coreConfigure = reinterpret_cast<decltype(&vscphelper_configureLogging)>(
        dlsym(core.get(), "vscphelper_configureLogging"));
    auto utilFlush = reinterpret_cast<decltype(&vscphelper_flushLogging)>(
        dlsym(util.get(), "vscphelper_flushLogging"));
    auto coreFlush = reinterpret_cast<decltype(&vscphelper_flushLogging)>(
        dlsym(core.get(), "vscphelper_flushLogging"));
    ASSERT_NE(nullptr, utilConfigure);
    ASSERT_NE(nullptr, coreConfigure);
    ASSERT_NE(nullptr, utilFlush);
    ASSERT_NE(nullptr, coreFlush);

    ASSERT_EQ(VSCP_ERROR_SUCCESS, utilConfigure(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_DEBUG, path.c_str(), 1024 * 1024, 2));
    vscpEventEx event{};
    std::string invalidJson = "{";
    EXPECT_FALSE(vscp_convertJSONToEventEx(&event, invalidJson));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, coreFlush());
    EXPECT_NE(std::string::npos, read().find("vscp_convertJSONToEventEx failed"));

    ASSERT_EQ(VSCP_ERROR_SUCCESS, coreConfigure(
        VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_DEBUG, path.c_str(), 1024 * 1024, 2));
    EXPECT_EQ(16, vscp_readStringValue("0x10"));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, utilFlush());
    EXPECT_NE(std::string::npos, read().find("[debug] VSCP helper: vscp_readStringValue: entered"));
}
#endif

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
