#pragma once

#define COLOR_RED 12
#define COLOR_GREEN 10
#define COLOR_WHITE 15
#define COLOR_YELLOW 14
#define COLOR_GREY 8

class Console
{
public:
	static void Debug(const char* format, ...);
	static void Error(const char* format, ...);
	static void Success(const char* format, ...);
	static void Info(const char* format, ...);
	static void Warning(const char* format, ...);
	static void PrintTitle(const char* name);
	static void WaitForInput();
	static void WaitAndExit();
private:
	static void Print(const char* prefix, const char* format, va_list args);
	static void PrintColored(const char* prefix, const char* format, int color, va_list args);
};