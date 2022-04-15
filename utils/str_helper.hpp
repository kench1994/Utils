#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/x509.h>
#include "unique_array.hpp"
namespace utils
{
    inline std::string to_lower_copy(const std::string& origin_val)
    {
        std::string result_val = origin_val;
        std::transform(result_val.begin(), result_val.end(), result_val.begin(), ::tolower);
        return result_val;
    }


    inline std::string to_lower_move(std::string&& origin_val)
    {
        std::transform(origin_val.begin(), origin_val.end(), origin_val.begin(), ::tolower);
        std::string result_val = origin_val;
        return result_val;
    }

    inline void to_lower(std::string& r_val)
    {
        std::transform(r_val.begin(), r_val.end(), r_val.begin(), ::tolower);
    }

    inline void to_upper(std::string& r_val)
    {
        std::transform(r_val.begin(), r_val.end(), r_val.begin(), ::toupper);
    }

    inline void str_erase(std::string &str_source, const std::string &str_erase)
    {
        for (;;)
        {
            if (str_source.size() <= 0)
                return;
            int nFind = str_source.find(str_erase);
            if (-1 != nFind)
                str_source.erase(nFind, str_erase.size());
            else
                break;
        }
    }

    inline std::string str_erase(const std::string &str_source, const std::string &str_erase)
    {
        std::string str_copy = str_source;
        for (;;)
        {
            if (str_copy.size() <= 0)
                return str_copy;
            int nFind = str_copy.find(str_erase);
            if (-1 != nFind)
                str_copy.erase(nFind, str_erase.size());
            else
                break;
        }
        return str_copy;
    }

    inline void replace_all(std::string &str, const std::string &from, const std::string &to)
    {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    inline bool check_prefix(const std::string& source, const std::string& prefix)
    {
        if (0 == source.find(prefix))
            return true;
        return false;
    }

    inline bool check_suffix(const std::string& source, const std::string& suffix)
    {
        auto pos = source.find(suffix);
        if (std::string::npos == pos)
            return false;

        if (pos == source.size() - suffix.size())
            return true;

        return false;
    }

    // base64 编码
    inline std::string base64Encode(const char* buffer, unsigned int length, bool line_feed = false)
    {
        BIO *bmem = NULL, *b64 = NULL;
        BUF_MEM* bptr;

        b64 = BIO_new(BIO_f_base64());
        if (!line_feed) {
            BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        }
        bmem = BIO_new(BIO_s_mem());
        b64 = BIO_push(b64, bmem);
        BIO_write(b64, buffer, length);
        BIO_flush(b64);
        BIO_get_mem_ptr(b64, &bptr);
        BIO_set_close(b64, BIO_NOCLOSE);

        std::string strResult(bptr->data, bptr->length);
        BIO_free_all(b64);

        return strResult;
    }

    inline std::string base64Decode(const char* input, int length, bool line_feed = false)
    {
        BIO* b64 = NULL;
        BIO* bmem = NULL;
        utils::unique_array<char> spszBuffer(new char[length]);
        memset(spszBuffer.get(), 0, length);
        b64 = BIO_new(BIO_f_base64());
        if (!line_feed) {
            BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        }
        bmem = BIO_new_mem_buf(input, length);
        bmem = BIO_push(b64, bmem);
        BIO_read(bmem, spszBuffer.get(), length);
        BIO_free_all(bmem);

        return std::string(spszBuffer.get(), length);
    }

    inline std::string ToHexString(const char* buf, unsigned int len, const char* pszTok = "0x")
    {
        std::stringstream ss;
        for (unsigned int i = 0; i < len; ++i)
        {
            if (pszTok)
                ss << pszTok;
            ss << std::setw(2) << std::setfill('0') << std::hex << (int)(buf[i] & 0xff);

            if (pszTok)
                ss << " ";
        }
        return ss.str();
    }
}