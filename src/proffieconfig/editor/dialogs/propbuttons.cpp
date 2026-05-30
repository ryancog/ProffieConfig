#include "propbuttons.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/propbuttons.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <wx/display.h>
#include <wx/gdicmn.h>
#include <wx/settings.h>

#include "data/context.hpp"
#include "ui/build.hpp"
#include "ui/dynamic_list.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/scrolled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/divider.hpp"
#include "ui/static/label.hpp"
#include "ui/utils.hpp"
#include "ui/values.hpp"

namespace {

extern const versions::props::Buttons DEFAULT_ZERO_BUTTON;
extern const versions::props::Buttons DEFAULT_ONE_BUTTON;
extern const versions::props::Buttons DEFAULT_TWO_BUTTON;

} // namespace

PropButtonsDlg::PropButtonsDlg(wxWindow *parent, config::Config& config) :
    pcui::Dialog(
        parent,
        wxID_ANY,
        _("Prop Buttons"),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    ),
    mConfig{config} {

    static const auto choiceTable{[] {
        data::base::Choice::RecvTable table;
        table.onChoice_=data::map(&PropButtonsDlg::onPropChoice);
        return table;
    }()};
    observeWith(mConfig.propChoice(), choiceTable);

    static const auto buttonsTable{[] {
        data::base::Vector::RecvTable table;
        table.onInsert_=data::map(&PropButtonsDlg::onButtonsChange);
        table.onRemove_=data::map(&PropButtonsDlg::onButtonsChange);
        return table;
    }()};
    observeWith(mConfig.buttons_, buttonsTable);

    activate();
}

PropButtonsDlg::~PropButtonsDlg() {
    deactivate();
}

void PropButtonsDlg::onActivate() {
    rebuildLinks();
    rebuildUI();
}

void PropButtonsDlg::onPropChoice() {
    rebuildLinks();
    rebuildUI();
}

void PropButtonsDlg::onButtonsChange(size) {
    rebuildLinks();
    rebuildUI();
}

void PropButtonsDlg::rebuildLinks() {
    static const auto settingTable{[] {
        data::base::Bool::RecvTable table;
        table.onSet_=data::map(&PropButtonsDlg::rebuildUI);
        return table;
    }()};

    // Clear out everything from the old prop.
    repealAllWithTable(settingTable);

    auto buttons{data::context(mConfig.buttons_)};

    mCurProp = mConfig.prop();
    if (mCurProp != nullptr) {
        mCurButtons = mCurProp->buttons(buttons.children().size());
    } else {
        switch (buttons.children().size()) {
            case 0:
                mCurButtons = &DEFAULT_ZERO_BUTTON;
                break;
            case 1:
                mCurButtons = &DEFAULT_ONE_BUTTON;
                break;
            default:
                mCurButtons = &DEFAULT_TWO_BUTTON;
                break;
        }
    }

    if (mCurButtons == nullptr)
        return;

    for (const auto& state : *mCurButtons) {
        for (const auto& button : state.buttons_) {
            for (const auto& [pred, desc] : button.descriptions_) {
                if (pred.empty() or mCurProp == nullptr)
                    continue;

                auto *setting{mCurProp->find(pred)};
                if (setting == nullptr)
                    continue;

                auto *bl{dynamic_cast<data::base::Bool *>(setting)};
                if (bl == nullptr)
                    // Really should warn on this...
                    continue;

                if (not mapped(*bl))
                    observeWith(*bl, settingTable);
            }
        }
    }
}

void PropButtonsDlg::rebuildUI() {
    // Only cripple from sizer and below, don't deactivate `this`.
    if (auto *sizer{GetSizer()})
        pcui::cripple(sizer);

    pcui::Stack stack{
      .base_={
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
    };

    if (mCurButtons == nullptr) {
        pcui::Label label{
          .label_=_("Selected number of buttons not supported by prop file."),
        };

        stack.children_.add(label());

        pcui::build(this, stack());
        return;
    }

    int width{};
    for (const auto& state : *mCurButtons) {
        for (const auto& [name, descriptions] : state.buttons_) {
            width = std::max(
                width,
                GetTextExtent(name).GetWidth()
            );
        }
    }

    for (const auto& state : *mCurButtons) {
        pcui::Group group{
          .win_={.base_={.expand_=true}},
          .label_=wxString::Format(
            _("Button controls while saber is %s:"),
            state.stateName_
          ),
          .orient_=wxHORIZONTAL,
        };

        pcui::Stack controlStack{
          .base_={.minSize_={width, -1}},
        };
        pcui::Divider divider{
          .base_={.expand_=true},
          .orient_=wxVERTICAL,
        };
        pcui::Stack descStack;

        bool any{false};
        for (const auto& [name, descriptions] : state.buttons_) {
            const std::string *activeDesc{nullptr};
            for (const auto& [pred, desc] : descriptions) {
                if (pred.empty()) {
                    activeDesc = &desc;
                    continue;
                }

                auto *setting{mCurProp->find(pred)};
                if (setting == nullptr)
                    continue;

                if (setting->isActive())
                    activeDesc = &desc;
            }

            if (activeDesc == nullptr)
                continue;

            if (*activeDesc == "DISABLED")
                continue;

            any = true;

            pcui::Label nameLabel{
              .win_={.base_={.align_=wxALIGN_RIGHT}},
              .label_=name,
            };
            pcui::Label descLabel{
              .win_={.base_={.align_=wxALIGN_LEFT}},
              .label_=*activeDesc,
            };

            controlStack.children_.add(nameLabel());
            descStack.children_.add(descLabel());
        }

        if (not any)
            continue;

        group.children_.add(pcui::DynamicList{
            controlStack(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            divider(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            descStack(),
        });

        stack.children_.add(pcui::DynamicList{
            group(),
            pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        });
    }

    if (stack.children_.empty())
        stack.children_.add(pcui::Label{.label_="CONTROLS???"}());
    else
        // Remove very last spacer
        stack.children_.pop_back();

    pcui::Scrolled desc{
        .win_={
          .base_={
            .minSize_={300, 100},
            .expand_=true,
            .proportion_=1,
          },
        },
        .scrollRate_={.x_=10, .y_=10},
        .child_=stack(),
    };
    pcui::build(this, desc());

    // Some props have a *lot* of controls, and these can easily go off-screen
    // for people with lower-resolution displays (1080p and lower, which is
    // still reasonable and plenty common).
    //
    // For people with such monitors, use a scrolled and do calculations to
    // make sure that the window is best-sized to both fit as many controls as
    // possible (all if the display is large enough), and clamp it so the
    // window is no larger than what can be fit on-screen.
    auto *scrolled{pcui::getUniqueChild(this)};
    auto bestContentSize{scrolled->GetSizer()->GetMinSize()};
    auto bestScrolledSize{scrolled->ClientToWindowSize(bestContentSize)};
    auto bestSize{ClientToWindowSize(bestScrolledSize)};

    wxDisplay display(this);
    if (display.IsOk()) {
        auto displayClientSize{display.GetClientArea().GetSize()};
        bestSize.DecTo(displayClientSize);
        SetSize(bestSize);
    }
}

namespace {
using namespace versions::props;

const Buttons DEFAULT_ZERO_BUTTON{[] {
    Buttons buttons;

    std::vector<Button> offButtons;
    offButtons.push_back({
        .name_="Twist",
        .descriptions_={
            {{}, "Ignite Saber"},
        }
    });
    offButtons.push_back({
        .name_="Shake while pointing up",
        .descriptions_={
            {{}, "Next Preset"},
        }
    });
    buttons.push_back({
        .stateName_="OFF", 
        .buttons_=std::move(offButtons)
    });

    std::vector<Button> onButtons;
    onButtons.push_back({
        .name_="Twist",
        .descriptions_={
            {{}, "Retract Saber"},
        }
    });
    onButtons.push_back({
        .name_="Hit Blade",
        .descriptions_={
            {{}, "Clash"},
        }
    });
    buttons.push_back({
        .stateName_="ON",
        .buttons_=std::move(onButtons)
    });

    return buttons;
}()};

const Buttons DEFAULT_ONE_BUTTON{[] {
    Buttons buttons;

    std::vector<Button> offButtons;
    offButtons.push_back({
        .name_="Click",
        .descriptions_={
            {{}, "Ignite Saber"},
        }
    });
    offButtons.push_back({
        .name_="Double click",
        .descriptions_={
            {{}, "Ignite Saber Muted"},
        }
    });
    offButtons.push_back({
        .name_="Hold button and hit blade",
        .descriptions_={
            {{}, "Next Preset"},
        }
    });
    offButtons.push_back({
        .name_="Long click",
        .descriptions_={
            {{}, "Start Soundtrack"},
        }
    });
    buttons.push_back({
        .stateName_="OFF",
        .buttons_=std::move(offButtons)
    });

    std::vector<Button> onButtons;
    onButtons.push_back({
        .name_="Click",
        .descriptions_={
            {{}, "Retract Saber"},
        }
    });
    onButtons.push_back({
        .name_="Hit blade",
        .descriptions_={
            {{}, "Clash"},
        }
    });
    onButtons.push_back({
        .name_="Hold button and clash",
        .descriptions_={
            {{}, "Lockup"},
        }
    });
    onButtons.push_back({
        .name_="Hold button and clash while pointing down",
        .descriptions_={
            {{}, "Drag"},
        }
    });
    onButtons.push_back({
        .name_="Hold button and stab",
        .descriptions_={
            {{}, "Melt"},
        }
    });
    onButtons.push_back({
        .name_="Long click",
        .descriptions_={
            {{}, "Force"},
        }
    });
    onButtons.push_back({
        .name_="Hold button and twist",
        .descriptions_={
            {{}, "Color Change"},
        }
    });
    buttons.push_back({
        .stateName_="ON",
        .buttons_=std::move(onButtons)
    });

    return buttons;
}()};

const Buttons DEFAULT_TWO_BUTTON{[] {
    Buttons buttons;

    std::vector<Button> offButtons;
    offButtons.push_back({
        .name_="Click power",
        .descriptions_={
            {{}, "Ignite Saber"},
        }
    });
    offButtons.push_back({
        .name_="Double click power",
        .descriptions_={
            {{}, "Ignite Saber Muted"},
        }
    });
    offButtons.push_back({
        .name_="Hold power and hit blade",
        .descriptions_={
            {{}, "Next Preset"},
        }
    });
    offButtons.push_back({
        .name_="Hold aux and click power",
        .descriptions_={
            {{}, "Previous Preset"},
        }
    });
    offButtons.push_back({
        .name_="Long click power",
        .descriptions_={
            {{}, "Start Soundtrack"},
        }
    });
    buttons.push_back({
        .stateName_="OFF",
        .buttons_=std::move(offButtons)
    });

    std::vector<Button> onButtons;
    onButtons.push_back({
        .name_="Click Power",
        .descriptions_={
            {{}, "Retract Saber"},
        }
    });
    onButtons.push_back({
        .name_="Hit blade",
        .descriptions_={
            {{}, "Clash"},
        }
    });
    onButtons.push_back({
        .name_="Hold power or aux and clash",
        .descriptions_={
            {{}, "Lockup"},
        }
    });
    onButtons.push_back({
        .name_="Hold power or aux and clash while pointing down",
        .descriptions_={
            {{}, "Drag"},
        }
    });
    onButtons.push_back({
        .name_="Hold power or aux and stab",
        .descriptions_={
            {{}, "Melt"},
        }
    });
    onButtons.push_back({
        .name_="Click aux while holding power",
        .descriptions_={
            {{}, "Lightning Block"},
        }
    });
    onButtons.push_back({
        .name_="Long click power",
        .descriptions_={
            {{}, "Force"},
        }
    });
    onButtons.push_back({
        .name_="Click aux",
        .descriptions_={
            {{}, "Blaster block"},
        }
    });
    onButtons.push_back({
        .name_="Hold aux and click power",
        .descriptions_={
            {{}, "Enter Color Change"},
        }
    });
    buttons.push_back({
        .stateName_="ON",
        .buttons_=std::move(onButtons)
    });

    std::vector<Button> ccButtons;
    ccButtons.push_back({
        .name_="Hold aux and click power",
        .descriptions_={
            {{}, "Exit Color Change"},
        }
    });
    buttons.push_back({
        .stateName_="in Color Change",
        .buttons_=std::move(ccButtons)
    });

    return buttons;
}()};

} // namespace

