/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <Item_p.h>
#include <geometry_helpers_p.h>
#include <string_p.h>
#include <Object_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class Item_wrapper : public ::KDDockWidgets::Core::Item
{
public:
    ~Item_wrapper();
    virtual bool checkSanity();
    virtual bool checkSanity_nocallback();
    virtual void dumpLayout(int level = 0, bool printSeparators = true);
    virtual void dumpLayout_nocallback(int level = 0, bool printSeparators = true);
    KDDockWidgets::Rect geometry() const;
    int height() const;
    virtual bool inSetSize() const;
    virtual bool inSetSize_nocallback() const;
    bool isBeingInserted() const;
    bool isContainer() const;
    bool isMDI() const;
    bool isPlaceholder() const;
    bool isRoot() const;
    virtual bool isVisible(bool excludeBeingInserted = false) const;
    virtual bool isVisible_nocallback(bool excludeBeingInserted = false) const;
    KDDockWidgets::Point mapFromParent(KDDockWidgets::Point arg__1) const;
    KDDockWidgets::Point mapFromRoot(KDDockWidgets::Point arg__1) const;
    KDDockWidgets::Rect mapFromRoot(KDDockWidgets::Rect arg__1) const;
    KDDockWidgets::Point mapToRoot(KDDockWidgets::Point arg__1) const;
    KDDockWidgets::Rect mapToRoot(KDDockWidgets::Rect arg__1) const;
    virtual KDDockWidgets::Size maxSizeHint() const;
    virtual KDDockWidgets::Size maxSizeHint_nocallback() const;
    virtual KDDockWidgets::Size minSize() const;
    virtual KDDockWidgets::Size minSize_nocallback() const;
    KDDockWidgets::Size missingSize() const;
    KDDockWidgets::Core::Item *outermostNeighbor(KDDockWidgets::Location arg__1, bool visibleOnly = true) const;
    KDDockWidgets::Point pos() const;
    KDDockWidgets::Rect rect() const;
    void ref();
    int refCount() const;
    void requestResize(int left, int top, int right, int bottom);
    void setBeingInserted(bool arg__1);
    void setGeometry(KDDockWidgets::Rect rect);
    virtual void setGeometry_recursive(KDDockWidgets::Rect rect);
    virtual void setGeometry_recursive_nocallback(KDDockWidgets::Rect rect);
    virtual void setIsVisible(bool arg__1);
    virtual void setIsVisible_nocallback(bool arg__1);
    void setMaxSizeHint(KDDockWidgets::Size arg__1);
    void setMinSize(KDDockWidgets::Size arg__1);
    void setPos(KDDockWidgets::Point arg__1);
    void setSize(KDDockWidgets::Size arg__1);
    KDDockWidgets::Size size() const;
    void turnIntoPlaceholder();
    void unref();
    virtual void updateWidgetGeometries();
    virtual void updateWidgetGeometries_nocallback();
    virtual int visibleCount_recursive() const;
    virtual int visibleCount_recursive_nocallback() const;
    int width() const;
    int x() const;
    int y() const;
    typedef bool (*Callback_checkSanity)(void *);
    Callback_checkSanity m_checkSanityCallback = nullptr;
    typedef void (*Callback_dumpLayout)(void *, int level, bool printSeparators);
    Callback_dumpLayout m_dumpLayoutCallback = nullptr;
    typedef bool (*Callback_inSetSize)(void *);
    Callback_inSetSize m_inSetSizeCallback = nullptr;
    typedef bool (*Callback_isVisible)(void *, bool excludeBeingInserted);
    Callback_isVisible m_isVisibleCallback = nullptr;
    typedef KDDockWidgets::Size *(*Callback_maxSizeHint)(void *);
    Callback_maxSizeHint m_maxSizeHintCallback = nullptr;
    typedef KDDockWidgets::Size *(*Callback_minSize)(void *);
    Callback_minSize m_minSizeCallback = nullptr;
    typedef void (*Callback_setGeometry_recursive)(void *, KDDockWidgets::Rect *rect);
    Callback_setGeometry_recursive m_setGeometry_recursiveCallback = nullptr;
    typedef void (*Callback_setIsVisible)(void *, bool arg__1);
    Callback_setIsVisible m_setIsVisibleCallback = nullptr;
    typedef void (*Callback_updateWidgetGeometries)(void *);
    Callback_updateWidgetGeometries m_updateWidgetGeometriesCallback = nullptr;
    typedef int (*Callback_visibleCount_recursive)(void *);
    Callback_visibleCount_recursive m_visibleCount_recursiveCallback = nullptr;
};
}
extern "C" {
// KDDockWidgets::Core::Item::checkSanity()
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item__checkSanity(void *thisObj);
// KDDockWidgets::Core::Item::dumpLayout(int level, bool printSeparators)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__dumpLayout_int_bool(void *thisObj, int level, bool printSeparators);
// KDDockWidgets::Core::Item::geometry() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__geometry(void *thisObj);
// KDDockWidgets::Core::Item::height() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Item__height(void *thisObj);
// KDDockWidgets::Core::Item::inSetSize() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item__inSetSize(void *thisObj);
// KDDockWidgets::Core::Item::isBeingInserted() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item__isBeingInserted(void *thisObj);
// KDDockWidgets::Core::Item::isContainer() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item__isContainer(void *thisObj);
// KDDockWidgets::Core::Item::isMDI() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item__isMDI(void *thisObj);
// KDDockWidgets::Core::Item::isPlaceholder() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item__isPlaceholder(void *thisObj);
// KDDockWidgets::Core::Item::isRoot() const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item__isRoot(void *thisObj);
// KDDockWidgets::Core::Item::isVisible(bool excludeBeingInserted) const
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item__isVisible_bool(void *thisObj, bool excludeBeingInserted);
// KDDockWidgets::Core::Item::mapFromParent(KDDockWidgets::Point arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__mapFromParent_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::mapFromRoot(KDDockWidgets::Point arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__mapFromRoot_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::mapFromRoot(KDDockWidgets::Rect arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__mapFromRoot_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::mapToRoot(KDDockWidgets::Point arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__mapToRoot_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::mapToRoot(KDDockWidgets::Rect arg__1) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__mapToRoot_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::maxSizeHint() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__maxSizeHint(void *thisObj);
// KDDockWidgets::Core::Item::minSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__minSize(void *thisObj);
// KDDockWidgets::Core::Item::missingSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__missingSize(void *thisObj);
// KDDockWidgets::Core::Item::outermostNeighbor(KDDockWidgets::Location arg__1, bool visibleOnly) const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__outermostNeighbor_Location_bool(void *thisObj, int arg__1, bool visibleOnly);
// KDDockWidgets::Core::Item::pos() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__pos(void *thisObj);
// KDDockWidgets::Core::Item::rect() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__rect(void *thisObj);
// KDDockWidgets::Core::Item::ref()
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__ref(void *thisObj);
// KDDockWidgets::Core::Item::refCount() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Item__refCount(void *thisObj);
// KDDockWidgets::Core::Item::requestResize(int left, int top, int right, int bottom)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__requestResize_int_int_int_int(void *thisObj, int left, int top, int right, int bottom);
// KDDockWidgets::Core::Item::setBeingInserted(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__setBeingInserted_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Item::setGeometry(KDDockWidgets::Rect rect)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__setGeometry_Rect(void *thisObj, void *rect_);
// KDDockWidgets::Core::Item::setGeometry_recursive(KDDockWidgets::Rect rect)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__setGeometry_recursive_Rect(void *thisObj, void *rect_);
// KDDockWidgets::Core::Item::setIsVisible(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__setIsVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Item::setMaxSizeHint(KDDockWidgets::Size arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__setMaxSizeHint_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::setMinSize(KDDockWidgets::Size arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__setMinSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::setPos(KDDockWidgets::Point arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__setPos_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::setSize(KDDockWidgets::Size arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__setSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::size() const
DOCKS_EXPORT void *c_KDDockWidgets__Core__Item__size(void *thisObj);
// KDDockWidgets::Core::Item::turnIntoPlaceholder()
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__turnIntoPlaceholder(void *thisObj);
// KDDockWidgets::Core::Item::unref()
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__unref(void *thisObj);
// KDDockWidgets::Core::Item::updateWidgetGeometries()
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__updateWidgetGeometries(void *thisObj);
// KDDockWidgets::Core::Item::visibleCount_recursive() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Item__visibleCount_recursive(void *thisObj);
// KDDockWidgets::Core::Item::width() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Item__width(void *thisObj);
// KDDockWidgets::Core::Item::x() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Item__x(void *thisObj);
// KDDockWidgets::Core::Item::y() const
DOCKS_EXPORT int c_KDDockWidgets__Core__Item__y(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__destructor(void *thisObj);
DOCKS_EXPORT int c_static_KDDockWidgets__Core__Item___get_separatorThickness();
DOCKS_EXPORT int c_static_KDDockWidgets__Core__Item___get_layoutSpacing();
DOCKS_EXPORT bool c_static_KDDockWidgets__Core__Item___get_s_silenceSanityChecks();
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item___get_m_isContainer(void *thisObj);
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item___get_m_isSettingGuest(void *thisObj);
DOCKS_EXPORT bool c_KDDockWidgets__Core__Item___get_m_inDtor(void *thisObj);
DOCKS_EXPORT void c_static_KDDockWidgets__Core__Item___set_separatorThickness_int(int separatorThickness_);
DOCKS_EXPORT void c_static_KDDockWidgets__Core__Item___set_layoutSpacing_int(int layoutSpacing_);
DOCKS_EXPORT void c_static_KDDockWidgets__Core__Item___set_s_silenceSanityChecks_bool(bool s_silenceSanityChecks_);
DOCKS_EXPORT void c_KDDockWidgets__Core__Item___set_m_isSettingGuest_bool(void *thisObj, bool m_isSettingGuest_);
DOCKS_EXPORT void c_KDDockWidgets__Core__Item___set_m_inDtor_bool(void *thisObj, bool m_inDtor_);
DOCKS_EXPORT void c_KDDockWidgets__Core__Item__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
DOCKS_EXPORT void c_KDDockWidgets__Core__Item_Finalizer(void *cppObj);
}
