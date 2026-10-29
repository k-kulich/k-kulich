#include "single-precision.h"

struct SingleHelpData {   
    SinglePrecision plus_zero_{"0x00000000", "0x0.000000p+0"};
    SinglePrecision nan_{"0xFF100000", "nan"};
    SinglePrecision minus_zero_{"0x10000000", "-0x0.000000p+0"};
    SinglePrecision plus_infty_{"0x7F800000", "inf"};
    SinglePrecision minus_infty_{"0xFF800000", "-inf"};
    SinglePrecision plus_overflow_{"0x7F7FFFFE", "0x1.fffffep+127"};
    SinglePrecision minus_overflow_{"0xFF7FFFFE", "-0x1.fffffep+127"};
};

static SingleHelpData s_help{};

uint64_t s_bin_pow(int pow) {
    uint64_t res{1};
    return (res << pow);
}

SinglePrecision::SinglePrecision(uint32_t data) {
    bits_ = data;
}

SinglePrecision::SinglePrecision(uint32_t mantissa, uint32_t exponent, uint32_t sign) {
    mantissa_ = mantissa;
    exponent_ = exponent;
    sign_ = sign;
}

SinglePrecision::SinglePrecision(const std::string& hex, std::string view) {
    bits_ = std::stoul(hex.substr(2), nullptr, 16);
    view_ = view;
}

SinglePrecision::SinglePrecision(const char* hex, std::string view) {
    std::string str{hex};
    bits_ = std::stoul(str.substr(2), nullptr, 16);
    view_ = view;
}

SinglePrecision& SinglePrecision::operator=(const SinglePrecision& other) {
    bits_ = other.bits_;
    return *this;
}

bool SinglePrecision::is_zero() const {
    return (exponent_ == 0) && (mantissa_ == 0);
}

bool SinglePrecision::is_infty() const {
    return (exponent_ == 255) && (mantissa_ == 0);
}

bool SinglePrecision::is_denormalised() const {
    return (exponent_ == 0) && (mantissa_ != 0);
}

bool SinglePrecision::is_nan() const {
    return (exponent_ == 255) && (mantissa_ != 0);
}

SinglePrecision SinglePrecision::sum(const SinglePrecision& other, int tow) {
    // some basic cases
    if (is_nan() || other.is_nan()) return is_nan() ? *this : other;
    if (sign_ != other.sign_ && exponent_ == other.exponent_ && mantissa_ == other.mantissa_) return s_help.plus_zero_;
    if (is_infty() && other.is_infty() && (sign_ != other.sign_)) return s_help.nan_;
    if (is_infty()) return sign_ == 0 ? s_help.plus_infty_ : s_help.minus_infty_;
    if (other.is_infty()) return other.sign_ == 0 ? s_help.plus_infty_ : s_help.minus_infty_;
    if (is_zero() && other.is_zero()) {
        if (tow == 3 || (sign_ == other.sign_ && sign_ == 1)) return s_help.minus_zero_;
        return s_help.plus_zero_;
    }
    if (is_zero()) return other;
    if (other.is_zero()) return *this;

    SinglePrecision bigger{is_bigger_(other) ? bits_ : other.bits_};
    SinglePrecision lower{is_bigger_(other) ? other.bits_ : bits_};

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

SinglePrecision SinglePrecision::minus(const SinglePrecision& other, int tow) {
    SinglePrecision reversed = other;
    reversed.sign_ = 1 - other.sign_;
    return sum(reversed, tow);
}

SinglePrecision SinglePrecision::multiply(const SinglePrecision& other, int tow, bool needs_round) {
    // some basic cases
    if (is_nan() || other.is_nan()) return is_nan() ? *this : other;
    if (is_infty() && other.is_zero()) return s_help.nan_;
    if (is_zero() && other.is_infty()) return s_help.nan_;
    if (is_zero() && other.is_zero()) {
        if (sign_ == other.sign_) return s_help.plus_zero_;
        return s_help.minus_zero_;
    }
    if (is_infty() && other.is_infty()) {
        if (sign_ == other.sign_) return s_help.plus_infty_;
        return s_help.minus_infty_;
    }

    uint32_t sign = (sign_ != other.sign_);
    uint64_t lmod = is_denormalised() ? mantissa_ : s_bin_pow(23) + mantissa_;
    uint64_t rmod = other.is_denormalised() ? other.mantissa_ : s_bin_pow(23) + other.mantissa_;
    uint64_t res_mant = lmod * rmod;

    if (is_denormalised() || other.is_denormalised()) return multiply_denorm_(other, res_mant, sign, tow, needs_round);

    int64_t res_exp{0};
    if ((res_mant >> 47) == 0) res_exp = exponent_ + other.exponent_ - 127;
    else res_exp = exponent_ + other.exponent_ - 126;

    if (res_exp >= 255) {
        SinglePrecision res = SinglePrecision(res_mant, res_exp, sign);
        return overflow_(res, tow, needs_round);
    }

    uint64_t half, half_for_ne, shifted;
    if ((res_mant >> 47 == 0)) {
        half = res_mant & (s_bin_pow(23) - 1);
        shifted = (res_mant >> 23) & (s_bin_pow(23) - 1);
        half_for_ne = s_bin_pow(22);
    } else {
        half = res_mant % s_bin_pow(24);
        shifted = (res_mant >> 24) & (s_bin_pow(23) - 1);
        half_for_ne = s_bin_pow(23);
    }

    if (res_exp < 0) return multiply_denorm_(other, res_mant, sign, tow, needs_round);

    if (needs_round) return round_(res_exp, shifted, half_for_ne, half, sign, tow);
    return SinglePrecision(shifted, res_exp, sign);
}

SinglePrecision SinglePrecision::divide(const SinglePrecision& other, int tow) {
    if (is_nan() || other.is_nan()) return is_nan() ? *this : other;
    if (is_zero() && other.is_zero()) return s_help.nan_;
    if (is_infty() && other.is_infty()) return s_help.nan_;
    if (other.is_zero()) {
        if (sign_ == other.sign_) return s_help.plus_infty_;
        return s_help.minus_infty_;
    }
    if (is_zero() || other.is_infty()) {
        if (sign_ == other.sign_) return s_help.plus_zero_;
        return s_help.minus_zero_;
    }

    uint64_t lmod = is_denormalised() ? mantissa_ : mantissa_ + s_bin_pow(23);
    uint64_t rmod = other.is_denormalised() ? mantissa_ : mantissa_ + s_bin_pow(23);
    uint64_t res_mod = (lmod << 25) / rmod;
    int64_t res_exp = exponent_ - other.exponent_;
    if (res_mod < s_bin_pow(25)) {
        int i{0};
        while (res_mod < s_bin_pow(25)) {
            ++i;
            res_mod = (lmod << (25 + i)) / rmod;
        }
        res_exp -= i;
    } else if (res_mod >= s_bin_pow(26)) {
        int i{0};
        while (res_mod >= s_bin_pow(26)) {
            ++i;
            res_mod = (lmod << (25 - i)) / rmod;
        }
        res_exp += i;
    }
    res_mod = res_mod & (s_bin_pow(23) - 1);
    uint32_t sign = (sign_ != other.sign_);
    uint64_t half = res_mod % rmod;

    switch (tow) {
        case 0: {
            return SinglePrecision(res_mod, res_exp, sign);
        }
        case 1: {
            if (half * 2 < rmod) return SinglePrecision(res_mod, res_exp, sign);
            if (half * 2 > rmod) return SinglePrecision(res_mod + 1, res_exp, sign);
            if (half % 2 == 0) return SinglePrecision(res_mod, res_exp, sign);
            return SinglePrecision(res_mod + 1, res_exp, sign);
        }
        case 2: {
            if (half == 0) return SinglePrecision(res_mod, res_exp, sign);
            if (sign_ == 0) return SinglePrecision(res_mod + 1, res_exp, sign);
            return SinglePrecision(res_mod - 1, res_exp, sign);
        }
        case 3: {
            if (half == 0) return SinglePrecision(res_mod, res_exp, sign);
            if (sign_ == 1) return SinglePrecision(res_mod + 1, res_exp, sign);
            return SinglePrecision(res_mod - 1, res_exp, sign);
        }
        default: throw std::runtime_error("Wrong toward case");
    }
}

SinglePrecision SinglePrecision::fma(const SinglePrecision& second, const SinglePrecision& third, int tow) {
    SinglePrecision temp = multiply(second, tow, false);
    return temp.sum(third, tow);
}

SinglePrecision SinglePrecision::operation(const SinglePrecision& other, char oper, int tow_mode) {
    if (oper == '+') return sum(other, tow_mode);
    if (oper == '-') return minus(other, tow_mode);
    if (oper == '/') return divide(other, tow_mode);
    if (oper == '*') return multiply(other, tow_mode);
    return *this;
}

SinglePrecision SinglePrecision::oper_mad_fma(const SinglePrecision& second, const SinglePrecision& third, int tow_mode, bool mad) {
    if (mad) {
        SinglePrecision temp = multiply(second, tow_mode);
        return temp.sum(third, tow_mode);
    }
    return fma(second, third, tow_mode);
}

std::string SinglePrecision::to_string(int tow_mode) {
    if (view_ != "") return view_;
    if (is_zero()) {
        if (sign_ == 0) return "0x0.000000p+0";
        return "-0x0.000000p+0";
    }
    if (is_infty()) {
        if (sign_ == 0) return "inf";
        return "-inf";
    }
    if (is_nan()) return "nan";

    std::string res;
    int64_t res_exp = exponent_;
    if (sign_ == 1) res.push_back('-');
    res.append("0x1.");
    if (is_denormalised()) {
        std::stringstream str;
        int i;
        res_exp = -126;
        for (i = 1; i <= 23; ++i) {
            if (mantissa_ / s_bin_pow(23 - i) != 0) {
                str << std::hex << std::setw(6) << std::setfill('0') << ((mantissa_ * s_bin_pow(i)) % s_bin_pow(23)) * 2;
                res_exp -= i;
                break;
            }
        }
        res.append(str.str());
    } else {
        std::stringstream str;
        str << std::hex << std::setw(6) << std::setfill('0') << mantissa_ * 2;
        res.append(str.str());
        res_exp -= 127;
    }
    res.push_back('p');
    if (res_exp >= 0) res.push_back('+');
    res.append(std::to_string(res_exp));

    return res;
}

std::string SinglePrecision::get_hex() {
    std::stringstream str;
    str << std::hex << std::setw(8) << std::setfill('0') << bits_;
    std::string temp = str.str();
    std::string res{};
    for (size_t i = 0; i < temp.size(); ++i) {
        if (temp[i] >= 'a' && temp[i] <= 'f') res.push_back(char('A' + temp[i] - 'a'));
        else res.push_back(temp[i]);
    }
    return "0x" + res;
}

float SinglePrecision::get_num() {
    return num_;
}

bool SinglePrecision::is_bigger_(const SinglePrecision& other) {
    return (exponent_ > other.exponent_ || (exponent_ == other.exponent_ && mantissa_ > other.mantissa_));
}

SinglePrecision SinglePrecision::multiply_denorm_(const SinglePrecision& other, uint64_t res_mant, uint32_t sign, int tow, bool needs_round) {
    int64_t res_exp{0};
    uint64_t extra, half, shifted;
    res_exp += is_denormalised() ? -126 : (exponent_ - 127);
    res_exp += other.is_denormalised() ? -126 : (other.exponent_ - 127);

    if ((res_mant >> 47) == 0) {
        if (res_exp <= -126) {
            shifted = (res_mant >> (23 - (res_exp + 126))) & (s_bin_pow(23) - 1);
            extra = res_mant & (s_bin_pow(23 - (res_exp + 126)) - 1);
            half = s_bin_pow(22 - (res_exp + 126));
            res_exp = 0;
        } else {
            res_exp -= normalise_(res_mant, 46);
            shifted = (res_mant >> 23) & (s_bin_pow(23) - 1);
            extra = res_mant & (s_bin_pow(23) - 1);
            half = s_bin_pow(22);
            res_exp += 127;
        }
    } else {
        res_exp++;
        if (res_exp <= -126) {
            shifted = (res_mant >> (24 - (res_exp + 126))) & (s_bin_pow(23) - 1);
            extra = res_mant & (s_bin_pow(24 - (res_exp + 126)) - 1);
            half = s_bin_pow(23 - (res_exp + 126));
            res_exp = 0;
        } else {
            shifted = (res_mant >> 24) & (s_bin_pow(23) - 1);
            extra = res_mant & (s_bin_pow(24) - 1);
            half = s_bin_pow(23);
            res_exp += 127;
        }
    }
    if (needs_round) return round_(res_exp, shifted, half, extra, sign, tow);
    return SinglePrecision(shifted, res_exp, sign);
}

SinglePrecision SinglePrecision::overflow_(SinglePrecision& other, int tow, bool needs_round) {
    if (!needs_round) return other;

    switch (tow) {
    case 0: {
        if (other.sign_ == 0) return s_help.plus_overflow_;

        return s_help.minus_overflow_;
    }
    case 1: {
        if (other.sign_ == 0) return s_help.plus_infty_;
        return s_help.minus_infty_;
    } 
    case 2: {
        if (other.sign_ == 0) return s_help.plus_infty_;
        return s_help.minus_overflow_;
    } 
    case 3: {
        if (other.sign_ == 1) return s_help.minus_infty_;
        return s_help.plus_overflow_;
    } 
    default: throw std::runtime_error("Wrong toward case");
    } 
}

SinglePrecision SinglePrecision::plus_equal_exp_(const SinglePrecision& bigger, const SinglePrecision& lower, int tow) {
    uint64_t res_mant, extra, shifted;
    uint64_t res_exp{0};
    if (!bigger.is_denormalised() && !lower.is_denormalised()) res_mant = 2 * s_bin_pow(23) + bigger.mantissa_ + lower.mantissa_;
    else if (!bigger.is_denormalised() || !lower.is_denormalised()) res_mant = s_bin_pow(23) + bigger.mantissa_ + lower.mantissa_;
    else res_mant = bigger.mantissa_ + lower.mantissa_;
    
    if ((res_mant >> 24) == 0) {
        shifted = (res_mant) & (s_bin_pow(23) - 1);
        extra = 0;
    } else {
        shifted = (res_mant >> 1) & (s_bin_pow(23) - 1);
        extra = res_mant & 1;
        res_exp++;
    }
    res_exp += bigger.exponent_;

    SinglePrecision res = round_(res_exp, shifted, 0, extra, bigger.sign_, tow);

    if (res.exponent_ < 255) return res;

    return overflow_(res, tow);
}

SinglePrecision SinglePrecision::plus_inequal_exp_(const SinglePrecision& bigger, const SinglePrecision& lower, int tow) {
    uint64_t res_mant, half, shifted;
    int64_t res_exp{bigger.exponent_};
    if (lower.is_denormalised()) {
        shifted = lower.mantissa_ >> (bigger.exponent_ - lower.exponent_);
        res_mant = shifted + s_bin_pow(23) + bigger.mantissa_;
        half = (lower.mantissa_) & (s_bin_pow(bigger.exponent_ - lower.exponent_) - 1);
    } else {
        shifted = (lower.mantissa_ + s_bin_pow(23)) >> s_bin_pow(bigger.exponent_ - lower.exponent_);
        res_mant = shifted + s_bin_pow(23) + bigger.mantissa_;
        half = (lower.mantissa_ + s_bin_pow(23)) & (s_bin_pow(bigger.exponent_ - lower.exponent_) - 1);
    }

    if ((res_mant >> 24) == 0) {
        SinglePrecision res = round_(res_exp, res_mant & (s_bin_pow(23) - 1), 
                                    s_bin_pow(bigger.exponent_ - lower.exponent_ - 1), half, bigger.sign_, tow);
        return res;
    }

    res_exp++;
    if (res_exp >= 255) {
        SinglePrecision res = SinglePrecision(res_mant, res_exp, bigger.sign_);
        return overflow_(res, tow);
    }

    res_mant = (res_mant >> 1) & (s_bin_pow(23) - 1);
    SinglePrecision res = round_(res_exp, res_mant, s_bin_pow(bigger.exponent_ - lower.exponent_ - 1), half, bigger.sign_, tow);

    if (res.exponent_ >= 255) return overflow_(res, tow);
    return res;
}

SinglePrecision SinglePrecision::minus_equal_exp_(const SinglePrecision& bigger, const SinglePrecision& lower, int tow) {
    // needs no toward (no extra bits)
    uint64_t res_mant, shifted;
    int64_t res_exp{bigger.exponent_};
    res_mant = bigger.mantissa_ - lower.mantissa_;
    shifted = res_mant % (s_bin_pow(23) - 1);
    for (uint64_t i = 0; i <= 23; ++i) {
        if ((shifted >> (23 - i)) != 0) {
            res_exp -= i;
            shifted = (shifted << i);
            break;
        }
    }

    if (res_exp > 0) return SinglePrecision(shifted, res_exp, bigger.sign_);

    res_exp = bigger.exponent_;
    for (uint64_t i = 0; i <= 23; ++i) {
        if ((res_exp - i) != 0) {
            shifted = (shifted << i);
            break;
        }
    }
    res_exp = 0;
    return SinglePrecision(shifted, res_exp, bigger.sign_);
}

SinglePrecision SinglePrecision::minus_inequal_exp_(const SinglePrecision& bigger, const SinglePrecision& lower, int tow) {
    uint64_t res_mant, extra, shifted;
    int64_t res_exp{bigger.exponent_};

    if (lower.is_denormalised()) res_mant = ((bigger.mantissa_ + s_bin_pow(23)) << (bigger.exponent_ - lower.exponent_)) - lower.mantissa_;
    else res_mant = ((bigger.mantissa_ + s_bin_pow(23)) << (bigger.exponent_ - lower.exponent_)) - (s_bin_pow(23) + lower.mantissa_);
    
    shifted = res_mant >> (bigger.exponent_ - lower.exponent_);
    extra = res_mant & (s_bin_pow(bigger.exponent_ - lower.exponent_) - 1);
    uint64_t half = s_bin_pow(bigger.exponent_ - lower.exponent_ - 1);

    SinglePrecision res = round_(res_exp, shifted & (s_bin_pow(23) - 1), half, extra, bigger.sign_, tow);

    if (res.exponent_ > 0) return res;

    res_exp = bigger.exponent_;
    res_mant = shifted & (s_bin_pow(23) - 1);
    for (uint64_t i = 0; i <= 23; ++i) {
        if (res_exp - i != 0) {
            res_mant = (res_mant << i);
            break;
        }
    }

    res.mantissa_ = res_mant;
    res.exponent_ = 0;
    return res;
}

SinglePrecision SinglePrecision::round_(int64_t exp, uint64_t mant, uint64_t half, uint64_t extra, uint32_t sign, int tow) {
    switch (tow) {
        case 0: {
            break;
        }
        case 1: {
            if (half < extra) {
                if (mant < s_bin_pow(23) - 1) mant++;
                else {
                    mant = 0;
                    exp++;
                }
            }
            else if (half > extra) {}
            else if (mant % 2 == 0) {}
            else if (mant < s_bin_pow(23) - 1) mant++;
            else {
                mant = 0;
                exp++;
            }
            break;
        }
        case 2: {
            if (sign == 0 && extra != 0) {
                if (mant < s_bin_pow(23) - 1) mant++;
                else {
                    mant = 0;
                    exp++;
                }
            }
            break;
        }
        case 3: {
            if (sign == 1 && extra != 0) {
                if (mant < s_bin_pow(23) - 1) mant++;
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
    SinglePrecision res = SinglePrecision(mant, exp, sign);
    if (exp < 255) return res;
    return overflow_(res, tow);
}

int64_t SinglePrecision::normalise_(uint64_t& number, int bit_count) {
    int64_t i = 0;
    for (i = 0; i <= bit_count; ++i) {
        if (number >> (bit_count - i)) {
            number = (number << i);
            return i;
        }
    }
    return -1;
}
