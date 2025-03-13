#pragma once

#include <Arduino.h>

class Base64 {
public:
    static String encode(const uint8_t* data, size_t length) {
        static const char* ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        String out;
        out.reserve(4 * ((length + 2) / 3));
        int i = 0;
        while (i < length) {
            uint32_t octet_a = i < length ? data[i++] : 0;
            uint32_t octet_b = i < length ? data[i++] : 0;
            uint32_t octet_c = i < length ? data[i++] : 0;
            uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;
            out += ALPHABET[(triple >> 18) & 0x3F];
            out += ALPHABET[(triple >> 12) & 0x3F];
            out += ALPHABET[(triple >> 6) & 0x3F];
            out += ALPHABET[triple & 0x3F];
        }
        switch (length % 3) {
            case 1: out[out.length() - 2] = '=';
            case 2: out[out.length() - 1] = '=';
        }
        return out;
    }

    static String encode(const String& data) {
        return encode((const uint8_t*)data.c_str(), data.length());
    }

    static String decode(const String& input) {
        static const uint8_t lookup[] = {
            62,  255, 62,  255, 63,  52,  53, 54, 55, 56, 57, 58, 59, 60, 61, 255,
            255, 255, 255, 255, 255, 255, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
            10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
            255, 255, 255, 255, 63,  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
            36,  37,  38,  39,  40,  41,  42, 43, 44, 45, 46, 47, 48, 49, 50, 51
        };

        size_t input_length = input.length();
        if (input_length % 4 != 0) return "";

        size_t output_length = input_length / 4 * 3;
        if (input[input_length - 1] == '=') output_length--;
        if (input[input_length - 2] == '=') output_length--;

        String out;
        out.reserve(output_length);

        uint32_t bits = 0;
        uint8_t char_count = 0;

        for (size_t i = 0; i < input_length; i++) {
            char c = input[i];
            if (c == '=') break;

            if (c < '+' || c > 'z') return "";
            uint8_t val = lookup[c - '+'];
            if (val == 255) return "";

            bits = (bits << 6) | val;
            char_count++;

            if (char_count == 4) {
                out += (char)((bits >> 16) & 0xFF);
                out += (char)((bits >> 8) & 0xFF);
                out += (char)(bits & 0xFF);
                bits = 0;
                char_count = 0;
            }
        }

        if (char_count) {
            bits <<= (6 * (4 - char_count));
            if (char_count == 3) {
                out += (char)((bits >> 16) & 0xFF);
                out += (char)((bits >> 8) & 0xFF);
            } else if (char_count == 2) {
                out += (char)((bits >> 16) & 0xFF);
            }
        }

        return out;
    }
}; 