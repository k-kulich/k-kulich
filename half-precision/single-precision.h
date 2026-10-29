#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <exception>

class SinglePrecision {
    public:
    SinglePrecision(uint32_t data);

    SinglePrecision(uint32_t mantissa, uint32_t exponent, uint32_t sign);

    SinglePrecision(const std::string& hex, std::string view = "");

    SinglePrecision(const char* hex, std::string view = "");

    SinglePrecision& operator=(const SinglePrecision& other);

    bool is_zero() const;

    bool is_infty() const;

    bool is_denormalised() const;

    bool is_nan() const;

    SinglePrecision sum(const SinglePrecision& other, int tow);

    SinglePrecision minus(const SinglePrecision& other, int tow);

    SinglePrecision multiply(const SinglePrecision& other, int tow, bool needs_round = true);

    SinglePrecision divide(const SinglePrecision& other, int tow);

    SinglePrecision fma(const SinglePrecision& second, const SinglePrecision& third, int tow);

    SinglePrecision operation(const SinglePrecision& other, char oper, int tow_mode);

    SinglePrecision oper_mad_fma(const SinglePrecision& second, const SinglePrecision& third, int tow_mode, bool mad = true);

    std::string to_string(int tow_mode);

    std::string get_hex();

    float get_num();

    private:
    std::string view_{""};
    union {
        float num_;
        uint32_t bits_;
        struct {
            uint32_t mantissa_: 23;
            uint32_t exponent_: 8;
            uint32_t sign_: 1;
        };
    };

    bool is_bigger_(const SinglePrecision& other);

    SinglePrecision multiply_denorm_(const SinglePrecision& other, uint64_t res_mant, uint32_t sign, int tow, bool needs_round = true);

    SinglePrecision overflow_(SinglePrecision& other, int tow, bool needs_round = true);

    SinglePrecision plus_equal_exp_(const SinglePrecision& bigger, const SinglePrecision& lower, int tow);

    SinglePrecision plus_inequal_exp_(const SinglePrecision& bigger, const SinglePrecision& lower, int tow);

    SinglePrecision minus_equal_exp_(const SinglePrecision& bigger, const SinglePrecision& lower, int tow);

    SinglePrecision minus_inequal_exp_(const SinglePrecision& bigger, const SinglePrecision& lower, int tow);

    SinglePrecision round_(int64_t exp, uint64_t mant, uint64_t half, uint64_t extra, uint32_t sign, int tow);

    int64_t normalise_(uint64_t& number, int bit_count = 23);
};

