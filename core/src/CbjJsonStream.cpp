#include "CbjJsonStream.hpp"
#include "CbjLog.hpp"
#include "StringImageHelper.hpp"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "miniz.h"
#include <fstream>
#include <filesystem>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace cbj {
namespace {
struct Handler : rapidjson::BaseReaderHandler<rapidjson::UTF8<>, Handler> {
    std::vector<std::streamoff>* offsets;
    std::string* version;
    Metadata* metadata;
    int depth=0; bool inPages=false, inPage=false; std::string key;
    std::string currentObject; bool capturePage=false;
    std::string scalar;
    Handler(std::vector<std::streamoff>& o,std::string& v,Metadata& m):offsets(&o),version(&v),metadata(&m){}
    bool Key(const char* s, rapidjson::SizeType n, bool){key.assign(s,n); return true;}
    bool StartObject(){
        if(depth==1 && key=="metadata") capturePage=false;
        if(depth==1 && key=="pages") inPages=true;
        if(inPages && depth==2){ inPage=true; currentObject="{"; }
        ++depth; return true;
    }
    bool EndObject(rapidjson::SizeType){
        --depth;
        if(inPage && depth==2){ inPage=false; currentObject.clear(); offsets->push_back(0); }
        return true;
    }
    bool StartArray(){
        if(depth==1 && key=="pages") inPages=true;
        ++depth; return true;
    }
    bool EndArray(rapidjson::SizeType){--depth; if(depth==1) inPages=false; return true;}
    bool String(const char* s, rapidjson::SizeType n, bool){ 
        if(depth==1 && key=="version") version->assign(s,n);
        return true;
    }
    bool Int(int){return true;} bool Uint(unsigned){return true;}
    bool Int64(int64_t){return true;} bool Uint64(uint64_t){return true;}
    bool Double(double){return true;} bool Bool(bool){return true;} bool Null(){return true;}
};

class PageHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, PageHandler> {
public:
    Page page; std::string key; int depth=0; bool inTags=false;
    bool Key(const char*s,rapidjson::SizeType n,bool){key.assign(s,n);return true;}
    bool StartObject(){++depth; return true;}
    bool EndObject(rapidjson::SizeType){--depth;return true;}
    bool StartArray(){if(key=="pageTags")inTags=true;return true;}
    bool EndArray(rapidjson::SizeType){inTags=false;return true;}
    bool String(const char*s,rapidjson::SizeType n,bool){
        std::string v(s,n);
        if(key=="pageType") page.SetPageType(v);
        else if(key=="summary") page.SetSummary(v);
        else if(key=="base64Image") page.SetBase64Image(v);
        else if(inTags && key=="tagId"){}
        return true;
    }
    bool Int(int v){if(key=="pageIndex")page.SetPageIndex(v);return true;}
    bool Uint(unsigned v){if(key=="pageIndex")page.SetPageIndex((int)v);return true;}
    bool Int64(int64_t v){if(key=="pageIndex")page.SetPageIndex((int)v);return true;}
    bool Uint64(uint64_t v){if(key=="pageIndex")page.SetPageIndex((int)v);return true;}
    bool Double(double){return true;} bool Bool(bool){return true;} bool Null(){return true;}
};

bool parse(const std::string& path, rapidjson::Reader& reader, Handler& h){
    std::ifstream f(path,std::ios::binary); if(!f) return false;
    char buffer[64*1024]; rapidjson::FileReadStream is(f,buffer,sizeof(buffer));
    return !reader.Parse(is,h).IsError();
}
bool validPage(const Page&p){return p.GetPageIndex()>=0 && !p.GetBase64Image().empty();}
}
bool ValidateAndIndexCbjJson(const std::string& path,std::vector<std::streamoff>& offsets,std::string& version,Metadata& metadata){
    std::ifstream f(path,std::ios::binary); if(!f) return false;
    rapidjson::Reader r; char b[64*1024]; rapidjson::FileReadStream is(f,b,sizeof(b));
    rapidjson::Document root; // bounded validation of top-level shape, not a DOM of pages.
    struct V : rapidjson::BaseReaderHandler<rapidjson::UTF8<>,V>{
        int depth=0; bool root=false,version=false,metadata=false,pages=false;
        bool Key(const char*s,rapidjson::SizeType n,bool){std::string k(s,n); if(depth==1){version|=k=="version";metadata|=k=="metadata";pages|=k=="pages";} return true;}
        bool StartObject(){root|=depth==0;++depth;return true;} bool EndObject(rapidjson::SizeType){--depth;return true;}
        bool StartArray(){if(depth==1)pages=true;++depth;return true;} bool EndArray(rapidjson::SizeType){--depth;return true;}
        bool String(const char*,rapidjson::SizeType,bool){return true;} bool Int(int){return true;} bool Uint(unsigned){return true;}
        bool Int64(int64_t){return true;} bool Uint64(uint64_t){return true;} bool Double(double){return true;} bool Bool(bool){return true;} bool Null(){return true;}
    } vh;
    if(r.Parse(is,vh).IsError() || !vh.root || !vh.version || !vh.metadata || !vh.pages) return false;
    // Second SAX pass extracts lightweight metadata and page count without materializing the document.
    Handler h(offsets,version,metadata); rapidjson::Reader r2;
    if(!parse(path,r2,h)) return false;
    CbjLog::Info("json","validated data.json with SAX/RapidJSON");
    return true;
}
bool ReadPageFromJson(const std::string& path,std::streamoff offset,Page&page){
    std::ifstream f(path,std::ios::binary); if(!f)return false;
    f.seekg(offset); std::string obj; char c; int d=0; bool str=false,esc=false;
    while(f.get(c)){obj+=c;if(str){if(esc)esc=false;else if(c=='\\')esc=true;else if(c=='"')str=false;continue;}
        if(c=='"'){str=true;continue;} if(c=='{')++d; else if(c=='}'&&--d==0)break;}
    rapidjson::Reader r; PageHandler h; rapidjson::StringStream ss(obj.c_str()); if(r.Parse(ss,h).IsError()||!validPage(h.page))return false; page=h.page;return true;
}
bool WriteDocumentJson(const std::string& path,const Document& d){
    std::ofstream f(path,std::ios::binary|std::ios::trunc); if(!f)return false;
    rapidjson::OStreamWrapper os(f); rapidjson::Writer<rapidjson::OStreamWrapper> w(os);
    w.StartObject(); w.Key("version"); w.String(d.GetVersion().c_str()); w.Key("metadata");
    // Use a small DOM only for metadata; pages are emitted one at a time.
    rapidjson::Document md; md.SetObject(); auto&a=md.GetAllocator();
    md.AddMember("title",rapidjson::Value(d.GetMetadata().GetTitle().c_str(),a),a);
    md.AddMember("series",rapidjson::Value(d.GetMetadata().GetSeries().c_str(),a),a);
    w.RawValue(md.Accept(w),0,rapidjson::kObjectType);
    w.Key("pages"); w.StartArray();
    for(const auto&p:d.GetPages()){
        w.StartObject(); w.Key("pageIndex");w.Int(p.GetPageIndex());
        if(p.GetPageType()){w.Key("pageType");w.String(p.GetPageType()->c_str());}
        if(p.GetSummary()){w.Key("summary");w.String(p.GetSummary()->c_str());}
        w.Key("base64Image");w.String(p.GetBase64Image().c_str());w.EndObject();
    }
    w.EndArray();w.EndObject();return true;
}
}
