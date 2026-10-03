#include "CbjBuilder.hpp"
#include "StringImageHelper.hpp"
#include <fstream>
namespace cbjz
{
    CbjBuilder::CbjBuilder() { document_.set_version("1.0"); }
    CbjBuilder &CbjBuilder::SetVersion(const std::string &v)
    {
        document_.set_version(v);
        return *this;
    }
    CbjBuilder &CbjBuilder::SetMetadata(const std::string &t, const std::string &s)
    {
        auto &m = document_.get_mutable_metadata();
        m.set_title(t);
        m.set_series(s);
        return *this;
    }
    CbjBuilder &CbjBuilder::SetIssue(const std::string &v)
    {
        document_.get_mutable_metadata().set_issue(v);
        return *this;
    }
    CbjBuilder &CbjBuilder::SetVolume(std::int64_t v)
    {
        document_.get_mutable_metadata().set_volume(v);
        return *this;
    }
    CbjBuilder &CbjBuilder::SetPublisher(const std::string &v)
    {
        document_.get_mutable_metadata().set_publisher(v);
        return *this;
    }
    CbjBuilder &CbjBuilder::SetLanguage(const std::string &v)
    {
        document_.get_mutable_metadata().set_language(v);
        return *this;
    }
    CbjBuilder &CbjBuilder::SetSummary(const std::string &v)
    {
        document_.get_mutable_metadata().set_summary(v);
        return *this;
    }
    CbjBuilder &CbjBuilder::AddGenre(const std::string &v)
    {
        auto &m = document_.get_mutable_metadata();
        if (!m.get_mutable_genres())
            m.set_genres(std::vector<std::string>{});
        m.get_mutable_genres()->push_back(v);
        return *this;
    }
    CbjBuilder &CbjBuilder::AddCreator(const std::string &n, const std::string &r)
    {
        auto &m = document_.get_mutable_metadata();
        if (!m.get_mutable_creators())
            m.set_creators(std::vector<CreatorElement>{});
        CreatorElement c;
        c.set_name(n);
        c.set_role(r);
        m.get_mutable_creators()->push_back(c);
        return *this;
    }
    CbjBuilder &CbjBuilder::AddTag(const std::string &i, const std::string &n, Type t)
    {
        auto &m = document_.get_mutable_metadata();
        if (!m.get_mutable_tags())
            m.set_tags(std::vector<TagElement>{});
        TagElement x;
        x.set_id(i);
        x.set_name(n);
        x.set_type(t);
        m.get_mutable_tags()->push_back(x);
        return *this;
    }
    CbjBuilder &CbjBuilder::AddChapter(const std::string &t, std::int64_t i, const std::string &s)
    {
        auto &m = document_.get_mutable_metadata();
        if (!m.get_mutable_chapters())
            m.set_chapters(std::vector<ChapterElement>{});
        ChapterElement c;
        c.set_title(t);
        c.set_start_page_index(i);
        if (!s.empty())
            c.set_summary(s);
        m.get_mutable_chapters()->push_back(c);
        return *this;
    }
    CbjBuilder &CbjBuilder::AddPageBase64(const std::string &b, const std::string &t, const std::string &s)
    {
        PageElement p;
        p.set_page_index(document_.get_pages().size());
        p.set_base64_image(b);
        if (!t.empty())
            p.set_page_type(t);
        if (!s.empty())
            p.set_summary(s);
        document_.get_mutable_pages().push_back(p);
        return *this;
    }
    CbjBuilder &CbjBuilder::AddPageBytes(const std::string &b, const std::string &t, const std::string &s) { return AddPageBase64(StringImageHelper::EncodeBase64(b), t, s); }
    CbjzV1Schema CbjBuilder::Build() const { return document_; }
    bool CbjBuilder::Save(const std::string &p) const
    {
        std::ofstream f(p, std::ios::binary | std::ios::trunc);
        if (!f)
            return false;
        nlohmann::json j = document_;
        f << j.dump();
        return bool(f);
    }
}