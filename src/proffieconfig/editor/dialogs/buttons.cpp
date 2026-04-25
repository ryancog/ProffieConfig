#include "buttons.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * proffieconfig/editor/dialogs/buttons.cpp
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config/buttons/button.hpp"
#include "config/strings.hpp"
#include "data/logic/adapter.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/choice.hpp"
#include "ui/controls/combobox.hpp"
#include "ui/controls/stepper.hpp"
#include "ui/controls/text.hpp"
#include "ui/helpers/labeled.hpp"
#include "ui/layout/group.hpp"
#include "ui/layout/scrolled.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/layout/vecstack.hpp"
#include "ui/static/divider.hpp"
#include "ui/static/hyperlink.hpp"
#include "ui/static/label.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

namespace {

constexpr auto PANEL_PADDING{3};

const std::vector<wxString> pinDefaults{
    _("powerButtonPin"),
    _("auxPin"),
    _("aux2Pin"),
};

} // namespace

ButtonsDlg::ButtonsDlg(wxWindow *parent, config::Config& config) :
    pcui::Dialog(
        parent,
        wxID_ANY,
        _("Buttons"),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    ),
    mConfig{config} {

    pcui::build(this, ui());
}

pcui::DescriptorPtr ButtonsDlg::ui() {
    return pcui::Stack{
      .base_={
        .minSize_={400, 500},
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Group{
          .win_={.base_={.expand_=true, .proportion_=1}},
          .padded_=false,
          .children_={
            pcui::Scrolled{
              .win_={.base_={.expand_=true, .proportion_=1}},
              .scrollRate_={.y_=4},
              .child_=pcui::VecStack{
                .base_={
                  .expand_=true,
                  .proportion_=1,
                  .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxALL},
                },
                .orient_=wxVERTICAL,
                .data_=mConfig.buttons_,
                .builder_=button,
                .separator_=pcui::Stack{
                  .base_={.expand_=true},
                  .children_={
                    pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
                    pcui::Divider{
                      .base_={.expand_=true},
                      .orient_=wxHORIZONTAL,
                    }(),
                    pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
                  }
                }(),
                .empty_=pcui::Stack{
                  .base_={.expand_=true, .proportion_=1},
                  .children_={
                    pcui::StretchSpacer{.proportion_=1}(),
                    pcui::Label{
                      .win_={
                        .base_={
                          .border_={.size_=pcui::interGroupSpacing(), .dirs_=wxALL},
                          .align_=wxALIGN_CENTER,
                        },
                      },
                      .label_=_("No Buttons"),
                      .color_=wxSYS_COLOUR_GRAYTEXT,
                    }(),
                    pcui::StretchSpacer{.proportion_=1}(),
                  }
                }(),
              }(),
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={.base_={.align_=wxALIGN_RIGHT}},
          .label_=_("New Button"),
          .func_=[this] { addButton(); },

        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        info(),
      }
    }();
}

pcui::DescriptorPtr ButtonsDlg::button(data::Model& model) {
    auto& button{static_cast<config::buttons::Button&>(model)};

    return pcui::Stack{
      .base_={.expand_=true},
      .orient_=wxVERTICAL,
      .children_={
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Labeled{
              .win_={
                .tooltip_=_("The event triggered when the button is pressed."),
              },
              .label_=_("Event"),
              .orient_=wxHORIZONTAL,
              .ctrl_=pcui::Choice{
                .win_={.base_={.proportion_=1}},
                .data_=button.event_,
                .style_=pcui::Choice::PopUp{
                  .unselected_=_("Select Event"),
                },
                .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                    switch (static_cast<config::ButtonEvent>(idx)) {
                        using enum config::ButtonEvent;
                        case eBtn_Evt_Power: return _("Power");
                        case eBtn_Evt_Aux: return _("Aux");
                        case eBtn_Evt_Aux2: return _("Aux 2");
                        case eBtn_Evt_Up: return _("Up/Fire");
                        case eBtn_Evt_Down: return _("Down/Mode Select");
                        case eBtn_Evt_Left: return _("Left/Magazine Detect");
                        case eBtn_Evt_Right: return _("Right/Reload");
                        case eBtn_Evt_Select: return _("Select/Range");
                        case eBtn_Evt_Max: break;
                    }
                    return {};
                }
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Labeled{
              .win_={
                .base_={.proportion_=1},
                .tooltip_=_("The button name for Serial Monitor and debugging."),
              },
              .label_=_("Name"),
              .orient_=wxHORIZONTAL,
              .ctrl_=pcui::Text{
                .win_={.base_={.proportion_=1}},
                .data_=button.name_,
                .style_=pcui::Text::SingleLine{
                  .hint_=_("\"Display\" Name"),
                }
              }(),
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Labeled{
              .win_={
                .tooltip_={
                  _("The physical kind of button wired to the board.") + '\n' +
                  _("Pull-Up means the switch is wired to its pin and GND.") + '\n' +
                  _("Pull-Down means the switch is wired to its pin and 3v3 or BATT+") + '\n' +
                  _("If you're using a Pull-Down button with BATT+, be sure the pin is tolerant! See link.")
                },
              },
              .label_=_("Type"),
              .orient_=wxHORIZONTAL,
              .ctrl_=pcui::Choice{
                .win_={.base_={.proportion_=1}},
                .data_=button.type_,
                .style_=pcui::Choice::PopUp{
                  .unselected_=_("Select Type"),
                },
                .labeler_=[](uint32 idx) -> pcui::Choice::Label {
                    switch (static_cast<config::ButtonType>(idx)) {
                        using enum config::ButtonType;
                        case eBtn_Type_Pullup:
                            return _("Momentary (Pull-Up)");
                        case eBtn_Type_Pulldown:
                            return _("Momentary (Pull-Down)");
                        case eBtn_Type_Latching:
                            return _("Latching (Pull-Up)");
                        case eBtn_Type_Latching_Inverted:
                            return _("Latching (Pull-Down");
                        case eBtn_Type_Touch:
                            return _("Touch");
                        case eBtn_Type_Max:
                            break;
                    }
                    return {};
                }
              }(),
            }(),
            pcui::Spacer{.size_=pcui::interControlSpacing()}(),
            pcui::Stack{
              .base_={.proportion_=1},
              .orient_=wxVERTICAL,
              .children_={
                pcui::Labeled{
                  .win_={
                    .base_={.expand_=true},
                    .tooltip_=_("The pin on the board the button is wired to."),
                  },
                  .label_=_("Pin"),
                  .orient_=wxHORIZONTAL,
                  .ctrl_=pcui::ComboBox{
                    .win_={.base_={.proportion_=1}},
                    .data_=button.pin_,
                    .hint_=_("Board Button Pin"),
                    .defaults_=pinDefaults,
                  }(),
                }(),
                pcui::Labeled{
                  .win_={
                    .base_={
                      .expand_=true,
                      .border_={.size_=pcui::interControlSpacing(), .dirs_=wxTOP},
                    },
                    .show_=button.type_ |
                        data::logic::HasSelection{{config::eBtn_Type_Touch}},
                    .tooltip_=_("Touch threshold. See the link for more information below."),
                  },
                  .label_=_("Threshold"),
                  .orient_=wxHORIZONTAL,
                  .ctrl_=pcui::Stepper{
                    .win_={.base_={.proportion_=1}},
                    .data_=button.touch_,
                  }(),
                }(),
              }
            }(),
          }
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Button{
          .win_={.base_={.align_=wxALIGN_RIGHT}},
          .label_=_("Remove"),
          .func_=[&button, &model](pcui::CallbackContext ctxt) {
              auto& vec{*button.parent<data::Vector>()};

              // Removing the model will destroy this UI first, and the UI
              // cannot be destroyed from inside the callback.
              ctxt.topLevel_->CallAfter([&vec, &model] {
                  data::Vector::Context vecCtxt{vec};
                  vecCtxt.remove(model);
              });
          }
        }(),
      }
    }();
}

void ButtonsDlg::addButton() {
    data::Vector::Context vec{mConfig.buttons_};
    auto& button{vec.addCreate<config::buttons::Button>()};

    data::Choice::Context event{button.event_};
    data::String::Context name{button.name_};

    data::Choice::Context type{button.type_};
    data::String::Context pin{button.pin_};

    // Assign some reasonable defaults for the first buttons created.
    switch (vec.children().size()) {
        case 1:
            event.choose(config::eBtn_Evt_Power);
            name.change("pow");
            type.choose(config::eBtn_Type_Pullup);
            pin.change(pinDefaults[0].ToStdString());
            break;
        case 2:
            event.choose(config::eBtn_Evt_Aux);
            name.change("aux");
            type.choose(config::eBtn_Type_Pullup);
            pin.change(pinDefaults[1].ToStdString());
            break;
        case 3:
            event.choose(config::eBtn_Evt_Aux2);
            name.change("aux2");
            type.choose(config::eBtn_Type_Pullup);
            pin.change(pinDefaults[2].ToStdString());
            break;
        default:
    }
}

pcui::DescriptorPtr ButtonsDlg::info() {
    return pcui::Group{
      .win_={.base_={.expand_=true}},
      .label_=_("Buttons Configuration Information"),
      .orient_=wxVERTICAL,
      .children_={
        pcui::Hyperlink{
          .label_=_("Touch Button Info"),
          .link_="https://pod.hubbe.net/hardware/touch-buttons.html"
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Hyperlink{
          .label_=_("Button Commands"),
          .link_="https://pod.hubbe.net/tools/button-commands.html"
        }(),
        pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        pcui::Hyperlink{
          .label_=_("Pins Safe for Pulldown Buttons"),
          .link_="https://crucible.hubbe.net/t/button-types/5137/16?u=ryryog25"
        }(),
      }
    }();
}

/*
void ButtonsDlg::createButtonsArea() {
    mButtonsArea = new wxScrolledWindow(this, wxID_ANY);
    mButtonsArea->Bind(wxEVT_SIZE, [this](wxSizeEvent& evt) {
        mButtonsArea->SetVirtualSize(
            mButtonsArea->GetSize().x,
            mButtonsArea->GetVirtualSize().y
        );
        evt.Skip();
    });
    auto *sizer{new wxWrapSizer(wxHORIZONTAL, wxREMOVE_LEADING_SPACES)};

    mButtonsArea->SetScrollRate(-1, 10);
    mButtonsArea->SetSizerAndFit(sizer);

    { 
        Config::Settings::ButtonData dummy;
        auto *dummyPanel{new ButtonPanel(mButtonsArea, mParent->getOpenConfig(), dummy)};
        mButtonsArea->SetMinSize({
            dummyPanel->GetBestSize().x + (PANEL_PADDING * 2),
            300
        });
        dummyPanel->Destroy();
    }

}
*/

