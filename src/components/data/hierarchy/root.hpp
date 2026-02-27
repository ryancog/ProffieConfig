#pragma once
/*
 * ProffieConfig, All-In-One Proffieboard Management Utility
 * Copyright (C) 2026 Ryan Ogurek
 *
 * components/data/hierarchy/root.hpp
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
#include <vector>

#include "data/hierarchy/action.hpp"
#include "data/hierarchy/node.hpp"
#include "utils/types.hpp"

#include "data_export.h"

namespace data {

struct DATA_EXPORT Root : Node {
    struct Context;
    struct Receiver;

    ~Root() override;

    [[nodiscard]] virtual std::unique_ptr<Root> clone() const = 0;

    void attachReciever(Receiver&);
    void detachReceiver(Receiver&);

protected:
    Root();
    Root(const Root&);

    /**
     * Used during object creation/setup phase, where many actions are taking
     * place programmatically.
     *
     * When this function is called, UI entry is denied and no actions are
     * recorded. All actions are performed.
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
    friend Node;
    
    /**
     * Whenever an action is originally performed, other cascading actions may
     * also be triggered. It is imported to capture these as well to be able to
     * properly reverse them.
     *
     * This state is cleared automatically and action properly recorded
     * whenever the nesting evens out (i.e. when the original triggering effect
     * is received).
     *
     * During this state, UI entry is denied.
     *
     * @param if the action originated from UI, or from code.
     *
     * @return if a performance is allowed to be captured right now.
     *         It may not be if, e.g., a replay is in progress.
     */
    [[nodiscard]] bool capturePerformance(bool fromUI);

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

    std::unique_ptr<Model> clone(Node *) const final;

    static constexpr auto ACT_IDX_FIRST{~0ULL};

    /**
     * The index corresponds to the "current" action. The one that represents
     * the state as it is currently. To undo, that action must be undone. To
     * redo, the next action (idx + 1) must be performed.
     *
     * A value of ACT_IDX_FIRST indicates the "current" state to be the
     * beginning of all known operations. I.e. there is no operation prior,
     * only (maybe) operations afterwards.
     */
    size mActionIdx{ACT_IDX_FIRST};
    std::vector<std::vector<std::unique_ptr<Action>>> mActions;

    Receiver *mReceiver{nullptr};

    enum class State {
        Normal,
        Suppressed,
        Performance,
        Replay,
    } mState{State::Normal};
    uint32 mPerformanceNesting{0};

    bool mUIAllowed{true};
};

struct DATA_EXPORT Root::Context {
    Context(Root&);
    ~Context();

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    void undo();
    void redo();

    [[nodiscard]] bool canUndo() const;
    [[nodiscard]] bool canRedo() const;

private:
    Root& mRoot;
};

struct DATA_EXPORT Root::Receiver {
    virtual ~Receiver();

protected:
    void attach(Root& root) {
        root.attachReciever(*this);
    }

    void detach() {
        pRoot->detachReceiver(*this);
    }

    /**
     * Listener is being detached from the root.
     */
    virtual void onDetach() {};

    /**
     * Root now has/no longer has actions to undo.
     */
    virtual void onCanUndo(bool) {};

    /**
     * Root now has/no longer has actions to redo.
     */
    virtual void onCanRedo(bool) {};

    friend Root;
    Root *pRoot{nullptr};
};

} // namespace data

