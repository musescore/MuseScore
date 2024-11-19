/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <Window_p.h>
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
namespace KDDWBindingsFlutter {
class Window_wrapper : public ::KDDockWidgets::flutter::Window
{
public:
    ~Window_wrapper();
    virtual void destroy();
    virtual void destroy_nocallback();
    virtual KDDockWidgets::Rect frameGeometry() const;
    virtual KDDockWidgets::Rect frameGeometry_nocallback() const;
    virtual KDDockWidgets::Point fromNativePixels(KDDockWidgets::Point arg__1) const;
    virtual KDDockWidgets::Point fromNativePixels_nocallback(KDDockWidgets::Point arg__1) const;
    virtual KDDockWidgets::Rect geometry() const;
    virtual KDDockWidgets::Rect geometry_nocallback() const;
    virtual bool isActive() const;
    virtual bool isActive_nocallback() const;
    virtual bool isFullScreen() const;
    virtual bool isFullScreen_nocallback() const;
    virtual bool isVisible() const;
    virtual bool isVisible_nocallback() const;
    virtual KDDockWidgets::Point mapFromGlobal(KDDockWidgets::Point globalPos) const;
    virtual KDDockWidgets::Point mapFromGlobal_nocallback(KDDockWidgets::Point globalPos) const;
    virtual KDDockWidgets::Point mapToGlobal(KDDockWidgets::Point localPos) const;
    virtual KDDockWidgets::Point mapToGlobal_nocallback(KDDockWidgets::Point localPos) const;
    virtual KDDockWidgets::Size maxSize() const;
    virtual KDDockWidgets::Size maxSize_nocallback() const;
    virtual KDDockWidgets::Size minSize() const;
    virtual KDDockWidgets::Size minSize_nocallback() const;
    virtual void resize(int width, int height);
    virtual void resize_nocallback(int width, int height);
    virtual void setFramePosition(KDDockWidgets::Point targetPos);
    virtual void setFramePosition_nocallback(KDDockWidgets::Point targetPos);
    virtual void setGeometry(KDDockWidgets::Rect arg__1);
    virtual void setGeometry_nocallback(KDDockWidgets::Rect arg__1);
    virtual void setVisible(bool arg__1);
    virtual void setVisible_nocallback(bool arg__1);
    virtual bool supportsHonouringLayoutMinSize() const;
    virtual bool supportsHonouringLayoutMinSize_nocallback() const;
    typedef void (*Callback_destroy)(void *);
    Callback_destroy m_destroyCallback = nullptr;
    typedef KDDockWidgets::Rect *(*Callback_frameGeometry)(void *);
    Callback_frameGeometry m_frameGeometryCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_fromNativePixels)(void *, KDDockWidgets::Point *arg__1);
    Callback_fromNativePixels m_fromNativePixelsCallback = nullptr;
    typedef KDDockWidgets::Rect *(*Callback_geometry)(void *);
    Callback_geometry m_geometryCallback = nullptr;
    typedef bool (*Callback_isActive)(void *);
    Callback_isActive m_isActiveCallback = nullptr;
    typedef bool (*Callback_isFullScreen)(void *);
    Callback_isFullScreen m_isFullScreenCallback = nullptr;
    typedef bool (*Callback_isVisible)(void *);
    Callback_isVisible m_isVisibleCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_mapFromGlobal)(void *, KDDockWidgets::Point *globalPos);
    Callback_mapFromGlobal m_mapFromGlobalCallback = nullptr;
    typedef KDDockWidgets::Point *(*Callback_mapToGlobal)(void *, KDDockWidgets::Point *localPos);
    Callback_mapToGlobal m_mapToGlobalCallback = nullptr;
    typedef KDDockWidgets::Size *(*Callback_maxSize)(void *);
    Callback_maxSize m_maxSizeCallback = nullptr;
    typedef KDDockWidgets::Size *(*Callback_minSize)(void *);
    Callback_minSize m_minSizeCallback = nullptr;
    typedef void (*Callback_resize)(void *, int width, int height);
    Callback_resize m_resizeCallback = nullptr;
    typedef void (*Callback_setFramePosition)(void *, KDDockWidgets::Point *targetPos);
    Callback_setFramePosition m_setFramePositionCallback = nullptr;
    typedef void (*Callback_setGeometry)(void *, KDDockWidgets::Rect *arg__1);
    Callback_setGeometry m_setGeometryCallback = nullptr;
    typedef void (*Callback_setVisible)(void *, bool arg__1);
    Callback_setVisible m_setVisibleCallback = nullptr;
    typedef bool (*Callback_supportsHonouringLayoutMinSize)(void *);
    Callback_supportsHonouringLayoutMinSize m_supportsHonouringLayoutMinSizeCallback = nullptr;
};
}
}
extern "C" {
// KDDockWidgets::flutter::Window::destroy()
DOCKS_EXPORT void c_KDDockWidgets__flutter__Window__destroy(void *thisObj);
// KDDockWidgets::flutter::Window::frameGeometry() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Window__frameGeometry(void *thisObj);
// KDDockWidgets::flutter::Window::fromNativePixels(KDDockWidgets::Point arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Window__fromNativePixels_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Window::geometry() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Window__geometry(void *thisObj);
// KDDockWidgets::flutter::Window::isActive() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Window__isActive(void *thisObj);
// KDDockWidgets::flutter::Window::isFullScreen() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Window__isFullScreen(void *thisObj);
// KDDockWidgets::flutter::Window::isVisible() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Window__isVisible(void *thisObj);
// KDDockWidgets::flutter::Window::mapFromGlobal(KDDockWidgets::Point globalPos) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Window__mapFromGlobal_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::flutter::Window::mapToGlobal(KDDockWidgets::Point localPos) const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Window__mapToGlobal_Point(void *thisObj, void *localPos_);
// KDDockWidgets::flutter::Window::maxSize() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Window__maxSize(void *thisObj);
// KDDockWidgets::flutter::Window::minSize() const
DOCKS_EXPORT void *c_KDDockWidgets__flutter__Window__minSize(void *thisObj);
// KDDockWidgets::flutter::Window::resize(int width, int height)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Window__resize_int_int(void *thisObj, int width, int height);
// KDDockWidgets::flutter::Window::setFramePosition(KDDockWidgets::Point targetPos)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Window__setFramePosition_Point(void *thisObj, void *targetPos_);
// KDDockWidgets::flutter::Window::setGeometry(KDDockWidgets::Rect arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Window__setGeometry_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Window::setVisible(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__flutter__Window__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::flutter::Window::supportsHonouringLayoutMinSize() const
DOCKS_EXPORT bool c_KDDockWidgets__flutter__Window__supportsHonouringLayoutMinSize(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__Window__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__flutter__Window__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__flutter__Window_Finalizer(void *cppObj);
}
