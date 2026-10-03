#pragma once

#include <string>
#include <vector>
#include <optional>

namespace cbj {

    class Tag {
    public:
        Tag() = default;
        virtual ~Tag() = default;

    private:
        std::string _id = "";
        std::string _name = "";
        std::string _type = "";

    public:
        /** A unique identifier for this tag within the document (e.g., 'char_batman'). */
        const std::string& GetId() const { return _id; }
        void SetId(const std::string& value) { _id = value; }
        std::string& MutableId() { return _id; }

        /** The display name of the tag (e.g., 'Batman', 'Gotham City'). */
        const std::string& GetName() const { return _name; }
        void SetName(const std::string& value) { _name = value; }
        std::string& MutableName() { return _name; }

        /** The category or type of the tag. */
        const std::string& GetType() const { return _type; }
        void SetType(const std::string& value) { _type = value; }
        std::string& MutableType() { return _type; }

    };

    class Creator {
    public:
        Creator() = default;
        virtual ~Creator() = default;

    private:
        std::string _name = "";
        std::string _role = "";

    public:
        const std::string& GetName() const { return _name; }
        void SetName(const std::string& value) { _name = value; }
        std::string& MutableName() { return _name; }

        /** The creator's role, such as 'Writer', 'Penciller', 'Inker', 'Colorist', 'Letterer', 'Cover'. */
        const std::string& GetRole() const { return _role; }
        void SetRole(const std::string& value) { _role = value; }
        std::string& MutableRole() { return _role; }

    };

    class Chapter {
    public:
        Chapter() = default;
        virtual ~Chapter() = default;

    private:
        std::string _title = "";
        int _startPageIndex;
        std::optional<std::string> _summary;

    public:
        /** The title of the chapter. */
        const std::string& GetTitle() const { return _title; }
        void SetTitle(const std::string& value) { _title = value; }
        std::string& MutableTitle() { return _title; }

        /** The index of the page where this chapter begins. */
        int GetStartPageIndex() const { return _startPageIndex; }
        void SetStartPageIndex(const int& value) { _startPageIndex = value; }
        int& MutableStartPageIndex() { return _startPageIndex; }

        /** A brief summary of what happens in this chapter. */
        std::optional<std::string> GetSummary() const { return _summary; }
        void SetSummary(const std::optional<std::string>& value) { _summary = value; }
        std::optional<std::string>& MutableSummary() { return _summary; }

    };

    class TagPosition {
    public:
        TagPosition() = default;
        virtual ~TagPosition() = default;

    private:
        std::string _tagId = "";
        double _x;
        double _y;

    public:
        /** The ID of the tag (referencing metadata.tags). */
        const std::string& GetTagId() const { return _tagId; }
        void SetTagId(const std::string& value) { _tagId = value; }
        std::string& MutableTagId() { return _tagId; }

        /** The relative X coordinate of the tag on the page, from 0.0 (left edge) to 1.0 (right edge). */
        double GetX() const { return _x; }
        void SetX(const double& value) { _x = value; }
        double& MutableX() { return _x; }

        /** The relative Y coordinate of the tag on the page, from 0.0 (top edge) to 1.0 (bottom edge). */
        double GetY() const { return _y; }
        void SetY(const double& value) { _y = value; }
        double& MutableY() { return _y; }

    };

    class Metadata {
    public:
        Metadata() = default;
        virtual ~Metadata() = default;

    private:
        std::string _title = "";
        std::string _series = "";
        std::optional<std::string> _issue;
        std::optional<int> _volume;
        std::optional<std::string> _publisher;
        std::optional<std::string> _publicationDate;
        std::optional<std::string> _summary;
        std::optional<std::string> _language;
        std::optional<std::vector<std::string>> _genres;
        std::optional<std::vector<Creator>> _creators;
        std::optional<std::vector<Tag>> _tags;
        std::optional<std::vector<Chapter>> _chapters;

    public:
        const std::string& GetTitle() const { return _title; }
        void SetTitle(const std::string& value) { _title = value; }
        std::string& MutableTitle() { return _title; }

        const std::string& GetSeries() const { return _series; }
        void SetSeries(const std::string& value) { _series = value; }
        std::string& MutableSeries() { return _series; }

        std::optional<std::string> GetIssue() const { return _issue; }
        void SetIssue(const std::optional<std::string>& value) { _issue = value; }
        std::optional<std::string>& MutableIssue() { return _issue; }

        std::optional<int> GetVolume() const { return _volume; }
        void SetVolume(const std::optional<int>& value) { _volume = value; }
        std::optional<int>& MutableVolume() { return _volume; }

        std::optional<std::string> GetPublisher() const { return _publisher; }
        void SetPublisher(const std::optional<std::string>& value) { _publisher = value; }
        std::optional<std::string>& MutablePublisher() { return _publisher; }

        std::optional<std::string> GetPublicationDate() const { return _publicationDate; }
        void SetPublicationDate(const std::optional<std::string>& value) { _publicationDate = value; }
        std::optional<std::string>& MutablePublicationDate() { return _publicationDate; }

        std::optional<std::string> GetSummary() const { return _summary; }
        void SetSummary(const std::optional<std::string>& value) { _summary = value; }
        std::optional<std::string>& MutableSummary() { return _summary; }

        std::optional<std::string> GetLanguage() const { return _language; }
        void SetLanguage(const std::optional<std::string>& value) { _language = value; }
        std::optional<std::string>& MutableLanguage() { return _language; }

        std::optional<std::vector<std::string>> GetGenres() const { return _genres; }
        void SetGenres(const std::optional<std::vector<std::string>>& value) { _genres = value; }
        std::optional<std::vector<std::string>>& MutableGenres() { return _genres; }

        std::optional<std::vector<Creator>> GetCreators() const { return _creators; }
        void SetCreators(const std::optional<std::vector<Creator>>& value) { _creators = value; }
        std::optional<std::vector<Creator>>& MutableCreators() { return _creators; }

        /** A dictionary/list of all tags that appear in the comic. */
        std::optional<std::vector<Tag>> GetTags() const { return _tags; }
        void SetTags(const std::optional<std::vector<Tag>>& value) { _tags = value; }
        std::optional<std::vector<Tag>>& MutableTags() { return _tags; }

        /** Table of contents or index for easy navigation. */
        std::optional<std::vector<Chapter>> GetChapters() const { return _chapters; }
        void SetChapters(const std::optional<std::vector<Chapter>>& value) { _chapters = value; }
        std::optional<std::vector<Chapter>>& MutableChapters() { return _chapters; }

    };

    class Page {
    public:
        Page() = default;
        virtual ~Page() = default;

    private:
        int _pageIndex;
        std::optional<std::string> _pageType = "Story";
        std::optional<std::string> _summary;
        std::string _base64Image = "";
        std::optional<std::vector<TagPosition>> _pageTags;

    public:
        int GetPageIndex() const { return _pageIndex; }
        void SetPageIndex(const int& value) { _pageIndex = value; }
        int& MutablePageIndex() { return _pageIndex; }

        /** Categorizes the page (e.g., 'FrontCover', 'Story', 'Ad', 'Letters', 'BackCover'). */
        std::optional<std::string> GetPageType() const { return _pageType; }
        void SetPageType(const std::optional<std::string>& value) { _pageType = value; }
        std::optional<std::string>& MutablePageType() { return _pageType; }

        /** Summary of events happening on this specific page. */
        std::optional<std::string> GetSummary() const { return _summary; }
        void SetSummary(const std::optional<std::string>& value) { _summary = value; }
        std::optional<std::string>& MutableSummary() { return _summary; }

        /** The Base64 encoded image string, preferably including the Data URI scheme prefix. */
        const std::string& GetBase64Image() const { return _base64Image; }
        void SetBase64Image(const std::string& value) { _base64Image = value; }
        std::string& MutableBase64Image() { return _base64Image; }

        /** A list of tags and their specific coordinates on this page. */
        std::optional<std::vector<TagPosition>> GetPageTags() const { return _pageTags; }
        void SetPageTags(const std::optional<std::vector<TagPosition>>& value) { _pageTags = value; }
        std::optional<std::vector<TagPosition>>& MutablePageTags() { return _pageTags; }

    };

    class Document {
    public:
        Document() = default;
        virtual ~Document() = default;

    private:
        std::string _version = "1.0";
        Metadata _metadata;
        std::vector<Page> _pages;

    public:
        /** The version of the CBJZ protocol. */
        const std::string& GetVersion() const { return _version; }
        void SetVersion(const std::string& value) { _version = value; }
        std::string& MutableVersion() { return _version; }

        const Metadata& GetMetadata() const { return _metadata; }
        void SetMetadata(const Metadata& value) { _metadata = value; }
        Metadata& MutableMetadata() { return _metadata; }

        /** The sequential list of all pages in the comic book. */
        const std::vector<Page>& GetPages() const { return _pages; }
        void SetPages(const std::vector<Page>& value) { _pages = value; }
        std::vector<Page>& MutablePages() { return _pages; }

    };

} // namespace cbj
