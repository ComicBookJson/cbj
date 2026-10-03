//  To parse this JSON data, first install
//
//      json.hpp  https://github.com/nlohmann/json
//
//  Then include this file, and then do
//
//     CbjzV1Schema data = nlohmann::json::parse(jsonString);

#pragma once

#include <optional>
#include "json.hpp"

#include <optional>
#include <stdexcept>
#include <regex>

#ifndef NLOHMANN_OPT_HELPER
#define NLOHMANN_OPT_HELPER
namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {
        static void to_json(json & j, const std::shared_ptr<T> & opt) {
            if (!opt) j = nullptr; else j = *opt;
        }

        static std::shared_ptr<T> from_json(const json & j) {
            if (j.is_null()) return std::shared_ptr<T>(); else return std::make_shared<T>(j.get<T>());
        }
    };
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json & j, const std::optional<T> & opt) {
            if (!opt) j = nullptr; else j = *opt;
        }

        static std::optional<T> from_json(const json & j) {
            if (j.is_null()) return std::optional<T>(); else return std::make_optional<T>(j.get<T>());
        }
    };
}
#endif

namespace cbjz {
    using nlohmann::json;

    class ClassMemberConstraints {
        private:
        std::optional<int64_t> min_int_value;
        std::optional<int64_t> max_int_value;
        std::optional<double> min_double_value;
        std::optional<double> max_double_value;
        std::optional<size_t> min_length;
        std::optional<size_t> max_length;
        std::optional<std::string> pattern;

        public:
        ClassMemberConstraints(
            std::optional<int64_t> min_int_value,
            std::optional<int64_t> max_int_value,
            std::optional<double> min_double_value,
            std::optional<double> max_double_value,
            std::optional<size_t> min_length,
            std::optional<size_t> max_length,
            std::optional<std::string> pattern
        ) : min_int_value(min_int_value), max_int_value(max_int_value), min_double_value(min_double_value), max_double_value(max_double_value), min_length(min_length), max_length(max_length), pattern(pattern) {}
        ClassMemberConstraints() = default;
        virtual ~ClassMemberConstraints() = default;

        void set_min_int_value(int64_t min_int_value) { this->min_int_value = min_int_value; }
        auto get_min_int_value() const { return min_int_value; }

        void set_max_int_value(int64_t max_int_value) { this->max_int_value = max_int_value; }
        auto get_max_int_value() const { return max_int_value; }

        void set_min_double_value(double min_double_value) { this->min_double_value = min_double_value; }
        auto get_min_double_value() const { return min_double_value; }

        void set_max_double_value(double max_double_value) { this->max_double_value = max_double_value; }
        auto get_max_double_value() const { return max_double_value; }

        void set_min_length(size_t min_length) { this->min_length = min_length; }
        auto get_min_length() const { return min_length; }

        void set_max_length(size_t max_length) { this->max_length = max_length; }
        auto get_max_length() const { return max_length; }

        void set_pattern(const std::string &  pattern) { this->pattern = pattern; }
        auto get_pattern() const { return pattern; }
    };

    class ClassMemberConstraintException : public std::runtime_error {
        public:
        ClassMemberConstraintException(const std::string &  msg) : std::runtime_error(msg) {}
    };

    class ValueTooLowException : public ClassMemberConstraintException {
        public:
        ValueTooLowException(const std::string &  msg) : ClassMemberConstraintException(msg) {}
    };

    class ValueTooHighException : public ClassMemberConstraintException {
        public:
        ValueTooHighException(const std::string &  msg) : ClassMemberConstraintException(msg) {}
    };

    class ValueTooShortException : public ClassMemberConstraintException {
        public:
        ValueTooShortException(const std::string &  msg) : ClassMemberConstraintException(msg) {}
    };

    class ValueTooLongException : public ClassMemberConstraintException {
        public:
        ValueTooLongException(const std::string &  msg) : ClassMemberConstraintException(msg) {}
    };

    class InvalidPatternException : public ClassMemberConstraintException {
        public:
        InvalidPatternException(const std::string &  msg) : ClassMemberConstraintException(msg) {}
    };

    inline void CheckConstraint(const std::string &  name, const ClassMemberConstraints & c, int64_t value) {
        if (c.get_min_int_value() != std::nullopt && value < *c.get_min_int_value()) {
            throw ValueTooLowException ("Value too low for " + name + " (" + std::to_string(value) + "<" + std::to_string(*c.get_min_int_value()) + ")");
        }

        if (c.get_max_int_value() != std::nullopt && value > *c.get_max_int_value()) {
            throw ValueTooHighException ("Value too high for " + name + " (" + std::to_string(value) + ">" + std::to_string(*c.get_max_int_value()) + ")");
        }
    }

    inline void CheckConstraint(const std::string &  name, const ClassMemberConstraints & c, const std::optional<int64_t> & value) {
        if (value) {
            CheckConstraint(name, c, *value);
        }
    }

    inline void CheckConstraint(const std::string &  name, const ClassMemberConstraints & c, double value) {
        if (c.get_min_double_value() != std::nullopt && value < *c.get_min_double_value()) {
            throw ValueTooLowException ("Value too low for " + name + " (" + std::to_string(value) + "<" + std::to_string(*c.get_min_double_value()) + ")");
        }

        if (c.get_max_double_value() != std::nullopt && value > *c.get_max_double_value()) {
            throw ValueTooHighException ("Value too high for " + name + " (" + std::to_string(value) + ">" + std::to_string(*c.get_max_double_value()) + ")");
        }
    }

    inline void CheckConstraint(const std::string &  name, const ClassMemberConstraints & c, const std::optional<double> & value) {
        if (value) {
            CheckConstraint(name, c, *value);
        }
    }

    inline void CheckConstraint(const std::string &  name, const ClassMemberConstraints & c, const std::string &  value) {
        if (c.get_min_length() != std::nullopt && value.length() < *c.get_min_length()) {
            throw ValueTooShortException ("Value too short for " + name + " (" + std::to_string(value.length()) + "<" + std::to_string(*c.get_min_length()) + ")");
        }

        if (c.get_max_length() != std::nullopt && value.length() > *c.get_max_length()) {
            throw ValueTooLongException ("Value too long for " + name + " (" + std::to_string(value.length()) + ">" + std::to_string(*c.get_max_length()) + ")");
        }

        if (c.get_pattern() != std::nullopt) {
            std::smatch result;
            std::regex_search(value, result, std::regex( *c.get_pattern() ));
            if (result.empty()) {
                throw InvalidPatternException ("Value doesn't match pattern for " + name + " (" + value +" != " + *c.get_pattern() + ")");
            }
        }
    }

    inline void CheckConstraint(const std::string &  name, const ClassMemberConstraints & c, const std::optional<std::string> & value) {
        if (value) {
            CheckConstraint(name, c, *value);
        }
    }

    #ifndef NLOHMANN_UNTYPED_cbjz_HELPER
    #define NLOHMANN_UNTYPED_cbjz_HELPER
    inline json get_untyped(const json & j, const char * property) {
        if (j.find(property) != j.end()) {
            return j.at(property).get<json>();
        }
        return json();
    }

    inline json get_untyped(const json & j, std::string property) {
        return get_untyped(j, property.data());
    }
    #endif

    #ifndef NLOHMANN_OPTIONAL_cbjz_HELPER
    #define NLOHMANN_OPTIONAL_cbjz_HELPER
    template <typename T>
    inline std::shared_ptr<T> get_heap_optional(const json & j, const char * property) {
        auto it = j.find(property);
        if (it != j.end() && !it->is_null()) {
            return j.at(property).get<std::shared_ptr<T>>();
        }
        return std::shared_ptr<T>();
    }

    template <typename T>
    inline std::shared_ptr<T> get_heap_optional(const json & j, std::string property) {
        return get_heap_optional<T>(j, property.data());
    }
    template <typename T>
    inline std::optional<T> get_stack_optional(const json & j, const char * property) {
        auto it = j.find(property);
        if (it != j.end() && !it->is_null()) {
            return j.at(property).get<std::optional<T>>();
        }
        return std::optional<T>();
    }

    template <typename T>
    inline std::optional<T> get_stack_optional(const json & j, std::string property) {
        return get_stack_optional<T>(j, property.data());
    }
    #endif

    /**
     * Defines a chapter or story arc for indexing purposes.
     */
    class ChapterElement {
        public:
        ChapterElement() :
            start_page_index_constraint(0, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt)
        {}
        virtual ~ChapterElement() = default;

        private:
        int64_t start_page_index;
        ClassMemberConstraints start_page_index_constraint;
        std::optional<std::string> summary;
        std::string title;

        public:
        /**
         * The index of the page where this chapter begins.
         */
        const int64_t & get_start_page_index() const { return start_page_index; }
        int64_t & get_mutable_start_page_index() { return start_page_index; }
        void set_start_page_index(const int64_t & value) { CheckConstraint("start_page_index", start_page_index_constraint, value); this->start_page_index = value; }

        /**
         * A brief summary of what happens in this chapter.
         */
        std::optional<std::string> get_summary() const { return summary; }
        void set_summary(std::optional<std::string> value) { this->summary = value; }

        /**
         * The title of the chapter.
         */
        const std::string & get_title() const { return title; }
        std::string & get_mutable_title() { return title; }
        void set_title(const std::string & value) { this->title = value; }
    };

    /**
     * Represents a creator involved in the comic book (e.g., Writer, Artist).
     */
    class CreatorElement {
        public:
        CreatorElement() = default;
        virtual ~CreatorElement() = default;

        private:
        std::string name;
        std::string role;

        public:
        const std::string & get_name() const { return name; }
        std::string & get_mutable_name() { return name; }
        void set_name(const std::string & value) { this->name = value; }

        /**
         * The creator's role, such as 'Writer', 'Penciller', 'Inker', 'Colorist', 'Letterer',
         * 'Cover'.
         */
        const std::string & get_role() const { return role; }
        std::string & get_mutable_role() { return role; }
        void set_role(const std::string & value) { this->role = value; }
    };

    /**
     * The category or type of the tag.
     */
    enum class Type : int { CHARACTER, CONCEPT, EVENT, LOCATION, OBJECT, OTHER };

    /**
     * Defines a global tag such as a character, location, or event.
     */
    class TagElement {
        public:
        TagElement() = default;
        virtual ~TagElement() = default;

        private:
        std::string id;
        std::string name;
        Type type;

        public:
        /**
         * A unique identifier for this tag within the document (e.g., 'char_batman').
         */
        const std::string & get_id() const { return id; }
        std::string & get_mutable_id() { return id; }
        void set_id(const std::string & value) { this->id = value; }

        /**
         * The display name of the tag (e.g., 'Batman', 'Gotham City').
         */
        const std::string & get_name() const { return name; }
        std::string & get_mutable_name() { return name; }
        void set_name(const std::string & value) { this->name = value; }

        /**
         * The category or type of the tag.
         */
        const Type & get_type() const { return type; }
        Type & get_mutable_type() { return type; }
        void set_type(const Type & value) { this->type = value; }
    };

    /**
     * Contains all descriptive information, indexing, and tag definitions.
     */
    class Metadata {
        public:
        Metadata() = default;
        virtual ~Metadata() = default;

        private:
        std::optional<std::vector<ChapterElement>> chapters;
        std::optional<std::vector<CreatorElement>> creators;
        std::optional<std::vector<std::string>> genres;
        std::optional<std::string> issue;
        std::optional<std::string> language;
        std::optional<std::string> publication_date;
        std::optional<std::string> publisher;
        std::string series;
        std::optional<std::string> summary;
        std::optional<std::vector<TagElement>> tags;
        std::string title;
        std::optional<int64_t> volume;

        public:
        /**
         * Table of contents or index for easy navigation.
         */
        const std::optional<std::vector<ChapterElement>> & get_chapters() const { return chapters; }
        std::optional<std::vector<ChapterElement>> & get_mutable_chapters() { return chapters; }
        void set_chapters(const std::optional<std::vector<ChapterElement>> & value) { this->chapters = value; }

        const std::optional<std::vector<CreatorElement>> & get_creators() const { return creators; }
        std::optional<std::vector<CreatorElement>> & get_mutable_creators() { return creators; }
        void set_creators(const std::optional<std::vector<CreatorElement>> & value) { this->creators = value; }

        const std::optional<std::vector<std::string>> & get_genres() const { return genres; }
        std::optional<std::vector<std::string>> & get_mutable_genres() { return genres; }
        void set_genres(const std::optional<std::vector<std::string>> & value) { this->genres = value; }

        std::optional<std::string> get_issue() const { return issue; }
        void set_issue(std::optional<std::string> value) { this->issue = value; }

        std::optional<std::string> get_language() const { return language; }
        void set_language(std::optional<std::string> value) { this->language = value; }

        std::optional<std::string> get_publication_date() const { return publication_date; }
        void set_publication_date(std::optional<std::string> value) { this->publication_date = value; }

        std::optional<std::string> get_publisher() const { return publisher; }
        void set_publisher(std::optional<std::string> value) { this->publisher = value; }

        const std::string & get_series() const { return series; }
        std::string & get_mutable_series() { return series; }
        void set_series(const std::string & value) { this->series = value; }

        std::optional<std::string> get_summary() const { return summary; }
        void set_summary(std::optional<std::string> value) { this->summary = value; }

        /**
         * A dictionary/list of all tags that appear in the comic.
         */
        const std::optional<std::vector<TagElement>> & get_tags() const { return tags; }
        std::optional<std::vector<TagElement>> & get_mutable_tags() { return tags; }
        void set_tags(const std::optional<std::vector<TagElement>> & value) { this->tags = value; }

        const std::string & get_title() const { return title; }
        std::string & get_mutable_title() { return title; }
        void set_title(const std::string & value) { this->title = value; }

        std::optional<int64_t> get_volume() const { return volume; }
        void set_volume(std::optional<int64_t> value) { this->volume = value; }
    };

    /**
     * Represents the spatial position of a tag on a specific page.
     */
    class PageTagElement {
        public:
        PageTagElement() :
            x_constraint(std::nullopt, std::nullopt, 0, 1, std::nullopt, std::nullopt, std::nullopt),
            y_constraint(std::nullopt, std::nullopt, 0, 1, std::nullopt, std::nullopt, std::nullopt)
        {}
        virtual ~PageTagElement() = default;

        private:
        std::string tag_id;
        double x;
        ClassMemberConstraints x_constraint;
        double y;
        ClassMemberConstraints y_constraint;

        public:
        /**
         * The ID of the tag (referencing metadata.tags).
         */
        const std::string & get_tag_id() const { return tag_id; }
        std::string & get_mutable_tag_id() { return tag_id; }
        void set_tag_id(const std::string & value) { this->tag_id = value; }

        /**
         * The relative X coordinate of the tag on the page, from 0.0 (left edge) to 1.0 (right
         * edge).
         */
        const double & get_x() const { return x; }
        double & get_mutable_x() { return x; }
        void set_x(const double & value) { CheckConstraint("x", x_constraint, value); this->x = value; }

        /**
         * The relative Y coordinate of the tag on the page, from 0.0 (top edge) to 1.0 (bottom
         * edge).
         */
        const double & get_y() const { return y; }
        double & get_mutable_y() { return y; }
        void set_y(const double & value) { CheckConstraint("y", y_constraint, value); this->y = value; }
    };

    /**
     * Represents a single page containing the image and its specific metadata/tags.
     */
    class PageElement {
        public:
        PageElement() :
            page_index_constraint(0, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt)
        {}
        virtual ~PageElement() = default;

        private:
        std::string base64_image;
        int64_t page_index;
        ClassMemberConstraints page_index_constraint;
        std::optional<std::vector<PageTagElement>> page_tags;
        std::optional<std::string> page_type;
        std::optional<std::string> summary;

        public:
        /**
         * The Base64 encoded image string, preferably including the Data URI scheme prefix.
         */
        const std::string & get_base64_image() const { return base64_image; }
        std::string & get_mutable_base64_image() { return base64_image; }
        void set_base64_image(const std::string & value) { this->base64_image = value; }

        const int64_t & get_page_index() const { return page_index; }
        int64_t & get_mutable_page_index() { return page_index; }
        void set_page_index(const int64_t & value) { CheckConstraint("page_index", page_index_constraint, value); this->page_index = value; }

        /**
         * A list of tags and their specific coordinates on this page.
         */
        const std::optional<std::vector<PageTagElement>> & get_page_tags() const { return page_tags; }
        std::optional<std::vector<PageTagElement>> & get_mutable_page_tags() { return page_tags; }
        void set_page_tags(const std::optional<std::vector<PageTagElement>> & value) { this->page_tags = value; }

        /**
         * Categorizes the page (e.g., 'FrontCover', 'Story', 'Ad', 'Letters', 'BackCover').
         */
        const std::optional<std::string> & get_page_type() const { return page_type; }
        std::optional<std::string> & get_mutable_page_type() { return page_type; }
        void set_page_type(const std::optional<std::string> & value) { this->page_type = value; }

        /**
         * Summary of events happening on this specific page.
         */
        std::optional<std::string> get_summary() const { return summary; }
        void set_summary(std::optional<std::string> value) { this->summary = value; }
    };

    /**
     * Schema for representing a comic book in a single JSON structure.
     */
    class CbjzV1Schema {
        public:
        CbjzV1Schema() = default;
        virtual ~CbjzV1Schema() = default;

        private:
        Metadata metadata;
        std::vector<PageElement> pages;
        std::string version;

        public:
        const Metadata & get_metadata() const { return metadata; }
        Metadata & get_mutable_metadata() { return metadata; }
        void set_metadata(const Metadata & value) { this->metadata = value; }

        /**
         * The sequential list of all pages in the comic book.
         */
        const std::vector<PageElement> & get_pages() const { return pages; }
        std::vector<PageElement> & get_mutable_pages() { return pages; }
        void set_pages(const std::vector<PageElement> & value) { this->pages = value; }

        /**
         * The version of the CBJZ protocol.
         */
        const std::string & get_version() const { return version; }
        std::string & get_mutable_version() { return version; }
        void set_version(const std::string & value) { this->version = value; }
    };
}

namespace cbjz {
    void from_json(const json & j, ChapterElement & x);
    void to_json(json & j, const ChapterElement & x);

    void from_json(const json & j, CreatorElement & x);
    void to_json(json & j, const CreatorElement & x);

    void from_json(const json & j, TagElement & x);
    void to_json(json & j, const TagElement & x);

    void from_json(const json & j, Metadata & x);
    void to_json(json & j, const Metadata & x);

    void from_json(const json & j, PageTagElement & x);
    void to_json(json & j, const PageTagElement & x);

    void from_json(const json & j, PageElement & x);
    void to_json(json & j, const PageElement & x);

    void from_json(const json & j, CbjzV1Schema & x);
    void to_json(json & j, const CbjzV1Schema & x);

    void from_json(const json & j, Type & x);
    void to_json(json & j, const Type & x);

    inline void from_json(const json & j, ChapterElement& x) {
        x.set_start_page_index(j.at("startPageIndex").get<int64_t>());
        x.set_summary(get_stack_optional<std::string>(j, "summary"));
        x.set_title(j.at("title").get<std::string>());
    }

    inline void to_json(json & j, const ChapterElement & x) {
        j = json::object();
        j["startPageIndex"] = x.get_start_page_index();
        j["summary"] = x.get_summary();
        j["title"] = x.get_title();
    }

    inline void from_json(const json & j, CreatorElement& x) {
        x.set_name(j.at("name").get<std::string>());
        x.set_role(j.at("role").get<std::string>());
    }

    inline void to_json(json & j, const CreatorElement & x) {
        j = json::object();
        j["name"] = x.get_name();
        j["role"] = x.get_role();
    }

    inline void from_json(const json & j, TagElement& x) {
        x.set_id(j.at("id").get<std::string>());
        x.set_name(j.at("name").get<std::string>());
        x.set_type(j.at("type").get<Type>());
    }

    inline void to_json(json & j, const TagElement & x) {
        j = json::object();
        j["id"] = x.get_id();
        j["name"] = x.get_name();
        j["type"] = x.get_type();
    }

    inline void from_json(const json & j, Metadata& x) {
        x.set_chapters(get_stack_optional<std::vector<ChapterElement>>(j, "chapters"));
        x.set_creators(get_stack_optional<std::vector<CreatorElement>>(j, "creators"));
        x.set_genres(get_stack_optional<std::vector<std::string>>(j, "genres"));
        x.set_issue(get_stack_optional<std::string>(j, "issue"));
        x.set_language(get_stack_optional<std::string>(j, "language"));
        x.set_publication_date(get_stack_optional<std::string>(j, "publicationDate"));
        x.set_publisher(get_stack_optional<std::string>(j, "publisher"));
        x.set_series(j.at("series").get<std::string>());
        x.set_summary(get_stack_optional<std::string>(j, "summary"));
        x.set_tags(get_stack_optional<std::vector<TagElement>>(j, "tags"));
        x.set_title(j.at("title").get<std::string>());
        x.set_volume(get_stack_optional<int64_t>(j, "volume"));
    }

    inline void to_json(json & j, const Metadata & x) {
        j = json::object();
        j["chapters"] = x.get_chapters();
        j["creators"] = x.get_creators();
        j["genres"] = x.get_genres();
        j["issue"] = x.get_issue();
        j["language"] = x.get_language();
        j["publicationDate"] = x.get_publication_date();
        j["publisher"] = x.get_publisher();
        j["series"] = x.get_series();
        j["summary"] = x.get_summary();
        j["tags"] = x.get_tags();
        j["title"] = x.get_title();
        j["volume"] = x.get_volume();
    }

    inline void from_json(const json & j, PageTagElement& x) {
        x.set_tag_id(j.at("tagId").get<std::string>());
        x.set_x(j.at("x").get<double>());
        x.set_y(j.at("y").get<double>());
    }

    inline void to_json(json & j, const PageTagElement & x) {
        j = json::object();
        j["tagId"] = x.get_tag_id();
        j["x"] = x.get_x();
        j["y"] = x.get_y();
    }

    inline void from_json(const json & j, PageElement& x) {
        x.set_base64_image(j.at("base64Image").get<std::string>());
        x.set_page_index(j.at("pageIndex").get<int64_t>());
        x.set_page_tags(get_stack_optional<std::vector<PageTagElement>>(j, "pageTags"));
        x.set_page_type(get_stack_optional<std::string>(j, "pageType"));
        x.set_summary(get_stack_optional<std::string>(j, "summary"));
    }

    inline void to_json(json & j, const PageElement & x) {
        j = json::object();
        j["base64Image"] = x.get_base64_image();
        j["pageIndex"] = x.get_page_index();
        j["pageTags"] = x.get_page_tags();
        j["pageType"] = x.get_page_type();
        j["summary"] = x.get_summary();
    }

    inline void from_json(const json & j, CbjzV1Schema& x) {
        x.set_metadata(j.at("metadata").get<Metadata>());
        x.set_pages(j.at("pages").get<std::vector<PageElement>>());
        x.set_version(j.at("version").get<std::string>());
    }

    inline void to_json(json & j, const CbjzV1Schema & x) {
        j = json::object();
        j["metadata"] = x.get_metadata();
        j["pages"] = x.get_pages();
        j["version"] = x.get_version();
    }

    inline void from_json(const json & j, Type & x) {
        if (j == "Character") x = Type::CHARACTER;
        else if (j == "Concept") x = Type::CONCEPT;
        else if (j == "Event") x = Type::EVENT;
        else if (j == "Location") x = Type::LOCATION;
        else if (j == "Object") x = Type::OBJECT;
        else if (j == "Other") x = Type::OTHER;
        else { throw std::runtime_error("Cannot deserialize to enumeration \"Type\""); }
    }

    inline void to_json(json & j, const Type & x) {
        switch (x) {
            case Type::CHARACTER: j = "Character"; break;
            case Type::CONCEPT: j = "Concept"; break;
            case Type::EVENT: j = "Event"; break;
            case Type::LOCATION: j = "Location"; break;
            case Type::OBJECT: j = "Object"; break;
            case Type::OTHER: j = "Other"; break;
            default: throw std::runtime_error("Unexpected value in enumeration \"Type\": " + std::to_string(static_cast<int>(x)));
        }
    }
}