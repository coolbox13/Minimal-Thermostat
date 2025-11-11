#ifndef MOCK_SPIFFS_H
#define MOCK_SPIFFS_H

#include "Arduino.h"
#include <map>
#include <string>

/**
 * Mock File class for SPIFFS
 */
class File {
private:
    std::string _content;
    size_t _position;
    bool _isOpen;
    std::string _name;

public:
    File()
        : _position(0)
        , _isOpen(false) {}

    File(const std::string& name, const std::string& content)
        : _content(content)
        , _position(0)
        , _isOpen(true)
        , _name(name) {}

    operator bool() const {
        return _isOpen;
    }

    size_t size() {
        return _content.size();
    }

    size_t available() {
        return _content.size() - _position;
    }

    int read() {
        if (_position < _content.size()) {
            return _content[_position++];
        }
        return -1;
    }

    size_t read(uint8_t* buf, size_t size) {
        size_t bytesRead = 0;
        while (bytesRead < size && _position < _content.size()) {
            buf[bytesRead++] = _content[_position++];
        }
        return bytesRead;
    }

    size_t write(uint8_t data) {
        if (_position < _content.size()) {
            _content[_position++] = data;
        } else {
            _content.push_back(data);
            _position++;
        }
        return 1;
    }

    size_t write(const uint8_t* buf, size_t size) {
        for (size_t i = 0; i < size; i++) {
            write(buf[i]);
        }
        return size;
    }

    void close() {
        _isOpen = false;
    }

    String name() {
        return String(_name.c_str());
    }

    void seek(size_t pos) {
        _position = pos;
    }

    size_t position() {
        return _position;
    }
};

/**
 * Mock SPIFFS filesystem for testing
 */
class SPIFFSClass {
private:
    bool _mounted;
    std::map<std::string, std::string> _files;

public:
    SPIFFSClass()
        : _mounted(false) {}

    bool begin(bool formatOnFail = false) {
        _mounted = true;
        return true;
    }

    void end() {
        _mounted = false;
    }

    File open(const char* path, const char* mode = "r") {
        if (!_mounted) {
            return File();
        }

        std::string pathStr(path);
        if (_files.count(pathStr)) {
            return File(pathStr, _files[pathStr]);
        }

        // Create new file if in write mode
        if (mode[0] == 'w' || mode[0] == 'a') {
            _files[pathStr] = "";
            return File(pathStr, "");
        }

        return File();
    }

    bool exists(const char* path) {
        return _files.count(path) > 0;
    }

    bool remove(const char* path) {
        return _files.erase(path) > 0;
    }

    bool rename(const char* pathFrom, const char* pathTo) {
        if (!_files.count(pathFrom)) {
            return false;
        }
        _files[pathTo] = _files[pathFrom];
        _files.erase(pathFrom);
        return true;
    }

    size_t totalBytes() {
        return 1024 * 1024; // 1MB
    }

    size_t usedBytes() {
        size_t total = 0;
        for (const auto& file : _files) {
            total += file.second.size();
        }
        return total;
    }

    // ===== Test Control Methods =====

    void setMockFileContent(const char* path, const char* content) {
        _files[path] = content;
    }

    std::string getMockFileContent(const char* path) {
        if (_files.count(path)) {
            return _files[path];
        }
        return "";
    }

    void resetMock() {
        _mounted = false;
        _files.clear();
    }
};

extern SPIFFSClass SPIFFS;

#endif // MOCK_SPIFFS_H
