#pragma once
#include <string>
using std::string;

std::string Base64Encode(const std::string& str);
std::string Base64Encode(const unsigned char* pSrc, size_t nSrcLen);

// ִ��BASE64�������
std::string Base64Decode(const std::string& str);
std::string Base64Decode(const unsigned char* pSrc, size_t nSrcLen);
