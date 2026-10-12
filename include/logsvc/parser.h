#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>

namespace logsvc {

enum class Level {
    Info,
    Warn,
    Error
};

struct LogRecord {
    Level level;
    std::string service;
    std::uint32_t latency_ms;
};

enum class ParseErrorCode {
    EmptyLine,
    WrongFieldCount,
    InvalidLevel,
    InvalidService,
    InvalidLatency,
    LatencyOutOfRange
};

struct ParseError {
    ParseErrorCode code;
    std::string message;
};

using ParseResult = std::variant<LogRecord, ParseError>;

ParseResult parse_line(std::string_view line);

} // namespace logsvc
