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
#include "wx/osx/radiobut.h"

#define PDEF_DEFAULT_CHECK [](const ProffieDefine* def) -> bool { return def->getState(); }

class Settings {
public:
    Settings(EditorWindow*);
    ~Settings();

    void parseDefines(std::vector<wxString>&);

    class ProffieDefine;
    std::unordered_map<wxString, ProffieDefine*> generalDefines;
    std::vector<wxString> readDefines;
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

    const wxString mIdentifier;
    union {
        void *const mElement{nullptr};
        PCUI::Numeric *const mNumeric;
        PCUI::NumericDec *const mNumericDec;
        wxCheckBox *const mCheckBox;
        wxRadioButton *const mRadioButton;
        PCUI::Choice *const mChoice;
        PCUI::Text *const mText;
    };

public:
    ProffieDefine(wxString name, PCUI::Numeric* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
    ProffieDefine(wxString name, PCUI::NumericDec* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
    ProffieDefine(wxString name, wxCheckBox* element, std::function<bool(const ProffieDefine*)> check = PDEF_DEFAULT_CHECK, bool loose = false);
    ProffieDefine(wxString name, wxRadioButton* element, std::function<bool(const ProffieDefine*)> check = PDEF_DEFAULT_CHECK, bool loose = false);
    ProffieDefine(wxString name, PCUI::Choice* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);
    ProffieDefine(wxString name, PCUI::Text* element, std::function<bool(const ProffieDefine*)> check, bool loose = false);

    static std::pair<wxString, wxString> parseKey(const wxString&);

    std::function<bool(const ProffieDefine*, const wxString&)> parse = [this](const ProffieDefine* def, const wxString& input) -> bool {
        auto key = parseKey(input);

        if (def->mLooseChecking ? std::strstr(key.first.c_str(), def->mIdentifier.c_str()) == nullptr : key.first != def->mIdentifier) return false;

        long intVal{0};
        float64 doubleVal{0};
        switch (def->mType) {
            case Type::STATE:
                mCheckBox->SetValue(true);
                break;
            case Type::RADIO:
                mRadioButton->SetValue(true);
                break;
            case Type::NUMERIC:
                mNumeric->entry()->SetValue(key.second);
                break;
            case Type::DECIMAL:
                mNumericDec->entry()->SetValue(key.second);
                break;
            case Type::COMBO:
                mChoice->entry()->SetStringSelection(key.second);
                break;
            case Type::TEXT:
                mText->entry()->SetValue(key.second);
                break;
        }

        return true;
    };
    std::function<wxString(const ProffieDefine*)> output = [](const ProffieDefine* def) -> wxString {
        switch (def->mType) {
            case Type::NUMERIC:
            case Type::DECIMAL:
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

    [[nodiscard]] wxString getOutput() const { return output(this); }
    [[nodiscard]] bool parseDefine(const wxString& input) const { return parse(this, input); }

    [[nodiscard]] wxString getName() const { return mIdentifier; }
    [[nodiscard]] bool shouldOutput() const { return checkOutput(this); }

    inline void overrideParser(std::function<bool(const ProffieDefine*, const wxString&)> _newParser) { parse = std::move(_newParser); }
    inline void overrideOutput(std::function<wxString(const ProffieDefine*)> _newOutput) { output = std::move(_newOutput); }

    template<typename T = int32>
    inline T getNum() const {
        switch (mType) {
            case Type::STATE:
                return mCheckBox->GetValue() ? 1 : 0;
            case Type::RADIO:
                return mRadioButton->GetValue() ? 1 : 0;
            case Type::NUMERIC:
                return mNumeric->entry()->GetValue();
            case Type::DECIMAL:
                return mNumericDec->entry()->GetValue();
            case Type::COMBO:
                return mChoice->entry()->GetSelection();
            case Type::TEXT:
                return mText->entry()->GetValue().length();
        }

        return 0;
    }

    inline bool getState() const {
        switch (mType) {
            case Type::STATE:
                return mCheckBox->GetValue();
            case Type::RADIO:
                return mRadioButton->GetValue();
            case Type::NUMERIC:
                return mNumeric->entry()->GetValue() != 0;
            case Type::DECIMAL:
                return mNumericDec->entry()->GetValue() != 0;
            case Type::COMBO:
                return mChoice->entry()->GetSelection() != 0;
            case Type::TEXT:
                return not mText->entry()->IsEmpty();
        }

        return false;
    }

    inline wxString getString() const {
        switch (mType) {
            case Type::STATE:
                return mCheckBox->GetValue() ? _("Enabled") : _("Disabled");
            case Type::RADIO:
                return mRadioButton->GetValue() ? _("Enabled") : _("Disabled");
            case Type::NUMERIC:
                return std::to_string(mNumeric->entry()->GetValue());
            case Type::DECIMAL:
                return std::to_string(mNumericDec->entry()->GetValue());
            case Type::COMBO:
                return mChoice->entry()->GetStringSelection().ToStdString();
            case Type::TEXT:
                return mText->entry()->GetValue().ToStdString();
        }

        return {};
    }
};
