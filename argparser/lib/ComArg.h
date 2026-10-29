#pragma once
#include <iostream>
#include <vector>

class ComArg {
    private:
        std::string name_;
        std::string second_name_;
        std::string description_;
        bool is_multivalue_{false};
        bool is_positional_{false};
        bool is_flag_{false};
        bool flag_value_{false};
        size_t min_count_{0};
        std::vector<std::string> string_values_;
        std::vector<int> int_values_;
        std::string data_type_;
    public:
        void SetName(const std::string& tp, const std::string& first, const std::string& second = "", const std::string& description = "");

        void SetDescription(const std::string& description);

        bool IsMultivalue();

        bool IsPositional();

        bool IsFlag();

        bool IsHelp();

        ComArg MultiValue(size_t min_count = 0);

        ComArg Positional();

        ComArg Flag();

        ComArg Default(const std::string& value);

        ComArg Default(int value);

        ComArg Default(bool value);

        bool SetValue(const std::string& value);

        bool SetValue(int value);

        bool SetValue(bool value);

        ComArg StoreValue(const std::string& value);

        ComArg StoreValue(const int& value);

        bool SetValues(const std::vector<std::string>& values);

        bool SetValues(const std::vector<int>& values);

        ComArg StoreValues(const std::vector<std::string>& values);

        ComArg StoreValues(const std::vector<int>& values);

        bool IsIt(const std::string& value_name);

        std::string GetStringValue();

        int GetIntValue();

        bool GetFlagValue();

        std::vector<std::string> GetStringValues();

        std::vector<int> GetIntValues();

        std::string GetDescription();

        std::string GetType();

        bool IsEqual(ComArg other);
};