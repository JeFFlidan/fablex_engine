#pragma once

#include "fmt/base.h"
#include "fmt/color.h"
#include "fmt/core.h"

#include <array>
#include <random>
#include <string_view>

enum FELogType
{
    INFO,
    SUCCESS,
    WARNING,
    ERROR,
    FATAL
};

#define DEFINE_LOG_CATEGORY(CategoryName)                  \
struct CategoryName                                        \
{                                                          \
    constexpr static const char* name = #CategoryName;     \
    inline static fmt::color color = fmt::color::black;    \
};

DEFINE_LOG_CATEGORY(LogDefault)

#define FE_LOG(LogCategory, LogType, Message, ...) FELogger::log<LogCategory, LogType>(Message, ##__VA_ARGS__)

class FELogger
{
public:
    template<typename LogCategory, FELogType LogType, typename... Params>
    static void log(std::string_view msg, Params... params)
    {
        set_log_category_color<LogCategory>();
        fmt::print(fg(LogCategory::color), "[{}]", LogCategory::name);

        const LogTypeInfo& logTypeInfo = s_logTypeInfos[LogType];
        fmt::print(fg(logTypeInfo.color), "[{}]: ", logTypeInfo.strName);
        fmt::print(fg(logTypeInfo.color), fmt::runtime(msg), params...);
        fmt::print("\n");

        if (LogType == FELogType::FATAL)
            abort();
    }

private:
    constexpr static std::array<fmt::color, 18> s_logCategoryColors = {
        fmt::color::aqua,
        fmt::color::aquamarine,
        fmt::color::blue_violet,
        fmt::color::burly_wood,
        fmt::color::cadet_blue,
        fmt::color::chartreuse,
        fmt::color::coral,
        fmt::color::cornflower_blue,
        fmt::color::crimson,
        fmt::color::cyan,
        fmt::color::dark_salmon,
        fmt::color::dark_sea_green,
        fmt::color::deep_sky_blue,
        fmt::color::light_sea_green,
        fmt::color::medium_orchid,
        fmt::color::plum,
        fmt::color::tomato,
        fmt::color::yellow_green
    };

    struct LogTypeInfo
    {
        fmt::color color;
        const char* strName;
    };

    constexpr static std::array<LogTypeInfo, 5> s_logTypeInfos = {{
        {fmt::color::white_smoke, "INFO"},
        {fmt::color::lime_green, "SUCCESS"},
        {fmt::color::orchid, "WARNING"},
        {fmt::color::red, "ERROR"},
        {fmt::color::dark_red, "FATAL"}
    }};

    template<typename LogCategory>
    static void set_log_category_color()
    {
        if (LogCategory::color != fmt::color::black)
        {
            return;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, s_logCategoryColors.size() - 1);

        LogCategory::color = s_logCategoryColors[dist(gen)];
    }
};
