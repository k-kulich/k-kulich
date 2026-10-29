#include "single-precision.h"
#include "half-precision.h"
#include <iostream>
#include <cstring>

int main(int argc, char** argv) {
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
