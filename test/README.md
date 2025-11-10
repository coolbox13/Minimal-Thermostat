# Unit Tests for ESP32 KNX Thermostat

This directory contains unit tests for the thermostat project using PlatformIO's Unity testing framework.

## Running Tests

### Run all tests:
```bash
pio test
```

### Run specific test:
```bash
pio test -f test_config_manager
```

### Run tests in verbose mode:
```bash
pio test -v
```

## Test Organization

- `test_config_manager/` - Tests for configuration management and validation
- `test_utils/` - Tests for utility functions
- (Future) `test_pid/` - Tests for PID controller logic

## Writing Tests

Tests use the Unity framework. Basic structure:

```cpp
#include <unity.h>

void test_function_name(void) {
    // Arrange
    int expected = 42;

    // Act
    int actual = myFunction();

    // Assert
    TEST_ASSERT_EQUAL(expected, actual);
}

void setUp(void) {
    // Runs before each test
}

void tearDown(void) {
    // Runs after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_function_name);
    return UNITY_END();
}
```

## Common Assertions

- `TEST_ASSERT_TRUE(condition)` - Assert condition is true
- `TEST_ASSERT_FALSE(condition)` - Assert condition is false
- `TEST_ASSERT_EQUAL(expected, actual)` - Assert values are equal
- `TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)` - Assert floats are within delta
- `TEST_ASSERT_NULL(pointer)` - Assert pointer is NULL
- `TEST_ASSERT_NOT_NULL(pointer)` - Assert pointer is not NULL
- `TEST_ASSERT_EQUAL_STRING(expected, actual)` - Assert strings are equal

## Mocking

For components that depend on hardware (WiFi, Sensors, etc.), create mock implementations:

```cpp
// Mock BME280 sensor for testing
class MockBME280 {
public:
    float getTemperature() { return mockTemperature; }
    void setMockTemperature(float temp) { mockTemperature = temp; }
private:
    float mockTemperature = 20.0;
};
```

## Coverage

To generate code coverage reports (requires native environment):

```bash
pio test --environment native --coverage
```

## Continuous Integration

Tests are automatically run by CI/CD pipeline on every commit. See `.github/workflows/tests.yml`.

## Best Practices

1. **Test one thing per test** - Keep tests focused and simple
2. **Use descriptive names** - `test_configManager_returnsDefaultValue_whenKeyNotSet`
3. **Arrange-Act-Assert** - Follow the AAA pattern for clarity
4. **Test edge cases** - Don't just test the happy path
5. **Keep tests fast** - Use mocks to avoid slow I/O or delays
6. **Independent tests** - Tests should not depend on each other

## Debugging Tests

Add `#define UNITY_OUTPUT_VERBOSE` before including unity.h for detailed output.

For debugging with GDB:
```bash
pio test -environment native --verbose
gdb .pio/build/native/program
```
