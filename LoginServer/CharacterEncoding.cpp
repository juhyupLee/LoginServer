#include <Windows.h>
#include <string>
#include <codecvt>
void UTF16ToUTF8(const wchar_t* string,std::string& outString)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
	outString = convert.to_bytes(string);

}
void UTF8ToUTF16(const char* string,std::wstring& outString)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
	outString = convert.from_bytes(string);
}