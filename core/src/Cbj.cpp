#include "Cbj.hpp"
#include "StringImageHelper.hpp"
#include "CbjJsonStream.hpp"
#include "CbjLog.hpp"

#include <unarr.h>
#include <mupdf/fitz/context.h>
#include <mupdf/fitz/document.h>
#include <mupdf/fitz/util.h>
#include <mupdf/fitz/color.h>
#include <mupdf/fitz/pixmap.h>
#include <mupdf/fitz/image.h>
#include <mupdf/fitz/write-pixmap.h>
#include <miniz.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace cbj
{
    namespace
    {
        /** Describes one image entry in a conventional comic archive. */
        struct Entry
        {
            /** Archive entry name. */
            std::string name;
            /** Unarr seek offset. */
            long long offset = 0;
            /** Uncompressed byte size. */
            size_t size = 0;
            /** Image width in pixels when available. */
            int width = 0;
            /** Image height in pixels when available. */
            int height = 0;
        };

        /** Identifies one logical page produced from an archive image. */
        struct LogicalPageRef
        {
            /** Source image entry index. */
            int entryIndex = 0;
            /** Split side: -1 for an unsplit image, 0 for left and 1 for right. */
            int side = -1;
        };

        /** Holds heuristic evidence for one possible double-page spread. */
        struct Detection
        {
            /** Confidence score in the inclusive range [0, 1]. */
            double score = 0.0;
            /** Human-readable reasons used to build the score. */
            std::vector<std::string> reasons;
        };

        /** Returns true for supported comic image extensions. */
        bool isImage(const std::string &name)
        {
            auto p = name.find_last_of('.');
            if (p == std::string::npos)
                return false;
            std::string e = name.substr(p + 1);
            std::transform(e.begin(), e.end(), e.begin(), [](char c) { return char(std::tolower((unsigned char)c)); });
            return e == "jpg" || e == "jpeg" || e == "png" || e == "webp" ||
                   e == "gif" || e == "bmp" || e == "avif" || e == "jxl";
        }

        /** Returns the MIME type used by a source image name. */
        std::string mime(const std::string &name)
        {
            auto p = name.find_last_of('.');
            std::string e = p == std::string::npos ? "" : name.substr(p + 1);
            std::transform(e.begin(), e.end(), e.begin(), [](char c) { return char(std::tolower((unsigned char)c)); });
            if (e == "png") return "image/png";
            if (e == "webp") return "image/webp";
            if (e == "gif") return "image/gif";
            if (e == "bmp") return "image/bmp";
            if (e == "avif") return "image/avif";
            if (e == "jxl") return "image/jxl";
            return "image/jpeg";
        }

        /** Lowercases a string for case-insensitive metadata and filename matching. */
        std::string lower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(),
                           [](char c) { return char(std::tolower((unsigned char)c)); });
            return value;
        }

        /** Extracts all integer tokens from a file name. */
        std::vector<int> numbersInName(const std::string &name)
        {
            std::vector<int> result;
            static const std::regex expression(R"(\d+)");
            for (std::sregex_iterator i(name.begin(), name.end(), expression), end; i != end; ++i)
            {
                try { result.push_back(std::stoi(i->str())); }
                catch (...) { /* An oversized token is not useful to ordering heuristics. */ }
            }
            return result;
        }

        /** Extracts a consecutive numeric range such as 12-13 from a file name. */
        bool hasConsecutiveRange(const std::string &name)
        {
            static const std::regex expression(R"(\b(\d{1,6})\s*[-_&]\s*(\d{1,6})\b)");
            std::smatch match;
            if (!std::regex_search(name, match, expression))
                return false;
            try
            {
                return std::stoi(match[2].str()) == std::stoi(match[1].str()) + 1;
            }
            catch (...)
            {
                return false;
            }
        }

        /** Returns true when the file name contains common spread indicators. */
        bool hasSpreadKeyword(const std::string &name)
        {
            const std::string n = lower(name);
            static const char *keywords[] = {
                "double", "doublepage", "double-page", "spread", "2page", "2-page",
                "two-page", "twopage", "wide", "panorama"
            };
            for (const char *keyword : keywords)
                if (n.find(keyword) != std::string::npos)
                    return true;
            return false;
        }

        /** Computes a robust median without requiring a sorting allocation at each call site. */
        double median(std::vector<double> values)
        {
            if (values.empty())
                return 0.0;
            const auto middle = values.begin() + values.size() / 2;
            std::nth_element(values.begin(), middle, values.end());
            double result = *middle;
            if (values.size() % 2 == 0)
            {
                const auto previous = std::max_element(values.begin(), middle);
                result = (result + *previous) / 2.0;
            }
            return result;
        }

        /** Returns the current exception message, including non-standard exceptions. */
        std::string currentExceptionMessage()
        {
            try
            {
                throw;
            }
            catch (const std::exception &e)
            {
                return e.what();
            }
            catch (...)
            {
                return "non-standard exception";
            }
        }
    }

    /**
     * Private implementation of the CBJ document facade.
     *
     * The implementation keeps source archive entries separate from logical
     * pages so a double-page image can expand into two normal Page objects
     * without changing the CBJ schema.
     */
    class Cbj::Impl
    {
    public:
        /** Source path currently open. */
        std::string path;
        /** Source container kind: cbjz, zip, rar, 7z, tar or pdf. */
        std::string kind;
        /** CBJ protocol version. */
        std::string version = "1.0";
        /** Document metadata. */
        Metadata metadata;
        /** Source archive images. */
        std::vector<Entry> entries;
        /** Logical pages exposed by the API. */
        std::vector<LogicalPageRef> logicalPages;
        /** CBJ JSON page ordinals. */
        std::vector<std::streamoff> pages;
        /** Decoded page cache. */
        std::unordered_map<int, Page> cache;
        /** Edited logical pages. */
        std::unordered_map<int, Page> edited;
        /** Cache radius around the requested page. */
        int radius = 2;
        /** Whether a source document is open. */
        bool open = false;
        /** Whether the source is a CBJ archive. */
        bool cbjz = false;
        /** MuPDF context shared by PDF and image import operations. */
        fz_context *fzContext = nullptr;
        /** Open PDF document, when kind is pdf. */
        fz_document *pdf = nullptr;
        /** Temporary extracted data.json path. */
        std::string jsonPath;
        /** Source indices selected by double-page detection. */
        std::vector<int> detectedDoublePages;
        /** Detection options retained for the current import. */
        DoublePageOptions importOptions;

        /** Releases all native resources. */
        ~Impl()
        {
            reset();
        }

        /** Drops the MuPDF document and context. */
        void closeNative()
        {
            if (pdf && fzContext)
            {
                fz_drop_document(fzContext, pdf);
                pdf = nullptr;
            }
            if (fzContext)
            {
                fz_drop_context(fzContext);
                fzContext = nullptr;
            }
        }

        /** Resets all document state and removes temporary files. */
        void reset()
        {
            closeNative();
            if (!jsonPath.empty())
            {
                std::error_code ec;
                std::filesystem::remove(jsonPath, ec);
            }
            jsonPath.clear();
            path.clear();
            kind.clear();
            version = "1.0";
            entries.clear();
            logicalPages.clear();
            pages.clear();
            cache.clear();
            edited.clear();
            detectedDoublePages.clear();
            metadata = Metadata();
            importOptions = DoublePageOptions();
            open = false;
            cbjz = false;
        }

        /** Ensures a MuPDF context exists for image decoding. */
        void ensureFzContext()
        {
            if (fzContext)
                return;
            fzContext = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
            if (!fzContext)
                throw std::runtime_error("Cannot create MuPDF context");
        }

        /** Returns the current number of logical pages. */
        int count() const
        {
            if (kind == "pdf" && pdf)
                return fz_count_pages(fzContext, pdf);
            if (cbjz)
                return int(pages.size());
            return int(logicalPages.size());
        }

        /** Extracts one archive entry by its unarr offset. */
        std::string extractEntry(const Entry &entry) const
        {
            ar_stream *stream = ar_open_file(path.c_str());
            if (!stream)
                throw std::runtime_error("Cannot reopen archive");

            ar_archive *archive =
                kind == "zip" ? ar_open_zip_archive(stream, false) :
                kind == "rar" ? ar_open_rar_archive(stream) :
                kind == "7z" ? ar_open_7z_archive(stream) :
                                ar_open_tar_archive(stream);

            if (!archive || !ar_parse_entry_at(archive, entry.offset))
            {
                if (archive)
                    ar_close_archive(archive);
                ar_close(stream);
                throw std::runtime_error("Cannot seek archive entry: " + entry.name);
            }

            const size_t size = ar_entry_get_size(archive);
            std::string data(size, '\0');
            if (size && !ar_entry_uncompress(archive, data.data(), size))
            {
                ar_close_archive(archive);
                ar_close(stream);
                throw std::runtime_error("Cannot extract archive entry: " + entry.name);
            }

            ar_close_archive(archive);
            ar_close(stream);
            return data;
        }

        /** Reads image dimensions through MuPDF without retaining decoded pixels. */
        std::pair<int, int> imageDimensions(const Entry &entry)
        {
            ensureFzContext();
            const std::string data = extractEntry(entry);
            fz_buffer *buffer = nullptr;
            fz_image *imageObject = nullptr;
            int width = 0;
            int height = 0;

            fz_try(fzContext)
            {
                buffer = fz_new_buffer_from_copied_data(fzContext,
                    reinterpret_cast<const unsigned char *>(data.data()), data.size());
                imageObject = fz_new_image_from_buffer(fzContext, buffer);
                width = imageObject->w;
                height = imageObject->h;
            }
            fz_catch(fzContext)
            {
                const std::string message = fz_caught_message(fzContext);
                if (imageObject) fz_drop_image(fzContext, imageObject);
                if (buffer) fz_drop_buffer(fzContext, buffer);
                CbjLog::Warn("double-page", "unable to inspect image '" + entry.name + "': " + message);
                return {0, 0};
            }

            if (imageObject) fz_drop_image(fzContext, imageObject);
            if (buffer) fz_drop_buffer(fzContext, buffer);
            return {width, height};
        }

        /** Scores an image using dimensions, statistics, filename sequence and population statistics. */
        Detection detectEntry(int index, const std::vector<double> &medianRatios,
                              double medianWidth, double medianHeight)
        {
            const Entry &entry = entries[index];
            Detection detection;
            const double ratio = entry.height > 0 ? double(entry.width) / double(entry.height) : 0.0;
            const double medianRatio = medianRatios.empty() ? 0.0 : median(medianRatios);

            if (ratio >= 1.85)
            {
                detection.score += 0.35;
                detection.reasons.push_back("wide aspect ratio");
            }
            if (medianRatio > 0.0 && ratio >= medianRatio * 1.45)
            {
                detection.score += 0.25;
                detection.reasons.push_back("aspect ratio is an outlier");
            }
            if (medianWidth > 0.0 && entry.width >= medianWidth * 1.35)
            {
                detection.score += 0.15;
                detection.reasons.push_back("width is a statistical outlier");
            }
            if (medianHeight > 0.0 && entry.height >= medianHeight * 0.85 &&
                entry.height <= medianHeight * 1.15)
            {
                detection.score += 0.05;
                detection.reasons.push_back("height matches the page population");
            }

            if (hasConsecutiveRange(entry.name))
            {
                detection.score += 0.30;
                detection.reasons.push_back("filename contains a consecutive page range");
            }
            if (hasSpreadKeyword(entry.name))
            {
                detection.score += 0.25;
                detection.reasons.push_back("filename contains a spread indicator");
            }

            const std::vector<int> currentNumbers = numbersInName(entry.name);
            if (!currentNumbers.empty())
            {
                const int current = currentNumbers.back();
                const int previous = index > 0 ? [&]() {
                    const auto n = numbersInName(entries[index - 1].name);
                    return n.empty() ? -1 : n.back();
                }() : -1;
                const int next = index + 1 < int(entries.size()) ? [&]() {
                    const auto n = numbersInName(entries[index + 1].name);
                    return n.empty() ? -1 : n.back();
                }() : -1;

                if (previous >= 0 && next >= 0 && (current - previous != 1 || next - current != 1))
                {
                    detection.score += 0.10;
                    detection.reasons.push_back("filename sequence is anomalous");
                }
            }

            detection.score = (std::min)(1.0, detection.score);
            return detection;
        }

        /** Analyzes the source archive and creates the logical-page mapping. */
        void analyzeDoublePages(const DoublePageOptions &options)
        {
            importOptions = options;
            detectedDoublePages.clear();

            if (options.mode == DoublePageMode::None)
            {
                CbjLog::Debug("double-page", "double-page detection disabled");
                logicalPages.clear();
                for (int i = 0; i < int(entries.size()); ++i)
                    logicalPages.push_back({i, -1});
                return;
            }

            std::vector<double> ratios;
            std::vector<double> widths;
            std::vector<double> heights;
            ratios.reserve(entries.size());
            widths.reserve(entries.size());
            heights.reserve(entries.size());

            for (int i = 0; i < int(entries.size()); ++i)
            {
                const auto dimensions = imageDimensions(entries[i]);
                entries[i].width = dimensions.first;
                entries[i].height = dimensions.second;
                if (entries[i].width > 0 && entries[i].height > 0)
                {
                    ratios.push_back(double(entries[i].width) / double(entries[i].height));
                    widths.push_back(double(entries[i].width));
                    heights.push_back(double(entries[i].height));
                }
            }

            const double medianWidth = median(widths);
            const double medianHeight = median(heights);
            std::unordered_set<int> selected;

            for (int index : options.pageIndices)
            {
                if (index >= 0 && index < int(entries.size()))
                    selected.insert(index);
                else
                    CbjLog::Warn("double-page", "explicit double-page index out of range: " + std::to_string(index));
            }

            for (int i = 0; i < int(entries.size()); ++i)
            {
                for (const auto &name : options.fileNames)
                {
                    if (entries[i].name == name)
                    {
                        selected.insert(i);
                        break;
                    }
                }
            }

            if (options.mode == DoublePageMode::Automatic)
            {
                const double populationRatio = median(ratios);
                for (int i = 0; i < int(entries.size()); ++i)
                {
                    Detection detection = detectEntry(i, {populationRatio}, medianWidth, medianHeight);
                    if (detection.score >= 0.55)
                    {
                        selected.insert(i);
                        detectedDoublePages.push_back(i);
                        std::string reasons;
                        for (const auto &reason : detection.reasons)
                            reasons += (reasons.empty() ? "" : "; ") + reason;
                        CbjLog::Debug("double-page", "detected '" + entries[i].name +
                            "' score=" + std::to_string(detection.score) + " (" + reasons + ")");
                    }
                }
            }
            else
            {
                for (int index : metadataPages)
                    if (index >= 0 && index < int(entries.size()))
                        selected.insert(index);
            }

            if (options.mode == DoublePageMode::Explicit)
            {
                detectedDoublePages.assign(selected.begin(), selected.end());
            }
            else
            {
                for (int index : selected)
                    if (std::find(detectedDoublePages.begin(), detectedDoublePages.end(), index) == detectedDoublePages.end())
                        detectedDoublePages.push_back(index);
            }

            std::sort(detectedDoublePages.begin(), detectedDoublePages.end());
            logicalPages.clear();
            for (int i = 0; i < int(entries.size()); ++i)
            {
                if (selected.count(i))
                {
                    logicalPages.push_back({i, 0});
                    logicalPages.push_back({i, 1});
                }
                else
                {
                    logicalPages.push_back({i, -1});
                }
            }

            CbjLog::Notice("double-page", "normalized " + std::to_string(entries.size()) +
                " source images into " + std::to_string(logicalPages.size()) + " logical pages");
        }

        /** Opens a conventional archive and applies the selected page strategy. */
        void archive(const std::string &p, const std::string &k, const DoublePageOptions &options)
        {
            reset();
            path = p;
            kind = k;
            ensureFzContext();

            ar_stream *stream = ar_open_file(p.c_str());
            if (!stream)
                throw std::runtime_error("Cannot open archive");

            ar_archive *archive =
                k == "zip" ? ar_open_zip_archive(stream, false) :
                k == "rar" ? ar_open_rar_archive(stream) :
                k == "7z" ? ar_open_7z_archive(stream) :
                            ar_open_tar_archive(stream);
            if (!archive)
            {
                ar_close(stream);
                throw std::runtime_error("Invalid archive");
            }

            while (ar_parse_entry(archive))
            {
                const char *name = ar_entry_get_name(archive);
                if (name)
                {
                    const std::string entryName = name;
                    if (isImage(entryName))
                        entries.push_back({entryName, (long long)ar_entry_get_offset(archive), ar_entry_get_size(archive)});
                }
            }

            ar_close_archive(archive);
            ar_close(stream);

            std::sort(entries.begin(), entries.end(),
                [](const Entry &a, const Entry &b) { return a.name < b.name; });

            if (entries.empty())
                throw std::runtime_error("Archive contains no supported images");

            metadata.SetTitle(std::filesystem::path(p).stem().string());
            metadata.SetSeries("");
            analyzeDoublePages(options);
            open = true;

            CbjLog::Info("archive", "opened " + k + " comic with " +
                std::to_string(entries.size()) + " source images");
        }

        /** Returns one logical page, splitting its source image when necessary. */
        Page archivePage(int logicalIndex)
        {
            if (logicalIndex < 0 || logicalIndex >= int(logicalPages.size()))
                throw std::out_of_range("Page index out of range");

            const LogicalPageRef ref = logicalPages[logicalIndex];
            const Entry &entry = entries[ref.entryIndex];
            const std::string data = extractEntry(entry);

            if (ref.side < 0)
            {
                Page page;
                page.SetPageIndex(logicalIndex);
                page.SetPageType(logicalIndex ? "Story" : "FrontCover");
                page.SetBase64Image("data:" + mime(entry.name) + ";base64," +
                    StringImageHelper::EncodeBase64(data));
                return page;
            }

            ensureFzContext();
            fz_buffer *buffer = nullptr;
            fz_image *imageObject = nullptr;
            fz_pixmap *pixmap = nullptr;
            fz_pixmap *part = nullptr;
            fz_buffer *png = nullptr;
            Page page;

            fz_try(fzContext)
            {
                buffer = fz_new_buffer_from_copied_data(fzContext,
                    reinterpret_cast<const unsigned char *>(data.data()), data.size());
                imageObject = fz_new_image_from_buffer(fzContext, buffer);
                pixmap = fz_get_unscaled_pixmap_from_image(fzContext, imageObject);

                const int middle = pixmap->x + pixmap->w / 2;
                const fz_irect rect = ref.side == 0
                    ? fz_irect{pixmap->x, pixmap->y, middle, pixmap->y + pixmap->h}
                    : fz_irect{middle, pixmap->y, pixmap->x + pixmap->w, pixmap->y + pixmap->h};

                part = fz_new_pixmap_from_pixmap(fzContext, pixmap, &rect);
                png = fz_new_buffer_from_pixmap_as_png(fzContext, part, fz_default_color_params);
                unsigned char *pngData = nullptr;
                const size_t pngSize = fz_buffer_storage(fzContext, png, &pngData);

                page.SetPageIndex(logicalIndex);
                page.SetPageType(logicalIndex ? "Story" : "FrontCover");
                page.SetBase64Image("data:image/png;base64," +
                    StringImageHelper::EncodeBase64(
                        std::string(reinterpret_cast<const char *>(pngData), pngSize)));
            }
            fz_catch(fzContext)
            {
                const std::string message = fz_caught_message(fzContext);
                if (png) fz_drop_buffer(fzContext, png);
                if (part) fz_drop_pixmap(fzContext, part);
                if (pixmap) fz_drop_pixmap(fzContext, pixmap);
                if (imageObject) fz_drop_image(fzContext, imageObject);
                if (buffer) fz_drop_buffer(fzContext, buffer);
                CbjLog::Error("double-page", "failed to split '" + entry.name + "': " + message);
                throw std::runtime_error("Cannot split double-page image: " + entry.name);
            }

            if (png) fz_drop_buffer(fzContext, png);
            if (part) fz_drop_pixmap(fzContext, part);
            if (pixmap) fz_drop_pixmap(fzContext, pixmap);
            if (imageObject) fz_drop_image(fzContext, imageObject);
            if (buffer) fz_drop_buffer(fzContext, buffer);
            return page;
        }

        /** Extracts data.json from a CBJ archive and builds its streaming index. */
        void indexCbjz()
        {
            mz_zip_archive zip{};
            if (!mz_zip_reader_init_file(&zip, path.c_str(), 0))
                throw std::runtime_error("Cannot open CBJ archive");
            if (mz_zip_reader_locate_file(&zip, "data.json", nullptr, 0) < 0)
            {
                mz_zip_reader_end(&zip);
                throw std::runtime_error("CBJ archive must contain data.json");
            }

            jsonPath = path + ".data.json.tmp";
            if (!mz_zip_reader_extract_file_to_file(&zip, "data.json", jsonPath.c_str(), 0))
            {
                mz_zip_reader_end(&zip);
                throw std::runtime_error("Cannot extract data.json");
            }
            mz_zip_reader_end(&zip);

            if (!ValidateAndIndexCbjJson(jsonPath, pages, version, metadata))
                throw std::runtime_error("Invalid CBJ data.json");

            cbjz = open = true;
            CbjLog::Info("cbj", "opened CBJ archive: " + path);
        }

        /** Reads one logical page from the CBJ JSON stream. */
        Page cbjzPage(int index)
        {
            auto editedPage = edited.find(index);
            if (editedPage != edited.end())
                return editedPage->second;
            if (index < 0 || index >= int(pages.size()))
                throw std::out_of_range("Page index out of range");

            Page page;
            if (!ReadPageFromJson(jsonPath, pages[index], page))
                throw std::runtime_error("Cannot read CBJ page");
            return page;
        }

        /** Renders one PDF page through MuPDF. */
        Page pdfPage(int index)
        {
            if (!pdf || !fzContext || index < 0 || index >= fz_count_pages(fzContext, pdf))
                throw std::out_of_range("Page index out of range");

            fz_pixmap *pix = nullptr;
            fz_matrix ctm = fz_scale(1.0f, 1.0f);
            fz_try(fzContext)
            {
                pix = fz_new_pixmap_from_page_number(
                    fzContext, pdf, index, ctm, fz_device_rgb(fzContext), 0);
            }
            fz_catch(fzContext)
            {
                const std::string message = fz_caught_message(fzContext);
                CbjLog::Error("pdf", "MuPDF exception rendering page " +
                    std::to_string(index) + ": " + message);
                throw std::runtime_error("Cannot render PDF page: " + message);
            }

            const int width = fz_pixmap_width(fzContext, pix);
            const int height = fz_pixmap_height(fzContext, pix);
            const int stride = fz_pixmap_stride(fzContext, pix);
            const size_t bytes = size_t(stride) * size_t(height);
            std::string data(reinterpret_cast<const char *>(
                fz_pixmap_samples(fzContext, pix)), bytes);
            fz_drop_pixmap(fzContext, pix);

            Page page;
            page.SetPageIndex(index);
            page.SetPageType(index ? "Story" : "FrontCover");
            page.SetBase64Image("data:image/rgb;width=" + std::to_string(width) +
                ";height=" + std::to_string(height) + ";stride=" +
                std::to_string(stride) + ";base64=" +
                StringImageHelper::EncodeBase64(data));
            return page;
        }

        /** Returns a logical page, honoring edits and the local cache. */
        Page page(int index)
        {
            auto editedPage = edited.find(index);
            if (editedPage != edited.end())
                return editedPage->second;

            auto cachedPage = cache.find(index);
            if (cachedPage != cache.end())
                return cachedPage->second;

            if (cbjz)
                return cbjzPage(index);
            if (kind == "pdf")
                return pdfPage(index);
            return archivePage(index);
        }

        /** Removes cached pages outside the configured radius. */
        void trim(int center)
        {
            for (auto i = cache.begin(); i != cache.end();)
            {
                if (std::abs(i->first - center) > radius)
                    i = cache.erase(i);
                else
                    ++i;
            }
        }

        /** Serializes the normalized document into a validated CBJ archive. */
        void save(const std::string &output)
        {
            Document document;
            document.SetVersion(version);
            document.SetMetadata(metadata);
            document.SetPages({});
            for (int i = 0; i < count(); ++i)
                document.MutablePages().push_back(page(i));

            const std::string json = output + ".data.json.tmp";
            const std::string archive = output + ".tmp";

            if (!WriteDocumentJson(json, document))
                throw std::runtime_error("Cannot write data.json");

            std::vector<std::streamoff> index;
            std::string savedVersion;
            Metadata savedMetadata;
            if (!ValidateAndIndexCbjJson(json, index, savedVersion, savedMetadata))
                throw std::runtime_error("Invalid data.json");

            mz_zip_archive zip{};
            if (!mz_zip_writer_init_file(&zip, archive.c_str(), 0))
                throw std::runtime_error("Cannot create CBJ");

            bool ok = mz_zip_writer_add_file(
                &zip, "data.json", json.c_str(), nullptr, 0, MZ_BEST_COMPRESSION);
            ok = ok && mz_zip_writer_finalize_archive(&zip);
            mz_zip_writer_end(&zip);
            std::filesystem::remove(json);

            if (!ok)
            {
                std::filesystem::remove(archive);
                throw std::runtime_error("Cannot finalize CBJ");
            }

            std::filesystem::remove(output);
            std::filesystem::rename(archive, output);
            CbjLog::Info("cbj", "saved normalized CBJ archive: " + output);
        }
    };

    /** Creates a CBJ facade with an empty private implementation. */
    Cbj::Cbj() : pImpl(new Impl)
    {
        CbjLog::Trace("cbj", "Cbj instance created");
    }

    /** Destroys the CBJ facade and all native resources. */
    Cbj::~Cbj()
    {
        CbjLog::Trace("cbj", "Cbj instance destroyed");
        delete pImpl;
    }

    /** Opens a supported comic format based on its extension. */
    bool Cbj::Open(const std::string &path)
    {
        try
        {
            CbjLog::Trace("cbj", "opening path: " + path);
            std::string extension = std::filesystem::path(path).extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(),
                           [](char c) { return char(std::tolower((unsigned char)c)); });

            if (extension == ".cbj" || extension == ".cbjz")
                return OpenCbjz(path);
            if (extension == ".cbz")
                return ImportFromCbz(path);
            if (extension == ".cbr")
                return ImportFromCbr(path);
            if (extension == ".cb7")
                return ImportFromCb7(path);
            if (extension == ".cbt")
                return ImportFromCbt(path);
            if (extension == ".pdf")
                return ImportFromPdf(path);

            CbjLog::Warn("cbj", "unsupported file extension: " + extension);
            return false;
        }
        catch (...)
        {
            CbjLog::Error("cbj", "exception opening '" + path + "': " + currentExceptionMessage());
            return false;
        }
    }

    /** Opens a CBJ archive and validates data.json using the streaming parser. */
    bool Cbj::OpenCbjz(const std::string &path)
    {
        try
        {
            pImpl->reset();
            pImpl->path = path;
            pImpl->indexCbjz();
            return true;
        }
        catch (...)
        {
            CbjLog::Error("cbj", "exception opening CBJ '" + path + "': " + currentExceptionMessage());
            pImpl->reset();
            return false;
        }
    }

    /** Imports a CBZ using automatic double-page detection. */
    bool Cbj::ImportFromCbz(const std::string &path)
    {
        return ImportFromCbz(path, DoublePageOptions{});
    }

    /** Imports a CBZ using the requested double-page strategy. */
    bool Cbj::ImportFromCbz(const std::string &path, const DoublePageOptions &options)
    {
        try
        {
            pImpl->archive(path, "zip", options);
            return true;
        }
        catch (...)
        {
            CbjLog::Error("archive", "exception importing CBZ '" + path + "': " + currentExceptionMessage());
            pImpl->reset();
            return false;
        }
    }

    /** Imports a CBR using automatic double-page detection. */
    bool Cbj::ImportFromCbr(const std::string &path)
    {
        return ImportFromCbr(path, DoublePageOptions{});
    }

    /** Imports a CBR using the requested double-page strategy. */
    bool Cbj::ImportFromCbr(const std::string &path, const DoublePageOptions &options)
    {
        try
        {
            pImpl->archive(path, "rar", options);
            return true;
        }
        catch (...)
        {
            CbjLog::Error("archive", "exception importing CBR '" + path + "': " + currentExceptionMessage());
            pImpl->reset();
            return false;
        }
    }

    /** Imports a CB7 using automatic double-page detection. */
    bool Cbj::ImportFromCb7(const std::string &path)
    {
        return ImportFromCb7(path, DoublePageOptions{});
    }

    /** Imports a CB7 using the requested double-page strategy. */
    bool Cbj::ImportFromCb7(const std::string &path, const DoublePageOptions &options)
    {
        try
        {
            pImpl->archive(path, "7z", options);
            return true;
        }
        catch (...)
        {
            CbjLog::Error("archive", "exception importing CB7 '" + path + "': " + currentExceptionMessage());
            pImpl->reset();
            return false;
        }
    }

    /** Imports a CBT using automatic double-page detection. */
    bool Cbj::ImportFromCbt(const std::string &path)
    {
        return ImportFromCbt(path, DoublePageOptions{});
    }

    /** Imports a CBT using the requested double-page strategy. */
    bool Cbj::ImportFromCbt(const std::string &path, const DoublePageOptions &options)
    {
        try
        {
            pImpl->archive(path, "tar", options);
            return true;
        }
        catch (...)
        {
            CbjLog::Error("archive", "exception importing CBT '" + path + "': " + currentExceptionMessage());
            pImpl->reset();
            return false;
        }
    }

    /** Opens a PDF for lazy page rasterization. */
    bool Cbj::ImportFromPdf(const std::string &path)
    {
        try
        {
            pImpl->reset();
            pImpl->ensureFzContext();
            fz_try(pImpl->fzContext)
            {
                fz_register_document_handlers(pImpl->fzContext);
                pImpl->pdf = fz_open_document(pImpl->fzContext, path.c_str());
            }
            fz_catch(pImpl->fzContext)
            {
                const std::string message = fz_caught_message(pImpl->fzContext);
                CbjLog::Error("pdf", "MuPDF exception opening '" + path + "': " + message);
                pImpl->closeNative();
                return false;
            }

            if (!pImpl->pdf)
            {
                CbjLog::Error("pdf", "MuPDF returned no document for: " + path);
                pImpl->closeNative();
                return false;
            }

            pImpl->path = path;
            pImpl->kind = "pdf";
            pImpl->metadata.SetTitle(std::filesystem::path(path).stem().string());
            pImpl->metadata.SetSeries("");
            pImpl->open = true;
            CbjLog::Info("pdf", "opened PDF with MuPDF: " + path);
            return true;
        }
        catch (...)
        {
            CbjLog::Error("pdf", "exception opening PDF '" + path + "': " + currentExceptionMessage());
            pImpl->reset();
            return false;
        }
    }

    /** Saves the current document as a normalized CBJ archive. */
    bool Cbj::SaveAsCbjz(const std::string &path)
    {
        if (!pImpl->open)
        {
            CbjLog::Warn("cbj", "save requested while no comic is open");
            return false;
        }

        try
        {
            pImpl->save(path);
            return true;
        }
        catch (...)
        {
            CbjLog::Error("cbj", "exception saving '" + path + "': " + currentExceptionMessage());
            return false;
        }
    }

    /** Returns a range of logical pages and populates the cache. */
    std::vector<Page> Cbj::GetPages(int startIndex, int count)
    {
        try
        {
            if (!pImpl->open)
                throw std::runtime_error("No comic open");
            if (startIndex < 0 || count < 0)
                throw std::invalid_argument("Invalid range");

            std::vector<Page> result;
            for (int i = startIndex; i < startIndex + count && i < pImpl->count(); ++i)
            {
                Page page = pImpl->page(i);
                pImpl->cache[i] = page;
                result.push_back(page);
            }
            pImpl->trim(startIndex);
            CbjLog::Debug("cbj", "returned " + std::to_string(result.size()) + " page(s)");
            return result;
        }
        catch (...)
        {
            CbjLog::Error("cbj", "exception getting pages: " + currentExceptionMessage());
            throw;
        }
    }

    /** Returns pages according to a viewer layout mode. */
    std::vector<Page> Cbj::GetPagesForView(int currentIndex, ViewMode mode)
    {
        try
        {
            if (mode == ViewMode::Single)
                return GetPages(currentIndex, 1);
            if (mode == ViewMode::Desktop)
                return GetPages(currentIndex, 2);
            return GetPages((std::max)(0, currentIndex - 1), 3);
        }
        catch (...)
        {
            CbjLog::Error("cbj", "exception getting pages for view: " + currentExceptionMessage());
            throw;
        }
    }

    /** Returns one logical page. */
    Page Cbj::GetPage(int index)
    {
        try
        {
            auto result = GetPages(index, 1);
            if (result.empty())
                throw std::out_of_range("Page index out of range");
            return result[0];
        }
        catch (...)
        {
            CbjLog::Error("cbj", "exception getting page " + std::to_string(index) +
                ": " + currentExceptionMessage());
            throw;
        }
    }

    /** Clears the decoded page cache. */
    void Cbj::ClearCache()
    {
        CbjLog::Debug("cache", "clearing page cache");
        pImpl->cache.clear();
    }

    /** Sets the cache radius around the current page. */
    void Cbj::SetCachePages(int count)
    {
        if (count < 0)
        {
            CbjLog::Error("cache", "cache page radius must be >= 0");
            throw std::invalid_argument("cache pages must be >=0");
        }
        pImpl->radius = count;
        ClearCache();
        CbjLog::Info("cache", "cache radius set to " + std::to_string(count));
    }

    /** Returns the configured cache radius. */
    int Cbj::GetCachePages() const
    {
        return pImpl->radius;
    }

    /** Returns a copy of the current metadata. */
    Metadata Cbj::GetMetadata() const
    {
        return pImpl->metadata;
    }

    /** Replaces the current metadata. */
    void Cbj::SetMetadata(const Metadata &metadata)
    {
        pImpl->metadata = metadata;