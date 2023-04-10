#pragma once
#include "math/Geometry.h"
#include "Console.h"
#include "ConsoleLevel.h"

class Console;

class ConsoleStream {
public:
	ConsoleStream(ConsoleLevel lvl, std::string origin = "");
	
	ConsoleStream& operator<<(short);
	ConsoleStream& operator<<(int);
	ConsoleStream& operator<<(long);
	ConsoleStream& operator<<(float);
	ConsoleStream& operator<<(Vec2);
	ConsoleStream& operator<<(Vec3);
	ConsoleStream& operator<<(double);

	ConsoleStream& operator<<(char);
	ConsoleStream& operator<<(const std::string&);
	ConsoleStream& operator<<(const char*);
	ConsoleStream& operator<<(const void*);
	ConsoleStream& operator<<(std::ostream os);

private:
	std::string header();
	
	ConsoleLevel _level;
	bool _ignored;
};