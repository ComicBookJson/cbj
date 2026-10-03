#pragma once
#include <string>
#include <vector>
#include "CbjzSchema.hpp"
namespace cbjz
{
    class Cbj
    {
    public:
        enum class ViewMode
        {
            Single = 1,
            Desktop = 2,
            Mobile = 3
        };
        Cbj();
        ~Cbj();
        Cbj(const Cbj &) = delete;
        Cbj &operator=(const Cbj &) = delete;
        bool Open(const std::string &);
        bool OpenCbjz(const std::string &);
        bool ImportFromCbz(const std::string &);
        bool ImportFromCbr(const std::string &);
        bool ImportFromCb7(const std::string &);
        bool ImportFromCbt(const std::string &);
        bool ImportFromPdf(const std::string &);
        bool SaveAsCbjz(const std::string &);
        std::vector<Page> GetPages(int startIndex, int count);
        std::vector<Page> GetPagesForView(int currentIndex, ViewMode);
        Page GetPage(int index);
        void ClearCache();
        void SetCachePages(int);
        int GetCachePages() const;
        Metadata GetMetadata() const;
        void SetMetadata(const Metadata &);
        void UpdatePage(int, const PageElement &);
        int GetTotalPages() const;
        std::string GetVersion() const;
        bool IsOpen() const;

    private:
        class Impl;
        Impl *pImpl;
    };
}