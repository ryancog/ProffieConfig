#pragma once
// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2025 Ryan Ogurek

#include <cstdint>
#include <cstring>
#include <utility>

#include <wx/checkbox.h>
#include <wx/radiobut.h>

#include "ui/controls.h"
#include "../../editor/editorwindow.h"

#define PDEF_DEFAULT_CHECK [](const ProffieDefine* def) -> bool { return def->getState(); }

class Settings {
public:
    Settings(EditorWindow*);
    ~Settings();

    void parseDefines(std::vector<string>&);

    class ProffieDefine;
    std::unordered_map<string, ProffieDefine*> generalDefines;
    std::vector<string> readDefines;
    int32_t numBlades{0};

private:
    EditorWindow* mParent{nullptr};

    void linkDefines();
    void setCustomInputParsers();
    void setCustomOutputParsers();
};

class Settings::ProffieDefine {
private:
    enum class Type {
        STATE,
        RADIO,
        NUMERIC,
        DECIMAL,
        COMBO,
        TEXT
    } const mType{Type::STATE};
    const bool mLooseChecking{false};

    const string mIdentifier;
    const void *mElement{nullptr};

public:

    ProffieDefine(string name, PCUI::Numeric* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
    ProffieDefine(string name, PCUI::NumericDec* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
    ProffieDefine(string name, wxCheckBox* element, std::function<bool(const ProffieDefine*)> check = PDEF_DEFAULT_CHECK, bool loose = false);
    ProffieDefine(string name, wxRadioButton* element, std::function<bool(const ProffieDefine*)> check = PDEF_DEFAULT_CHECK, bool loose = false);
    ProffieDefine(string name, PCUI::Choice* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
    ProffieDefine(string name, PCUI::Text* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);

    static std::pair<string, string> parseKey(const string&);

    std::function<bool(const ProffieDefine*, const string&)> parse = [](const ProffieDefine* def, const string& input) -> bool {
        auto key = parseKey(input);

        if (def->mLooseChecking ? std::strstr(key.first.c_str(), def->mIdentifier.c_str()) == nullptr : key.first != def->mIdentifier) return false;

        long intVal{0};
        float64 doubleVal{0};
        switch (def->mType) {
            case Type::STATE:
                const_cast<wxCheckBox*>(static_cast<const wxCheckBox*>(def->mElement))->SetValue(true);
                break;
            case Type::RADIO:
                const_cast<wxRadioButton*>(static_cast<const wxRadioButton*>(def->mElement))->SetValue(true);
                break;
            case Type::NUMERIC:
                const_cast<PCUI::Numeric*>(static_cast<const PCUI::Numeric*>(def->mElement))->entry()->SetValue(key.second);
                break;
            case Type::DECIMAL:
                const_cast<PCUI::NumericDec*>(static_cast<const PCUI::NumericDec*>(def->mElement))->entry()->SetValue(key.second);
                break;
            case Type::COMBO:
                const_cast<PCUI::Choice*>(static_cast<const PCUI::Choice*>(def->mElement))->entry()->SetStringSelection(key.second);
                break;
            case Type::TEXT:
                const_cast<PCUI::Text*>(static_cast<const PCUI::Text*>(def->mElement))->entry()->SetValue(key.second);
                break;
        }

        return true;
    };
    std::function<string(const ProffieDefine*)> output = [](const ProffieDefine* def) -> string {
        switch (def->mType) {
            case Type::NUMERIC:
            return def->mIdentifier + " " + std::to_string(def->getNum());
            case Type::DECIMAL:
            return def->mIdentifier + " " + std::to_string(def->getDec());
            case Type::COMBO:
            case Type::TEXT:
            return def->mIdentifier + " " + def->getString();
            case Type::STATE:
            case Type::RADIO:
            default:
            return def->mIdentifier;
        }
    };
    std::function<bool(const ProffieDefine*)> checkOutput;

    [[nodiscard]] string getOutput() const { return output(this); }
    [[nodiscard]] bool parseDefine(const string& input) const { return parse(this, input); }

    [[nodiscard]] string getName() const { return mIdentifier; }
    [[nodiscard]] bool shouldOutput() const { return checkOutput(this); }
    [[nodiscard]] int32_t getNum() const;
    [[nodiscard]] double getDec() const;
    [[nodiscard]] bool getState() const;
    [[nodiscard]] string getString() const;

    inline void overrideParser(std::function<bool(const ProffieDefine*, const string&)> _newParser) { parse = std::move(_newParser); }
    inline void overrideOutput(std::function<string(const ProffieDefine*)> _newOutput) { output = std::move(_newOutput); }
};
