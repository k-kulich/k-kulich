#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "ComArg.h"

namespace ArgumentParser {

class ArgParser {
    private:
        std::string name_;
        std::string small_description_;
        std::vector<ComArg> params_;
        ComArg zero_arg;

        ComArg GetParam_(const std::string& name);
        ComArg GetPositional_(const std::string& arg_type);
    public:
    // Your Implementation here!
    ArgParser(const std::string& name);

    bool Parse(const std::vector<std::string>& splitted_string);

    bool Parse(int argc, char** argv);

    ComArg AddStringArgument(const std::string& name1, const std::string& name2 = "", const std::string& description = "");

    ComArg AddStringArgument(const char& name1, const std::string& name2 = "", const std::string& description = "");

    ComArg AddIntArgument(const std::string& name1, const std::string& name2 = "", const std::string& description = "");

    ComArg AddIntArgument(const char& name1, const std::string& name2 = "", const std::string& description = "");

    ComArg AddFlag(const std::string& name1, const std::string& name2 = "", const std::string& description = "");

    ComArg AddFlag(const char& name1, const std::string& name2 = "", const std::string& description = "");

    ComArg AddHelp(const std::string& name1, const std::string& name2 = "", const std::string& description = "");

    ComArg AddHelp(const char& name1, const std::string& name2 = "", const std::string& description = "");

    // void StoreValue(T& value);

    std::string GetStringValue(const std::string& name, size_t index = -1);

    // void MultiValue(size_t min_count = 0);

    // void StoreValues(std::vector<int>& array);

    int GetIntValue(const std::string& name, size_t index = -1);

    bool GetFlag(const std::string& name);

    // void Default(T& value);

    // void Positional();

    bool Help();

    std::string HelpDescription();
};

} // namespace ArgumentParser