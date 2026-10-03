#pragma once

#include <string>
#include <vector>
#include <optional>

namespace cbj {

    /** Generated CBJ schema DTO. */
    class Tag {
    public:
        Tag() = default;
        virtual ~Tag() = default;

    private:
        /** Backing storage for the generated schema property. */
        std::string _id = "";
        /** Backing storage for the generated schema property. */
        std::string _name = "";
        /** Backing storage for the generated schema property. */
        std::string _type = "";

    public:
        /** A unique identifier for this tag within the document (e.g., 'char_batman'). */
        /** Returns the generated schema property. */
        const std::string& GetId() const { return _id; }
        /** Sets the generated schema property. */
        void SetId(const std::string& value) { _id = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableId() { return _id; }

        /** The display name of the tag (e.g., 'Batman', 'Gotham City'). */
        /** Returns the generated schema property. */
        const std::string& GetName() const { return _name; }
        /** Sets the generated schema property. */
        void SetName(const std::string& value) { _name = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableName() { return _name; }

        /** The category or type of the tag. */
        /** Returns the generated schema property. */
        const std::string& GetType() const { return _type; }
        /** Sets the generated schema property. */
        void SetType(const std::string& value) { _type = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableType() { return _type; }

    };

    /** Generated CBJ schema DTO. */
    class Creator {
    public:
        Creator() = default;
        virtual ~Creator() = default;

    private:
        /** Backing storage for the generated schema property. */
        std::string _name = "";
        /** Backing storage for the generated schema property. */
        std::string _role = "";

    public:
        /** Returns the generated schema property. */
        const std::string& GetName() const { return _name; }
        /** Sets the generated schema property. */
        void SetName(const std::string& value) { _name = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableName() { return _name; }

        /** The creator's role, such as 'Writer', 'Penciller', 'Inker', 'Colorist', 'Letterer', 'Cover'. */
        /** Returns the generated schema property. */
        const std::string& GetRole() const { return _role; }
        /** Sets the generated schema property. */
        void SetRole(const std::string& value) { _role = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableRole() { return _role; }

    };

    /** Generated CBJ schema DTO. */
    class Chapter {
    public:
        Chapter() = default;
        virtual ~Chapter() = default;

    private:
        /** Backing storage for the generated schema property. */
        std::string _title = "";
        /** Backing storage for the generated schema property. */
        int _startPageIndex;
        /** Backing storage for the generated schema property. */
        std::optional<std::string> _summary;

    public:
        /** The title of the chapter. */
        /** Returns the generated schema property. */
        const std::string& GetTitle() const { return _title; }
        /** Sets the generated schema property. */
        void SetTitle(const std::string& value) { _title = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableTitle() { return _title; }

        /** The index of the page where this chapter begins. */
        int GetStartPageIndex() const { return _startPageIndex; }
        /** Sets the generated schema property. */
        void SetStartPageIndex(const int& value) { _startPageIndex = value; }
        /** Returns a mutable reference to the generated schema property. */
        int& MutableStartPageIndex() { return _startPageIndex; }

        /** A brief summary of what happens in this chapter. */
        /** Backing storage for the generated schema property. */
        std::optional<std::string> GetSummary() const { return _summary; }
        /** Sets the generated schema property. */
        void SetSummary(const std::optional<std::string>& value) { _summary = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::string>& MutableSummary() { return _summary; }

    };

    /** Generated CBJ schema DTO. */
    class TagPosition {
    public:
        TagPosition() = default;
        virtual ~TagPosition() = default;

    private:
        /** Backing storage for the generated schema property. */
        std::string _tagId = "";
        /** Backing storage for the generated schema property. */
        double _x;
        /** Backing storage for the generated schema property. */
        double _y;

    public:
        /** The ID of the tag (referencing metadata.tags). */
        /** Returns the generated schema property. */
        const std::string& GetTagId() const { return _tagId; }
        /** Sets the generated schema property. */
        void SetTagId(const std::string& value) { _tagId = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableTagId() { return _tagId; }

        /** The relative X coordinate of the tag on the page, from 0.0 (left edge) to 1.0 (right edge). */
        double GetX() const { return _x; }
        /** Sets the generated schema property. */
        void SetX(const double& value) { _x = value; }
        /** Returns a mutable reference to the generated schema property. */
        double& MutableX() { return _x; }

        /** The relative Y coordinate of the tag on the page, from 0.0 (top edge) to 1.0 (bottom edge). */
        double GetY() const { return _y; }
        /** Sets the generated schema property. */
        void SetY(const double& value) { _y = value; }
        /** Returns a mutable reference to the generated schema property. */
        double& MutableY() { return _y; }

    };

    /** Generated CBJ schema DTO. */
    class Metadata {
    public:
        Metadata() = default;
        virtual ~Metadata() = default;

    private:
        /** Backing storage for the generated schema property. */
        std::string _title = "";
        /** Backing storage for the generated schema property. */
        std::string _series = "";
        /** Backing storage for the generated schema property. */
        std::optional<std::string> _issue;
        /** Backing storage for the generated schema property. */
        std::optional<int> _volume;
        /** Backing storage for the generated schema property. */
        std::optional<std::string> _publisher;
        /** Backing storage for the generated schema property. */
        std::optional<std::string> _publicationDate;
        /** Backing storage for the generated schema property. */
        std::optional<std::string> _summary;
        /** Backing storage for the generated schema property. */
        std::optional<std::string> _language;
        /** Backing storage for the generated schema property. */
        std::optional<std::vector<std::string>> _genres;
        /** Backing storage for the generated schema property. */
        std::optional<std::vector<Creator>> _creators;
        /** Backing storage for the generated schema property. */
        std::optional<std::vector<Tag>> _tags;
        /** Backing storage for the generated schema property. */
        std::optional<std::vector<Chapter>> _chapters;

    public:
        /** Returns the generated schema property. */
        const std::string& GetTitle() const { return _title; }
        /** Sets the generated schema property. */
        void SetTitle(const std::string& value) { _title = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableTitle() { return _title; }

        /** Returns the generated schema property. */
        const std::string& GetSeries() const { return _series; }
        /** Sets the generated schema property. */
        void SetSeries(const std::string& value) { _series = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableSeries() { return _series; }

        /** Backing storage for the generated schema property. */
        std::optional<std::string> GetIssue() const { return _issue; }
        /** Sets the generated schema property. */
        void SetIssue(const std::optional<std::string>& value) { _issue = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::string>& MutableIssue() { return _issue; }

        /** Backing storage for the generated schema property. */
        std::optional<int> GetVolume() const { return _volume; }
        /** Sets the generated schema property. */
        void SetVolume(const std::optional<int>& value) { _volume = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<int>& MutableVolume() { return _volume; }

        /** Backing storage for the generated schema property. */
        std::optional<std::string> GetPublisher() const { return _publisher; }
        /** Sets the generated schema property. */
        void SetPublisher(const std::optional<std::string>& value) { _publisher = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::string>& MutablePublisher() { return _publisher; }

        /** Backing storage for the generated schema property. */
        std::optional<std::string> GetPublicationDate() const { return _publicationDate; }
        /** Sets the generated schema property. */
        void SetPublicationDate(const std::optional<std::string>& value) { _publicationDate = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::string>& MutablePublicationDate() { return _publicationDate; }

        /** Backing storage for the generated schema property. */
        std::optional<std::string> GetSummary() const { return _summary; }
        /** Sets the generated schema property. */
        void SetSummary(const std::optional<std::string>& value) { _summary = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::string>& MutableSummary() { return _summary; }

        /** Backing storage for the generated schema property. */
        std::optional<std::string> GetLanguage() const { return _language; }
        /** Sets the generated schema property. */
        void SetLanguage(const std::optional<std::string>& value) { _language = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::string>& MutableLanguage() { return _language; }

        /** Backing storage for the generated schema property. */
        std::optional<std::vector<std::string>> GetGenres() const { return _genres; }
        /** Sets the generated schema property. */
        void SetGenres(const std::optional<std::vector<std::string>>& value) { _genres = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::vector<std::string>>& MutableGenres() { return _genres; }

        /** Backing storage for the generated schema property. */
        std::optional<std::vector<Creator>> GetCreators() const { return _creators; }
        /** Sets the generated schema property. */
        void SetCreators(const std::optional<std::vector<Creator>>& value) { _creators = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::vector<Creator>>& MutableCreators() { return _creators; }

        /** A dictionary/list of all tags that appear in the comic. */
        /** Backing storage for the generated schema property. */
        std::optional<std::vector<Tag>> GetTags() const { return _tags; }
        /** Sets the generated schema property. */
        void SetTags(const std::optional<std::vector<Tag>>& value) { _tags = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::vector<Tag>>& MutableTags() { return _tags; }

        /** Table of contents or index for easy navigation. */
        /** Backing storage for the generated schema property. */
        std::optional<std::vector<Chapter>> GetChapters() const { return _chapters; }
        /** Sets the generated schema property. */
        void SetChapters(const std::optional<std::vector<Chapter>>& value) { _chapters = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::vector<Chapter>>& MutableChapters() { return _chapters; }

    };

    /** Generated CBJ schema DTO. */
    class Page {
    public:
        Page() = default;
        virtual ~Page() = default;

    private:
        /** Backing storage for the generated schema property. */
        int _pageIndex;
        /** Backing storage for the generated schema property. */
        std::optional<std::string> _pageType = "Story";
        /** Backing storage for the generated schema property. */
        std::optional<std::string> _summary;
        /** Backing storage for the generated schema property. */
        std::string _base64Image = "";
        /** Backing storage for the generated schema property. */
        std::optional<std::vector<TagPosition>> _pageTags;

    public:
        int GetPageIndex() const { return _pageIndex; }
        /** Sets the generated schema property. */
        void SetPageIndex(const int& value) { _pageIndex = value; }
        /** Returns a mutable reference to the generated schema property. */
        int& MutablePageIndex() { return _pageIndex; }

        /** Categorizes the page (e.g., 'FrontCover', 'Story', 'Ad', 'Letters', 'BackCover'). */
        /** Backing storage for the generated schema property. */
        std::optional<std::string> GetPageType() const { return _pageType; }
        /** Sets the generated schema property. */
        void SetPageType(const std::optional<std::string>& value) { _pageType = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::string>& MutablePageType() { return _pageType; }

        /** Summary of events happening on this specific page. */
        /** Backing storage for the generated schema property. */
        std::optional<std::string> GetSummary() const { return _summary; }
        /** Sets the generated schema property. */
        void SetSummary(const std::optional<std::string>& value) { _summary = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::string>& MutableSummary() { return _summary; }

        /** The Base64 encoded image string, preferably including the Data URI scheme prefix. */
        /** Returns the generated schema property. */
        const std::string& GetBase64Image() const { return _base64Image; }
        /** Sets the generated schema property. */
        void SetBase64Image(const std::string& value) { _base64Image = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableBase64Image() { return _base64Image; }

        /** A list of tags and their specific coordinates on this page. */
        /** Backing storage for the generated schema property. */
        std::optional<std::vector<TagPosition>> GetPageTags() const { return _pageTags; }
        /** Sets the generated schema property. */
        void SetPageTags(const std::optional<std::vector<TagPosition>>& value) { _pageTags = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::optional<std::vector<TagPosition>>& MutablePageTags() { return _pageTags; }

    };

    /** Generated CBJ schema DTO. */
    class Document {
    public:
        Document() = default;
        virtual ~Document() = default;

    private:
        /** Backing storage for the generated schema property. */
        std::string _version = "1.0";
        Metadata _metadata;
        /** Backing storage for the generated schema property. */
        std::vector<Page> _pages;

    public:
        /** The version of the CBJZ protocol. */
        /** Returns the generated schema property. */
        const std::string& GetVersion() const { return _version; }
        /** Sets the generated schema property. */
        void SetVersion(const std::string& value) { _version = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::string& MutableVersion() { return _version; }

        /** Returns the generated schema property. */
        const Metadata& GetMetadata() const { return _metadata; }
        /** Sets the generated schema property. */
        void SetMetadata(const Metadata& value) { _metadata = value; }
        /** Returns a mutable reference to the generated schema property. */
        Metadata& MutableMetadata() { return _metadata; }

        /** The sequential list of all pages in the comic book. */
        /** Returns the generated schema property. */
        const std::vector<Page>& GetPages() const { return _pages; }
        /** Sets the generated schema property. */
        void SetPages(const std::vector<Page>& value) { _pages = value; }
        /** Backing storage for the generated schema property. */
        /** Returns a mutable reference to the generated schema property. */
        std::vector<Page>& MutablePages() { return _pages; }

    };

} // namespace cbj
