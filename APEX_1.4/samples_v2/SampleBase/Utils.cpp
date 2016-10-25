#include "Utils.h"

#include <string>
#include <cstdarg>

HRESULT messagebox_printf(const char* caption, UINT mb_type, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char formatted_text[512];
	_vsnprintf(formatted_text, 512, format, args);
	return MessageBoxA(nullptr, formatted_text, caption, mb_type);
}
