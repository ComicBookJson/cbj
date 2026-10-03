#pragma once

#include <string>
#include <vector>
#include "CbjSchema.hpp"
#include "CbjImportOptions.hpp"

namespace cbj
{
    /**
     * Main CBJ comic document reader, importer, editor and writer.
     *
     * Conventional comic archives are normalized to one CBJ Page object per
     * logical page. Double-page source images can be detected automatically
     * or selected explicitly during import.
     */
    class Cbj
    {
    public:
        /** Selects the page grouping used when requesting pages for a viewer. */
        enum class ViewMode
        {
            /** Return one logical page. */
            Single = 1,
            /** Return two logical pages for desktop layouts. */
            Desktop = 2,
            /** Return a mobile-oriented neighborhood of pages. */
            Mobile = 3
        };

        /** Creates an empty, closed CBJ document instance. */
        Cbj();
        /** Releases all native resources owned by the document. */
        ~Cbj();
        Cbj(const Cbj &) = delete;
        Cbj &operator=(const Cbj &) = delete;

        /** Opens a file and selects the importer from its extension. */
        bool Open(const std::string &path);
        /** Opens an existing CBJ/CBJZ archive. */
        bool OpenCbjz(const std::string &path);
        /** Imports a CBZ archive using automatic double-page detection. */
        bool ImportFromCbz(const std::string &path);
        /** Imports a CBZ archive using the supplied double-page options. */
        bool ImportFromCbz(const std::string &path, const DoublePageOptions &options);
        /** Imports a CBR archive using automatic double-page detection. */
        bool ImportFromCbr(const std::string &path);
        /** Imports a CBR archive using the supplied double-page options. */
        bool ImportFromCbr(const std::string &path, const DoublePageOptions &options);
        /** Imports a CB7 archive using automatic double-page detection. */
        bool ImportFromCb7(const std::string &path);
        /** Imports a CB7 archive using the supplied double-page options. */
        bool ImportFromCb7(const std::string &path, const DoublePageOptions &options);
        /** Imports a CBT archive using automatic double-page detection. */
        bool ImportFromCbt(const std::string &path);
        /** Imports a CBT archive using the supplied double-page options. */
        bool ImportFromCbt(const std::string &path, const DoublePageOptions &options);
        /** Imports a PDF document using MuPDF rasterization. */
        bool ImportFromPdf(const std::string &path);
        /** Saves the current logical pages as a CBJ/CBJZ archive. */
        bool SaveAsCbjz(const std::string &path);
        /** Returns a contiguous range of logical pages. */
        std::vector<Page> GetPages(int startIndex, int count);
        /** Returns pages arranged for the requested viewer layout. */
        std::vector<Page> GetPagesForView(int currentIndex, ViewMode mode);
        /** Returns one logical page by zero-based index. */
        Page GetPage(int index);
        /** Removes all cached decoded pages. */
        void ClearCache();
        /** Sets the number of pages retained around the current page. */
        void SetCachePages(int count);
        /** Returns the configured page-cache radius. */
        int GetCachePages() const;
        /** Returns the current document metadata. */
        Metadata GetMetadata() const;
        /** Replaces the current document metadata. */
        void SetMetadata(const Metadata &metadata);
        /** Replaces one logical page in the editable document. */
        void UpdatePage(int index, const Page &page);
        /** Returns the number of logical pages after double-page normalization. */
        int GetTotalPages() const;
        /** Returns the CBJ protocol version. */
        std::string GetVersion() const;
        /** Returns true when a document is currently open. */
        bool IsOpen() const;
        /** Returns zero-based source indices detected as double-page spreads. */
        std::vector<int> GetDetectedDoublePageIndices() const;
        /** Returns source file names detected as double-page spreads. */
        std::vector<std::string> GetDetectedDoublePageNames() const;

    private:
        /** Private implementation containing archive, PDF and cache state. */
        class Impl;
        /** Opaque implementation owned by this facade. */
        Impl *pImpl;
    };
}
