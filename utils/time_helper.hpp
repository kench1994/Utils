#pragma once
#include <ctime>
#include <chrono>
#include <string>

namespace utils
{
    namespace time_helper
    {
		constexpr uint64_t DayMillseconds = 24 * 60 * 60 * 1000;

        /* Returns microseconds since epoch */
        inline uint64_t timestamp_now()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

		inline uint64_t timestamp_today()
		{
			time_t now = time(0);
			struct tm tm_now;
#ifdef _WIN32
			localtime_s(&tm_now, &now);
#else
			localtime_r(&now, &tm_now);
#endif
			tm_now.tm_hour = 0;
			tm_now.tm_min = 0;
			tm_now.tm_sec = 0;
			auto tp = std::chrono::system_clock::from_time_t(mktime(&tm_now));
			return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
		}

		//todo: show microseconds
		inline std::string GetDateTime(const char* pszFormat = "%Y%m%d")
		{
			time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			struct tm tm_now;
#ifdef _WIN32
			localtime_s(&tm_now, &now);
#else
			localtime_r(&now, &tm_now);
#endif
			char szTimeBuf[32]{'\0'};
			std::strftime(szTimeBuf, sizeof(szTimeBuf), pszFormat, &tm_now);
			return szTimeBuf;
		}

		inline std::string Stamp2TimeString(uint64_t timestamp, const char* pszFormat = "%Y-%m-%d %H:%M:%S", bool bSuffixMillseconds = true)
		{
			char s[40]{ '\0' };

			time_t tick = static_cast<time_t>(timestamp / 1000);//转换时间
			struct tm stTm;
#ifdef _WIN32
			localtime_s(&stTm, &tick);
#else
			localtime_r(&tick, &stTm);
#endif
			std::strftime(s, sizeof(s), pszFormat, &stTm);
			if (bSuffixMillseconds)
			{
				int ms = timestamp % 1000;//取毫秒
				auto shift = strlen(s);
				sprintf_s(s + shift, sizeof(s) - shift - 1, ".%03d", ms);
			}
			return s;
		}
    }
}