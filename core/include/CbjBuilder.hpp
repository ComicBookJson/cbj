#pragma once
#include <cstdint>
#include <string>
#include "CbjSchema.hpp"
namespace cbj
{
    class CbjBuilder
    {
    public:
        CbjBuilder();
        CbjBuilder &SetVersion(const std::string &);
        CbjBuilder &SetMetadata(const std::string &title, const std::string &series);
        CbjBuilder &SetIssue(const std::string &);
        CbjBuilder &SetVolume(std::int64_t);
        CbjBuilder &SetPublisher(const std::string &);
        CbjBuilder &SetLanguage(const std::string &);
        CbjBuilder &SetSummary(const std::string &);
        CbjBuilder &AddGenre(const std::string &);
        CbjBuilder &AddCreator(const std::string &, const std::string &);
        CbjBuilder &AddTag(const std::string &, const std::string &, const std::string &);
        CbjBuilder &AddChapter(const std::string &, std::int64_t, const std::string &summary = "");
        CbjBuilder &AddPageBase64(const std::string &, const std::string &pageType = "Story", const std::string &summary = "");
        CbjBuilder &AddPageBytes(const std::string &, const std::string &pageType = "Story", const std::string &summary = "");
        Document Build() const;
        bool Save(const std::string &) const;

    private:
        Document document_;
    };
}