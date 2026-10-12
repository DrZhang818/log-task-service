#include "logsvc/parser.h"

#include <iostream>
#include <string>
#include <string_view>
#include <variant>

namespace {

bool expect_record(std::string_view input,
                   const logsvc::LogRecord& expected) {
    const auto result = logsvc::parse_line(input);
    const auto* record = std::get_if<logsvc::LogRecord>(&result);

    if(record &&
       record->level == expected.level &&
       record->service == expected.service &&
       record->latency_ms == expected.latency_ms) {
        return true;
    }

    std::cerr << "Record mismatch: [" << input << "]\n";
    return false;
}

bool expect_error(std::string_view input,
                  const logsvc::ParseErrorCode& expected) {
    const auto result = logsvc::parse_line(input);
    const auto* error = std::get_if<logsvc::ParseError>(&result);

    if(error && error->code == expected && !error->message.empty()) {
        return true;
    }

    std::cerr << "Error mismatch: [" << input << "]\n";
    return false;
}

} // namespace

int main() {
    using logsvc::Level;
    using E = logsvc::ParseErrorCode;

    int failed = 0;

    failed += !expect_record("INFO api 12", {Level::Info, "api", 12});
    failed += !expect_record("WARN worker 35", {Level::Warn, "worker", 35});
    failed += !expect_record("ERROR db 0", {Level::Error, "db", 0});
    failed += !expect_record("INFO api 60000", {Level::Info, "api", 60000});
    failed += !expect_record("INFO api 00012", {Level::Info, "api", 12});
    failed += !expect_record(" \tINFO\tapi_1-x\t12\r",
                            {Level::Info, "api_1-x", 12});

    const std::string service32(32, 'a');
    failed += !expect_record("INFO " + service32 + " 12",
                            {Level::Info, service32, 12});

    failed += !expect_error("", E::EmptyLine);
    failed += !expect_error(" \t\r ", E::EmptyLine);
    failed += !expect_error("INFO api", E::WrongFieldCount);
    failed += !expect_error("INFO api 12 extra", E::WrongFieldCount);
    failed += !expect_error("Info api 12", E::InvalidLevel);
    failed += !expect_error("INFO api! 12", E::InvalidService);
    failed += !expect_error("INFO " + std::string(33, 'a') + " 12",
                           E::InvalidService);
    failed += !expect_error("INFO api -1", E::InvalidLatency);
    failed += !expect_error("INFO api +1", E::InvalidLatency);
    failed += !expect_error("INFO api 12ms", E::InvalidLatency);
    failed += !expect_error("INFO api 1.5", E::InvalidLatency);
    failed += !expect_error("INFO api 60001", E::LatencyOutOfRange);
    failed += !expect_error("INFO api 4294967296", E::LatencyOutOfRange);

    std::string input = "INFO api 12";
    const auto result = logsvc::parse_line(input);
    input.assign(input.size(), 'x');

    const auto* record = std::get_if<logsvc::LogRecord>(&result);
    if (!record || record->service != "api") {
        std::cerr << "Returned record does not own its service\n";
        ++failed;
    }

    if (failed != 0) {
        std::cerr << failed << " check(s) failed\n";
        return 1;
    }

    std::cout << "All parser checks passed\n";
    return 0;
}
