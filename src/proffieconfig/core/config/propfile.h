#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include "log/logger.h"
#include "pconf/pconf.h"
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/panel.h>

namespace std {

template <>
struct hash<std::vector<std::string>> {
    size_t operator()(const std::vector<std::string>& vector) const {
        size_t hash = 0;
        for (const auto& string : vector) {
            hash ^= std::hash<std::string>{}(string) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

}

class PropFile : public wxPanel {
public:
    ~PropFile();
    struct Setting;
    typedef std::unordered_map<std::string, Setting> SettingMap;
    struct Button;
    typedef std::vector<std::pair<std::string, std::vector<Button>>> ButtonArray;

    static PropFile* createPropConfig(const std::string&, wxWindow*);

    std::string getName() const;
    std::string getFileName() const;
    std::string getInfo() const;
    SettingMap* getSettings();
    const std::array<ButtonArray, 4>* getButtons();

private:
    PropFile() = delete;
    PropFile(wxWindow*);

    std::string name{};
    std::string fileName{};
    std::string info{};
    SettingMap* settings{nullptr};
    std::array<ButtonArray, 4>* buttons{nullptr};

    wxBoxSizer* sizer{nullptr};

    void readSettings(const PConf::HashedData&, Log::Branch&);
    void readLayout(const PConf::Data&, Log::Branch&);
    void readButtons(const std::shared_ptr<PConf::Section>&, Log::Branch&);

    [[nodiscard]] static optional<PConf::HashedData> parseSettingCommon(Setting&, const std::shared_ptr<PConf::Entry>&, Log::Logger&);
};


struct PropFile::Setting {
    void setValue(double) const;
    void enable(bool = true) const;
    std::string getOutput() const;
    bool checkRequiredSatisfied(const std::unordered_map<std::string, Setting>&) const;

    std::string name{};
    std::string define{};
    std::string description{};

    std::vector<std::string> required{};
    std::vector<std::string> requiredAny{};
    std::vector<std::string> disables{};
    bool disabled{false};

    double min{0};
    double max{100};
    double increment{1};
    double defaultVal{0};

    std::vector<std::string> others{};
    bool isDefault{false};
    bool shouldOutput{true};

    enum class SettingType {
        TOGGLE,
        OPTION,
        NUMERIC,
        DECIMAL,
    } type{SettingType::TOGGLE};

    // Tried using a union... it broke wx
    void* control{nullptr};
};

struct PropFile::Button {
    std::string name{};

    // <Predicate, Description>
    std::unordered_map<std::string, std::string> descriptions{};
};
