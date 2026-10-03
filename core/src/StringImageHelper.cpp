#include "StringImageHelper.hpp"
#include <fstream>
#include <iterator>
#include <stdexcept>
namespace cbjz {
static const char* A="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static int V(unsigned char c){if(c>='A'&&c<='Z')return c-'A';if(c>='a'&&c<='z')return c-'a'+26;if(c>='0'&&c<='9')return c-'0'+52;if(c=='+')return 62;if(c=='/')return 63;return -1;}
std::string StringImageHelper::EncodeBase64(const std::string& b){std::string o;o.reserve((b.size()+2)/3*4);for(size_t i=0;i<b.size();i+=3){unsigned a=(unsigned char)b[i],c=i+1<b.size()?(unsigned char)b[i+1]:0,d=i+2<b.size()?(unsigned char)b[i+2]:0;o+=A[a>>2];o+=A[((a&3)<<4)|(c>>4)];o+=i+1<b.size()?A[((c&15)<<2)|(d>>6)]:'=';o+=i+2<b.size()?A[d&63]:'=';}return o;}
std::string StringImageHelper::EncodeBase64(const std::vector<std::uint8_t>& b){return EncodeBase64(std::string(reinterpret_cast<const char*>(b.data()),b.size()));}
std::string StringImageHelper::DecodeBase64(const std::string& in){std::string s=in;auto comma=s.find(',');if(comma!=std::string::npos&&s.find("base64",0,comma)!=std::string::npos)s=s.substr(comma+1);std::string o;int val=0,bits=-8;for(unsigned char c:s){if(c=='=')break;int v=V(c);if(v<0)continue;val=(val<<6)|v;bits+=6;if(bits>=0){o.push_back(char((val>>bits)&255));bits-=8;}}return o;}
std::vector<std::uint8_t> StringImageHelper::DecodeBase64Bytes(const std::string& s){auto b=DecodeBase64(s);return {b.begin(),b.end()};}
std::string StringImageHelper::ReadFile(const std::string& p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Cannot open file: "+p);return {std::istreambuf_iterator<char>(f),{}};}
bool StringImageHelper::WriteFile(const std::string& p,const std::string& b){std::ofstream f(p,std::ios::binary|std::ios::trunc);if(!f)return false;f.write(b.data(),b.size());return bool(f);}
}