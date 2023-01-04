#pragma once
#include <string>
using std::string;

std::string Base64Encode(const std::string& str);
std::string Base64Encode(const unsigned char* pSrc, size_t nSrcLen);

// Ö´ÐÐBASE64½âÂë²Ù×÷
std::string Base64Decode(const std::string& str);
std::string Base64Decode(const unsigned char* pSrc, size_t nSrcLen);
