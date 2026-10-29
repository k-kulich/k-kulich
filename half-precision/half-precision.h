#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <exception>

class HalfPrecision {
    public:
    HalfPrecision(uint16_t data);

    HalfPrecision(uint16_t mantissa, uint16_t exponent, uint16_t sign);

    HalfPrecision(const std::string& hex, std::string view = "");

    HalfPrecision(const char* hex, std::string view = "");

    HalfPrecision& operator=(const HalfPrecision& other);

    bool is_zero() const;

    bool is_infty() const;

    bool is_denormalised() const;

    bool is_nan() const;

    HalfPrecision sum(const HalfPrecision& other, int tow);

    HalfPrecision minus(const HalfPrecision& other, int tow);

    HalfPrecision multiply(const HalfPrecision& other, int tow, bool needs_round = true);

    HalfPrecision divide(const HalfPrecision& other, int tow);

    HalfPrecision fma(const HalfPrecision& second, const HalfPrecision& third, int tow);

    HalfPrecision operation(const HalfPrecision& other, char oper, int tow_mode);

    HalfPrecision oper_mad_fma(const HalfPrecision& second, const HalfPrecision& third, int tow_mode, bool mad = true);

    std::string to_string(int tow_mode);

    std::string get_hex();

    float get_num();

    private:
    std::string view_{""};
    union {
        float num_;
        uint16_t bits_;
        struct {
            uint16_t mantissa_: 10;
            uint16_t exponent_: 5;
            uint16_t sign_: 1;
        };
    };

    bool is_bigger_(const HalfPrecision& other);

    HalfPrecision multiply_denorm_(const HalfPrecision& other, uint32_t res_mant, uint16_t sign, int tow, bool needs_round = true);

    HalfPrecision overflow_(HalfPrecision& other, int tow, bool needs_round = true);

    HalfPrecision plus_equal_exp_(const HalfPrecision& bigger, const HalfPrecision& lower, int tow);

    HalfPrecision plus_inequal_exp_(const HalfPrecision& bigger, const HalfPrecision& lower, int tow);

    HalfPrecision minus_equal_exp_(const HalfPrecision& bigger, const HalfPrecision& lower, int tow);

    HalfPrecision minus_inequal_exp_(const HalfPrecision& bigger, const HalfPrecision& lower, int tow);

    HalfPrecision round_(int32_t exp, uint32_t mant, uint32_t half, uint32_t extra, uint16_t sign, int tow);

    int32_t normalise_(uint32_t& number, int bit_count = 10);
};

