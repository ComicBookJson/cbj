#pragma once
#include "CbjSchema.hpp"
#include <string>
namespace cbj {
bool ValidateAndIndexCbjJson(const std::string& path, std::vector<std::streamoff>& pageOffsets, std::string& version, Metadata& metadata);
bool ReadPageFromJson(const std::string& path, std::streamoff offset, Page& page);
bool WriteDocumentJson(const std::string& path, const Document& document);
}