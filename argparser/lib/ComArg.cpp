#include "ComArg.h"
#include <iostream>
#include <string>
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
    std::string string_value_;
    int int_value_;
    size_t min_count_{0};
    std::vector<std::string> string_values_;
    std::vector<int> int_values_;
    std::string data_type_;

public:
    void SetName(const std::string& tp, const std::string& first, const std::string& second = "", const std::string& description = "") {
        data_type_ = tp;
        if (first.length() == 1) {
            name_ = first;
            second_name_ = second;
            if (description.length()) {
                description_ = description;
            }
        } else {
            second_name_ = first;
            if (!second.empty() && second.find(" ") != second.npos) {
                description_ = second;
            }
            if (!description.empty()) {
                description_ = description;
            }
        }
    }

    void SetDescription(const std::string& description) {
        description_ = description;
    }

    bool IsMultivalue() {
        return is_multivalue_;
    }

    bool IsPositional() {
        return is_positional_;
    }

    bool IsFlag() {
        return is_flag_;
    }

    bool IsHelp() {
        return data_type_ == "help";
    }

    ComArg MultiValue(size_t min_count = 0) {
        is_multivalue_ = true;
        min_count_ = min_count;
        return *this;
    }

    ComArg Positional() {
        is_positional_ = true;
        return *this;
    }

    ComArg Flag() {
        is_flag_ = true;
        return *this;
    }

    ComArg Default(const std::string& value) {
        if (!is_multivalue_) {
            string_value_ = value;
        } else string_values_.push_back(value);
        return *this;
    }

    ComArg Default(int value) {
        if (!is_multivalue_) {
            int_value_ = value;
        } else int_values_.push_back(value);
        return *this;
    }

    ComArg Default(bool value) {
        flag_value_ = value;
        return *this;
    }

    bool SetValue(std::string value) {
        if (data_type_ != "string") return false;
        if (!is_multivalue_) string_value_ = value;
        else string_values_.push_back(value);
    }

    bool SetValue(int value) {
        if (data_type_ != "int") return false;
        if (!is_multivalue_) int_value_ = value;
        else int_values_.push_back(value);
    }

    bool SetValue(bool value) {
        if (!is_flag_) return false;
        flag_value_ = value;
    }

    ComArg StoreValue(const std::string& value) {
        if (data_type_ != "string") return *this;
        if (!is_multivalue_) string_value_ = value;
        else string_values_.push_back(value);
        return *this;
    }

    ComArg StoreValue(const int& value) {
        if (data_type_ != "int") return *this;
        if (!is_multivalue_) int_value_ = value;
        else int_values_.push_back(value);
        return *this;
    }

    bool SetValues(const std::vector<std::string>& values) {
        if (values.size() < min_count_ || !is_multivalue_ || data_type_ != "string") return false;
        string_values_ = values;
        return true;
    }

    bool SetValues(const std::vector<int>& values) {
        if (values.size() < min_count_ || !is_multivalue_ || data_type_ != "int") return false;
        int_values_ = values;
        return true;
    }

    ComArg StoreValues(const std::vector<std::string>& values) {
        if (values.size() < min_count_ || !is_multivalue_ || data_type_ != "string") return *this;
        string_values_ = values;
        return *this;
    }

    ComArg StoreValues(const std::vector<int>& values) {
        if (values.size() < min_count_ || !is_multivalue_ || data_type_ != "int") return *this;
        int_values_ = values;
        return *this;
    }

    bool IsIt(const std::string& value_name) {
        return (name_ == value_name || second_name_ == value_name);
    }

    std::string GetStringValue() {
        return string_value_;
    }

    int GetIntValue() {
        return int_value_;
    }

    bool GetFlagValue() {
        return flag_value_;
    }

    std::vector<std::string> GetStringValues() {
        return string_values_;
    }

    std::vector<int> GetIntValues() {
        return int_values_;
    }

    std::string GetDescription() {
        std::string result;
        if (!name_.empty()) {
            result.append("-");
            result.append(name_);
            result.append(",");
        }
        result.append(",\t");
        if (!second_name_.empty()) {
            result.append("--");
            result.append(second_name_);
            if (data_type_ == "string" || data_type_ == "int") {
                result.append("=<").append(data_type_).append(">,");
            }
        }
                result.append("\t").append(description_).append("\n");
        return result;
    }

    std::string GetType() {
        return data_type_;
    }

    bool IsEqual(ComArg other) {
        return other.IsIt(name_);
    }
};