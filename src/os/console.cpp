#include "console.h"
#include <Arduino.h>

const String Console::endl = "\n";
ConsoleLevel Console::m_level = ConsoleLevel::INFO;

ConsoleStream Console::trace(const ServiceID& origin) {
	return ConsoleStream(ConsoleLevel::VERBOSE, origin);
}

ConsoleStream Console::info(const ServiceID& origin) {
	return ConsoleStream(ConsoleLevel::INFO, origin);
}

ConsoleStream Console::warn(const ServiceID& origin) {
	return ConsoleStream(ConsoleLevel::WARNING, origin);
}

ConsoleStream Console::error(const ServiceID& origin) {
	return ConsoleStream(ConsoleLevel::CRITICAL, origin);
}

ConsoleStream Console::success(const ServiceID& origin) {
	return ConsoleStream( ConsoleLevel::SUCCESS, origin);
}


ConsoleStream Console::info(const String& origin) {
	return ConsoleStream(ConsoleLevel::INFO, origin);
}

ConsoleStream Console::warn(const String& origin) {
	return ConsoleStream(ConsoleLevel::WARNING, origin);
}

ConsoleStream Console::error(const String& origin) {
	return ConsoleStream(ConsoleLevel::CRITICAL, origin);
}

ConsoleStream Console::trace(const String& origin) {
	return ConsoleStream(ConsoleLevel::VERBOSE, origin);
}

ConsoleStream Console::success(const String& origin) {
	return ConsoleStream(ConsoleLevel::SUCCESS, origin);
}


void header(){
	Serial.println("");
	Serial.println("   _______       _       _      _     _");
	Serial.println("  |__   __|     (_)     | |    (_)   | |");
	Serial.println("     | |_      ___ _ __ | |     _  __| | __ _ _ __");
	Serial.println("     | \\ \\ /\\ / / | '_ \\| |    | |/ _` |/ _` | '__|");
	Serial.println("     | |\\ V  V /| | | | | |____| | (_| | (_| | |");
	Serial.println("     |_| \\_/\\_/ |_|_| |_|______|_|\\__,_|\\__,_|_|");
	Serial.println("");
	Serial.println();
	Serial.println("Author  : Nadarbreicq, JulesTopart ");
	Serial.println();
	Serial.print("Twinsystem... compiled  ");
	Serial.print(__DATE__);
	Serial.print(" at ");
	Serial.println(__TIME__);

}

void Console::init(){
	Serial.begin(115200);
	header();
}

void Console::write(const char* str) {
	Serial.write(str);
}

void Console::plot(const String& n, String s){
	print(">" + n + ":"); println(s);
}

void Console::print(const String& s){
	Serial.print(s);
}

void Console::println(const String& s){
	Serial.println(s);
}

void Console::prettyPrint(const String& s){
	int l = 0;
	line();
	Serial.print(l);
	Serial.print(":\t");
	for (size_t i = 0; i <= s.length(); i++){
		if(s[i] == '\n'){
			Serial.println();
			Serial.print(++l);
			Serial.print(":\t");
		}else{
			Serial.print(s[i]);
		}
	}
	Serial.println(s[s.length() - 1]);
	line();
}

void Console::line(){
	println("_________________________________________");
}