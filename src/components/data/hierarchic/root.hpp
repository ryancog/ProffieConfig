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

// TODO: There's a lot left over from changing and leftoever ideas.
//       The intent is generally poorly conveyed, and I need to give this
//       proper effort to clean it up eventually.
struct DATA_EXPORT Root : Model {
    struct ROContext;
    struct Context;
    struct RecvTable;

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
     * When this function is called, UI entry is denied and no actions are
     * recorded until unsuppressed. All actions are performed, but they do not
     * get recorded into the undo/redo like normal (not that it matters since
     * it's cleared soon anyways.)
     */
    void suppressActions();

    /**
     * Counterpart to suppressActions()
     *
     * Calling this function releases that suppressed state, and necessarily
     * purges any undo/redo info present, as the state is assumed to have been
     * tampered with in a way that makes it incompatible with any prior
     * recordings.
     */
    void unsuppressActions();

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
     * aborted. This has the same effect as the state clearing out (in terms of
     * restoring UI input and allowing other actions), except no effect is
     * actually recorded.
     */
    void abortCapture();

    /**
     * A previously-recorded compound action is about to performed.
     *
     * During this time UI entry is denied to ensure chronology, and cascading
     * actions are suppressed in favor of the recorded actions being
     * performed.
     *
     * @return If a replay is allowed right now.
     */
    [[nodiscard]] bool beginReplay();

    /**
     * End replay state.
     */
    void endReplay();

    /**
     * According to the capture state, record action and update accordingly.
     */
    void recordAction(std::unique_ptr<Action>&&);

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
    std::vector<std::vector<std::unique_ptr<Action>>> mActions;

    enum class State {
        Normal,
        Suppressed,
        Performance,
        Replay,
    } mState{State::Normal};
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
     * Listener is being detached from the root.
     */
    Mapping<> onDetach_;

    /**
     * Undone now at action
     */
    Mapping<size> onActionIdx_;

    /**
     * Actions have been cleared
     *
     * size lastIdx
     */
    Mapping<size> onActionClear_;

    /**
     * Root now has/no longer has actions to undo.
     */
    Mapping<bool> onCanUndo_;

    /**
     * Root now has/no longer has actions to redo.
     */
    Mapping<bool> onCanRedo_;
};

} // namespace data::hier

