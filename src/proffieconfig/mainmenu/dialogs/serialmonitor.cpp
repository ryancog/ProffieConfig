#include "serialmonitor.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2024-2025 Ryan Ogurek
 *
 * proffieconfig/mainmenu/dialogs/serialmonitor.cpp
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

#include <chrono>
#include <iomanip>

#include <wx/busyinfo.h>
#include <wx/clipbrd.h>
#include <wx/frame.h>
#include <wx/gdicmn.h>
#include <wx/listbase.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/thread.h>
#include <wx/statusbr.h>

#include "data/context.hpp"
#include "log/context.hpp"
#include "ui/build.hpp"
#include "ui/controls/button.hpp"
#include "ui/controls/list.hpp"
#include "ui/controls/text.hpp"
#include "ui/controls/toggle_button.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/symbols.hpp"
#include "ui/types.hpp"
#include "ui/values.hpp"

namespace {

std::string formatStamp(std::chrono::system_clock::time_point, bool = false);

} // namespace

std::vector<std::string> SerialMonitorDlg::smHistory;

SerialMonitorDlg::SerialMonitorDlg(wxWindow *parent, std::string str) :
    pcui::Frame(parent, wxID_ANY, _("Serial Monitor")),
    mDev(std::move(str)) {

    mInput.disable();
    mNumLines.update({.max_=std::numeric_limits<int32>::max()});
    mAutoScroll.set(true);
    mHistoryIdx = smHistory.size();
    mMon.setOnDisconnect([this] { onDisconnect(); });

    CreateStatusBar(1);
    SetStatusBarPane(-1);
    SetStatusText(wxString::Format(
        _("Connecting to %s..."), mDev
    ));

    pcui::build(this, ui());
    bindEvents();

    activate();

    mConnectThread = std::thread{[this] { connectLoop(); }};
}

pcui::DescriptorPtr SerialMonitorDlg::ui() {
    return pcui::Stack{
      .base_={
        .minSize_={450, 300},
        .expand_=true,
        .proportion_=1,
        .border_={.size_=pcui::winEdgeSpacing(), .dirs_=wxALL},
      },
      .orient_=wxVERTICAL,
      .children_={
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Text{
              .win_={
                .base_={.proportion_=1},
                .id_=eID_Input,
              },
              .data_=mInput,
              .font_=pcui::Font::Monospace,
              .style_=pcui::Text::SingleLine{
                .hint_=_("Type Command"),
                .onEnter_=[this] { onCmdEnter(); },
              },
            }(),
            pcui::Button{
              .win_={
                .base_={.minSize_=pcui::iconButtonSize()},
                .tooltip_=_("View serial monitor commands"),
              },
              .label_="?",
              .style_=pcui::ToggleButton::Style::Companion,
              .exactFit_=true,
              .func_=[this] { doHelp(); },
            }(),
          },
        }(),
        pcui::Spacer{.size_=pcui::interGroupSpacing()}(),
        pcui::List{
          .win_={
            .base_={.expand_=true, .proportion_=1},
            .id_=eID_Output,
          },
          .columns_=2U,
          .rows_=mNumLines,
          .labeler_=[this](size row, size col) {
              return getLabel(row, col);
          },
          // wxNO_BORDER? Why did I have that?
          // Also, AlwaysShowScrollbars(false, false)?
        }(),
        pcui::Stack{
          .base_={.expand_=true},
          .orient_=wxHORIZONTAL,
          .children_={
            pcui::Button{
              .win_={
                .base_={.minSize_=pcui::iconButtonSize()},
                .tooltip_=_("Clear output"),
              },
              .label_=pcui::syms::CLEAR,
              .style_=pcui::ToggleButton::Style::Companion,
              .exactFit_=true,
              .func_=[this] { doClear(); },
            }(),
            pcui::StretchSpacer{}(),
            pcui::ToggleButton{
              .win_={
                .base_={.minSize_=pcui::iconButtonSize()},
                .tooltip_=_("Auto-scroll to keep new messages in view"),
              },
              .label_=pcui::syms::DOWN_ARROW,
              .data_=mAutoScroll,
              .style_=pcui::ToggleButton::Style::Companion,
              .exactFit_=true,
            }(),
          }
        }(),
      }
    }();
}

void SerialMonitorDlg::bindEvents() {
    static const auto inputTable{[] {
        data::base::String::RecvTable table;
        table.onChange_ = data::map(&SerialMonitorDlg::onCmdChange);
        return table;
    }()};
    observeWith(mInput, inputTable);

    static const auto autoScrollTable{[] {
        data::base::Bool::RecvTable table;
        table.onSet_ = data::map(&SerialMonitorDlg::onAutoScroll);
        return table;
    }()};
    observeWith(mAutoScroll, autoScrollTable);

    auto *input{FindWindow(eID_Input)};
    input->Bind(
        wxEVT_KEY_DOWN,
        &SerialMonitorDlg::onKey,
        this
    );

    auto *output{FindWindow(eID_Output)};
    output->Bind(
        wxEVT_CONTEXT_MENU,
        &SerialMonitorDlg::onOutputContext,
        this
    );

    Bind(
        wxEVT_MENU,
        &SerialMonitorDlg::onOutputMenu,
        this
    );

    Bind(wxEVT_CLOSE_WINDOW, &SerialMonitorDlg::onClose, this);
}

void SerialMonitorDlg::onCmdChange() {
    if (not wxIsMainThread())
        // Definitely not from a user input. Also can't touch smHistory from
        // non-main thread.
        return;

    auto ctxt{data::context(mInput)};

    if (mHistoryIdx == smHistory.size())
        // Already where history should be.
        return;

    if (smHistory[mHistoryIdx] == ctxt.val())
        // History entry matches current text, keep the idx.
        return;

    // Text is new and so history idx should be the "current"/working idx.
    mHistoryIdx = smHistory.size();
}

void SerialMonitorDlg::onCmdEnter() {
    auto ctxt{data::context(mInput)};

    bool found{false};
    for (auto iter{smHistory.rbegin()}; iter != smHistory.rend(); ++iter) {
        if (*iter != ctxt.val()) continue;

        auto val{std::move(*iter)};
        smHistory.erase(std::prev(iter.base()));
        smHistory.push_back(std::move(val));

        found = true;
        break;
    }

    if (not found)
        smHistory.push_back(ctxt.val());

    mHistoryIdx = smHistory.size();
    mInput.clear();

    if (smHistory.back() == "clear") {
        doClear();
    } else {
        if (auto err{mMon.write(smHistory.back())}) {
            // TODO: Display error
            logging::Context::getGlobal().quickLog(
                logging::Severity::Warn,
                "SerialMonitorDlg::onCmdEnter()",
                wxString::Format(
                    "Error writing: %d (%d)",
                    err.code_,
                    err.rsn_
                ).utf8_string()
            );
            return;
        }

        mLines.emplace_back(std::make_unique<LineData>(
            formatStamp(std::chrono::system_clock::now(), true),
            std::in_place_type_t<UserLine>{},
            std::string{smHistory.back()}
        ));
        mNumLines.set(static_cast<int32>(mLines.size()));

        doAutoScroll();
    }
}

void SerialMonitorDlg::onAutoScroll() {
    doAutoScroll();
}

void SerialMonitorDlg::doAutoScroll() {
    if (not data::context(mAutoScroll).val())
        return;

    // If the count/state was just futzed with, let the UI update first.
    // Otherwise, the call to EnsureVisible() causes flicker on (at least)
    // macOS. I'm not sure exactly why.
    wxYield();

    auto *output{static_cast<wxListCtrl *>(FindWindow(eID_Output))};
    output->EnsureVisible(static_cast<long>(mLines.size() - 1));
}

void SerialMonitorDlg::doClear() {
    mNumLines.set(0);
    mLines.clear();
}

void SerialMonitorDlg::doHelp() {
    wxLaunchDefaultBrowser("https://pod.hubbe.net/tools/serial-monitor-commands.html");
}

void SerialMonitorDlg::onKey(wxKeyEvent& evt) {
    switch (evt.GetKeyCode()) {
        case WXK_UP:
        case WXK_NUMPAD_UP:
            onUp();
            break;
        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            onDown();
            break;
        default:
            evt.Skip();
    }
}

void SerialMonitorDlg::onUp() {
    if (mHistoryIdx == 0) {
        wxBell();
        return;
    }

    --mHistoryIdx;
    mInput.change(std::string{smHistory[mHistoryIdx]});
}

void SerialMonitorDlg::onDown() {
    if (mHistoryIdx >= smHistory.size()) {
        wxBell();
        return;
    }

    ++mHistoryIdx;

    if (mHistoryIdx == smHistory.size())
        mInput.clear();
    else
        mInput.change(std::string{smHistory[mHistoryIdx]});
}

void SerialMonitorDlg::onOutputContext(wxContextMenuEvent&) {
    wxMenu menu;
    menu.Append(eID_Menu_Copy, _("Copy Output"));
    menu.Append(eID_Menu_Copy_With_Time, _("Copy Output With Time-Stamps"));
    PopupMenu(&menu);
}

void SerialMonitorDlg::onOutputMenu(wxCommandEvent& evt) {
    doCopyOutput(evt.GetId() == eID_Menu_Copy_With_Time);
}

void SerialMonitorDlg::doCopyOutput(bool withStamps) {
    auto *list{static_cast<wxListCtrl *>(FindWindow(eID_Output))};

    std::string data;

    const bool onlySelected{0 != list->GetSelectedItemCount()};

    long item{-1};
    while (not false) {
        item = list->GetNextItem(
            item,
            wxLIST_NEXT_ALL,
            onlySelected ?
                wxLIST_STATE_SELECTED :
                wxLIST_STATE_DONTCARE
        );

        if (item == -1)
            break;

        if (not data.empty())
            data.push_back('\n');

        if (withStamps) {
            data += data::context(mLines[item]->stamp_).val();
            data.push_back(' ');
        }

        auto& var{mLines[item]->var_};
        if (auto *ptr{std::get_if<DeviceLine>(&var)}) {
            data += data::context(ptr->line_).val();
        } else if (auto *ptr{std::get_if<UserLine>(&var)}) {
            if (not withStamps)
                data += "> ";

            data += data::context(ptr->line_).val();
        } else if (auto *ptr{std::get_if<EventLine>(&var)}) {
            data += "???";
        }
    }

    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(data));
        wxTheClipboard->Close();
    }
}

void SerialMonitorDlg::onClose(wxCloseEvent& evt) {
    pcui::cripple(this);

    if (mConnectThread.joinable()) {
        mStopConnecting.release();

        mConnectThread.join();
    }

    // If it's open, this'll call onDisconnect, and that'll make sure the
    // listen thread is joined.
    mMon.close();
    
    evt.Skip();
}

void SerialMonitorDlg::onDisconnect() {
    mInput.disable();

    CallAfter([this] {
        SetStatusText(wxString::Format(
            _("Disconnected (%s)"), mDev
        ));

        // It'll get terminated by `close()`
        if (mListenThread.joinable())
            mListenThread.join();

        // This thread isn't joined until disconnection, even though it terminates
        // (probably) long before.
        if (mConnectThread.joinable())
            mConnectThread.join();

        // If we're active (deactivated in onClose()), retry the connection.
        if (active()) {
            SetStatusText(wxString::Format(
                _("Reconnecting to %s..."), mDev
            ));

            mConnectThread = std::thread{[this] { connectLoop(); }};
        }
    });
}

void SerialMonitorDlg::connectLoop() {
    static constexpr std::chrono::milliseconds CONNECT_RETRY{500};

    while (auto err{mMon.open(mDev)}) {
        CallAfter([this, err] {
            GetStatusBar()->SetFieldsCount(2);

            const auto errStr{wxString::Format(
                "%u (%d)", err.rsn_, err.code_
            )};
            SetStatusText(errStr, 1);

            std::array<int, 2> sizes{
                -1,
                GetTextExtent(errStr).GetWidth() + 10
            };
            SetStatusWidths(2, sizes.data());
        });

        if (mStopConnecting.try_acquire_for(CONNECT_RETRY))
            return;
    }

    CallAfter([this] {
        GetStatusBar()->SetFieldsCount(1);
        SetStatusText(wxString::Format(
            _("Connected to %s"), mDev
        ));
    });

    mInput.enable();
    mInput.focus();
    mListenThread = std::thread{[this] { listenLoop(); }};
}

void SerialMonitorDlg::listenLoop() {
    bool newline{true};

    while (not false) {
        char chr{};

        if (auto err{mMon.read(chr)}) {
            // TODO: Display error
            logging::Context::getGlobal().quickLog(
                logging::Severity::Warn,
                "SerialMonitorDlg::listenLoop()",
                wxString::Format(
                    "Error reading: %d (%d)",
                    err.code_,
                    err.rsn_
                ).utf8_string()
            );
            break;
        }

        bool newlineForThis{newline};

        newline = false;
        if (chr == '\n') {
            // Even though the line isn't changed, we might create a new 
            // line, so the rest of the handling should still be run.
            newline = true;
        }

        // Do all this on the main thread to avoid deadlock situations if
        // access to mLines needed to be locked.
        CallAfter([this, newlineForThis, chr] {
            LineData *data{nullptr};
            if (newlineForThis) {
                data = mLines.emplace_back(std::make_unique<LineData>(
                    std::in_place_type_t<DeviceLine>{}
                )).get();
            } else {
                for (auto iter{mLines.rbegin()}; iter != mLines.rend(); ++iter) {
                    if (std::holds_alternative<DeviceLine>((*iter)->var_)) {
                        data = iter->get();
                        break;
                    }
                }
                assert(data != nullptr);
            }

            data->stamp_.change(formatStamp(std::chrono::system_clock::now()));

            auto& devLine{std::get<DeviceLine>(data->var_)};
            if (std::isgraph(chr) or std::isblank(chr)) {
                auto ctxt{data::context(devLine.line_)};
                if (ctxt.pos() < ctxt.val().size()) {
                    auto tmp{ctxt.val()};
                    tmp[ctxt.pos()] = chr;
                    ctxt.change(std::move(tmp), ctxt.pos() + 1);
                } else {
                    ctxt.change(ctxt.val() + chr);
                }
            } else if (chr == '\r') {
                devLine.line_.move(0);
            }

            mNumLines.set(static_cast<int32>(mLines.size()));

            doAutoScroll();
        });
    }
}

pcui::List::Label SerialMonitorDlg::getLabel(size row, size col) {
    auto ctxt{data::context(mNumLines)};

    if (col == 0)
        return mLines[row]->stamp_;

    auto& var{mLines[row]->var_};

    if (auto *ptr{std::get_if<DeviceLine>(&var)})
        return ptr->line_;

    if (auto *ptr{std::get_if<UserLine>(&var)})
        return ptr->line_;

    if (auto *ptr{std::get_if<EventLine>(&var)})
        return "???";

    assert(0);
    __builtin_unreachable();

    /*
    for (auto line : mLines) {
        if (auto *ptr{std::get_if<DeviceLine>(&line)}) {
            newOut.append(formatStamp(ptr->stamp_));
            newOut.append(" | ");
            newOut.append(ptr->line_);
            newOut.push_back('\n');
        } else if (auto *ptr{std::get_if<UserLine>(&line)}) {
            newOut.append(formatStamp(ptr->stamp_));
            newOut.append(" >>> ");
            newOut.append(ptr->line_);
            newOut.push_back('\n');
        } else if (auto *ptr{std::get_if<EventLine>(&line)}) {
            switch (ptr->type_) {

            }
        }
    }
    */
}

namespace {

std::string formatStamp(
    std::chrono::system_clock::time_point point, bool user
) {
    auto millis{std::chrono::duration_cast<std::chrono::milliseconds>(
        point.time_since_epoch()
    )};
    millis %= 1000;

    auto rawTime{std::chrono::system_clock::to_time_t(point)};
    auto *time{localtime(&rawTime)};

    // Cannot use std::format. It accesses stuff that the macOS SDK C++
    // library doesn't have. (std::to_chars, available in 13.3)
    std::ostringstream str;
    str.fill('0');

    str << std::setw(2) << time->tm_hour;
    str << ':';
    str << std::setw(2) << time->tm_min;
    str << ':';
    str << std::setw(2) << time->tm_sec;
    str << ':';
    str << std::setw(3) << millis.count();
    str << ' ';
    str << (user ? '>' : ' ');

    return str.str();
}

} // namespace

