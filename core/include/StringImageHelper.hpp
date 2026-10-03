#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace cbjz
{
    class StringImageHelper
    {
    public:
        static std::string EncodeBase64(const std::string &bytes);
        static std::string EncodeBase64(const std::vector<std::uint8_t> &bytes);
        static std::string DecodeBase64(const std::string &value);
        static std::vector<std::uint8_t> DecodeBase64Bytes(const std::string &value);
        static std::string ReadFile(const std::string &path);
        static bool WriteFile(const std::string &path, const std::string &bytes);
    };
}