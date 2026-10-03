#pragma once

#include <string>
#include <vector>

namespace cbj
{
    /**
     * Selects how conventional comic-book image files are interpreted as pages.
     */
    enum class DoublePageMode
    {
        /** Treat every source image as one page and do not split images. */
        None = 0,
        /** Detect double-page spreads using the SDK heuristics. */
        Automatic = 1,
        /** Split only the explicitly supplied source page numbers or file names. */
        Explicit = 2
    };

    /**
     * Controls double-page detection and splitting during archive import.
     */
    struct DoublePageOptions
    {
        /** The detection strategy used by the importer. */
        DoublePageMode mode = DoublePageMode::Automatic;
        /** Zero-based source image indices that must be treated as double pages. */
        std::vector<int> pageIndices;
        /** Archive image names that must be treated as double pages. */
        std::vector<std::string> fileNames;
    };
}
