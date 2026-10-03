#include "Cbj.hpp"
#include "StringImageHelper.hpp"
#include "CbjJsonStream.hpp"
#include "CbjLog.hpp"
#include <unarr.h>
#include <mupdf/fitz/context.h>
#include <mupdf/fitz/document.h>
#include <mupdf/fitz/util.h>
#include <mupdf/fitz/colorspace.h>
#include <mupdf/fitz/pixmap.h>
#include <miniz.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <stdexcept>
#include <iterator>
#include <cmath>
namespace cbj
{
    namespace
    {
        struct Entry
        {
            std::string name;
            long long offset;
            size_t size;
        };

        struct Loc
        {
            std::streamoff objectBegin, objectEnd, base64Begin, base64End;
        };

        bool image(const std::string &n)
        {
            auto p = n.find_last_of('.');
            if (p == std::string::npos)
                return false;
            auto e = n.substr(p + 1);
            std::transform(e.begin(), e.end(), e.begin(), [](char c)
                           { return char(std::tolower((unsigned char)c)); });
            return e == "jpg" || e == "jpeg" || e == "png" || e == "webp" || e == "gif" || e == "bmp" || e == "avif";
        }
        
        std::string mime(const std::string &n)
        {
            auto p = n.find_last_of('.');
            auto e = p == std::string::npos ? "" : n.substr(p + 1);
            std::transform(e.begin(), e.end(), e.begin(), [](char c)
                           { return char(std::tolower((unsigned char)c)); });
            if (e == "png")
                return "image/png";
            if (e == "webp")
                return "image/webp";
            if (e == "gif")
                return "image/gif";
            if (e == "bmp")
                return "image/bmp";
            if (e == "avif")
                return "image/avif";
            return "image/jpeg";
        }
        
        void ws(std::istream &f)
        {
            char c;
            while (f.get(c) && std::isspace((unsigned char)c))
            {
            }
            if (f)
                f.unget();
        }
        
        std::string jstr(std::istream &f)
        {
            char c;
            if (!f.get(c) || c != '"')
                throw std::runtime_error("JSON string expected");
            std::string s;
            bool x = false;
            while (f.get(c))
            {
                if (x)
                {
                    s += c;
                    x = false;
                    continue;
                }
                if (c == '\\')
                {
                    x = true;
                    continue;
                }
                if (c == '"')
                    return s;
                s += c;
            }
            throw std::runtime_error("Unterminated JSON string");
        }
        
        void skipstr(std::istream &f)
        {
            char c;
            bool x = false;
            while (f.get(c))
            {
                if (x)
                {
                    x = false;
                    continue;
                }
                if (c == '\\')
                {
                    x = true;
                    continue;
                }
                if (c == '"')
                    return;
            }
            throw std::runtime_error("Unterminated JSON string");
        }
    }

    class Cbj::Impl
    {
    public:
        std::string path = std::string::empty, kind = std::string::empty, version = "1.0";
        Metadata metadata;
        std::vector<Entry> entries;
        std::vector<std::streamoff> pages;
        std::unordered_map<int, Page> cache, edited;
        int radius = 2;
        bool open = false, cbjz = false;
        fz_context *pdfContext = nullptr;
        fz_document *pdf = nullptr;
        std::string jsonPath;
        
        ~Impl() 
        { 
            closePdf(); 
        }
        
        void closePdf()
        {
            if (pdf && pdfContext) { fz_drop_document(pdfContext, pdf); pdf = nullptr; }
            if (pdfContext) { fz_drop_context(pdfContext); pdfContext = nullptr; }
        }
        
        void reset()
        {
            closePdf();
            if (!jsonPath.empty()) { std::error_code ec; std::filesystem::remove(jsonPath, ec); }
            jsonPath.clear();
            path.clear();
            kind.clear();
            version = "1.0";
            entries.clear();
            pages.clear();
            cache.clear();
            edited.clear();
            metadata = Metadata();
            open = cbjz = false;
        }
        
        int count() const { return kind == "pdf" && pdf ? fz_count_pages(pdfContext, pdf) : cbjz ? int(pages.size())
                                                                                        : int(entries.size()); }
        void archive(const std::string &p, const std::string &k)
        {
            reset();
            path = p;
            kind = k;
            ar_stream *s = ar_open_file(p.c_str());
            if (!s)
                throw std::runtime_error("Cannot open archive");
            ar_archive *a = k == "zip" ? ar_open_zip_archive(s, false) : k == "rar" ? ar_open_rar_archive(s)
                                                                     : k == "7z"    ? ar_open_7z_archive(s)
                                                                                    : ar_open_tar_archive(s);
            if (!a)
            {
                ar_close(s);
                throw std::runtime_error("Invalid archive");
            }
            while (ar_parse_entry(a))
            {
                auto n = ar_entry_get_name(a);
                if (n && image(n))
                    entries.push_back({n, (long long)ar_entry_get_offset(a), ar_entry_get_size(a)});
            }
            ar_close_archive(a);
            ar_close(s);
            std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b)
                      { return a.name < b.name; });
            metadata.set_title(std::filesystem::path(p).stem().string());
            metadata.set_series("");
            open = true;
        }
        
        Page archivePage(int i)
        {
            if (i < 0 || i >= int(entries.size()))
                throw std::out_of_range("Page index out of range");
            ar_stream *s = ar_open_file(path.c_str());
            if (!s)
                throw std::runtime_error("Cannot reopen archive");
            ar_archive *a = kind == "zip" ? ar_open_zip_archive(s, false) : kind == "rar" ? ar_open_rar_archive(s)
                                                                        : kind == "7z"    ? ar_open_7z_archive(s)
                                                                                          : ar_open_tar_archive(s);
            if (!a || !ar_parse_entry_at(a, entries[i].offset))
            {
                if (a)
                    ar_close_archive(a);
                ar_close(s);
                throw std::runtime_error("Cannot seek entry");
            }
            size_t n = ar_entry_get_size(a);
            std::string b(n, '\0');
            if (n && !ar_entry_uncompress(a, b.data(), n))
            {
                ar_close_archive(a);
                ar_close(s);
                throw std::runtime_error("Cannot extract entry");
            }
            ar_close_archive(a);
            ar_close(s);
            Page p;
            p.set_page_index(i);
            p.set_page_type(i ? "Story" : "FrontCover");
            p.set_base64_image("data:" + mime(entries[i].name) + ";base64," + StringImageHelper::EncodeBase64(b));
            return p;
        }
        
        void indexCbjz()
        {
            mz_zip_archive zip{};
            if (!mz_zip_reader_init_file(&zip, path.c_str(), 0))
                throw std::runtime_error("Cannot open CBJ archive");
            if (mz_zip_reader_locate_file(&zip, "data.json", nullptr, 0) < 0) {
                mz_zip_reader_end(&zip);
                throw std::runtime_error("CBJ archive must contain data.json");
            }
            jsonPath = path + ".data.json.tmp";
            if (!mz_zip_reader_extract_file_to_file(&zip, "data.json", jsonPath.c_str(), 0)) {
                mz_zip_reader_end(&zip);
                throw std::runtime_error("Cannot extract data.json");
            }
            mz_zip_reader_end(&zip);
            if (!ValidateAndIndexCbjJson(jsonPath, pages, version, metadata)) {
                throw std::runtime_error("Invalid CBJ data.json");
            }
            cbjz = open = true;
        }

        Page cbjzPage(int i)
        {
            auto e = edited.find(i);
            if (e != edited.end()) return e->second;
            if (i < 0 || i >= int(pages.size())) throw std::out_of_range("Page index out of range");
            Page p;
            if (!ReadPageFromJson(jsonPath, pages[i], p))
                throw std::runtime_error("Cannot read CBJ page");
            return p;
        }

        Page pdfPage(int i)
        {
            if (!pdf || !pdfContext || i < 0 || i >= fz_count_pages(pdfContext, pdf))
                throw std::out_of_range("Page index out of range");
            fz_pixmap *pix = nullptr;
            fz_matrix ctm = fz_scale(1.0f, 1.0f);
            fz_try(pdfContext) {
                pix = fz_new_pixmap_from_page_number(pdfContext, pdf, i, ctm, fz_device_rgb(pdfContext), 0);
            } fz_catch(pdfContext) {
                throw std::runtime_error("Cannot render PDF page");
            }
            const int w = fz_pixmap_width(pdfContext, pix), h = fz_pixmap_height(pdfContext, pix);
            const int stride = fz_pixmap_stride(pdfContext, pix);
            const size_t bytes = size_t(stride) * size_t(h);
            std::string b(reinterpret_cast<const char*>(fz_pixmap_samples(pdfContext, pix)), bytes);
            fz_drop_pixmap(pdfContext, pix);
            Page p;
            p.set_page_index(i);
            p.set_page_type(i ? "Story" : "FrontCover");
            p.set_base64_image("data:image/rgb;width=" + std::to_string(w) + ";height=" + std::to_string(h) +
                               ";stride=" + std::to_string(stride) + ";base64," + StringImageHelper::EncodeBase64(b));
            return p;
        }

        Page page(int i)
        {
            auto e = edited.find(i);
            if (e != edited.end())
                return e->second;

            auto c = cache.find(i);
            if (c != cache.end())
                return c->second;
            
            return cbjz ? cbjzPage(i) : kind == "pdf" ? pdfPage(i)
                                                      : archivePage(i);
        }
        
        void trim(int center)
        {
            for (auto i = cache.begin(); i != cache.end();)
                if (std::abs(i->first - center) > radius)
                    i = cache.erase(i);
                else
                    ++i;
        }
        
        void save(const std::string &o)
        {
            Document d;
            d.SetVersion(version);
            d.SetMetadata(metadata);
            d.SetPages({});
            for (int i=0;i<count();++i) d.MutablePages().push_back(page(i));
            cbj::CbjBuilder builder;
            // CbjBuilder's public setters are intentionally used by clients; the
            // internal SaveAsCbjz path writes the same validated CBJ archive.
            const std::string json = o + ".data.json.tmp";
            const std::string archive = o + ".tmp";
            if (!WriteDocumentJson(json, d)) throw std::runtime_error("Cannot write data.json");
            std::vector<std::streamoff> idx; std::string v; Metadata m;
            if (!ValidateAndIndexCbjJson(json, idx, v, m)) throw std::runtime_error("Invalid data.json");
            mz_zip_archive zip{};
            if (!mz_zip_writer_init_file(&zip, archive.c_str(), 0)) throw std::runtime_error("Cannot create CBJ");
            bool ok = mz_zip_writer_add_file(&zip, "data.json", json.c_str(), nullptr, 0, MZ_BEST_COMPRESSION);
            ok = ok && mz_zip_writer_finalize_archive(&zip);
            mz_zip_writer_end(&zip);
            std::filesystem::remove(json);
            if (!ok) { std::filesystem::remove(archive); throw std::runtime_error("Cannot finalize CBJ"); }
            std::filesystem::remove(o); std::filesystem::rename(archive, o);
        }
    };

    Cbj::Cbj() : pImpl(new Impl) 
    {
    }

    Cbj::~Cbj() 
    { 
        delete pImpl; 
    }

    bool Cbj::Open(const std::string &p)
    {
        auto e = std::filesystem::path(p).extension().string();
        std::transform(e.begin(), e.end(), e.begin(), [](char c)
                       { return char(std::tolower((unsigned char)c)); });
        if (e == ".cbj" || e == ".cbjz")
            return OpenCbjz(p);
        if (e == ".cbz")
            return ImportFromCbz(p);
        if (e == ".cbr")
            return ImportFromCbr(p);
        if (e == ".cb7")
            return ImportFromCb7(p);
        if (e == ".cbt")
            return ImportFromCbt(p);
        if (e == ".pdf")
            return ImportFromPdf(p);
        return false;
    }

    bool Cbj::OpenCbjz(const std::string &p)
    {
        try
        {
            pImpl->reset();
            pImpl->path = p;
            pImpl->indexCbjz();
            return true;
        }
        catch (...)
        {
            pImpl->reset();
            return false;
        }
    }

    bool Cbj::ImportFromCbz(const std::string &p)
    {
        try
        {
            pImpl->archive(p, "zip");
            return true;
        }
        catch (...)
        {
            pImpl->reset();
            return false;
        }
    }

    bool Cbj::ImportFromCbr(const std::string &p)
    {
        try
        {
            pImpl->archive(p, "rar");
            return true;
        }
        catch (...)
        {
            pImpl->reset();
            return false;
        }
    }

    bool Cbj::ImportFromCb7(const std::string &p)
    {
        try
        {
            pImpl->archive(p, "7z");
            return true;
        }
        catch (...)
        {
            pImpl->reset();
            return false;
        }
    }

    bool Cbj::ImportFromCbt(const std::string &p)
    {
        try
        {
            pImpl->archive(p, "tar");
            return true;
        }
        catch (...)
        {
            pImpl->reset();
            return false;
        }
    }

    bool Cbj::ImportFromPdf(const std::string &p)
    {
        try {
            pImpl->reset();
            pImpl->pdfContext = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
            if (!pImpl->pdfContext) return false;
            fz_register_document_handlers(pImpl->pdfContext);
            pImpl->pdf = fz_open_document(pImpl->pdfContext, p.c_str());
            if (!pImpl->pdf) { pImpl->closePdf(); return false; }
            pImpl->path = p;
            pImpl->kind = "pdf";
            pImpl->metadata.set_title(std::filesystem::path(p).stem().string());
            pImpl->metadata.set_series("");
            pImpl->open = true;
            CbjLog::Info("pdf", "opened PDF with MuPDF: " + p);
            return true;
        } catch (...) { pImpl->reset(); return false; }
    }

    bool Cbj::SaveAsCbjz(const std::string &p)
    {
        if (!pImpl->open)
            return false;
        try
        {
            pImpl->save(p);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::vector<Page> Cbj::GetPages(int s, int n)
    {
        if (!pImpl->open)
            throw std::runtime_error("No comic open");
        if (s < 0 || n < 0)
            throw std::invalid_argument("Invalid range");
        std::vector<Page> r;
        for (int i = s; i < s + n && i < pImpl->count(); ++i)
        {
            auto p = pImpl->page(i);
            pImpl->cache[i] = p;
            r.push_back(p);
        }
        pImpl->trim(s);
        return r;
    }

    std::vector<Page> Cbj::GetPagesForView(int i, ViewMode m) { return m == ViewMode::Single ? GetPages(i, 1) : m == ViewMode::Desktop ? GetPages(i, 2)
                                                                                                                                              : GetPages((std::max)(0, i - 1), 3); }
    Page Cbj::GetPage(int i)
    {
        auto r = GetPages(i, 1);
        if (r.empty())
            throw std::out_of_range("Page index out of range");
        return r[0];
    }

    void Cbj::ClearCache() 
    {
        pImpl->cache.clear(); 
    }

    void Cbj::SetCachePages(int n)
    {
        if (n < 0)
            throw std::invalid_argument("cache pages must be >=0");
        pImpl->radius = n;
        ClearCache();
    }
    
    int Cbj::GetCachePages() const { return pImpl->radius; }
    
    Metadata Cbj::GetMetadata() const { return pImpl->metadata; }
    
    void Cbj::SetMetadata(const Metadata &m) { pImpl->metadata = m; }
    
    void Cbj::UpdatePage(int i, const Page &p)
    {
        if (i < 0 || i >= pImpl->count())
        throw std::out_of_range("Page index out of range");
        pImpl->edited[i] = p;
        pImpl->cache[i] = p;
    }
    
    int Cbj::GetTotalPages() const { return pImpl->count(); }
 
    std::string Cbj::GetVersion() const { return pImpl->version; }
 
    bool Cbj::IsOpen() const { return pImpl->open; }
}