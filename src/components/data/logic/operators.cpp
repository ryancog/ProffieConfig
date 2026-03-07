#include "operators.hpp"
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/logic/operators.cpp
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

#include "data/logic/logic.hpp"

auto data::logic::operator not(
    Element&& child
) -> Element {
    struct Operator : detail::Base {
        Operator(Element&& child) :
            child_{std::move(child)} {}

        bool doActivate(ChangeFunc changeFunc) override {
            const auto onChange{[changeFunc=std::move(changeFunc)](bool val) {
                changeFunc(not val);
            }};
            return not child_->activate(onChange);
        }

        Element child_;
    };

    return std::make_unique<Operator>(std::move(child));
}

auto data::logic::operator or(
    Element&& lhs, Element&& rhs
) -> Element {
    struct Operator : detail::Base {
        Operator(Element&& lhs, Element&& rhs) :
            lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}

        bool doActivate(ChangeFunc changeFunc) override {
            const auto onLHSChange{[this, changeFunc](bool val) {
                const auto oldVal{lhsVal_};
                lhsVal_ = val;
                
                if (oldVal or rhsVal_ != lhsVal_ or rhsVal_) {
                    changeFunc(lhsVal_ or rhsVal_);
                }
            }};
            lhsVal_ = lhs_->activate(onLHSChange);

            const auto onRHSChange{[this, changeFunc](bool val) {
                const auto oldVal{rhsVal_};
                rhsVal_ = val;

                if (lhsVal_ or oldVal != lhsVal_ or rhsVal_) {
                    changeFunc(lhsVal_ or rhsVal_);
                }
            }};
            rhsVal_ = rhs_->activate(onRHSChange);

            return lhsVal_ or rhsVal_;
        }

        bool lhsVal_;
        bool rhsVal_;
        Element lhs_;
        Element rhs_;
    };

    return std::make_unique<Operator>(std::move(lhs), std::move(rhs));
}

auto data::logic::operator and(
    Element&& lhs, Element&& rhs
) -> Element {
    struct Operator : detail::Base {
        Operator(Element&& lhs, Element&& rhs) :
            lhs_{std::move(lhs)}, rhs_{std::move(rhs)} {}

        bool doActivate(ChangeFunc changeFunc) override {
            const auto onLHSChange{[this, changeFunc](bool val) {
                const auto oldVal{lhsVal_};
                lhsVal_ = val;
                
                if (oldVal and rhsVal_ != lhsVal_ and rhsVal_) {
                    changeFunc(lhsVal_ and rhsVal_);
                }
            }};
            lhsVal_ = lhs_->activate(onLHSChange);

            const auto onRHSChange{[this, changeFunc](bool val) {
                const auto oldVal{rhsVal_};
                rhsVal_ = val;

                if (lhsVal_ and oldVal != lhsVal_ and rhsVal_) {
                    changeFunc(lhsVal_ and rhsVal_);
                }
            }};
            rhsVal_ = rhs_->activate(onRHSChange);

            return lhsVal_ and rhsVal_;
        }

        bool lhsVal_;
        bool rhsVal_;
        Element lhs_;
        Element rhs_;
    };

    return std::make_unique<Operator>(std::move(lhs), std::move(rhs));
}

