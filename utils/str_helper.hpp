#include <string>
#include <sstream>
#include <iomanip>
#include <openssl/x509.h>
#include "unique_array.hpp"
namespace utils
{
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