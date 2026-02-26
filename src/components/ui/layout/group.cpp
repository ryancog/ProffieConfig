#include "group.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2025-2026 Ryan Ogurek
 *
 * components/ui/layout/group.cpp
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

#include <wx/panel.h>
#include <wx/statbox.h>

#include "ui/priv/helpers.hpp"

using namespace pcui;

namespace {

constexpr auto PADDING{
#ifdef __WXOSX__
    0
#else
    10
#endif
};

/**
 * It's like a wxStaticBoxSizer, but cooler.
 *
 * Ensures cross-platform visual consistency.
 * macOS has a nice border around the box, but other platforms do not.
 */
class GroupBox : public wxStaticBox {
public:
    GroupBox(
        wxOrientation orient, wxWindow *parent, const wxString& label
    ) : wxStaticBox(parent, wxID_ANY, label) {
        mSizer = new wxBoxSizer(orient);
        mPanel = new wxPanel(
            this,
            wxID_ANY,
            wxDefaultPosition,
            wxDefaultSize,
            wxTAB_TRAVERSAL | wxNO_BORDER,
            "GroupBox Inside"
        );
        mPanel->SetSizer(mSizer);

        Bind(wxEVT_SIZE, [this](wxSizeEvent& evt) {
            int32 topBorder{};
            int32 otherBorder{};
            GetBordersForSizer(&topBorder, &otherBorder);

            auto size{GetSize()};
            wxPoint pos{0, 0};

            size.x -= PADDING * 2;
            size.x -= otherBorder * 2;

            size.y -= PADDING * 2;
            size.y -= otherBorder + topBorder;

#       if defined (__WXMSW__)
            pos.x += otherBorder + PADDING;
            pos.y += topBorder + PADDING;
#       elif defined (__WXOSX__)
            pos.x += otherBorder + PADDING;
            pos.y += otherBorder + PADDING;
#       else
            pos.x += PADDING;
            pos.y += PADDING;
#       endif

            mPanel->SetSize(pos.x, pos.y, size.x, size.y);
            evt.Skip();
        });

        SetAutoLayout(true);
    }

    wxSizer *sizer() { return mSizer; }
    wxWindow *childParent() { return mPanel; }

    bool Layout() final {
        mPanel->Layout();
        return true;
    }

    wxSize DoGetBestClientSize() const final {
        int32 topBorder{};
        int32 otherBorder{};
        GetBordersForSizer(&topBorder, &otherBorder);
        auto ret{mSizer->CalcMin()};
        ret.x += 2 * otherBorder;
        ret.y += otherBorder + topBorder;
        ret.x += PADDING * 2;
        ret.y += PADDING * 2;
        return ret;
    }

private:
    wxPanel *mPanel;
    wxSizer *mSizer;
};

} // namespace

std::unique_ptr<detail::Descriptor> Group::operator()() {
    return std::make_unique<Group::Desc>(std::move(*this));
}

Group::Desc::Desc(Group&& data) :
    Group{std::move(data)} {}

wxSizerItem *Group::Desc::build(const detail::Scaffold& scaffold) const {
    auto *box{new GroupBox(orient_, scaffold.childParent_, label_)};
    detail::Scaffold childScaffold{
        .childParent_ = box->childParent()
    };

    for (const auto& child : children_) {
        box->sizer()->Add(child->build(childScaffold));
    }

    auto *item{new wxSizerItem(box)};
    priv::apply(base_, item);

    return item;
}

