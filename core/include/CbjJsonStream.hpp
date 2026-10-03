#pragma once
#include "CbjSchema.hpp"
#include <string>
#include <vector>
#include <streamoff>

namespace cbj
{
    /** Validates CBJ JSON and creates a streaming page index without a DOM. */
    bool ValidateAndIndexCbjJson(const std::string &path, std::vector<std::streamoff> &offsets,
                                 std::string &version, Metadata &metadata);
    /** Reads one page from CBJ JSON using its zero-based stream ordinal. */
    bool ReadPageFromJson(const std::string &path, std::streamoff offset, Page &page);
    /** Serializes a CBJ document as data.json using RapidJSON. */
    bool WriteDocumentJson(const std::string &path, const Document &document);
}
