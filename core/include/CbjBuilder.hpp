#pragma once

#include <cstdint>
#include <string>
#include "CbjSchema.hpp"

namespace cbj
{
    /**
     * Fluent builder for creating and serializing CBJ documents.
     */
    class CbjBuilder
    {
    public:
        /** Creates a builder initialized with CBJ protocol version 1.0. */
        CbjBuilder();

        /** Sets the CBJ protocol version. */
        CbjBuilder &SetVersion(const std::string &version);
        /** Sets the required title and series metadata. */
        CbjBuilder &SetMetadata(const std::string &title, const std::string &series);
        /** Sets the issue metadata. */
        CbjBuilder &SetIssue(const std::string &issue);
        /** Sets the volume metadata. */
        CbjBuilder &SetVolume(std::int64_t volume);
        /** Sets the publisher metadata. */
        CbjBuilder &SetPublisher(const std::string &publisher);
        /** Sets the language metadata. */
        CbjBuilder &SetLanguage(const std::string &language);
        /** Sets the summary metadata. */
        CbjBuilder &SetSummary(const std::string &summary);
        /** Adds a genre to the metadata collection. */
        CbjBuilder &AddGenre(const std::string &genre);
        /** Adds a creator and role to the metadata collection. */
        CbjBuilder &AddCreator(const std::string &name, const std::string &role);
        /** Adds a metadata tag. */
        CbjBuilder &AddTag(const std::string &id, const std::string &name, const std::string &type);
        /** Adds a chapter entry beginning at the supplied zero-based page index. */
        CbjBuilder &AddChapter(const std::string &title, std::int64_t startPageIndex,
                               const std::string &summary = "");
        /** Adds a page whose image is already Base64 encoded. */
        CbjBuilder &AddPageBase64(const std::string &base64Image,
                                   const std::string &pageType = "Story",
                                   const std::string &summary = "");
        /** Adds a page by encoding raw image bytes as Base64. */
        CbjBuilder &AddPageBytes(const std::string &imageBytes,
                                 const std::string &pageType = "Story",
                                 const std::string &summary = "");
        /** Returns the current document value. */
        Document Build() const;
        /** Validates and writes the document as a CBJ archive. */
        bool Save(const std::string &path) const;

    private:
        /** Mutable document assembled by the builder methods. */
        Document document_;
    };
}
