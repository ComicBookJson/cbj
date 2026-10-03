%module cbj

%{
#include "Cbj.hpp"
#include "CbjBuilder.hpp"
#include "StringImageHelper.hpp"
#include "CbjJsonStream.hpp"
#include "CbjLog.hpp"
%}

%pragma(java) jniclasscode=%{
  static {
    System.loadLibrary("cbz");
  }
%}

%include <std_string.i>
%include <std_vector.i>
%include <std_optional.i>

#ifdef SWIGCSHARP
%include <attribute.i>

/* Expose the schema Get/Set pairs as idiomatic C# properties. */
%attributestring(Tag, std::string, Id, GetId, SetId);
%attributestring(Tag, std::string, Name, GetName, SetName);
%attributestring(Tag, std::string, Type, GetType, SetType);

%attributestring(Creator, std::string, Name, GetName, SetName);
%attributestring(Creator, std::string, Role, GetRole, SetRole);

%attributestring(Chapter, std::string, Title, GetTitle, SetTitle);
%attribute(Chapter, int, StartPageIndex, GetStartPageIndex, SetStartPageIndex);
%attributeval(Chapter, std::optional<std::string>, Summary, GetSummary, SetSummary);

%attributestring(TagPosition, std::string, TagId, GetTagId, SetTagId);
%attribute(TagPosition, double, X, GetX, SetX);
%attribute(TagPosition, double, Y, GetY, SetY);

%attributestring(Metadata, std::string, Title, GetTitle, SetTitle);
%attributestring(Metadata, std::string, Series, GetSeries, SetSeries);
%attributeval(Metadata, std::optional<std::string>, Issue, GetIssue, SetIssue);
%attributeval(Metadata, std::optional<int>, Volume, GetVolume, SetVolume);
%attributeval(Metadata, std::optional<std::string>, Publisher, GetPublisher, SetPublisher);
%attributeval(Metadata, std::optional<std::string>, PublicationDate, GetPublicationDate, SetPublicationDate);
%attributeval(Metadata, std::optional<std::string>, Summary, GetSummary, SetSummary);
%attributeval(Metadata, std::optional<std::string>, Language, GetLanguage, SetLanguage);
%attributeval(Metadata, std::optional<std::vector<std::string>>, Genres, GetGenres, SetGenres);
%attributeval(Metadata, std::optional<std::vector<Creator>>, Creators, GetCreators, SetCreators);
%attributeval(Metadata, std::optional<std::vector<Tag>>, Tags, GetTags, SetTags);
%attributeval(Metadata, std::optional<std::vector<Chapter>>, Chapters, GetChapters, SetChapters);

%attribute(Page, int, PageIndex, GetPageIndex, SetPageIndex);
%attributeval(Page, std::optional<std::string>, PageType, GetPageType, SetPageType);
%attributeval(Page, std::optional<std::string>, Summary, GetSummary, SetSummary);
%attributestring(Page, std::string, Base64Image, GetBase64Image, SetBase64Image);
%attributeval(Page, std::optional<std::vector<TagPosition>>, PageTags, GetPageTags, SetPageTags);

%attributestring(Document, std::string, Version, GetVersion, SetVersion);
%attributeval(Document, Metadata, Metadata, GetMetadata, SetMetadata);
%attributeval(Document, std::vector<Page>, Pages, GetPages, SetPages);
#endif
%include "../core/include/CbjSchema.hpp"
%include "../core/include/CbjImportOptions.hpp"
%include "../core/include/Cbj.hpp"
%include "../core/include/CbjBuilder.hpp"
%include "../core/include/StringImageHelper.hpp"
%include "../core/include/CbjJsonStream.hpp"
%include "../core/include/CbjLog.hpp"
