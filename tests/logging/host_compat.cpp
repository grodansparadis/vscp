#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>
#include <spdlog/version.h>
#include <vscphelper.h>
#include <vscphelperlogging.h>

namespace fs = std::filesystem;

int main()
{
    const fs::path directory = fs::temp_directory_path() /
        ("vscp-core-host-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    if (!fs::create_directory(directory)) {
        std::cerr << "Cannot create host logging test directory\n";
        return 1;
    }
    const std::string path = (directory / "core.log").string();
    std::ostringstream hostOutput;
    auto original = spdlog::default_logger();
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(hostOutput);
    auto host = std::make_shared<spdlog::logger>("host-default", sink);
    spdlog::set_default_logger(host);

    bool success = vscphelper_configureLogging(VSCP_HELPER_LOG_LEVEL_OFF,
                                                VSCP_HELPER_LOG_LEVEL_ERROR,
                                                path.c_str(), 4096, 1) == VSCP_ERROR_SUCCESS;
    if (success) {
        vscpEventEx event{};
        std::string invalidJson = "{";
        success = !vscp_convertJSONToEventEx(&event, invalidJson);
        success = vscphelper_flushLogging() == VSCP_ERROR_SUCCESS && success;
        std::ifstream stream(path);
        const std::string output{std::istreambuf_iterator<char>(stream), {}};
        host->info("host message");
        success = output.find("VSCP helper: Parse error") != std::string::npos && success;
        success = output.find("host message") == std::string::npos && success;
        success = hostOutput.str().find("host message") != std::string::npos && success;
        success = spdlog::default_logger() == host && spdlog::get("vscp-helper") == nullptr && success;
    }

    success = vscphelper_disableLogging() == VSCP_ERROR_SUCCESS && success;
    spdlog::set_default_logger(original);
    fs::remove_all(directory);
    if (!success) {
        std::cerr << "Host spdlog compatibility check failed\n";
        return 1;
    }
    std::cout << "Host spdlog " << SPDLOG_VER_MAJOR << '.' << SPDLOG_VER_MINOR << '.'
              << SPDLOG_VER_PATCH << " remains independent of VSCP logging\n";
    return 0;
}
