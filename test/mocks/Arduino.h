#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

// Forward declare std::string for conversion
#ifdef __cplusplus
#include <string>
#endif

// Simple String class mock (with std::string compatibility)
class String {
public:
    String() : data(nullptr), len(0), capacity(0) {}

    String(const char* str) : data(nullptr), len(0), capacity(0) {
        if (str) {
            len = strlen(str);
            capacity = len + 1;
            data = (char*)malloc(capacity);
            strcpy(data, str);
        }
    }

    String(const String& other) : data(nullptr), len(0), capacity(0) {
        if (other.data) {
            len = other.len;
            capacity = other.capacity;
            data = (char*)malloc(capacity);
            strcpy(data, other.data);
        }
    }

#ifdef __cplusplus
    // Constructor from std::string
    String(const std::string& str) : data(nullptr), len(0), capacity(0) {
        if (!str.empty()) {
            len = str.length();
            capacity = len + 1;
            data = (char*)malloc(capacity);
            memcpy(data, str.c_str(), len);
            data[len] = '\0';
        }
    }

    // Conversion operator to std::string
    operator std::string() const {
        return data ? std::string(data) : std::string();
    }
#endif

    ~String() { if (data) free(data); }

    String& operator=(const String& other) {
        if (this != &other) {
            if (data) free(data);
            data = nullptr;
            len = 0;
            capacity = 0;
            if (other.data) {
                len = other.len;
                capacity = other.capacity;
                data = (char*)malloc(capacity);
                strcpy(data, other.data);
            }
        }
        return *this;
    }

    const char* c_str() const { return data ? data : ""; }
    size_t length() const { return len; }

    bool concat(const char* str) {
        if (!str) return false;
        size_t strLen = strlen(str);
        size_t newLen = len + strLen;
        if (newLen + 1 > capacity) {
            capacity = (newLen + 1) * 2; // grow with some headroom
            char* newData = (char*)realloc(data, capacity);
            if (!newData) return false;
            data = newData;
        }
        strcpy(data + len, str);
        len = newLen;
        return true;
    }

    bool concat(const String& str) {
        return concat(str.c_str());
    }

    bool concat(char c) {
        char buf[2] = {c, '\0'};
        return concat(buf);
    }

    // Comparison operators
    bool operator==(const char* str) const {
        if (!data && !str) return true;
        if (!data || !str) return false;
        return strcmp(data, str) == 0;
    }

    bool operator!=(const char* str) const {
        return !(*this == str);
    }

    bool operator==(const String& other) const {
        return (*this == other.c_str());
    }

    bool operator!=(const String& other) const {
        return !(*this == other);
    }

private:
    char* data;
    size_t len;
    size_t capacity;
};

// Arduino types
typedef bool boolean;
typedef uint8_t byte;

// Arduino constants
#define HIGH 0x1
#define LOW  0x0
#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

// Math constants
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif

// Mock time functions
extern unsigned long _mock_millis;
extern unsigned long _mock_micros;

inline unsigned long millis() { return _mock_millis; }
inline unsigned long micros() { return _mock_micros; }
inline void delay(unsigned long ms) { _mock_millis += ms; }
inline void delayMicroseconds(unsigned int us) { _mock_micros += us; }

// Math functions
inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline long constrain(long x, long a, long b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

inline double constrain(double x, double a, double b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

// Utility functions
inline void pinMode(uint8_t pin, uint8_t mode) {}
inline void digitalWrite(uint8_t pin, uint8_t val) {}
inline int digitalRead(uint8_t pin) { return LOW; }

// Print base class (for ArduinoJson compatibility)
class Print {
public:
    virtual size_t write(uint8_t) = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) {
        size_t n = 0;
        while (size--) {
            n += write(*buffer++);
        }
        return n;
    }
    size_t print(const char* str) {
        return write((const uint8_t*)str, strlen(str));
    }
    size_t println(const char* str) {
        size_t n = print(str);
        n += print("\n");
        return n;
    }
};

// Printable interface (for ArduinoJson compatibility)
class Printable {
public:
    virtual size_t printTo(Print& p) const = 0;
};

// Stream base class (for ArduinoJson compatibility)
class Stream : public Print {
public:
    virtual int available() { return 0; }
    virtual int read() { return -1; }
    virtual int peek() { return -1; }
    virtual size_t write(uint8_t) { return 0; }

    size_t readBytes(char *buffer, size_t length) {
        size_t count = 0;
        while (count < length) {
            int c = read();
            if (c == -1) break;
            *buffer++ = (char)c;
            count++;
        }
        return count;
    }
};

// Serial mock
class SerialMock : public Stream {
public:
    void begin(unsigned long baud) {}
    void print(const char* str) {}
    void print(int val) {}
    void print(float val) {}
    void println(const char* str) {}
    void println(int val) {}
    void println(float val) {}
    void println() {}
    template<typename T> void print(T val) {}
    template<typename T> void println(T val) {}
};

extern SerialMock Serial;

// Flash string helper (for ArduinoJson PROGMEM support)
class __FlashStringHelper;
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper *>(pstr_pointer))
#define PSTR(s) (s)

// PROGMEM support for native platform
#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef PGM_P
#define PGM_P const char *
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif

#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

#ifndef pgm_read_float
#define pgm_read_float(addr) (*(const float *)(addr))
#endif

#ifndef pgm_read_ptr
#define pgm_read_ptr(addr) (*(void * const *)(addr))
#endif

#ifndef strlen_P
#define strlen_P(s) strlen(s)
#endif

#ifndef memcpy_P
#define memcpy_P(dest, src, num) memcpy((dest), (src), (num))
#endif

#ifndef strcpy_P
#define strcpy_P(dest, src) strcpy((dest), (src))
#endif

#ifndef strcmp_P
#define strcmp_P(s1, s2) strcmp((s1), (s2))
#endif

#ifndef strncmp_P
#define strncmp_P(s1, s2, n) strncmp((s1), (s2), (n))
#endif

#ifndef F
#define F(string_literal) (string_literal)
#endif

#endif // MOCK_ARDUINO_H
