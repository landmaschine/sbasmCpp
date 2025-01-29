#pragma once
#include "common.h"

class SymbolTable {
private:
    std::unordered_map<std::string, int> labels;
    std::unordered_map<std::string, int> defines;

public:
    void addLabel(const std::string& name, int address) {
        if (labels.find(name) != labels.end()) {
            throw std::runtime_error("Duplicate label: " + name);
        }
        labels[name] = address;
    }

    void addDefine(const std::string& name, int value) {
        if (defines.find(name) != defines.end()) {
            throw std::runtime_error("Duplicate define: " + name);
        }
        defines[name] = value;
    }

    int getLabelAddress(const std::string& name) const {
        auto it = labels.find(name);
        if (it == labels.end()) {
            throw std::runtime_error("Undefined label: " + name);
        }
        return it->second;
    }

    int getDefineValue(const std::string& name) const {
        auto it = defines.find(name);
        if (it == defines.end()) {
            throw std::runtime_error("Undefined symbol: " + name);
        }
        return it->second;
    }

    bool hasLabel(const std::string& name) const {
        return labels.find(name) != labels.end();
    }

    bool hasDefine(const std::string& name) const {
        return defines.find(name) != defines.end();
    }
};