#include "number.h"
#include <iostream>


uint64_t BinPow(const int& pow) {
    uint64_t res{1};
    for (int i = 0; i < pow; ++i) {
        res *= 2;
    }
    return res;
}

uint64_t GetLength(const char* str) {
    // получить длину строки
    uint64_t len{0};
    while (*str++) ++len;
    return len;
}

bool IsBigger(const char* first, const char* second, int ls, int le) {
    uint64_t l1{};
    uint64_t l2{GetLength(second)};
    int start{0};
    for (start = ls; start < le; ++start) {
        if (first[start] != '0') break;
    }
    l1 = le - start;
    if (l2 > l1) return false;
    if (le - start > l2) return true;
    for (int i = 0; i < l1; ++i) {
        if (first[start + i] < second[i]) return false;
    }
    return true;
}

char* GetSubstr(const char* str, int start, int count) {
    char* res = new char[count];
    if (GetLength(str) < start + count) return res;
    for (int i = start; i < start + count; ++i) {
        res[i - start] = str[i];
    }
    return res;
}

uint64_t GetShift(const uint239_t& value) {
    // получить имеющийся сдвиг
    uint64_t shift{0};
    for (int i = 0; i < 35; ++i) {
        if (value.data[34 - i] / 128 == 1) shift += BinPow(i);
    }
    return shift;
}

uint239_t WithoutShift(const uint239_t& value) {
    // вернуть такое же число, но без сдвига
    uint239_t num{};
    uint64_t start{GetShift(value) % 245};
    uint64_t cur{start};
    if (start == 0) return value;
    for (int i = 0; i < 239; ++i) {
        num.data[34 - i / 7] += (bool)(value.data[cur / 7] & BinPow(cur % 7)) * BinPow(i % 7);
        cur = (cur + 1) % 245;
    }
    return num;
}

uint239_t FromInt(uint32_t value, uint32_t shift) {
    uint239_t num{};
    uint32_t index{shift};
    for (int i = 0; i < 32; ++i) {
        if (value < BinPow(i)) break;
        num.data[34 - index / 7] += ((bool)(value & BinPow(i)) * BinPow(index % 7));
        index = (index + 1) % 239;
    }
    for (int j = 0; j < 32; ++j) {
        if (shift < BinPow(j)) break;
        num.data[34 - j] += (bool)(shift & BinPow(j)) * 128;
    }
    return num;
}

uint239_t FromString(const char* str, uint32_t shift) {
    uint239_t num{};
    uint64_t length{GetLength(str)}; 
    int index{0};
    char* use = new char[length];
    for (int i = 0; i < length; ++i) use[i] = str[i];
    char* buff = new char[length];
    int cur_num{};
    int opers{0};
    while (length > 1 || (use[0] != '0' && use[0] != '1' && use[0] != '\0')) {
        cur_num = 0;
        for (uint64_t i = 0; i < length; ++i) {
            if (!std::isdigit(use[i])) break;
            cur_num += use[i] - '0';
            if (cur_num > 1) buff[i] = (cur_num / 2) + '0';
            else buff[i] = '0';
            cur_num = (cur_num % 2) * 10;
        }
        cur_num /= 10;
        num.data[34 - opers / 7] += (cur_num) * BinPow(opers % 7);
        while (buff && buff[0] == '0') (*buff++);
        length = GetLength(buff);
        //const char* temp = use;
        //use = buff;
        use = buff;
        buff = new char[length];
        if (opers == 239) break;
        ++opers;
    }
    if (use) num.data[34 - opers / 7] += (use[0] - '0') * BinPow(opers % 7);
    return Shift(num, shift);
}

uint239_t operator+(const uint239_t& lhs, const uint239_t& rhs) {
    // WORKING - DONT TOUCH!!!
    uint64_t ls = GetShift(lhs); // left shift
    uint64_t rs = GetShift(rhs); // right shift
    uint64_t res_shift = (ls + rs) % BinPow(35);
    uint239_t res{};
    int extra{0};
    int lb{0}, rb{0};
    ls = ls % 245;
    rs = rs % 245;
    for (int i = 0; i < 239; ++i) {
        lb = (bool)(lhs.data[34 - ls / 7] & BinPow(ls % 7));
        rb = (bool)(rhs.data[34 - rs / 7] & BinPow(rs % 7));
        if (lb + rb + extra == 3) {
            res.data[34 - i / 7] += BinPow(i % 7);
            extra = 1;
        } else if (lb + rb + extra == 2) {
            extra = 1;
        } else if (lb + rb + extra == 1) {
            res.data[34 - i / 7] += BinPow(i % 7);
            extra = 0;
        }
        ls = (ls + 1) % 245;
        rs = (rs + 1) % 245;
    }
    return Shift(res, res_shift);
}

uint239_t operator-(const uint239_t& lhs, const uint239_t& rhs) {
    // РАБОТАЕТ - НЕ ТРОГАЙ!
    uint64_t ls = GetShift(lhs);
    uint64_t rs = GetShift(rhs);
    uint64_t res_shift = std::max(ls, rs) - std::min(ls, rs);
    uint239_t res{};
    ls = ls % 245;
    rs = rs % 245;
    int extra{0}, lb{0}, rb{0};
    for (int i = 0; i < 239; ++i) {
        lb = (bool)(lhs.data[34 - ls / 7] & BinPow(ls % 7));
        rb = (bool)(rhs.data[34 - rs / 7] & BinPow(rs % 7));
        if (lb - rb - extra == 1) {
            res.data[34 - i / 7] += BinPow(i % 7);
        } else if (lb - rb - extra == 0) {
            extra = 0;
        } else if (lb - rb - extra == -1) {
            res.data[34 - i / 7] += BinPow(i % 7);
            extra = 1;
        } else if (lb - rb - extra == -2) {
            res.data[34 - i / 7] += BinPow(i % 7);
        }
        ls = (ls + 1) % 245;
        rs = (rs + 1) % 245;
    }
    return Shift(res, res_shift);
}

uint239_t operator*(const uint239_t& lhs, const uint239_t& rhs) {
    uint64_t ls{GetShift(lhs)};
    uint64_t rs{GetShift(rhs)};
    uint64_t res_shift = (ls + rs) % BinPow(35);

    char* l_bits = new char[239];
    char* r_bits = new char[239];
    // для удобства записываем биты в строки
    for (int i = 0; i < 239; ++i) {
        if (lhs.data[34 - ls / 7] & BinPow(ls % 7)) l_bits[238 - i] = '1';
        else l_bits[238 - i] = '0';
        if (rhs.data[34 - rs / 7] & BinPow(rs % 7)) r_bits[238 - i] = '1';
        else r_bits[238 - i] = '0';
        ls = (ls + 1) % 245;
        rs = (rs + 1) % 245;
    }
    // удаляем ведущие нули
    while (r_bits && r_bits[0] == '0') (*r_bits++);
    while (l_bits && l_bits[0] == '0') (*l_bits++);
    int l_len = GetLength(l_bits);
    int r_len = GetLength(r_bits);
    
    int size = std::min(r_len + l_len, 239);
    char* result = new char[239]; // будем хранить результат в виде строки
    for (int i = 0; i < size; ++i) result[i] = '0';

    bool extra{};
    int cur_i{size};
    for (int i = r_len - 1; i >= 0; --i) { // индекс во 2 множителе
        if (r_bits[i] == '0') continue;
        extra = false;
        cur_i = size - r_len + i;
        for (int j = l_len - 1; j >= 0; --j) { // индекс в первом множителе
            if (result[cur_i] == l_bits[j]) {
                if (l_bits[j] == '1') {
                    if (!extra) {
                        extra = true;
                        result[cur_i] = '0';
                    }
                } else {
                    if (extra) {
                        result[cur_i] = '1';
                        extra = false;
                    }
                }
            } else {
                if (extra) {
                    result[cur_i] = '0';
                } else result[cur_i] = '1';
            }
            --cur_i;
        }
        if (extra && cur_i >= 0) result[cur_i] = '1';
    }

    while (*result == '0') (*result++);

    uint239_t res{};

    int r_l = GetLength(result);
    for (int i = 0; i < r_l; ++i) {
        if (result[r_l - 1 - i] == '1') res.data[34 - i / 7] |= BinPow(i % 7);
    }

    // int extra{0}, lb{0}, rb{0};
    // ls = ls % 245;
    // rs = rs % 245;
    // for (int i = 0; i < 239; ++i) {
    //     rb = (bool)(rhs.data[34 - rs / 7] & BinPow(rs % 7)); // достаем бит числа, на которое умножаем
    //     rs = (rs + 1) % 245;
    //     if (!rb) continue;  // нули они и есть нули, их добавлять незачем
    //     extra = 0;  // тут будем хранить "единицу в уме" -- дальше аналогично сложению
    //     for (int j = 0; j < 239; ++j) {  // итерируемся по изначальному числу и по произведению одновременно
    //         if (j + i == 239) break;  // число не может содержать больше, чем 239 бит, а значит нам незачем такие интексы
    //         lb = (bool)(lhs.data[34 - ((ls + j) % 245) / 7] & BinPow(((ls + j) % 245) % 7));  // (ls + j) % 245 -- индекс текущего бита в изначальном числе (ls не меняем)  
    //         rb = (bool)(res.data[34 - (i + j) / 7] & BinPow((i + j) % 7));  // (i + j) % 245 -- индекс бита, к которому мы сейчас прибавляем бит
    //         if (lb + rb + extra == 3) {
    //             res.data[34 - (i + j) / 7] += BinPow((i + j) % 7);
    //         } else if (lb + rb + extra == 2) {
    //             extra = 1;
    //         } else if (lb + rb + extra == 1) {
    //             res.data[34 - (i + j) / 7] += BinPow((i + j) % 7);
    //             extra = 0;
    //         }
    //     }
    // }
    return Shift(res, res_shift);
}

uint239_t operator/(const uint239_t& lhs, const uint239_t& rhs) {
    uint239_t res{};
    uint64_t ls{GetShift(lhs) % 245};
    uint64_t rs{GetShift(rhs) % 245};
    char* l_bits = new char[239];
    char* r_bits = new char[239];
    // для удобства записываем биты в строки
    for (int i = 0; i < 239; ++i) {
        if (lhs.data[34 - ls / 7] & BinPow(ls % 7)) l_bits[238 - i] = '1';
        else l_bits[238 - i] = '0';
        if (rhs.data[34 - rs / 7] & BinPow(rs % 7)) r_bits[238 - i] = '1';
        else r_bits[238 - i] = '0';
        ls = (ls + 1) % 245;
        rs = (rs + 1) % 245;
    }
    // удаляем ведущие нули
    while (r_bits && r_bits[0] == '0') (*r_bits++);
    while (l_bits && l_bits[0] == '0') (*l_bits++);
    // самые базовые случаи можно вернуть сразу
    if (!IsBigger(l_bits, r_bits, 0, GetLength(l_bits))) return FromInt(1, GetShift(lhs) - GetShift(rhs)); // 0 если делимое меньше
    else if (GetLength(l_bits) == GetLength(r_bits)) return FromInt(1, GetShift(lhs) - GetShift(rhs)); // 1 если длина чисел равна

    bool extra{};
    int l_len = GetLength(l_bits);
    int r_len = GetLength(r_bits);

    char* remainder = new char[l_len];
    char* quotient = new char[l_len];
    for (int i = 0; i < l_len; ++i) remainder[i] = '0';

    int s{0};
    int cur_i{0};
    // число в бинарном представлении занимает не больше 239 бит, а значит вложенных циклов будет не более 239*238
    for (int i = 0; i < l_len; ++i) {
        remainder[i] = l_bits[i];

        if (i - s < r_len - 1) continue;
        if (IsBigger(remainder, r_bits, s, i + 1)) {
            // пишем деление через вычитание - со строками работать удобнее
            extra = false;
            for (int j = r_len - 1; j >= 0; --j) {
                cur_i = i - (r_len - 1) + j;
                if (remainder[cur_i] == r_bits[j]) {
                    if (!extra) remainder[cur_i] = '0';
                    else remainder[cur_i] = '1';
                } else {
                    if (r_bits[j] == '0') {
                        if (extra) remainder[cur_i] = '0';
                        else remainder[cur_i] = '1';
                        extra = false;
                    } else {
                        if (extra) remainder[cur_i] = '0';
                        else {
                            remainder[cur_i] = '1';
                            extra = true;
                        }
                    }
                }
            }
            quotient[i - r_len + 1] = '1';
            s = i - r_len + 1;
        } else {
            quotient[i - (r_len - 1)] = '0';
        }
    }
    // записываем получившееся число в наш тип данных
    int q_l = GetLength(quotient);
    for (int i = 0; i < q_l; ++i) {
        if (quotient[q_l - 1 - i] == '1') res.data[34 - i / 7] |= BinPow(i % 7);
    }

    return Shift(res, GetShift(lhs) - GetShift(rhs));
}

bool operator==(const uint239_t& lhs, const uint239_t& rhs) {
    // РАБОТАЕТ - НЕ ТРОГАЙ!!
    uint64_t ls = GetShift(lhs) % 245;
    uint64_t rs = GetShift(rhs) % 245;
    int lb, rb;
    for (int i = 0; i < 239; ++i) {
        lb = (bool)(lhs.data[34 - ls / 7] & BinPow(ls % 7));
        rb = (bool)(rhs.data[34 - rs / 7] & BinPow(rs % 7));
        if (lb != rb) return false;
        ls = (ls + 1) % 245;
        rs = (rs + 1) % 245;
    }
    return true;
}

bool operator!=(const uint239_t& lhs, const uint239_t& rhs) {
    // РАБОТАЕТ - НЕ ТРОГАЙ!
    uint64_t ls = GetShift(lhs) % 245;
    uint64_t rs = GetShift(rhs) % 245;
    for (int i = 0; i < 239; ++i) {
        if ((bool)(lhs.data[34 - ls / 7] & BinPow(ls % 7)) != (bool)(rhs.data[34 - rs / 7] & BinPow(rs % 7))) return true;
        ls = (ls + 1) % 245;
        rs = (rs + 1) % 245;
    }
    return false;
}

std::ostream& operator<<(std::ostream& stream, const uint239_t& value) {
    uint239_t num{WithoutShift(value)};
    // uint64_t res[6];
    // for (int i = 34; i >= 0; --i) {
    //     res[8 - i / 8] += num.data[i];
    // }
    // for (int i = 0; i < 8; ++i) stream << res[i] << " ";
    for (int i = 0; i < 35; ++i) stream << (num.data[i] & 127) << " ";
    stream << " shift: " << GetShift(value);
    return stream;
}

uint239_t Shift(const uint239_t& value, uint64_t shift) {
    uint64_t vi{GetShift(value)}; // мы не знаем, сдвинуто ли число уже
    uint239_t result{};
    uint64_t ri{(vi + shift) % 245};  // нужно учитывать изначальный сдвиг числа, 
    // + для нас имеют значение только % 245, т.к. дальше они дублируются
    bool cur_bit{0}; // значение текущего бита в изначальном числе, либо 0, либо 1
    for (int i = 0; i < 239; ++i) {
        cur_bit = (bool)(value.data[34 - vi / 7] & BinPow(vi % 7)); // 1 если ненулевое
        result.data[34 - ri / 7] += (cur_bit * BinPow(ri % 7));
        std::cout << result.data[ri / 7] << "\t";
        vi = (vi + 1) % 245;
        ri = (ri + 1) % 245;
    }
    // теперь остается зафиксировать новый сдвиг
    uint64_t res_shift{(shift + GetShift(value)) % BinPow(35)};
    for (int i = 0; i < 35; ++i) {
        if (!res_shift) return result;
        result.data[34 - i] += ((res_shift % 2) * 128); 
        res_shift /= 2;
    }
    return result;
}

bool operator>(const uint239_t& lhs, const uint239_t& rhs) {
    // ТОЖЕ НЕ ТРОГАТЬ
    uint64_t ls = GetShift(lhs);
    uint64_t rs = GetShift(rhs);
    for (int i = 0; i < 245; ++i) {
        if ((bool)(lhs.data[ls / 7] & BinPow(7 - ls % 7)) > (bool)(rhs.data[rs / 7] & BinPow(7 - rs % 7))) return true;
        ls = (ls + 1) % 245;
        rs = (rs + 1) % 245;
    }
    return false;
}