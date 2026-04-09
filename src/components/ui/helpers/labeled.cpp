#include "labeled.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/ui/helpers/labeled.cpp
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

#include "ui/helpers/if.hpp"
#include "ui/layout/spacer.hpp"
#include "ui/layout/stack.hpp"
#include "ui/static/label.hpp"
#include "ui/values.hpp"

using namespace pcui;

DescriptorPtr Labeled::operator()() {
    return pcui::Stack{
      .base_=base_,
      .orient_=orient_,
      .children_={
        pcui::Label{
          .win_={
            .base_={
              .align_=orient_==wxHORIZONTAL? wxALIGN_CENTER : wxALIGN_NOT,
            },
          },
          .label_=label_,
        }(),
        pcui::If{
          .cond_=orient_==wxHORIZONTAL,
          .then_=pcui::Spacer{.size_=pcui::interControlSpacing()}(),
        }(),
        std::move(ctrl_),
      }
    }();
}

