#include <iostream>
#include <string>
#include <vector>
#include "ArgParser.h"
#include "ComArg.h"


class ArgParser {
    private:
        std::string name_{};
        std::string small_description_{};
        std::vector<ComArg> params_{};
        ComArg zero_arg_{};
        ComArg current_argument_{};

    ComArg GetParam_(const std::string& name) {
        for (int i = 0; i < params_.size(); ++i) {
            if (params_[i].IsIt(name)) return params_[i];
        }
        return zero_arg_;
    }

    ComArg GetPositional_(const std::string& arg_type) {
        for (int i = 0; i < params_.size(); ++i) {
            if (params_[i].IsPositional() && params_[i].GetType() == arg_type) return params_[i];
        }
        return zero_arg_;
    }

    public:
    ArgParser(const std::string& name) {
        name_ = name;
        zero_arg_.SetName("NULL_ARG", "", "");
    }

    bool Parse(const std::vector<std::string>& splitted_string) {
        std::vector<std::string> string_values{};
        std::vector<int> int_values{};
        std::string curr{};
        size_t pos{};
        ComArg curr_arg{};
        ComArg last{};
        bool is_last_multivalue{false};
        bool is_last_value{false};
        std::string filename{*splitted_string.begin()};
        if (splitted_string.begin() == splitted_string.end() && params_.size()) return false;
        while (splitted_string.begin() != splitted_string.end()) {
            if (!is_last_multivalue) curr = *splitted_string.begin();

            if (curr.size() && curr.substr(0, 1) == "-") {
                if (curr.length() == 1 || curr.substr(1).find_first_not_of("0123456789") == curr.npos) {
                    is_last_value = true;
                    if (curr.substr(1).find_first_not_of("0123456789") == curr.npos) int_values.push_back(std::stoi(curr));
                    else string_values.push_back(curr);
                    continue;
                }
                pos = curr.find("=");
                if (curr.size() > 1 && curr.substr(0, 2) == "-") {
                    if (pos == curr.npos) curr_arg = GetParam_(curr.substr(2));
                    else curr_arg = GetParam_(curr.substr(2, pos));
                } else {
                    if (pos == curr.npos) curr_arg = GetParam_(curr.substr(1));
                    else curr_arg = GetParam_(curr.substr(1, pos));
                }
                if (!curr_arg.IsEqual(zero_arg_)) {
                    if (!(curr.size() > 1 && curr.substr(0, 2) == "-")) {
                        for (int i = 1; i < curr.size(); ++i) {
                            curr_arg = GetParam_(curr.substr(i, 1));
                            if (!curr_arg.IsEqual(zero_arg_) && curr_arg.IsFlag()) curr_arg.SetValue(true);
                            else {
                                is_last_value = true;
                                if (curr.substr(1).find_first_not_of("0123456789") == curr.npos) int_values.push_back(std::stoi(curr));
                                else string_values.push_back(curr);
                                break;
                            }
                        }
                    } else {
                        is_last_value = true;
                        if (curr.substr(1).find_first_not_of("0123456789") == curr.npos) int_values.push_back(std::stoi(curr));
                        else string_values.push_back(curr);
                    }
                    continue;
                }
                if (is_last_multivalue) {
                    if (last.GetType() == "int") {
                        last.SetValues(int_values);
                        int_values.clear();
                    } else {
                        last.SetValues(string_values);
                        string_values.clear();
                    }
                }

                if (int_values.size() || string_values.size()) {
                    ComArg positional{};
                    if (string_values[0].length()) {
                            positional = GetPositional_("sring");
                    } else positional = GetPositional_("int");
                    if (positional.IsEqual(zero_arg_)) return false;
                    if (positional.GetType() == "int") positional.SetValues(int_values);
                    int_values.clear();
                    if (positional.GetType() == "string") positional.SetValues(string_values);
                    string_values.clear();
                }

                if (!curr_arg.IsMultivalue()) {
                    is_last_multivalue = false;
                    if (pos != curr.npos) {
                        if (curr_arg.GetType() == "int") {
                            curr_arg.SetValue(std::stoi(curr.substr(pos + 1)));
                        } else curr_arg.SetValue(curr.substr(pos + 1));
                    } else {
                        curr = *splitted_string.begin();
                        if (curr_arg.GetType() == "int") {
                            curr_arg.SetValue(std::stoi(curr));
                        } else curr_arg.SetValue(curr);
                    }

                } else {
                    is_last_multivalue = true;
                    int_values.clear();
                    string_values.clear();
                    if (pos != curr.npos) {
                        if (curr_arg.GetType() == "int") {
                            int_values.push_back(std::stoi(curr.substr(pos + 1)));
                        } else string_values.push_back(curr.substr(pos + 1));
                    } else {
                        if (curr_arg.GetType() == "int") {
                            int_values.push_back(std::stoi(curr));
                        } else string_values.push_back(curr);
                    }
                    while (splitted_string.begin() != splitted_string.end()) {
                        curr = *splitted_string.begin();
                        if (curr.size() && curr.substr(0, 1) == "-") {
                            if (curr_arg.GetType() == "int" && !curr_arg.SetValues(int_values)) return false;
                            if (!curr_arg.SetValues(string_values)) return false;
                            break;
                        }
                        if (curr_arg.GetType() == "int") {
                            int_values.push_back(std::stoi(curr));
                        } else string_values.push_back(curr); 
                    }
                    last = curr_arg;
                    
                }
            }

        }
        return true;
    }

    bool Parse(int argc, char** argv) {
        std::vector<std::string> string_values{};
        std::vector<int> int_values{};
        std::string curr{};
        size_t pos{};
        ComArg curr_arg{};
        ComArg last{};
        bool is_last_multivalue{false};
        bool is_last_value{false};
        std::string filename{argv[0]};
        if (argc == 1 && params_.size()) return false;
        for (int i = 1; i < argc; ++i) {
            if (!is_last_multivalue) curr = std::string(argv[i]);

            if (curr.size() && curr.substr(0, 1) == "-") {
                if (curr.length() == 1 || curr.substr(1).find_first_not_of("0123456789") == curr.npos) {
                    is_last_value = true;
                    if (curr.substr(1).find_first_not_of("0123456789") == curr.npos) int_values.push_back(std::stoi(curr));
                    else string_values.push_back(curr);
                    continue;
                }
                pos = curr.find("=");
                if (curr.size() > 1 && curr.substr(0, 2) == "-") {
                    if (pos == curr.npos) curr_arg = GetParam_(curr.substr(2));
                    else curr_arg = GetParam_(curr.substr(2, pos));
                } else {
                    if (pos == curr.npos) curr_arg = GetParam_(curr.substr(1));
                    else curr_arg = GetParam_(curr.substr(1, pos));
                }
                if (!curr_arg.IsEqual(zero_arg_)) {
                    if (!(curr.size() > 1 && curr.substr(0, 2) == "-")) {
                        for (int i = 1; i < curr.size(); ++i) {
                            curr_arg = GetParam_(curr.substr(i, 1));
                            if (!curr_arg.IsEqual(zero_arg_) && curr_arg.IsFlag()) curr_arg.SetValue(true);
                            else {
                                is_last_value = true;
                                if (curr.substr(1).find_first_not_of("0123456789") == curr.npos) int_values.push_back(std::stoi(curr));
                                else string_values.push_back(curr);
                                break;
                            }
                        }
                    } else {
                        is_last_value = true;
                        if (curr.substr(1).find_first_not_of("0123456789") == curr.npos) int_values.push_back(std::stoi(curr));
                        else string_values.push_back(curr);
                    }
                    continue;
                }
                if (is_last_multivalue) {
                    if (last.GetType() == "int") {
                        last.SetValues(int_values);
                        int_values.clear();
                    } else {
                        last.SetValues(string_values);
                        string_values.clear();
                    }
                }

                if (int_values.size() || string_values.size()) {
                    ComArg positional{};
                    if (string_values[0].length()) {
                            positional = GetPositional_("sring");
                    } else positional = GetPositional_("int");
                    if (positional.IsEqual(zero_arg_)) return false;
                    if (positional.GetType() == "int") positional.SetValues(int_values);
                    int_values.clear();
                    if (positional.GetType() == "string") positional.SetValues(string_values);
                    string_values.clear();
                }

                if (!curr_arg.IsMultivalue()) {
                    is_last_multivalue = false;
                    if (pos != curr.npos) {
                        if (curr_arg.GetType() == "int") {
                            curr_arg.SetValue(std::stoi(curr.substr(pos + 1)));
                        } else curr_arg.SetValue(curr.substr(pos + 1));
                    } else {
                        ++i;
                        curr = std::string(argv[i]);
                        if (curr_arg.GetType() == "int") {
                            curr_arg.SetValue(std::stoi(curr));
                        } else curr_arg.SetValue(curr);
                    }

                } else {
                    is_last_multivalue = true;
                    int_values.clear();
                    string_values.clear();
                    if (pos != curr.npos) {
                        if (curr_arg.GetType() == "int") {
                            int_values.push_back(std::stoi(curr.substr(pos + 1)));
                        } else string_values.push_back(curr.substr(pos + 1));
                    } else {
                        if (curr_arg.GetType() == "int") {
                            int_values.push_back(std::stoi(curr));
                        } else string_values.push_back(curr);
                    }
                    while (i < argc) {
                        ++i;
                        curr = std::string(argv[i]);
                        if (curr.size() && curr.substr(0, 1) == "-") {
                            if (curr_arg.GetType() == "int" && !curr_arg.SetValues(int_values)) return false;
                            if (!curr_arg.SetValues(string_values)) return false;
                            break;
                        }
                        if (curr_arg.GetType() == "int") {
                            int_values.push_back(std::stoi(curr));
                        } else string_values.push_back(curr); 
                    }
                    last = curr_arg;
                    
                }
            }

        }
        return true;
    }

    ComArg AddStringArgument(const std::string& name1, const std::string& name2 = "", const std::string& description = "") {
        ComArg new_param{};
        new_param.SetName("string", name1, name2, description);
        params_.push_back(new_param);
        return new_param;
    }

    ComArg AddStringArgument(const char& name1, const std::string& name2 = "", const std::string& description = "") {
        ComArg new_param{};
        new_param.SetName("string", std::to_string(name1), name2, description);
        params_.push_back(new_param);
        return new_param;
    }

    ComArg AddIntArgument(const std::string& name1, const std::string& name2 = "", const std::string& description = "") {
        ComArg new_param{};
        new_param.SetName("int", name1, name2, description);
        params_.push_back(new_param);
        return new_param;
    }

    ComArg AddIntArgument(const char& name1, const std::string& name2 = "", const std::string& description = "") {
        ComArg new_param{};
        new_param.SetName("int", std::to_string(name1), name2, description);
        params_.push_back(new_param);
        return new_param;
    }

    ComArg AddFlag(const std::string& name1, const std::string& name2 = "", const std::string& description = "") {
        ComArg new_param{};
        new_param.SetName("flag", name1, name2, description);
        new_param.Flag();
        params_.push_back(new_param);
        return new_param;
    }

    ComArg AddFlag(const char& name1, const std::string& name2 = "", const std::string& description = "") {
        ComArg new_param{};
        new_param.SetName("flag", std::to_string(name1), name2, description);
        new_param.Flag();
        params_.push_back(new_param);
        return new_param;
    }

    ComArg AddHelp(const std::string& name1, const std::string& name2 = "", const std::string& description = "") {
        ComArg new_param{};
        new_param.SetName("help", name1, name2, "Display this help and exit");
        small_description_.append(description);
        params_.push_back(new_param);
        return new_param;
    }

    ComArg AddHelp(const char& name1, const std::string& name2 = "", const std::string& description = "") {
        ComArg new_param{};
        new_param.SetName("help", std::to_string(name1), name2, "Display this help and exit");
        small_description_.append(description);
        params_.push_back(new_param);
        return new_param;
    }

    std::string GetStringValue(const std::string& name, size_t index = -1) {
        ComArg param{GetParam_(name)};
        if (param.IsEqual(zero_arg_)) return "";
        if (param.IsMultivalue() && index == -1) {
            std::vector<std::string> data{param.GetStringValues()};
            if (index == -1) {
                std::string result{};
                for (int i = 0; i < data.size(); ++i) {
                    result.append(data[i]);
                    if (i != data.size() - 1) result.append(", ");
                }
                return result;
            } else {
                if (index < data.size()) return data[index];
                else return "";
            }
        }
        return param.GetStringValue();
    }

    int GetIntValue(const std::string& name, size_t index = -1) {
        ComArg param{GetParam_(name)};
        if (param.IsEqual(zero_arg_)) return INT_MIN;
        if (param.GetType() == "string") return INT_MIN;
        if (param.IsMultivalue()) {
            std::vector<int> data{param.GetIntValues()};
            return data[index];
        }
        return param.GetIntValue();
    }

    bool GetFlag(const std::string& name) {
        ComArg param{GetParam_(name)};
        if (param.IsEqual(zero_arg_) || !param.IsFlag()) return false;
        return param.GetFlagValue();
    }

    bool Help() {
        std::string help = HelpDescription();
        if (help == "") return false;
        std::cout << help;
        return true;
    }

    std::string HelpDescription() {
        std::string result{};
        ComArg param{};
        for (int i = 0; i < params_.size(); ++i) {
            if (params_[i].IsHelp()) {
                param = params_[i];
                break;
            }
        }
        if (param.IsEqual(zero_arg_)) return "";
        result.append(name_);
        result.append("\n");
        result.append(small_description_);
        result.append("\n\n");
        for (int i = 0; i < params_.size(); ++i) {
            if (!params_[i].IsHelp()) result.append(params_[i].GetDescription());
        }
        result.append(param.GetDescription());
        return result;
    }
};