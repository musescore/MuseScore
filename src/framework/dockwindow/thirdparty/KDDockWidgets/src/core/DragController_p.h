/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#ifndef KD_DRAGCONTROLLER_P_H
#define KD_DRAGCONTROLLER_P_H

#include "kddockwidgets/docks_export.h"

#include "WindowBeingDragged_p.h"
#include "core/EventFilterInterface.h"
#include "kddockwidgets/QtCompat_p.h"

#include <kdbindings/signal.h>

#include <memory>

#ifdef KDDW_FRONTEND_QT_WINDOWS
#include <QTimer>
#endif

class TestQtWidgets;

namespace KDDockWidgets {

namespace Core {

class StateBase;
class StatePreDrag;
class StateDragging;
class StateNone;
class StateInternalMDIDragging;
class FallbackMouseGrabber;
class MinimalStateMachine;
class DropArea;
class Draggable;

class State : public Core::Object
{
    Q_OBJECT
public:
    explicit State(MinimalStateMachine *parent);
    ~State() override;

    template<typename Signal>
    void addTransition(Signal &, State *dest);

    bool isCurrentState() const;

    virtual void onEntry() = 0;
    virtual void onExit()
    {
    }

private:
    MinimalStateMachine *const m_machine;
};

class MinimalStateMachine : public Core::Object
{
    Q_OBJECT
public:
    explicit MinimalStateMachine(Core::Object *parent = nullptr);

    State *currentState() const;
    void setCurrentState(State *);

    KDBindings::Signal<> currentStateChanged;

private:
    State *m_currentState = nullptr;
};

class DOCKS_EXPORT_FOR_UNIT_TESTS DragController : public MinimalStateMachine, public EventFilterInterface
{
    Q_OBJECT
public:
    enum State {
        State_None = 0,
        State_PreDrag,
        State_Dragging
    };
    Q_ENUM(State)

    static DragController *instance();

    // Registers something that wants to be able to be dragged
    void registerDraggable(Draggable *);
    void unregisterDraggable(Draggable *);

    bool isDragging() const;
    bool isInNonClientDrag() const;
    bool isInClientDrag() const;
    bool isInProgrammaticDrag() const;
    bool isIdle() const;

    void grabMouseFor(View *);
    void releaseMouse(View *);

    FloatingWindow *floatingWindowBeingDragged() const;

    /// @brief Returns the current drop area under the mouse
    DropArea *dropAreaUnderCursor() const;

    ///@brief Returns the window being dragged
    WindowBeingDragged *windowBeingDragged() const;

    /// Experimental, internal, not for general use.
    void enableFallbackMouseGrabber();

    // Returns the active state
    StateBase *activeState() const;

    /// Returns the current drop location
    /// The user needs to be dragging a window and be over a drop indicator, otherwise DropLocation_None is returned
    DropLocation currentDropLocation() const;

    /// Triggers a drag start without user interaction
    ///
    /// This method is unneeded for almost everyone as state machine changes states based
    /// on mouse events from user interaction. It's currently used for users who are developing for touch interfaces
    /// and want other means of starting a drag.
    ///
    /// Do not rely on it. We're still trying to understand the touch use case.
    /// @internal
    /// @param draggable The draggable we want to move (a title-bar, tab-bar or floating window)
    bool programmaticStartDrag(Draggable *draggable, Point globalPos, Point offset);
    void programmaticStopDrag();

    KDBindings::Signal<> mousePressed;
    KDBindings::Signal<> manhattanLengthMove;
    KDBindings::Signal<> manhattanLengthMoveMDI;
    KDBindings::Signal<> mdiPopOut;
    KDBindings::Signal<> dragCanceled;
    KDBindings::Signal<> dropped;
    KDBindings::Signal<> isDraggingChanged;

    /// Wayland only
    bool isInQDrag() const;

private:
    friend class StateBase;
    friend class StateNone;
    friend class StatePreDrag;
    friend class StateDragging;
    friend class StateInternalMDIDragging;
    friend class StateDropped;
    friend class StateDraggingWayland;
    friend class ::TestQtWidgets;

    explicit DragController(Core::Object * = nullptr);
    std::shared_ptr<Core::View> qtTopLevelUnderCursor() const;
    Core::Draggable *draggableForView(Core::View *) const;
    bool onDnDEvent(Core::View *, Event *) override;
    bool onMoveEvent(Core::View *) override;
    bool onMouseEvent(Core::View *, MouseEvent *) override;

    Point m_pressPos;
    Point m_offset;

    Vector<Core::Draggable *> m_draggables;
    Core::Draggable *m_draggable = nullptr;
    Core::ViewGuard m_draggableGuard =
        nullptr; // Just so we know if the draggable was destroyed for some reason
    std::unique_ptr<WindowBeingDragged> m_windowBeingDragged;
    Core::DropArea *m_currentDropArea = nullptr;
    FallbackMouseGrabber *m_fallbackMouseGrabber = nullptr;
    StateNone *const m_stateNone;
    StatePreDrag *const m_statePreDrag;
    StateDragging *const m_stateDragging;
    StateInternalMDIDragging *m_stateDraggingMDI = nullptr;
    bool m_nonClientDrag = false; // native title bar drag
    bool m_inQDrag = false; // wayland drag
    bool m_inProgrammaticDrag = false; // via DockWidget::startDrag()
};

class StateBase : public State
{
    Q_OBJECT
public:
    explicit StateBase(DragController *parent);
    ~StateBase();

    // Not using QEvent here, to abstract platform differences regarding production of such events
    virtual bool handleMouseButtonPress(Core::Draggable * /*receiver*/, Point /*globalPos*/,
                                        Point /*pos*/)
    {
        return false;
    }
    virtual bool handleMouseMove(Point /*globalPos*/)
    {
        return false;
    }
    virtual bool handleMouseButtonRelease(Point /*globalPos*/)
    {
        return false;
    }
    virtual bool handleMouseDoubleClick()
    {
        return false;
    }

    // Only interesting for Wayland
    virtual bool handleDragEnter(DragMoveEvent *, DropArea *)
    {
        return false;
    }
    virtual bool handleDragLeave(DropArea *)
    {
        return false;
    }
    virtual bool handleDragMove(DragMoveEvent *, DropArea *)
    {
        return false;
    }
    virtual bool handleDrop(DropEvent *, DropArea *)
    {
        return false;
    }

    // Returns whether this is the current state
    bool isActiveState() const;

    DragController *const q;
};

class StateNone : public StateBase
{
    Q_OBJECT
public:
    explicit StateNone(DragController *parent);
    ~StateNone() override;
    void onEntry() override;
    bool handleMouseButtonPress(Draggable *draggable, Point globalPos, Point pos) override;
};

class StatePreDrag : public StateBase
{
    Q_OBJECT
public:
    explicit StatePreDrag(DragController *parent);
    ~StatePreDrag() override;
    void onEntry() override;
    bool handleMouseMove(Point globalPos) override;
    bool handleMouseButtonRelease(Point) override;
    bool handleMouseDoubleClick() override;
};

// Used on all platforms except Wayland. @see StateDraggingWayland
class StateDragging : public StateBase
{
    Q_OBJECT
public:
    explicit StateDragging(DragController *parent);
    ~StateDragging() override;
    void onEntry() override;
    void onExit() override;
    bool handleMouseButtonRelease(Point globalPos) override;
    bool handleMouseMove(Point globalPos) override;
    bool handleMouseDoubleClick() override;

#if defined(KDDW_FRONTEND_QT_WINDOWS)
private:
    QTimer m_maybeCancelDrag;
#endif
};


/// @brief State when we're moving an MDI dock widget around the main window
/// without it becoming floating
class StateInternalMDIDragging : public StateBase
{
    Q_OBJECT
public:
    explicit StateInternalMDIDragging(DragController *parent);
    ~StateInternalMDIDragging() override;
    void onEntry() override;
    bool handleMouseButtonRelease(Point globalPos) override;
    bool handleMouseMove(Point globalPos) override;
    bool handleMouseDoubleClick() override;
};

}

}

#endif
