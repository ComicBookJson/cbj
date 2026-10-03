#include "CbjBuilder.hpp"
#include "StringImageHelper.hpp"
#include "CbjJsonStream.hpp"
#include "CbjLog.hpp"
#include "miniz.h"
#include <fstream>
#include <filesystem>
namespace cbj
{
    CbjBuilder::CbjBuilder() 
    { 
        document_.SetVersion("1.0"); 
    }

    CbjBuilder &CbjBuilder::SetVersion(const std::string &v)
    {
        document_.SetVersion(v);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::SetMetadata(const std::string &t, const std::string &s)
    {
        auto &m = document_.MutableMetadata();
        m.SetTitle(t);
        m.SetSeries(s);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::SetIssue(const std::string &v)
    {
        document_.MutableMetadata().SetIssue(v);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::SetVolume(std::int64_t v)
    {
        document_.MutableMetadata().SetVolume(v);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::SetPublisher(const std::string &v)
    {
        document_.MutableMetadata().SetPublisher(v);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::SetLanguage(const std::string &v)
    {
        document_.MutableMetadata().SetLanguage(v);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::SetSummary(const std::string &v)
    {
        document_.MutableMetadata().SetSummary(v);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::AddGenre(const std::string &v)
    {
        auto &m = document_.MutableMetadata();
        if (!m.MutableGenres())
            m.SetGenres(std::vector<std::string>{});
        m.MutableGenres()->push_back(v);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::AddCreator(const std::string &n, const std::string &r)
    {
        auto &m = document_.MutableMetadata();
        if (!m.MutableCreators())
            m.SetCreators(std::vector<Creator>{});
        Creator c;
        c.SetName(n);
        c.SetRole(r);
        m.MutableCreators()->push_back(c);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::AddTag(const std::string &i, const std::string &n, const std::string &t)
    {
        auto &m = document_.MutableMetadata();
        if (!m.MutableTags())
            m.SetTags(std::vector<Tag>{});
        Tag x;
        x.SetId(i);
        x.SetName(n);
        x.SetType(t);
        m.MutableTags()->push_back(x);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::AddChapter(const std::string &t, std::int64_t i, const std::string &s)
    {
        auto &m = document_.MutableMetadata();
        if (!m.GetChapters())
            m.SetChapters(std::vector<Chapter>{});
        Chapter c;
        c.SetTitle(t);
        c.SetStartPageIndex(i);
        if (!s.empty())
            c.SetSummary(s);
        m.GetChapters()->push_back(c);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::AddPageBase64(const std::string &b, const std::string &t, const std::string &s)
    {
        Page p;
        p.SetPageIndex(document_.GetPages().size());
        p.SetBase64Image(b);
        if (!t.empty())
            p.SetPageType(t);
        if (!s.empty())
            p.SetSummary(s);
        document_.MutablePages().push_back(p);
        return *this;
    }
    
    CbjBuilder &CbjBuilder::AddPageBytes(const std::string &b, const std::string &t, const std::string &s) 
    {
        return AddPageBase64(StringImageHelper::EncodeBase64(b), t, s);
    }

    Document CbjBuilder::Build() const { return document_; }
    
    bool CbjBuilder::Save(const std::string &p) const
    {
        namespace fs = std::filesystem;
        fs::path output(p);
        if (output.extension() != ".cbj")
            output.replace_extension(".cbj");
        const fs::path json = output.string() + ".data.json.tmp";
        const fs::path archive = output.string() + ".tmp";
        try {
            if (!WriteDocumentJson(json.string(), document_))
                return false;
            std::vector<std::streamoff> pages;
            std::string version;
            Metadata metadata;
            if (!ValidateAndIndexCbjJson(json.string(), pages, version, metadata)) {
                CbjLog::Error("builder", "generated data.json failed validation");
                fs::remove(json);
                return false;
            }
            mz_zip_archive zip{};
            if (!mz_zip_writer_init_file(&zip, archive.string().c_str(), 0)) {
                fs::remove(json); return false;
            }
            const bool added = mz_zip_writer_add_file(&zip, "data.json", json.string().c_str(), nullptr, 0, MZ_BEST_COMPRESSION);
            const bool finalized = added && mz_zip_writer_finalize_archive(&zip);
            mz_zip_writer_end(&zip);
            fs::remove(json);
            if (!finalized) { fs::remove(archive); return false; }
            fs::remove(output);
            fs::rename(archive, output);
            CbjLog::Info("builder", "saved validated CBJ archive: " + output.string());
            return true;
        } catch (const std::exception &e) {
            CbjLog::Error("builder", std::string("exception saving CBJ: ") + e.what());
            fs::remove(json);
            fs::remove(archive);
            return false;
        } catch (...) {
            CbjLog::Error("builder", "exception saving CBJ: non-standard exception");
            fs::remove(json);
            fs::remove(archive);
            return false;
        }
    }
}
