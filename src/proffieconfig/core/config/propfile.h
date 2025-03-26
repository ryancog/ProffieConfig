#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <memory>
#include <unordered_map>

#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/radiobut.h>
#include <wx/combobox.h>
#include <wx/panel.h>

#include "log/logger.h"
#include "pconf/pconf.h"
#include "utils/types.h"

namespace std {

template <>
struct hash<vector<string>> {
    size_t operator()(const vector<string>& vector) const {
        size_t hash = 0;
        for (const auto& string : vector) {
            hash ^= std::hash<std::string>{}(string) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};

} // namespace std

class PropFile : public wxPanel {
public:
    PropFile() = delete;
    ~PropFile() override;
    struct Setting;
    using SettingMap = std::unordered_map<string, Setting>;
    struct Button;
    struct ButtonState {
        ButtonState(string stateName, vector<Button> buttons);
        string stateName;
        vector<Button> buttons;
    };
    using ButtonControls = vector<ButtonState>;
    struct MappedError {
        MappedError(string arduinoError, string displayError);
        string arduinoError;
        string displayError;
    };

    static PropFile* createPropConfig(const string&, wxWindow*, bool builtin = false);

    [[nodiscard]] string getName() const;
    [[nodiscard]] string getFileName() const;
    [[nodiscard]] string getInfo() const;
    SettingMap* getSettings();
    const array<ButtonControls, 4>& getButtons();
    const vector<MappedError>& getMappedErrors();

private:
    PropFile(wxWindow*);

    string mName;
    string mFileName;
    string mInfo;
    SettingMap* mSettings{nullptr};
    array<ButtonControls, 4> mButtons;

    vector<MappedError> mMappedErrors;

    wxBoxSizer* mSizer{nullptr};

    void readSettings(const PConf::HashedData&, Log::Branch&);
    void readLayout(const PConf::Data&, Log::Branch&);
    void readButtons(const std::shared_ptr<PConf::Section>&, Log::Branch&);
    void readErrors(const PConf::HashedData&, Log::Branch&);

    [[nodiscard]] static optional<PConf::HashedData> parseSettingCommon(Setting&, const std::shared_ptr<PConf::Entry>&, Log::Logger&);
};


struct PropFile::Setting {
    void setValue(double) const;
    void enable(bool = true) const;
    [[nodiscard]] string getOutput() const;
    [[nodiscard]] bool checkRequiredSatisfied(const std::unordered_map<string, Setting>&) const;

    string name;
    string define;
    string description;

    vector<string> required;
    vector<string> requiredAny;
    vector<string> disables;
    bool disabled{false};

    double min{0};
    double max{100};
    double increment{1};
    double defaultVal{0};

    vector<string> others;
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
    string name;

    // <Predicate, Description>
    std::unordered_map<string, string> descriptions;
};
