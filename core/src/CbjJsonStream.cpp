#include "CbjJsonStream.hpp"
#include "CbjLog.hpp"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stream.h"
#include <fstream>
#include <string>

namespace cbj {
namespace {
struct SaxHandler : rapidjson::BaseReaderHandler<rapidjson::UTF8<>, SaxHandler> {
    std::vector<std::streamoff>& pageNumbers; std::string& version; Metadata& metadata;
    int depth=0, pageOrdinal=-1, selected=-1; bool pages=false, page=false, metadataObject=false;
    std::string key; Page selectedPage; bool pageIndexSeen=false, base64Seen=false;
    explicit SaxHandler(std::vector<std::streamoff>& p,std::string& v,Metadata&m):pageNumbers(p),version(v),metadata(m){}
    bool Key(const char*s,rapidjson::SizeType n,bool){key.assign(s,n);return true;}
    bool StartObject(){
        if(depth==1 && key=="metadata") metadataObject=true;
        if(pages && depth==2){ page=true; ++pageOrdinal; pageNumbers.push_back(pageOrdinal); pageIndexSeen=base64Seen=false; }
        ++depth; return true;
    }
    bool EndObject(rapidjson::SizeType){
        --depth;
        if(page && pages && depth==2){ page=false; }
        if(metadataObject && depth==1) metadataObject=false;
        return true;
    }
    bool StartArray(){if(depth==1 && key=="pages")pages=true; ++depth; return true;}
    bool EndArray(rapidjson::SizeType){--depth; if(depth==1)pages=false; return true;}
    bool String(const char*s,rapidjson::SizeType n,bool){
        if(depth==1 && key=="version") version.assign(s,n);
        if(metadataObject){
            std::string v(s,n);
            if(key=="title")metadata.SetTitle(v); else if(key=="series")metadata.SetSeries(v);
            else if(key=="issue")metadata.SetIssue(v); else if(key=="publisher")metadata.SetPublisher(v);
            else if(key=="publicationDate")metadata.SetPublicationDate(v); else if(key=="summary")metadata.SetSummary(v);
            else if(key=="language")metadata.SetLanguage(v);
        }
        if(page && pageOrdinal==selected){
            if(key=="pageType")selectedPage.SetPageType(std::string(s,n));
            else if(key=="summary")selectedPage.SetSummary(std::string(s,n));
            else if(key=="base64Image"){selectedPage.SetBase64Image(std::string(s,n));base64Seen=true;}
        }
        return true;
    }
    bool Int(int v){if(page&&pageOrdinal==selected&&key=="pageIndex"){selectedPage.SetPageIndex(v);pageIndexSeen=true;}return true;}
    bool Uint(unsigned v){return Int((int)v);} bool Int64(int64_t v){return Int((int)v);} bool Uint64(uint64_t v){return Int((int)v);}
    bool Double(double){return true;} bool Bool(bool){return true;} bool Null(){return true;}
};

bool run(const std::string&path,SaxHandler&h){
    std::ifstream f(path,std::ios::binary); if(!f)return false;
    char buffer[64*1024]; rapidjson::FileReadStream is(f,buffer,sizeof(buffer));
    rapidjson::Reader r; return !r.Parse(is,h).IsError();
}
struct Validator : rapidjson::BaseReaderHandler<rapidjson::UTF8<>, Validator>{
    int depth=0; bool root=false, version=false, metadata=false, pages=false, page=false;
    int pageNo=-1; bool pageIndex=false, base64=false; bool valid=true; std::string key;
    bool Key(const char*s,rapidjson::SizeType n,bool){key.assign(s,n);if(depth==1){version|=key=="version";metadata|=key=="metadata";pages|=key=="pages";}return true;}
    bool StartObject(){root|=depth==0;if(pages&&depth==2){page=true;++pageNo;pageIndex=base64=false;}++depth;return true;}
    bool EndObject(rapidjson::SizeType){--depth;if(page&&pages&&depth==2){valid&=pageIndex&&base64;page=false;}return true;}
    bool StartArray(){++depth;return true;} bool EndArray(rapidjson::SizeType){--depth;return true;}
    bool String(const char*s,rapidjson::SizeType n,bool){if(page&&key=="base64Image")base64=n>0;return true;}
    bool Int(int){if(page&&key=="pageIndex")pageIndex=true;return true;} bool Uint(unsigned){return Int(0);}
    bool Int64(int64_t){return Int(0);} bool Uint64(uint64_t){return Int(0);}
    bool Double(double){return true;} bool Bool(bool){return true;} bool Null(){return true;}
};
void writeString(rapidjson::Writer<rapidjson::OStreamWrapper>&w,const char*k,const std::string&v){w.Key(k);w.String(v.c_str(),(rapidjson::SizeType)v.size());}
}
bool ValidateAndIndexCbjJson(const std::string&path,std::vector<std::streamoff>&offsets,std::string&version,Metadata&metadata){
    std::ifstream f(path,std::ios::binary);if(!f)return false;char b[64*1024];rapidjson::FileReadStream is(f,b,sizeof(b));rapidjson::Reader r;Validator v;
    if(r.Parse(is,v).IsError()||!v.valid||!v.root||!v.version||!v.metadata||!v.pages||v.pageNo<0)return false;
    offsets.clear();SaxHandler h(offsets,version,metadata);if(!run(path,h))return false;
    CbjLog::Info("json","validated data.json using RapidJSON SAX streaming");
    return true;
}
bool ReadPageFromJson(const std::string&path,std::streamoff ordinal,Page&page){
    std::vector<std::streamoff> dummy;std::string version;Metadata metadata;SaxHandler h(dummy,version,metadata);h.selected=(int)ordinal;
    if(!run(path,h)||h.selected<0||h.selected>=(int)dummy.size()||h.selectedPage.GetBase64Image().empty())return false;
    page=h.selectedPage;return true;
}
bool WriteDocumentJson(const std::string&path,const Document&d){
    std::ofstream f(path,std::ios::binary|std::ios::trunc);if(!f)return false;rapidjson::OStreamWrapper os(f);rapidjson::Writer<rapidjson::OStreamWrapper>w(os);
    w.StartObject();writeString(w,"version",d.GetVersion());w.Key("metadata");w.StartObject();const auto&m=d.GetMetadata();
    writeString(w,"title",m.GetTitle());writeString(w,"series",m.GetSeries());
    if(m.GetIssue())writeString(w,"issue",*m.GetIssue());if(m.GetVolume()){w.Key("volume");w.Int(*m.GetVolume());}
    if(m.GetPublisher())writeString(w,"publisher",*m.GetPublisher());if(m.GetPublicationDate())writeString(w,"publicationDate",*m.GetPublicationDate());
    if(m.GetSummary())writeString(w,"summary",*m.GetSummary());if(m.GetLanguage())writeString(w,"language",*m.GetLanguage());
    if(m.GetGenres()){w.Key("genres");w.StartArray();for(auto&x:*m.GetGenres())w.String(x.c_str());w.EndArray();}
    w.EndObject();w.Key("pages");w.StartArray();for(const auto&p:d.GetPages()){w.StartObject();w.Key("pageIndex");w.Int(p.GetPageIndex());
        if(p.GetPageType())writeString(w,"pageType",*p.GetPageType());if(p.GetSummary())writeString(w,"summary",*p.GetSummary());
        writeString(w,"base64Image",p.GetBase64Image());w.EndObject();}w.EndArray();w.EndObject();return true;
}
}
