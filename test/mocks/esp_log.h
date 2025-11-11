#ifndef MOCK_ESP_LOG_H
#define MOCK_ESP_LOG_H

// Mock ESP32 logging for native platform
#define ESP_LOGE(tag, format, ...)
#define ESP_LOGW(tag, format, ...)
#define ESP_LOGI(tag, format, ...)
#define ESP_LOGD(tag, format, ...)
#define ESP_LOGV(tag, format, ...)

#endif // MOCK_ESP_LOG_H
