#include "logsvc/parser.h"

#include <array>
#include <charconv>
#include <cstddef>
#include <optional>
#include <system_error>

namespace logsvc {
namespace {

constexpr std::size_t kMaxServiceLength = 32;
constexpr std::uint32_t kMaxLatencyMs = 60'000;

using Fields = std::array<std::string_view, 3>;
using FieldsResult = std::variant<Fields, ParseError>;

constexpr bool is_ascii_space(char ch) noexcept {
    return ch == ' ' || ch == '\t' || ch == '\r' ||
           ch == '\n' || ch == '\f' || ch == '\v';
}

FieldsResult split_fields(std::string_view line) {
    Fields field{};
    std::size_t count = 0;
    std::size_t pos = 0;

    while(pos < line.size()) {
        while(pos < line.size() && is_ascii_space(line[pos])) {
            pos++;
        }
        if(pos == line.size()) {
            break;
        }
        if(count == field.size()) {
            return ParseError{
                ParseErrorCode::WrongFieldCount,
                "Expected exactly three fields: Level, Service, Latency"
            };
        }
        const std::size_t begin = pos;
        while(pos < line.size() && !is_ascii_space(line[pos])) {
            pos++;
        }
        field[count++] = line.substr(begin, pos - begin);
    }

    if(count == 0) {
        return ParseError{
            ParseErrorCode::EmptyLine,
            "Line is empty or contain only whitespace"
        };
    }
    if(count != field.size()) {
        return ParseError{
            ParseErrorCode::WrongFieldCount,
            "Expected exactly three fields: Level, Service, Latency"
        };
    }

    return field;
}

std::optional<Level> parse_level(std::string_view text) noexcept {
    if(text == "INFO") return Level::Info;
    if(text == "WARN") return Level::Warn;
    if(text == "ERROR") return Level::Error;
    return std::nullopt;
}

bool is_vaild_service(std::string_view service) noexcept {
    if(service.empty() || service.size() > kMaxServiceLength) {
        return false;
    }
    for(char ch : service) {
        const bool allowed =
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '-' ||
            ch == '_';

        if(!allowed) return false;
    }
    return true;
}

} // namespace

ParseResult parse_line(std::string_view line) {
    const auto fields_result = split_fields(line);
    if(const auto* error = std::get_if<ParseError>(&fields_result)) {
        return *error;
    }
    const auto& field = std::get<Fields>(fields_result);

    const auto level = parse_level(field[0]);
    if(!level) {
        return ParseError{
            ParseErrorCode::InvalidLevel,
            "Level must be INFO, WARN, or ERROR"
        };
    }

    if(!is_vaild_service(field[1])) {
        return ParseError{
            ParseErrorCode::InvalidService,
            "Service must contain 1-32 ASCII letters, digits, '_' or '-'"
        };
    }

    const std::string_view latency_text = field[2];
    for(char ch : latency_text) {
        if(ch < '0' || ch > '9') {
            return ParseError{
                ParseErrorCode::InvalidLatency,
                "Latency must contain only decimal digits"
            };
        }
    }

    std::uint32_t latency_ms = 0;
    const char* first = latency_text.data();
    const char* last = first + latency_text.size();

    const auto [ptr, ec] = std::from_chars(first, last, latency_ms, 10);

    if(ec == std::errc::result_out_of_range) {
        return ParseError{
            ParseErrorCode::LatencyOutOfRange,
            "Latency must be in the range [0, 60000]"
        };
    }

    if(ec != std::errc{} || ptr != last) {
        return ParseError{
            ParseErrorCode::InvalidLatency,
            "Latency is not a valid decimal integer"
        };
    }

    if(latency_ms > kMaxLatencyMs) {
        return ParseError{
            ParseErrorCode::LatencyOutOfRange,
            "Latency must be in the range [0, 60000]"
        };
    }

    return LogRecord{
        *level,
        std::string{field[1]},
        latency_ms
    };
}

} // namespace logsvc
