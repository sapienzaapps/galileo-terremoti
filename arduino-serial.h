//
// Created by enrico on 21/06/15.
//

#ifndef ARDUINO

#ifndef GALILEO_TERREMOTI_ARDUINO_SERIAL_H
#define GALILEO_TERREMOTI_ARDUINO_SERIAL_H

class _Serial {
public:
	void begin(int);
	void print(const char*);
	void print(int);
	void print(long);
	void print(float);
	void print(int, int);
	void println();
	void println(const char*);
	void println(int);
	void println(long);
	void println(float);
	void println(int, int);
	void write(void*, ssize_t size);
};

void _Serial::begin(int p) {}

void _Serial::print(const char* p) {}
void _Serial::print(int p) {}
void _Serial::print(long p) {}
void _Serial::print(float p) {}
void _Serial::print(int p, int t) {}

void _Serial::println() {}
void _Serial::println(const char* p) {}
void _Serial::println(int p) {}
void _Serial::println(long p) {}
void _Serial::println(float p) {}
void _Serial::println(int p, int t) {}

void _Serial::write(void* p, ssize_t t) {}

_Serial Serial;

#endif //GALILEO_TERREMOTI_ARDUINO_SERIAL_H

#endif