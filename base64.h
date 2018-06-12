//
// Created by enrico on 11/1/17.
//

#ifndef GALILEO_TERREMOTI_BASE64_H
#define GALILEO_TERREMOTI_BASE64_H

#include <string>
#include <vector>

#define TCHAR char    //Not unicode
#define TEXT(x) x     //Not unicode
#define DWORD long
#define BYTE unsigned char
std::basic_string<TCHAR> base64Encode(std::string str);

#endif //GALILEO_TERREMOTI_BASE64_H
