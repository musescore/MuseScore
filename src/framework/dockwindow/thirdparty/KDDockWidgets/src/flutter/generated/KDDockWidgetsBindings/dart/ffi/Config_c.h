/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "kddockwidgets_export.h"
#include <Config.h>
#include <ViewFactory.h>
#include <geometry_helpers_p.h>

namespace KDDockWidgetsBindings_wrappersNS {
class Config_wrapper : public ::KDDockWidgets::Config
{
public:
    ~Config_wrapper();
    KDDockWidgets::Size absoluteWidgetMaxSize() const;
    KDDockWidgets::Size absoluteWidgetMinSize() const;
    double draggedWindowOpacity() const;
    bool dropIndicatorsInhibited() const;
    bool layoutSaverUsesStrictMode() const;
    int layoutSpacing() const;
    int mdiPopupThreshold() const;
    bool onlyProgrammaticDrag() const;
    void printDebug();
    static KDDockWidgets::Config &self();
    int separatorThickness() const;
    void setAbsoluteWidgetMaxSize(KDDockWidgets::Size size);
    void setAbsoluteWidgetMinSize(KDDockWidgets::Size size);
    void setDraggedWindowOpacity(double opacity);
    void setDropIndicatorsInhibited(bool inhibit) const;
    void setLayoutSaverStrictMode(bool arg__1);
    void setLayoutSpacing(int arg__1);
    void setMDIPopupThreshold(int arg__1);
    void setOnlyProgrammaticDrag(bool arg__1);
    void setSeparatorThickness(int value);
    void setStartDragDistance(int arg__1);
    void setTransparencyOnlyOverDropIndicator(bool only);
    void setViewFactory(KDDockWidgets::Core::ViewFactory *arg__1);
    int startDragDistance() const;
    bool transparencyOnlyOverDropIndicator() const;
    KDDockWidgets::Core::ViewFactory *viewFactory() const;
};
}
extern "C" {
// KDDockWidgets::Config::absoluteWidgetMaxSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Config__absoluteWidgetMaxSize(void *thisObj);
// KDDockWidgets::Config::absoluteWidgetMinSize() const
DOCKS_EXPORT void *c_KDDockWidgets__Config__absoluteWidgetMinSize(void *thisObj);
// KDDockWidgets::Config::draggedWindowOpacity() const
DOCKS_EXPORT double c_KDDockWidgets__Config__draggedWindowOpacity(void *thisObj);
// KDDockWidgets::Config::dropIndicatorsInhibited() const
DOCKS_EXPORT bool c_KDDockWidgets__Config__dropIndicatorsInhibited(void *thisObj);
// KDDockWidgets::Config::layoutSaverUsesStrictMode() const
DOCKS_EXPORT bool c_KDDockWidgets__Config__layoutSaverUsesStrictMode(void *thisObj);
// KDDockWidgets::Config::layoutSpacing() const
DOCKS_EXPORT int c_KDDockWidgets__Config__layoutSpacing(void *thisObj);
// KDDockWidgets::Config::mdiPopupThreshold() const
DOCKS_EXPORT int c_KDDockWidgets__Config__mdiPopupThreshold(void *thisObj);
// KDDockWidgets::Config::onlyProgrammaticDrag() const
DOCKS_EXPORT bool c_KDDockWidgets__Config__onlyProgrammaticDrag(void *thisObj);
// KDDockWidgets::Config::printDebug()
DOCKS_EXPORT void c_KDDockWidgets__Config__printDebug(void *thisObj);
// KDDockWidgets::Config::self()
DOCKS_EXPORT void *c_static_KDDockWidgets__Config__self();
// KDDockWidgets::Config::separatorThickness() const
DOCKS_EXPORT int c_KDDockWidgets__Config__separatorThickness(void *thisObj);
// KDDockWidgets::Config::setAbsoluteWidgetMaxSize(KDDockWidgets::Size size)
DOCKS_EXPORT void c_KDDockWidgets__Config__setAbsoluteWidgetMaxSize_Size(void *thisObj, void *size_);
// KDDockWidgets::Config::setAbsoluteWidgetMinSize(KDDockWidgets::Size size)
DOCKS_EXPORT void c_KDDockWidgets__Config__setAbsoluteWidgetMinSize_Size(void *thisObj, void *size_);
// KDDockWidgets::Config::setDraggedWindowOpacity(double opacity)
DOCKS_EXPORT void c_KDDockWidgets__Config__setDraggedWindowOpacity_double(void *thisObj, double opacity);
// KDDockWidgets::Config::setDropIndicatorsInhibited(bool inhibit) const
DOCKS_EXPORT void c_KDDockWidgets__Config__setDropIndicatorsInhibited_bool(void *thisObj, bool inhibit);
// KDDockWidgets::Config::setLayoutSaverStrictMode(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Config__setLayoutSaverStrictMode_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Config::setLayoutSpacing(int arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Config__setLayoutSpacing_int(void *thisObj, int arg__1);
// KDDockWidgets::Config::setMDIPopupThreshold(int arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Config__setMDIPopupThreshold_int(void *thisObj, int arg__1);
// KDDockWidgets::Config::setOnlyProgrammaticDrag(bool arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Config__setOnlyProgrammaticDrag_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Config::setSeparatorThickness(int value)
DOCKS_EXPORT void c_KDDockWidgets__Config__setSeparatorThickness_int(void *thisObj, int value);
// KDDockWidgets::Config::setStartDragDistance(int arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Config__setStartDragDistance_int(void *thisObj, int arg__1);
// KDDockWidgets::Config::setTransparencyOnlyOverDropIndicator(bool only)
DOCKS_EXPORT void c_KDDockWidgets__Config__setTransparencyOnlyOverDropIndicator_bool(void *thisObj, bool only);
// KDDockWidgets::Config::setViewFactory(KDDockWidgets::Core::ViewFactory * arg__1)
DOCKS_EXPORT void c_KDDockWidgets__Config__setViewFactory_ViewFactory(void *thisObj, void *arg__1_);
// KDDockWidgets::Config::startDragDistance() const
DOCKS_EXPORT int c_KDDockWidgets__Config__startDragDistance(void *thisObj);
// KDDockWidgets::Config::transparencyOnlyOverDropIndicator() const
DOCKS_EXPORT bool c_KDDockWidgets__Config__transparencyOnlyOverDropIndicator(void *thisObj);
// KDDockWidgets::Config::viewFactory() const
DOCKS_EXPORT void *c_KDDockWidgets__Config__viewFactory(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Config__destructor(void *thisObj);
DOCKS_EXPORT void c_KDDockWidgets__Config_Finalizer(void *cppObj);
}
