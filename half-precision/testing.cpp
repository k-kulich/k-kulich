#include "single-precision.h"
#include "half-precision.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>

std::vector<std::string> split(const std::string& data, const std::string& sep) {
    std::vector<std::string> result;
    size_t sep_len = sep.size();
    std::string cur{""};
    for (size_t i = 0; i + sep_len < data.size(); ++i) {
        if (data.substr(i, sep_len) == sep) {
            result.push_back(cur);
            cur.clear();
            i += (sep_len - 1);
        } else {
            cur.push_back(data[i]);
        }
    }
    if (!cur.empty()) result.push_back(cur);
    return result;
}

void test_pm() {
    std::fstream pm{"TestsFloat+-.txt", pm.in};
    std::fstream log_pm{"ResPM.txt", log_pm.out};
    std::string l, tow, num1, num2, oper, ans;
    size_t passed{0}, total{0};
    while (pm >> l) {
        if (l == "f") {
            pm >> tow >> num1 >> oper >> num2 >> ans;
            SinglePrecision n1{num1};
            SinglePrecision n2{num2};
            SinglePrecision res = n1.operation(n2, oper[0], tow[0] - '0');
            if (res.to_string(tow[0]) == ans) passed++;
            else {
                log_pm << "\n--FAILED test " << total << " with answer " << res.to_string(tow[0]) << "\n | correct: " << ans << "\n";
                log_pm << " | operation: " << n1.get_num() << " " << oper << " " << n2.get_num() << " = " << res.get_num() << "\n";
            }

        } 
        ++total;
    }
    log_pm << "\n=== PASSED: " << passed << "/" << total << " (" << passed * 100 / total << "." << (passed * 100 % total) * 10 / total << "%)\n";
    std::cout << "Plus / minus results:";
    std::cout << "\n=== PASSED: " << passed << "/" << total << " (" << passed * 100 / total << "." << (passed * 100 % total) * 10 / total << "%)\n";
    pm.close();
    log_pm.close();
}

void test_md() {
    std::fstream pm{"TEST_FLOAT_div_mult.txt", pm.in};
    std::fstream log_pm{"ResMD.txt", log_pm.out};
    std::string l, tow, num1, num2, oper, ans;
    size_t passed{0}, total{0};
    while (pm >> l) {
        if (l == "f") {
            pm >> tow >> num1 >> oper >> num2 >> ans;
            SinglePrecision n1{num1};
            SinglePrecision n2{num2};
            SinglePrecision res = n1.operation(n2, oper[0] == 'M' ? '*' : '/', tow[0] - '0');
            if (res.to_string(tow[0]) == ans) passed++;
            else {
                log_pm << "\n--FAILED test " << total << " with answer " << res.to_string(tow[0]) << "\n | correct: " << ans << "\n";
                log_pm << " | operation: " << n1.get_num() << " " << oper << " " << n2.get_num() << " = " << res.get_num() << "\n";
            }

        } 
        ++total;
    }
    log_pm << "\n=== PASSED: " << passed << "/" << total << " (" << passed * 100 / total << "." << (passed * 100 % total) * 10 / total << "%)\n";
    std::cout << "Multiply / division results:";
    std::cout << "\n=== PASSED: " << passed << "/" << total << " (" << passed * 100 / total << "." << (passed * 100 % total) * 10 / total << "%)\n";
    pm.close();
    log_pm.close();
}

int main(int argc, char** argv) {
    test_pm();
    test_md();
    return 0;
    if (argc < 4) {
        std::cout << "Not enough arguments\t" << argc << "\n";
        return 0;
    }
    // while (*argv) {
    //     std::cout << *argv << "\n";
    //     argv++;
    // } 
    char tp = *argv[1];
    int tow = *argv[2] - '0';
    if (tow < 0 || tow > 3) {
        std::cout << "Wrong toward\t" << tow << *argv[2] << "\n";
        return 0;
    }
    if (tp == 'h') {
        if (argc == 4) {
            HalfPrecision res = HalfPrecision(argv[argc - 1]);
            std::cout << res.to_string(tow) << " " << res.get_hex() << "\n";
        } else if (argc == 6) {
            HalfPrecision num1 = HalfPrecision(argv[argc - 2]);
            HalfPrecision num2 = HalfPrecision(argv[argc - 1]);
            HalfPrecision res = num1.operation(num2, *argv[3], tow);
            std::cout << res.to_string(tow) << " " << res.get_hex() << "\n";
        } else {
            HalfPrecision num1 = HalfPrecision(argv[argc - 3]);
            HalfPrecision num2 = HalfPrecision(argv[argc - 2]);
            HalfPrecision num3 = HalfPrecision(argv[argc - 1]);
            if (std::strcmp(argv[3], "mad")) {
                HalfPrecision res = num1.oper_mad_fma(num2, num3, tow, true);
                std::cout << res.to_string(tow) << " " << res.get_hex() << "\n";
            } else {
                HalfPrecision res = num1.oper_mad_fma(num2, num3, tow, false);
                std::cout << res.to_string(tow) << " " << res.get_hex() << "\n";
            }
        }
    } else {
        if (argc == 4) {
            SinglePrecision res = SinglePrecision(argv[argc - 1]);
            std::cout << res.to_string(tow) << " " << res.get_hex() << "\n";
        } else if (argc == 6) {
            SinglePrecision num1 = SinglePrecision(argv[argc - 2]);
            SinglePrecision num2 = SinglePrecision(argv[argc - 1]);
            SinglePrecision res = num1.operation(num2, *argv[3], tow);
            std::cout << res.to_string(tow) << " " << res.get_hex() << "\n";
        } else {
            SinglePrecision num1 = SinglePrecision(argv[argc - 3]);
            SinglePrecision num2 = SinglePrecision(argv[argc - 2]);
            SinglePrecision num3 = SinglePrecision(argv[argc - 1]);
            if (std::strcmp(argv[3], "mad")) {
                SinglePrecision res = num1.oper_mad_fma(num2, num3, tow, true);
                std::cout << res.to_string(tow) << " " << res.get_hex() << "\n";
            } else {
                SinglePrecision res = num1.oper_mad_fma(num2, num3, tow, false);
                std::cout << res.to_string(tow) << " " << res.get_hex() << "\n";
            }
        }
    }

    return 0;
}
