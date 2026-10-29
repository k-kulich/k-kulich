#include "half-precision.h"

struct HalfHelpData {   
    HalfPrecision plus_zero_{"0x0000", "0x0.000p+0"};
    HalfPrecision nan_{"0xFC10", "nan"};
    HalfPrecision minus_zero_{"0x1000", "-0x0.000p+0"};
    HalfPrecision plus_infty_{"0x7C00", "inf"};
    HalfPrecision minus_infty_{"0xFC00", "-inf"};
    HalfPrecision plus_overflow_{"0x7BFE", "0x1.ffep+15"};
    HalfPrecision minus_overflow_{"0xFBFE", "-0x1.ffep+15"};
};

static HalfHelpData h_help{};

uint32_t h_bin_pow(int pow) {
    uint32_t res{1};
    return (res << pow);
}

HalfPrecision::HalfPrecision(uint16_t data) {
    bits_ = data;
}

HalfPrecision::HalfPrecision(uint16_t mantissa, uint16_t exponent, uint16_t sign) {
    mantissa_ = mantissa;
    exponent_ = exponent;
    sign_ = sign;
}

HalfPrecision::HalfPrecision(const std::string& hex, std::string view) {
    bits_ = std::stoul(hex.substr(2), nullptr, 16);
    view_ = view;
}

HalfPrecision::HalfPrecision(const char* hex, std::string view) {
    std::string str{hex};
    bits_ = std::stoul(str.substr(2), nullptr, 16);
    view_ = view;
}

HalfPrecision& HalfPrecision::operator=(const HalfPrecision& other) {
    bits_ = other.bits_;
    return *this;
}

bool HalfPrecision::is_zero() const {
    return (exponent_ == 0) && (mantissa_ == 0);
}

bool HalfPrecision::is_infty() const {
    return (exponent_ == 31) && (mantissa_ == 0);
}

bool HalfPrecision::is_denormalised() const {
    return (exponent_ == 0) && (mantissa_ != 0);
}

bool HalfPrecision::is_nan() const {
    return (exponent_ == 31) && (mantissa_ != 0);
}

HalfPrecision HalfPrecision::sum(const HalfPrecision& other, int tow) {
    // some basic cases
    if (is_nan() || other.is_nan()) return is_nan() ? *this : other;
    if (sign_ != other.sign_ && exponent_ == other.exponent_ && mantissa_ == other.mantissa_) return h_help.plus_zero_;
    if (is_infty() && other.is_infty() && (sign_ != other.sign_)) return h_help.nan_;
    if (is_infty()) return sign_ == 0 ? h_help.plus_infty_ : h_help.minus_infty_;
    if (other.is_infty()) return other.sign_ == 0 ? h_help.plus_infty_ : h_help.minus_infty_;
    if (is_zero() && other.is_zero()) {
        if (tow == 3 || (sign_ == other.sign_ && sign_ == 1)) return h_help.minus_zero_;
        return h_help.plus_zero_;
    }
    if (is_zero()) return other;
    if (other.is_zero()) return *this;

    HalfPrecision bigger{is_bigger_(other) ? bits_ : other.bits_};
    HalfPrecision lower{is_bigger_(other) ? other.bits_ : bits_};

    if (sign_ == other.sign_) {
        // sum
        if (exponent_ == other.exponent_) {
            return plus_equal_exp_(bigger, lower, tow);
        } 
        return plus_inequal_exp_(bigger, lower, tow);
    }
    // subtrack
    if (exponent_ == other.exponent_) {
        return minus_equal_exp_(bigger, lower, tow);
    }
    return minus_inequal_exp_(bigger, lower, tow);
}

HalfPrecision HalfPrecision::minus(const HalfPrecision& other, int tow) {
    HalfPrecision reversed = other;
    reversed.sign_ = 1 - other.sign_;
    return sum(reversed, tow);
}

HalfPrecision HalfPrecision::multiply(const HalfPrecision& other, int tow, bool needs_round) {
    // some basic cases
    if (is_nan() || other.is_nan()) return is_nan() ? *this : other;
    if (is_infty() && other.is_zero()) return h_help.nan_;
    if (is_zero() && other.is_infty()) return h_help.nan_;
    if (is_zero() && other.is_zero()) {
        if (sign_ == other.sign_) return h_help.plus_zero_;
        return h_help.minus_zero_;
    }
    if (is_infty() && other.is_infty()) {
        if (sign_ == other.sign_) return h_help.plus_infty_;
        return h_help.minus_infty_;
    }

    uint16_t sign = (sign_ != other.sign_);
    uint32_t lmod = is_denormalised() ? mantissa_ : h_bin_pow(10) + mantissa_;
    uint32_t rmod = other.is_denormalised() ? other.mantissa_ : h_bin_pow(10) + other.mantissa_;
    uint32_t res_mant = lmod * rmod;  // 11 1011 0100 0101 0111

    if (is_denormalised() || other.is_denormalised()) return multiply_denorm_(other, res_mant, sign, tow, needs_round);

    int32_t res_exp{0};
    if ((res_mant >> 21) == 0) res_exp = exponent_ + other.exponent_ - 15;  // 16 + 16 - 15 
    else res_exp = exponent_ + other.exponent_ - 14;

    if (res_exp >= 31) {
        HalfPrecision res = HalfPrecision(res_mant, res_exp, sign);
        return overflow_(res, tow, needs_round);
    }

    uint32_t half, half_for_ne, shifted;
    if ((res_mant >> 21 == 0)) {
        half = res_mant & (h_bin_pow(10) - 1);  //  00 0101 0111
        shifted = (res_mant >> 10) & (h_bin_pow(10) - 1);  // 00 1110 1101
        half_for_ne = h_bin_pow(9);  // 10 0000 0000
    } else {
        half = res_mant % h_bin_pow(11);
        shifted = (res_mant >> 11) & (h_bin_pow(10) - 1);
        half_for_ne = h_bin_pow(10);
    }

    if (res_exp < 0) return multiply_denorm_(other, res_mant, sign, tow, needs_round);

    if (needs_round) return round_(res_exp, shifted, half_for_ne, half, sign, tow); // 00 1110 1110
    return HalfPrecision(shifted, res_exp, sign);
}

HalfPrecision HalfPrecision::divide(const HalfPrecision& other, int tow) {
    if (is_nan() || other.is_nan()) return is_nan() ? *this : other;
    if (is_zero() && other.is_zero()) return h_help.nan_;
    if (is_infty() && other.is_infty()) return h_help.nan_;
    if (other.is_zero()) {
        if (sign_ == other.sign_) return h_help.plus_infty_;
        return h_help.minus_infty_;
    }
    if (is_zero() || other.is_infty()) {
        if (sign_ == other.sign_) return h_help.plus_zero_;
        return h_help.minus_zero_;
    }

    uint32_t lmod = is_denormalised() ? mantissa_ : mantissa_ + h_bin_pow(10);
    uint32_t rmod = other.is_denormalised() ? mantissa_ : mantissa_ + h_bin_pow(10);
    uint32_t res_mod = (lmod << 12) / rmod;
    int32_t res_exp = exponent_ - other.exponent_;
    if (res_mod < h_bin_pow(12)) {
        int i{0};
        while (res_mod < h_bin_pow(12)) {
            ++i;
            res_mod = (lmod << (12 + i)) / rmod;
        }
        res_exp -= i;
    } else if (res_mod >= h_bin_pow(13)) {
        int i{0};
        while (res_mod >= h_bin_pow(13)) {
            ++i;
            res_mod = (lmod << (12 - i)) / rmod;
        }
        res_exp += i;
    }
    res_mod = res_mod & (h_bin_pow(10) - 1);
    uint16_t sign = (sign_ != other.sign_);
    uint32_t half = res_mod % rmod;

    switch (tow) {
        case 0: {
            return HalfPrecision(res_mod, res_exp, sign);
        }
        case 1: {
            if (half * 2 < rmod) return HalfPrecision(res_mod, res_exp, sign);
            if (half * 2 > rmod) return HalfPrecision(res_mod + 1, res_exp, sign);
            if (half % 2 == 0) return HalfPrecision(res_mod, res_exp, sign);
            return HalfPrecision(res_mod + 1, res_exp, sign);
        }
        case 2: {
            if (half == 0) return HalfPrecision(res_mod, res_exp, sign);
            if (sign_ == 0) return HalfPrecision(res_mod + 1, res_exp, sign);
            return HalfPrecision(res_mod - 1, res_exp, sign);
        }
        case 3: {
            if (half == 0) return HalfPrecision(res_mod, res_exp, sign);
            if (sign_ == 1) return HalfPrecision(res_mod + 1, res_exp, sign);
            return HalfPrecision(res_mod - 1, res_exp, sign);
        }
        default: throw std::runtime_error("Wrong toward case");
    }
}

HalfPrecision HalfPrecision::fma(const HalfPrecision& second, const HalfPrecision& third, int tow) {
    HalfPrecision temp = multiply(second, tow, false);
    return temp.sum(third, tow);
}

HalfPrecision HalfPrecision::operation(const HalfPrecision& other, char oper, int tow_mode) {
    if (oper == '+') return sum(other, tow_mode);
    if (oper == '-') return minus(other, tow_mode);
    if (oper == '/') return divide(other, tow_mode);
    if (oper == '*') return multiply(other, tow_mode);
    return *this;
}

HalfPrecision HalfPrecision::oper_mad_fma(const HalfPrecision& second, const HalfPrecision& third, int tow_mode, bool mad) {
    if (mad) {
        HalfPrecision temp = multiply(second, tow_mode);
        return temp.sum(third, tow_mode);
    }
    return fma(second, third, tow_mode);
}

std::string HalfPrecision::to_string(int tow_mode) {
    if (view_ != "") return view_;
    if (is_zero()) {
        if (sign_ == 0) return "0x0.000p+0";
        return "-0x0.000p+0";
    }
    if (is_infty()) {
        if (sign_ == 0) return "inf";
        return "-inf";
    }
    if (is_nan()) return "nan";

    std::string res;
    int32_t res_exp = exponent_;
    if (sign_ == 1) res.push_back('-');
    res.append("0x1.");
    if (is_denormalised()) {
        std::stringstream str;
        int i;
        res_exp = -14;
        for (i = 1; i <= 10; ++i) {
            if (mantissa_ / h_bin_pow(10 - i) != 0) {
                str << std::hex << std::setw(3) << std::setfill('0') << (((mantissa_ * h_bin_pow(i)) % h_bin_pow(10)) << 2);
                res_exp -= i;
                break;
            }
        }
        res.append(str.str());
    } else {
        std::stringstream str;
        str << std::hex << std::setw(3) << std::setfill('0') << (mantissa_ << 2); // 1000111100  have: 100011110
        res.append(str.str());                                                  // 0 10010 0010001111
        res_exp -= 15;  // 0 10000 0101000101   *    0 10000 1011101011   = mant: 11 0100 0101 0111
    }
    res.push_back('p');
    if (res_exp >= 0) res.push_back('+');
    res.append(std::to_string(res_exp));

    return res;
}

std::string HalfPrecision::get_hex() {
    std::stringstream str;
    str << std::hex << std::setw(4) << std::setfill('0') << bits_;
    std::string temp = str.str();
    std::string res{};
    for (size_t i = 0; i < temp.size(); ++i) {
        if (temp[i] >= 'a' && temp[i] <= 'f') res.push_back(char('A' + temp[i] - 'a'));
        else res.push_back(temp[i]);
    }
    return "0x" + res;
}

float HalfPrecision::get_num() {
    return num_;
}

bool HalfPrecision::is_bigger_(const HalfPrecision& other) {
    return (exponent_ > other.exponent_ || (exponent_ == other.exponent_ && mantissa_ > other.mantissa_));
}

HalfPrecision HalfPrecision::multiply_denorm_(const HalfPrecision& other, uint32_t res_mant, uint16_t sign, int tow, bool needs_round) {
    int32_t res_exp{0};
    uint32_t extra, half, shifted;
    res_exp += is_denormalised() ? -14 : (exponent_ - 15);
    res_exp += other.is_denormalised() ? -14 : (other.exponent_ - 15);

    if ((res_mant >> 21) == 0) {
        if (res_exp <= -14) {
            shifted = (res_mant >> (10 - (res_exp + 14))) & (h_bin_pow(10) - 1);
            extra = res_mant & (h_bin_pow(10 - (res_exp + 14)) - 1);
            half = h_bin_pow(9 - (res_exp + 14));
            res_exp = 0;
        } else {
            res_exp -= normalise_(res_mant, 20);
            shifted = (res_mant >> 10) & (h_bin_pow(10) - 1);
            extra = res_mant & (h_bin_pow(10) - 1);
            half = h_bin_pow(9);
            res_exp += 15;
        }
    } else {
        res_exp++;
        if (res_exp <= -14) {
            shifted = (res_mant >> (11 - (res_exp + 14))) & (h_bin_pow(10) - 1);
            extra = res_mant & (h_bin_pow(11 - (res_exp + 14)) - 1);
            half = h_bin_pow(10 - (res_exp + 14));
            res_exp = 0;
        } else {
            shifted = (res_mant >> 11) & (h_bin_pow(10) - 1);
            extra = res_mant & (h_bin_pow(11) - 1);
            half = h_bin_pow(10);
            res_exp += 15;
        }
    }
    if (needs_round) return round_(res_exp, shifted, half, extra, sign, tow);
    return HalfPrecision(shifted, res_exp, sign);
}

HalfPrecision HalfPrecision::overflow_(HalfPrecision& other, int tow, bool needs_round) {
    if (!needs_round) return other;

    switch (tow) {
    case 0: {
        if (other.sign_ == 0) return h_help.plus_overflow_;

        return h_help.minus_overflow_;
    }
    case 1: {
        if (other.sign_ == 0) return h_help.plus_infty_;
        return h_help.minus_infty_;
    } 
    case 2: {
        if (other.sign_ == 0) return h_help.plus_infty_;
        return h_help.minus_overflow_;
    } 
    case 3: {
        if (other.sign_ == 1) return h_help.minus_infty_;
        return h_help.plus_overflow_;
    } 
    default: throw std::runtime_error("Wrong toward case");
    } 
}

HalfPrecision HalfPrecision::plus_equal_exp_(const HalfPrecision& bigger, const HalfPrecision& lower, int tow) {
    uint32_t res_mant, extra, shifted;
    uint32_t res_exp{0};
    if (!bigger.is_denormalised() && !lower.is_denormalised()) res_mant = 2 * h_bin_pow(10) + bigger.mantissa_ + lower.mantissa_;
    else if (!bigger.is_denormalised() || !lower.is_denormalised()) res_mant = h_bin_pow(10) + bigger.mantissa_ + lower.mantissa_;
    else res_mant = bigger.mantissa_ + lower.mantissa_;
    
    if ((res_mant >> 11) == 0) {
        shifted = (res_mant) & (h_bin_pow(10) - 1);
        extra = 0;
    } else {
        shifted = (res_mant >> 1) & (h_bin_pow(10) - 1);
        extra = res_mant & 1;
        res_exp++;
    }
    res_exp += bigger.exponent_;

    HalfPrecision res = round_(res_exp, shifted, 0, extra, bigger.sign_, tow);

    if (res.exponent_ < 31) return res;

    return overflow_(res, tow);
}

HalfPrecision HalfPrecision::plus_inequal_exp_(const HalfPrecision& bigger, const HalfPrecision& lower, int tow) {
    uint32_t res_mant, half, shifted;
    int32_t res_exp{bigger.exponent_};
    if (lower.is_denormalised()) {
        shifted = lower.mantissa_ >> (bigger.exponent_ - lower.exponent_);
        res_mant = shifted + h_bin_pow(10) + bigger.mantissa_;
        half = (lower.mantissa_) & (h_bin_pow(bigger.exponent_ - lower.exponent_) - 1);
    } else {
        shifted = (lower.mantissa_ + h_bin_pow(10)) >> h_bin_pow(bigger.exponent_ - lower.exponent_);
        res_mant = shifted + h_bin_pow(10) + bigger.mantissa_;
        half = (lower.mantissa_ + h_bin_pow(10)) & (h_bin_pow(bigger.exponent_ - lower.exponent_) - 1);
    }

    if ((res_mant >> 11) == 0) {
        HalfPrecision res = round_(res_exp, res_mant & (h_bin_pow(10) - 1), 
                                    h_bin_pow(bigger.exponent_ - lower.exponent_ - 1), half, bigger.sign_, tow);
        return res;
    }

    res_exp++;
    if (res_exp >= 31) {
        HalfPrecision res = HalfPrecision(res_mant, res_exp, bigger.sign_);
        return overflow_(res, tow);
    }

    res_mant = (res_mant >> 1) & (h_bin_pow(10) - 1);
    HalfPrecision res = round_(res_exp, res_mant, h_bin_pow(bigger.exponent_ - lower.exponent_ - 1), half, bigger.sign_, tow);

    if (res.exponent_ >= 31) return overflow_(res, tow);
    return res;
}

HalfPrecision HalfPrecision::minus_equal_exp_(const HalfPrecision& bigger, const HalfPrecision& lower, int tow) {
    // needs no toward (no extra bits)
    uint32_t res_mant, shifted;
    int32_t res_exp{bigger.exponent_};
    res_mant = bigger.mantissa_ - lower.mantissa_;
    shifted = res_mant % (h_bin_pow(10) - 1);
    for (uint32_t i = 0; i <= 10; ++i) {
        if ((shifted >> (10 - i)) != 0) {
            res_exp -= i;
            shifted = (shifted << i);
            break;
        }
    }

    if (res_exp > 0) return HalfPrecision(shifted, res_exp, bigger.sign_);

    res_exp = bigger.exponent_;
    for (uint32_t i = 0; i <= 10; ++i) {
        if ((res_exp - i) != 0) {
            shifted = (shifted << i);
            break;
        }
    }
    res_exp = 0;
    return HalfPrecision(shifted, res_exp, bigger.sign_);
}

HalfPrecision HalfPrecision::minus_inequal_exp_(const HalfPrecision& bigger, const HalfPrecision& lower, int tow) {
    uint32_t res_mant, extra, shifted;
    int32_t res_exp{bigger.exponent_};

    if (lower.is_denormalised()) res_mant = ((bigger.mantissa_ + h_bin_pow(10)) << (bigger.exponent_ - lower.exponent_)) - lower.mantissa_;
    else res_mant = ((bigger.mantissa_ + h_bin_pow(10)) << (bigger.exponent_ - lower.exponent_)) - (h_bin_pow(10) + lower.mantissa_);
    
    shifted = res_mant >> (bigger.exponent_ - lower.exponent_);
    extra = res_mant & (h_bin_pow(bigger.exponent_ - lower.exponent_) - 1);
    uint32_t half = h_bin_pow(bigger.exponent_ - lower.exponent_ - 1);

    HalfPrecision res = round_(res_exp, shifted & (h_bin_pow(10) - 1), half, extra, bigger.sign_, tow);

    if (res.exponent_ > 0) return res;

    res_exp = bigger.exponent_;
    res_mant = shifted & (h_bin_pow(10) - 1);
    for (uint32_t i = 0; i <= 10; ++i) {
        if (res_exp - i != 0) {
            res_mant = (res_mant << i);
            break;
        }
    }

    res.mantissa_ = res_mant;
    res.exponent_ = 0;
    return res;
}

HalfPrecision HalfPrecision::round_(int32_t exp, uint32_t mant, uint32_t half, uint32_t extra, uint16_t sign, int tow) {
    switch (tow) {
        case 0: {
            break;
        }
        case 1: {
            if (half < extra) {
                if (mant < h_bin_pow(10) - 1) mant++;
                else {
                    mant = 0;
                    exp++;
                }
            }
            else if (half > extra) {}
            else if (mant % 2 == 0) {}
            else if (mant < h_bin_pow(10) - 1) mant++;
            else {
                mant = 0;
                exp++;
            }
            break;
        }
        case 2: {
            if (sign == 0 && extra != 0) {
                if (mant < h_bin_pow(10) - 1) mant++;
                else {
                    mant = 0;
                    exp++;
                }
            }
            break;
        }
        case 3: {
            if (sign == 1 && extra != 0) {
                if (mant < h_bin_pow(10) - 1) mant++;
                else {
                    mant = 0;
                    exp++;
                }
            }
            break;
        }
        default:
        throw std::runtime_error("Wrong toward case");
    }
    HalfPrecision res = HalfPrecision(mant, exp, sign);
    if (exp < 31) return res;
    return overflow_(res, tow);
}

int32_t HalfPrecision::normalise_(uint32_t& number, int bit_count) {
    int32_t i = 0;
    for (i = 0; i <= bit_count; ++i) {
        if (number >> (bit_count - i)) {
            number = (number << i);
            return i;
        }
    }
    return -1;
}
