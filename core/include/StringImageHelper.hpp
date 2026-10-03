#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace cbj
{
    /** Utility functions for Base64 and binary image file conversion. */
    class StringImageHelper
    {
    public:
        /** Encodes binary bytes as RFC 4648 Base64 text. */
        static std::string EncodeBase64(const std::string &bytes);
        /** Encodes a byte vector as RFC 4648 Base64 text. */
        static std::string EncodeBase64(const std::vector<std::uint8_t> &bytes);
        /** Decodes Base64 text and accepts optional data-URI prefixes. */
        static std::string DecodeBase64(const std::string &value);
        /** Decodes Base64 text into a byte vector. */
        static std::vector<std::uint8_t> DecodeBase64Bytes(const std::string &value);
        /** Reads an entire binary file into a string. */
        static std::string ReadFile(const std::string &path);
        /** Writes binary bytes to a file, returning false on I/O failure. */
        static bool WriteFile(const std::string &path, const std::string &bytes);
    };
}
