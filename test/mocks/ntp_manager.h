#ifndef MOCK_NTP_MANAGER_H
#define MOCK_NTP_MANAGER_H

#include <time.h>

/**
 * Mock NTP Manager for testing
 */
class NTPManager {
private:
    time_t _mockTime;
    bool _timeValid;

    NTPManager() : _mockTime(1700000000), _timeValid(true) {}

public:
    static NTPManager& getInstance() {
        static NTPManager instance;
        return instance;
    }

    time_t getCurrentTime() {
        if (_timeValid) {
            return _mockTime;
        }
        return 0;
    }

    bool isTimeValid() {
        return _timeValid;
    }

    void begin() {}
    void update() {}

    // Test control methods
    void setMockTime(time_t time) {
        _mockTime = time;
    }

    void setMockTimeValid(bool valid) {
        _timeValid = valid;
    }

    void incrementMockTime(time_t seconds) {
        _mockTime += seconds;
    }

    void resetMock() {
        _mockTime = 1700000000;
        _timeValid = true;
    }
};

#endif // MOCK_NTP_MANAGER_H
