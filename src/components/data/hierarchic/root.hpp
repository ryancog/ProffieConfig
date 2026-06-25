#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchic/root.hpp
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

#include <memory>
#include <mutex>
#include <vector>

#include "data/hierarchic/action.hpp"
#include "data/hierarchic/model.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data::hier {

// TODO: There's a lot left over from changing and old ideas.
//       The intent is generally poorly conveyed, and I need to give this
//       proper effort to clean it up eventually.
struct DATA_EXPORT Root : Model {
    struct DATA_EXPORT ROContext;
    struct DATA_EXPORT Context;
    struct DATA_EXPORT RecvTable;

    ~Root() override;

    /**
     * This is an enum because the export when compiling for windows makes
     * things stupid. (Tries to externally link static constexpr var)
     *
     * Of course, why not.
     */
    enum : uint64 {
        eAct_Idx_First = ~0ULL,
    };

protected:
    Root();
    Root(const Root&);

    /**
     * Do not record any actions until unsuppressed. All actions are performed,
     * but they do not get recorded into the undo/redo like normal.
     */
    void suppressActions();

    /**
     * Counterpart to suppressActions()
     *
     * Calling this function releases that suppressed state, and, if set,
     * purges any undo/redo info present, as the state is assumed to have been
     * tampered with in a way that makes it incompatible with any prior
     * recordings.
     *
     * For special cases, the undo/redo information can be left, but this must
     * be used with care. (e.g. new object creation)
     */
    void unsuppressActions(bool clearHistory = true);

private:
    friend Model;
    
    /**
     * Whenever an action is originally performed, other cascading actions may
     * also be triggered. It is imported to capture these as well to be able to
     * properly reverse them.
     *
     * This state is cleared automatically and action properly recorded
     * whenever the nesting evens out (i.e. when the original triggering effect
     * is received).
     *
     * @return if a performance is allowed right now.
     *         It may not be if, e.g., a replay is in progress.
     */
    [[nodiscard]] bool capturePerformance();

    /**
     * capturePerformance() will return true indicating that performance is
     * allowed, but it may not have actually setup a capture.
     *
     * @return if capturing/action send is actually expected.
     */
    [[nodiscard]] bool isActuallyCapturing();

    /**
     * If it is determined the action is unnecessary, the capture state may be
     * aborted.
     */
    void abortCapture();

    /**
     * A previously-recorded compound action is about to performed.
     *
     * During this time UI entry is denied to ensure chronology, and cascading
     * actions are suppressed in favor of the recorded actions being
     * performed.
     */
    void beginReplay(bool undo, Action&);

    /**
     * End replay state.
     */
    void endReplay();

    /**
     * Record action in current list.
     */
    void recordAction(std::unique_ptr<Action>&&);

    /**
     * Call after action has been performed to finish processing.
     */
    void finishCapture();

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;

    /**
     * The index corresponds to the "current" action. The one that represents
     * the state as it is currently. To undo, that action must be undone. To
     * redo, the next action (idx + 1) must be performed.
     *
     * A value of ACT_IDX_FIRST indicates the "current" state to be the
     * beginning of all known operations. I.e. there is no operation prior,
     * only (maybe) operations afterwards.
     */
    size mActionIdx{eAct_Idx_First};
    std::vector<std::unique_ptr<Action>> mActions;

    enum class State {
        Normal,
        Suppressed,
        Performance,
        Replay_Undo,
        Replay_Redo,

        // Special case to check that an observer doesn't try causing actions.
        In_Observer,
    };
    std::vector<State> mStates{State::Normal};

    struct ActionFrame {
        union {
            // Unique identifier for the current level of responding, or 0 to
            // indicate direct action invocations.
            uint64 responderId_{0};

            // For replay
            // Don't use iterators, because the implementation might have non-
            // trivial cdtors which break the union.
            //
            // For redo, mChildren.size() is end
            // For undo, -1UZ is end
            size replayIdx_;
        };

        Action *action_;
    };
    std::vector<ActionFrame> mActionFrames;

    uint32 mPerformanceNesting{0};

    std::recursive_mutex mMutex;
};

struct DATA_EXPORT Root::ROContext : virtual Model::ROContext {
    ROContext(const Root&);

    template <typename M = Root>
    [[nodiscard]] auto& model() const { return Model::ROContext::model<M>(); }

    [[nodiscard]] size actionIndex() const;

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;
};

struct DATA_EXPORT Root::Context : Model::Context, ROContext {
    Context(Root&);

    template <typename M = Root>
    [[nodiscard]] auto& model() const { return Model::Context::model<M>(); }

    void undo() const;
    void redo() const;
};

struct DATA_EXPORT Root::RecvTable : Model::RecvTable {
    /**
     * At different/new action, or the action was updated.
     */
    Mapping<> onAction_;

    /**
     * Actions have been cleared
     *
     * size lastIdx
     */
    Mapping<size> onActionClear_;

    /**
     * Root now has/no longer has actions to undo.
     */
    Mapping<> onCanUndo_;

    /**
     * Root now has/no longer has actions to redo.
     */
    Mapping<> onCanRedo_;
};

} // namespace data::hier

