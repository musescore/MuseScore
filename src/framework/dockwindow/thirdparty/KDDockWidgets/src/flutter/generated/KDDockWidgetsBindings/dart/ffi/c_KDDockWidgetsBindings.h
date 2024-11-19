/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include <stdbool.h>

void c_Qt_Finalizer(void *cppObj); // QString::QString()
void *c_QString__constructor();
// QString::QString(const char * str)
void *c_QString__constructor_char(const char *str);
// QString::data() const
const char *c_QString__data(void *thisObj);
// QString::fromUtf8(const char * str)
void *c_static_QString__fromUtf8_char(const char *str);
// QString::isEmpty() const
bool c_QString__isEmpty(void *thisObj);
void c_QString__destructor(void *thisObj);
void c_QString_Finalizer(void *cppObj); // KDDockWidgets::fuzzyCompare(double a, double b, double epsilon)
bool c_static_KDDockWidgets__fuzzyCompare_double_double_double(double a, double b, double epsilon);
// KDDockWidgets::initFrontend(KDDockWidgets::FrontendType arg__1)
void c_static_KDDockWidgets__initFrontend_FrontendType(int arg__1);
// KDDockWidgets::spdlogLoggerName()
const char *c_static_KDDockWidgets__spdlogLoggerName();
void c_KDDockWidgets_Finalizer(void *cppObj); // KDDockWidgets::flutter::asView_flutter(KDDockWidgets::Core::Controller * controller)
void *c_static_KDDockWidgets__flutter__asView_flutter_Controller(void *controller_);
// KDDockWidgets::flutter::asView_flutter(KDDockWidgets::Core::View * view)
void *c_static_KDDockWidgets__flutter__asView_flutter_View(void *view_);
void c_KDDockWidgets__flutter_Finalizer(void *cppObj); // KDDockWidgets::flutter::Window::destroy()
void c_KDDockWidgets__flutter__Window__destroy(void *thisObj);
// KDDockWidgets::flutter::Window::frameGeometry() const
void *c_KDDockWidgets__flutter__Window__frameGeometry(void *thisObj);
// KDDockWidgets::flutter::Window::fromNativePixels(KDDockWidgets::Point arg__1) const
void *c_KDDockWidgets__flutter__Window__fromNativePixels_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Window::geometry() const
void *c_KDDockWidgets__flutter__Window__geometry(void *thisObj);
// KDDockWidgets::flutter::Window::isActive() const
bool c_KDDockWidgets__flutter__Window__isActive(void *thisObj);
// KDDockWidgets::flutter::Window::isFullScreen() const
bool c_KDDockWidgets__flutter__Window__isFullScreen(void *thisObj);
// KDDockWidgets::flutter::Window::isVisible() const
bool c_KDDockWidgets__flutter__Window__isVisible(void *thisObj);
// KDDockWidgets::flutter::Window::mapFromGlobal(KDDockWidgets::Point globalPos) const
void *c_KDDockWidgets__flutter__Window__mapFromGlobal_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::flutter::Window::mapToGlobal(KDDockWidgets::Point localPos) const
void *c_KDDockWidgets__flutter__Window__mapToGlobal_Point(void *thisObj, void *localPos_);
// KDDockWidgets::flutter::Window::maxSize() const
void *c_KDDockWidgets__flutter__Window__maxSize(void *thisObj);
// KDDockWidgets::flutter::Window::minSize() const
void *c_KDDockWidgets__flutter__Window__minSize(void *thisObj);
// KDDockWidgets::flutter::Window::resize(int width, int height)
void c_KDDockWidgets__flutter__Window__resize_int_int(void *thisObj, int width, int height);
// KDDockWidgets::flutter::Window::setFramePosition(KDDockWidgets::Point targetPos)
void c_KDDockWidgets__flutter__Window__setFramePosition_Point(void *thisObj, void *targetPos_);
// KDDockWidgets::flutter::Window::setGeometry(KDDockWidgets::Rect arg__1)
void c_KDDockWidgets__flutter__Window__setGeometry_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Window::setVisible(bool arg__1)
void c_KDDockWidgets__flutter__Window__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::flutter::Window::supportsHonouringLayoutMinSize() const
bool c_KDDockWidgets__flutter__Window__supportsHonouringLayoutMinSize(void *thisObj);
void c_KDDockWidgets__flutter__Window__destructor(void *thisObj);
void c_KDDockWidgets__flutter__Window__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__Window_Finalizer(void *cppObj); // KDDockWidgets::Size::Size()
void *c_KDDockWidgets__Size__constructor();
// KDDockWidgets::Size::Size(int width, int height)
void *c_KDDockWidgets__Size__constructor_int_int(int width, int height);
// KDDockWidgets::Size::boundedTo(KDDockWidgets::Size sz) const
void *c_KDDockWidgets__Size__boundedTo_Size(void *thisObj, void *sz_);
// KDDockWidgets::Size::expandedTo(KDDockWidgets::Size sz) const
void *c_KDDockWidgets__Size__expandedTo_Size(void *thisObj, void *sz_);
// KDDockWidgets::Size::height() const
int c_KDDockWidgets__Size__height(void *thisObj);
// KDDockWidgets::Size::isEmpty() const
bool c_KDDockWidgets__Size__isEmpty(void *thisObj);
// KDDockWidgets::Size::isNull() const
bool c_KDDockWidgets__Size__isNull(void *thisObj);
// KDDockWidgets::Size::isValid() const
bool c_KDDockWidgets__Size__isValid(void *thisObj);
// KDDockWidgets::Size::setHeight(int h)
void c_KDDockWidgets__Size__setHeight_int(void *thisObj, int h);
// KDDockWidgets::Size::setWidth(int w)
void c_KDDockWidgets__Size__setWidth_int(void *thisObj, int w);
// KDDockWidgets::Size::width() const
int c_KDDockWidgets__Size__width(void *thisObj);
void c_KDDockWidgets__Size__destructor(void *thisObj);
void c_KDDockWidgets__Size_Finalizer(void *cppObj); // KDDockWidgets::Rect::Rect()
void *c_KDDockWidgets__Rect__constructor();
// KDDockWidgets::Rect::Rect(KDDockWidgets::Point pos, KDDockWidgets::Size size)
void *c_KDDockWidgets__Rect__constructor_Point_Size(void *pos_, void *size_);
// KDDockWidgets::Rect::Rect(int x, int y, int width, int height)
void *c_KDDockWidgets__Rect__constructor_int_int_int_int(int x, int y, int width, int height);
// KDDockWidgets::Rect::adjust(int l, int t, int r, int b)
void c_KDDockWidgets__Rect__adjust_int_int_int_int(void *thisObj, int l, int t, int r, int b);
// KDDockWidgets::Rect::adjusted(int l, int t, int r, int b) const
void *c_KDDockWidgets__Rect__adjusted_int_int_int_int(void *thisObj, int l, int t, int r, int b);
// KDDockWidgets::Rect::bottom() const
int c_KDDockWidgets__Rect__bottom(void *thisObj);
// KDDockWidgets::Rect::bottomLeft() const
void *c_KDDockWidgets__Rect__bottomLeft(void *thisObj);
// KDDockWidgets::Rect::bottomRight() const
void *c_KDDockWidgets__Rect__bottomRight(void *thisObj);
// KDDockWidgets::Rect::center() const
void *c_KDDockWidgets__Rect__center(void *thisObj);
// KDDockWidgets::Rect::contains(KDDockWidgets::Point pt) const
bool c_KDDockWidgets__Rect__contains_Point(void *thisObj, void *pt_);
// KDDockWidgets::Rect::contains(KDDockWidgets::Rect other) const
bool c_KDDockWidgets__Rect__contains_Rect(void *thisObj, void *other_);
// KDDockWidgets::Rect::height() const
int c_KDDockWidgets__Rect__height(void *thisObj);
// KDDockWidgets::Rect::intersected(KDDockWidgets::Rect other) const
void *c_KDDockWidgets__Rect__intersected_Rect(void *thisObj, void *other_);
// KDDockWidgets::Rect::intersects(KDDockWidgets::Rect other) const
bool c_KDDockWidgets__Rect__intersects_Rect(void *thisObj, void *other_);
// KDDockWidgets::Rect::isEmpty() const
bool c_KDDockWidgets__Rect__isEmpty(void *thisObj);
// KDDockWidgets::Rect::isNull() const
bool c_KDDockWidgets__Rect__isNull(void *thisObj);
// KDDockWidgets::Rect::isValid() const
bool c_KDDockWidgets__Rect__isValid(void *thisObj);
// KDDockWidgets::Rect::left() const
int c_KDDockWidgets__Rect__left(void *thisObj);
// KDDockWidgets::Rect::marginsAdded(KDDockWidgets::Margins m) const
void *c_KDDockWidgets__Rect__marginsAdded_Margins(void *thisObj, void *m_);
// KDDockWidgets::Rect::moveBottom(int b)
void c_KDDockWidgets__Rect__moveBottom_int(void *thisObj, int b);
// KDDockWidgets::Rect::moveCenter(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__moveCenter_Point(void *thisObj, void *pt_);
// KDDockWidgets::Rect::moveLeft(int x)
void c_KDDockWidgets__Rect__moveLeft_int(void *thisObj, int x);
// KDDockWidgets::Rect::moveRight(int r)
void c_KDDockWidgets__Rect__moveRight_int(void *thisObj, int r);
// KDDockWidgets::Rect::moveTo(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__moveTo_Point(void *thisObj, void *pt_);
// KDDockWidgets::Rect::moveTo(int x, int y)
void c_KDDockWidgets__Rect__moveTo_int_int(void *thisObj, int x, int y);
// KDDockWidgets::Rect::moveTop(int y)
void c_KDDockWidgets__Rect__moveTop_int(void *thisObj, int y);
// KDDockWidgets::Rect::moveTopLeft(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__moveTopLeft_Point(void *thisObj, void *pt_);
// KDDockWidgets::Rect::pos() const
void *c_KDDockWidgets__Rect__pos(void *thisObj);
// KDDockWidgets::Rect::right() const
int c_KDDockWidgets__Rect__right(void *thisObj);
// KDDockWidgets::Rect::setBottom(int b)
void c_KDDockWidgets__Rect__setBottom_int(void *thisObj, int b);
// KDDockWidgets::Rect::setHeight(int h)
void c_KDDockWidgets__Rect__setHeight_int(void *thisObj, int h);
// KDDockWidgets::Rect::setLeft(int x)
void c_KDDockWidgets__Rect__setLeft_int(void *thisObj, int x);
// KDDockWidgets::Rect::setRight(int r)
void c_KDDockWidgets__Rect__setRight_int(void *thisObj, int r);
// KDDockWidgets::Rect::setSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__Rect__setSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::Rect::setTop(int y)
void c_KDDockWidgets__Rect__setTop_int(void *thisObj, int y);
// KDDockWidgets::Rect::setTopLeft(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__setTopLeft_Point(void *thisObj, void *pt_);
// KDDockWidgets::Rect::setWidth(int w)
void c_KDDockWidgets__Rect__setWidth_int(void *thisObj, int w);
// KDDockWidgets::Rect::setX(int x)
void c_KDDockWidgets__Rect__setX_int(void *thisObj, int x);
// KDDockWidgets::Rect::setY(int y)
void c_KDDockWidgets__Rect__setY_int(void *thisObj, int y);
// KDDockWidgets::Rect::size() const
void *c_KDDockWidgets__Rect__size(void *thisObj);
// KDDockWidgets::Rect::top() const
int c_KDDockWidgets__Rect__top(void *thisObj);
// KDDockWidgets::Rect::topLeft() const
void *c_KDDockWidgets__Rect__topLeft(void *thisObj);
// KDDockWidgets::Rect::topRight() const
void *c_KDDockWidgets__Rect__topRight(void *thisObj);
// KDDockWidgets::Rect::translate(KDDockWidgets::Point pt)
void c_KDDockWidgets__Rect__translate_Point(void *thisObj, void *pt_);
// KDDockWidgets::Rect::width() const
int c_KDDockWidgets__Rect__width(void *thisObj);
// KDDockWidgets::Rect::x() const
int c_KDDockWidgets__Rect__x(void *thisObj);
// KDDockWidgets::Rect::y() const
int c_KDDockWidgets__Rect__y(void *thisObj);
void c_KDDockWidgets__Rect__destructor(void *thisObj);
void c_KDDockWidgets__Rect_Finalizer(void *cppObj); // KDDockWidgets::Point::Point()
void *c_KDDockWidgets__Point__constructor();
// KDDockWidgets::Point::Point(int x, int y)
void *c_KDDockWidgets__Point__constructor_int_int(int x, int y);
// KDDockWidgets::Point::isNull() const
bool c_KDDockWidgets__Point__isNull(void *thisObj);
// KDDockWidgets::Point::manhattanLength() const
int c_KDDockWidgets__Point__manhattanLength(void *thisObj);
// KDDockWidgets::Point::setX(int x)
void c_KDDockWidgets__Point__setX_int(void *thisObj, int x);
// KDDockWidgets::Point::setY(int y)
void c_KDDockWidgets__Point__setY_int(void *thisObj, int y);
// KDDockWidgets::Point::x() const
int c_KDDockWidgets__Point__x(void *thisObj);
// KDDockWidgets::Point::y() const
int c_KDDockWidgets__Point__y(void *thisObj);
void c_KDDockWidgets__Point__destructor(void *thisObj);
void c_KDDockWidgets__Point_Finalizer(void *cppObj); // KDDockWidgets::Margins::Margins()
void *c_KDDockWidgets__Margins__constructor();
// KDDockWidgets::Margins::Margins(int l, int t, int r, int b)
void *c_KDDockWidgets__Margins__constructor_int_int_int_int(int l, int t, int r, int b);
// KDDockWidgets::Margins::bottom() const
int c_KDDockWidgets__Margins__bottom(void *thisObj);
// KDDockWidgets::Margins::left() const
int c_KDDockWidgets__Margins__left(void *thisObj);
// KDDockWidgets::Margins::right() const
int c_KDDockWidgets__Margins__right(void *thisObj);
// KDDockWidgets::Margins::top() const
int c_KDDockWidgets__Margins__top(void *thisObj);
void c_KDDockWidgets__Margins__destructor(void *thisObj);
void c_KDDockWidgets__Margins_Finalizer(void *cppObj); // KDDockWidgets::LayoutSaver::LayoutSaver()
void *c_KDDockWidgets__LayoutSaver__constructor();
// KDDockWidgets::LayoutSaver::restoreFromFile(const QString & jsonFilename)
bool c_KDDockWidgets__LayoutSaver__restoreFromFile_QString(void *thisObj, const char *jsonFilename_);
// KDDockWidgets::LayoutSaver::restoreInProgress()
bool c_static_KDDockWidgets__LayoutSaver__restoreInProgress();
// KDDockWidgets::LayoutSaver::saveToFile(const QString & jsonFilename)
bool c_KDDockWidgets__LayoutSaver__saveToFile_QString(void *thisObj, const char *jsonFilename_);
void c_KDDockWidgets__LayoutSaver__destructor(void *thisObj);
void c_KDDockWidgets__LayoutSaver_Finalizer(void *cppObj); // KDDockWidgets::InitialOption::InitialOption()
void *c_KDDockWidgets__InitialOption__constructor();
// KDDockWidgets::InitialOption::InitialOption(KDDockWidgets::DefaultSizeMode mode)
void *c_KDDockWidgets__InitialOption__constructor_DefaultSizeMode(int mode);
// KDDockWidgets::InitialOption::InitialOption(KDDockWidgets::InitialVisibilityOption v)
void *c_KDDockWidgets__InitialOption__constructor_InitialVisibilityOption(int v);
// KDDockWidgets::InitialOption::InitialOption(KDDockWidgets::InitialVisibilityOption v, KDDockWidgets::Size size)
void *c_KDDockWidgets__InitialOption__constructor_InitialVisibilityOption_Size(int v, void *size_);
// KDDockWidgets::InitialOption::InitialOption(KDDockWidgets::Size size)
void *c_KDDockWidgets__InitialOption__constructor_Size(void *size_);
// KDDockWidgets::InitialOption::preservesCurrentTab() const
bool c_KDDockWidgets__InitialOption__preservesCurrentTab(void *thisObj);
// KDDockWidgets::InitialOption::startsHidden() const
bool c_KDDockWidgets__InitialOption__startsHidden(void *thisObj);
void c_KDDockWidgets__InitialOption__destructor(void *thisObj);
void c_KDDockWidgets__InitialOption_Finalizer(void *cppObj); // KDDockWidgets::Event::Event(KDDockWidgets::Event::Type type)
void *c_KDDockWidgets__Event__constructor_Type(int type);
// KDDockWidgets::Event::accept()
void c_KDDockWidgets__Event__accept(void *thisObj);
// KDDockWidgets::Event::ignore()
void c_KDDockWidgets__Event__ignore(void *thisObj);
// KDDockWidgets::Event::isAccepted() const
bool c_KDDockWidgets__Event__isAccepted(void *thisObj);
// KDDockWidgets::Event::spontaneous() const
bool c_KDDockWidgets__Event__spontaneous(void *thisObj);
// KDDockWidgets::Event::type() const
int c_KDDockWidgets__Event__type(void *thisObj);
void c_KDDockWidgets__Event__destructor(void *thisObj);
bool c_KDDockWidgets__Event___get_m_accepted(void *thisObj);
bool c_KDDockWidgets__Event___get_m_spontaneous(void *thisObj);
void c_KDDockWidgets__Event___set_m_accepted_bool(void *thisObj, bool m_accepted_);
void c_KDDockWidgets__Event___set_m_spontaneous_bool(void *thisObj, bool m_spontaneous_);
void c_KDDockWidgets__Event_Finalizer(void *cppObj);
void c_KDDockWidgets__Core_Finalizer(void *cppObj); // KDDockWidgets::Core::Platform::Platform()
void *c_KDDockWidgets__Core__Platform__constructor();
// KDDockWidgets::Core::Platform::applicationName() const
void *c_KDDockWidgets__Core__Platform__applicationName(void *thisObj);
// KDDockWidgets::Core::Platform::createDefaultViewFactory()
void *c_KDDockWidgets__Core__Platform__createDefaultViewFactory(void *thisObj);
// KDDockWidgets::Core::Platform::createMainWindow(const QString & uniqueName, KDDockWidgets::Core::CreateViewOptions arg__2, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::Core::View * parent, Qt::WindowFlags arg__5) const
void *c_KDDockWidgets__Core__Platform__createMainWindow_QString_CreateViewOptions_MainWindowOptions_View_WindowFlags(void *thisObj, const char *uniqueName_, void *arg__2_, int options_, void *parent_, int arg__5);
// KDDockWidgets::Core::Platform::createView(KDDockWidgets::Core::Controller * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__Platform__createView_Controller_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::Platform::cursorPos() const
void *c_KDDockWidgets__Core__Platform__cursorPos(void *thisObj);
// KDDockWidgets::Core::Platform::dumpManagedBacktrace()
void c_KDDockWidgets__Core__Platform__dumpManagedBacktrace(void *thisObj);
// KDDockWidgets::Core::Platform::hasActivePopup() const
bool c_KDDockWidgets__Core__Platform__hasActivePopup(void *thisObj);
// KDDockWidgets::Core::Platform::inDisallowedDragView(KDDockWidgets::Point globalPos) const
bool c_KDDockWidgets__Core__Platform__inDisallowedDragView_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::Platform::installMessageHandler()
void c_KDDockWidgets__Core__Platform__installMessageHandler(void *thisObj);
// KDDockWidgets::Core::Platform::instance()
void *c_static_KDDockWidgets__Core__Platform__instance();
// KDDockWidgets::Core::Platform::isInitialized()
bool c_static_KDDockWidgets__Core__Platform__isInitialized();
// KDDockWidgets::Core::Platform::isLeftMouseButtonPressed() const
bool c_KDDockWidgets__Core__Platform__isLeftMouseButtonPressed(void *thisObj);
// KDDockWidgets::Core::Platform::isProcessingAppQuitEvent() const
bool c_KDDockWidgets__Core__Platform__isProcessingAppQuitEvent(void *thisObj);
// KDDockWidgets::Core::Platform::isQt() const
bool c_KDDockWidgets__Core__Platform__isQt(void *thisObj);
// KDDockWidgets::Core::Platform::isQtQuick() const
bool c_KDDockWidgets__Core__Platform__isQtQuick(void *thisObj);
// KDDockWidgets::Core::Platform::isQtWidgets() const
bool c_KDDockWidgets__Core__Platform__isQtWidgets(void *thisObj);
// KDDockWidgets::Core::Platform::name() const
const char *c_KDDockWidgets__Core__Platform__name(void *thisObj);
// KDDockWidgets::Core::Platform::onFloatingWindowCreated(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__Core__Platform__onFloatingWindowCreated_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::onFloatingWindowDestroyed(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__Core__Platform__onFloatingWindowDestroyed_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::onMainWindowCreated(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__Core__Platform__onMainWindowCreated_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::onMainWindowDestroyed(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__Core__Platform__onMainWindowDestroyed_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::pauseForDebugger()
void c_KDDockWidgets__Core__Platform__pauseForDebugger(void *thisObj);
// KDDockWidgets::Core::Platform::restoreMouseCursor()
void c_KDDockWidgets__Core__Platform__restoreMouseCursor(void *thisObj);
// KDDockWidgets::Core::Platform::runDelayed(int ms, KDDockWidgets::Core::DelayedCall * c)
void c_KDDockWidgets__Core__Platform__runDelayed_int_DelayedCall(void *thisObj, int ms, void *c_);
// KDDockWidgets::Core::Platform::screenNumberForView(KDDockWidgets::Core::View * arg__1) const
int c_KDDockWidgets__Core__Platform__screenNumberForView_View(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::screenSizeFor(KDDockWidgets::Core::View * arg__1) const
void *c_KDDockWidgets__Core__Platform__screenSizeFor_View(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::sendEvent(KDDockWidgets::Core::View * arg__1, KDDockWidgets::Event * arg__2) const
void c_KDDockWidgets__Core__Platform__sendEvent_View_Event(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::Core::Platform::setCursorPos(KDDockWidgets::Point arg__1)
void c_KDDockWidgets__Core__Platform__setCursorPos_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Platform::setMouseCursor(Qt::CursorShape arg__1, bool discardLast)
void c_KDDockWidgets__Core__Platform__setMouseCursor_CursorShape_bool(void *thisObj, int arg__1, bool discardLast);
// KDDockWidgets::Core::Platform::startDragDistance() const
int c_KDDockWidgets__Core__Platform__startDragDistance(void *thisObj);
// KDDockWidgets::Core::Platform::startDragDistance_impl() const
int c_KDDockWidgets__Core__Platform__startDragDistance_impl(void *thisObj);
// KDDockWidgets::Core::Platform::supportsAeroSnap() const
bool c_KDDockWidgets__Core__Platform__supportsAeroSnap(void *thisObj);
// KDDockWidgets::Core::Platform::tests_createFocusableView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__Core__Platform__tests_createFocusableView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::Platform::tests_createNonClosableView(KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__Core__Platform__tests_createNonClosableView_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Platform::tests_createView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__Core__Platform__tests_createView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::Platform::tests_deinitPlatform_impl()
void c_KDDockWidgets__Core__Platform__tests_deinitPlatform_impl(void *thisObj);
// KDDockWidgets::Core::Platform::tests_initPlatform_impl()
void c_KDDockWidgets__Core__Platform__tests_initPlatform_impl(void *thisObj);
// KDDockWidgets::Core::Platform::ungrabMouse()
void c_KDDockWidgets__Core__Platform__ungrabMouse(void *thisObj);
// KDDockWidgets::Core::Platform::uninstallMessageHandler()
void c_KDDockWidgets__Core__Platform__uninstallMessageHandler(void *thisObj);
// KDDockWidgets::Core::Platform::usesFallbackMouseGrabber() const
bool c_KDDockWidgets__Core__Platform__usesFallbackMouseGrabber(void *thisObj);
void c_KDDockWidgets__Core__Platform__destructor(void *thisObj);
void c_KDDockWidgets__Core__Platform__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__Platform_Finalizer(void *cppObj); // KDDockWidgets::flutter::Platform::Platform()
void *c_KDDockWidgets__flutter__Platform__constructor();
// KDDockWidgets::flutter::Platform::applicationName() const
void *c_KDDockWidgets__flutter__Platform__applicationName(void *thisObj);
// KDDockWidgets::flutter::Platform::createDefaultViewFactory()
void *c_KDDockWidgets__flutter__Platform__createDefaultViewFactory(void *thisObj);
// KDDockWidgets::flutter::Platform::createMainWindow(const QString & uniqueName, KDDockWidgets::Core::CreateViewOptions viewOpts, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::Core::View * parent, Qt::WindowFlags flags) const
void *c_KDDockWidgets__flutter__Platform__createMainWindow_QString_CreateViewOptions_MainWindowOptions_View_WindowFlags(void *thisObj, const char *uniqueName_, void *viewOpts_, int options_, void *parent_, int flags);
// KDDockWidgets::flutter::Platform::createView(KDDockWidgets::Core::Controller * controller, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__Platform__createView_Controller_View(void *thisObj, void *controller_, void *parent_);
// KDDockWidgets::flutter::Platform::cursorPos() const
void *c_KDDockWidgets__flutter__Platform__cursorPos(void *thisObj);
// KDDockWidgets::flutter::Platform::dumpManagedBacktrace()
void c_KDDockWidgets__flutter__Platform__dumpManagedBacktrace(void *thisObj);
// KDDockWidgets::flutter::Platform::hasActivePopup() const
bool c_KDDockWidgets__flutter__Platform__hasActivePopup(void *thisObj);
// KDDockWidgets::flutter::Platform::inDisallowedDragView(KDDockWidgets::Point globalPos) const
bool c_KDDockWidgets__flutter__Platform__inDisallowedDragView_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::flutter::Platform::init()
void c_KDDockWidgets__flutter__Platform__init(void *thisObj);
// KDDockWidgets::flutter::Platform::installMessageHandler()
void c_KDDockWidgets__flutter__Platform__installMessageHandler(void *thisObj);
// KDDockWidgets::flutter::Platform::isLeftMouseButtonPressed() const
bool c_KDDockWidgets__flutter__Platform__isLeftMouseButtonPressed(void *thisObj);
// KDDockWidgets::flutter::Platform::isProcessingAppQuitEvent() const
bool c_KDDockWidgets__flutter__Platform__isProcessingAppQuitEvent(void *thisObj);
// KDDockWidgets::flutter::Platform::name() const
const char *c_KDDockWidgets__flutter__Platform__name(void *thisObj);
// KDDockWidgets::flutter::Platform::onDropIndicatorOverlayCreated(KDDockWidgets::flutter::IndicatorWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onDropIndicatorOverlayCreated_IndicatorWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onDropIndicatorOverlayDestroyed(KDDockWidgets::flutter::IndicatorWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onDropIndicatorOverlayDestroyed_IndicatorWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onFloatingWindowCreated(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onFloatingWindowCreated_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onFloatingWindowDestroyed(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onFloatingWindowDestroyed_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onMainWindowCreated(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onMainWindowCreated_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::onMainWindowDestroyed(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__flutter__Platform__onMainWindowDestroyed_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::pauseForDartDebugger()
void c_KDDockWidgets__flutter__Platform__pauseForDartDebugger(void *thisObj);
// KDDockWidgets::flutter::Platform::pauseForDebugger()
void c_KDDockWidgets__flutter__Platform__pauseForDebugger(void *thisObj);
// KDDockWidgets::flutter::Platform::platformFlutter()
void *c_static_KDDockWidgets__flutter__Platform__platformFlutter();
// KDDockWidgets::flutter::Platform::rebuildWindowOverlay()
void c_KDDockWidgets__flutter__Platform__rebuildWindowOverlay(void *thisObj);
// KDDockWidgets::flutter::Platform::restoreMouseCursor()
void c_KDDockWidgets__flutter__Platform__restoreMouseCursor(void *thisObj);
// KDDockWidgets::flutter::Platform::resumeCoRoutines()
void c_KDDockWidgets__flutter__Platform__resumeCoRoutines(void *thisObj);
// KDDockWidgets::flutter::Platform::runDelayed(int ms, KDDockWidgets::Core::DelayedCall * c)
void c_KDDockWidgets__flutter__Platform__runDelayed_int_DelayedCall(void *thisObj, int ms, void *c_);
// KDDockWidgets::flutter::Platform::runTests()
void c_KDDockWidgets__flutter__Platform__runTests(void *thisObj);
// KDDockWidgets::flutter::Platform::scheduleResumeCoRoutines(int ms) const
void c_KDDockWidgets__flutter__Platform__scheduleResumeCoRoutines_int(void *thisObj, int ms);
// KDDockWidgets::flutter::Platform::screenNumberForView(KDDockWidgets::Core::View * arg__1) const
int c_KDDockWidgets__flutter__Platform__screenNumberForView_View(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::screenSizeFor(KDDockWidgets::Core::View * arg__1) const
void *c_KDDockWidgets__flutter__Platform__screenSizeFor_View(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::sendEvent(KDDockWidgets::Core::View * arg__1, KDDockWidgets::Event * arg__2) const
void c_KDDockWidgets__flutter__Platform__sendEvent_View_Event(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::flutter::Platform::setCursorPos(KDDockWidgets::Point arg__1)
void c_KDDockWidgets__flutter__Platform__setCursorPos_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::Platform::setMouseCursor(Qt::CursorShape arg__1, bool discardLast)
void c_KDDockWidgets__flutter__Platform__setMouseCursor_CursorShape_bool(void *thisObj, int arg__1, bool discardLast);
// KDDockWidgets::flutter::Platform::startDragDistance_impl() const
int c_KDDockWidgets__flutter__Platform__startDragDistance_impl(void *thisObj);
// KDDockWidgets::flutter::Platform::supportsAeroSnap() const
bool c_KDDockWidgets__flutter__Platform__supportsAeroSnap(void *thisObj);
// KDDockWidgets::flutter::Platform::tests_createFocusableView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__Platform__tests_createFocusableView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::Platform::tests_createNonClosableView(KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__Platform__tests_createNonClosableView_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::Platform::tests_createView(KDDockWidgets::Core::CreateViewOptions arg__1, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__Platform__tests_createView_CreateViewOptions_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::Platform::tests_deinitPlatform_impl()
void c_KDDockWidgets__flutter__Platform__tests_deinitPlatform_impl(void *thisObj);
// KDDockWidgets::flutter::Platform::tests_initPlatform_impl()
void c_KDDockWidgets__flutter__Platform__tests_initPlatform_impl(void *thisObj);
// KDDockWidgets::flutter::Platform::ungrabMouse()
void c_KDDockWidgets__flutter__Platform__ungrabMouse(void *thisObj);
// KDDockWidgets::flutter::Platform::uninstallMessageHandler()
void c_KDDockWidgets__flutter__Platform__uninstallMessageHandler(void *thisObj);
// KDDockWidgets::flutter::Platform::usesFallbackMouseGrabber() const
bool c_KDDockWidgets__flutter__Platform__usesFallbackMouseGrabber(void *thisObj);
void c_KDDockWidgets__flutter__Platform__destructor(void *thisObj);
void c_KDDockWidgets__flutter__Platform__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__Platform_Finalizer(void *cppObj); // KDDockWidgets::Core::Object::Object(KDDockWidgets::Core::Object * parent)
void *c_KDDockWidgets__Core__Object__constructor_Object(void *parent_);
// KDDockWidgets::Core::Object::objectName() const
void *c_KDDockWidgets__Core__Object__objectName(void *thisObj);
// KDDockWidgets::Core::Object::parent() const
void *c_KDDockWidgets__Core__Object__parent(void *thisObj);
// KDDockWidgets::Core::Object::setObjectName(const QString & arg__1)
void c_KDDockWidgets__Core__Object__setObjectName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::Core::Object::setParent(KDDockWidgets::Core::Object * parent)
void c_KDDockWidgets__Core__Object__setParent_Object(void *thisObj, void *parent_);
// KDDockWidgets::Core::Object::tr(const char * arg__1)
void *c_static_KDDockWidgets__Core__Object__tr_char(const char *arg__1);
void c_KDDockWidgets__Core__Object__destructor(void *thisObj);
void c_KDDockWidgets__Core__Object__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__Object_Finalizer(void *cppObj); // KDDockWidgets::DockRegistry::clear()
void c_KDDockWidgets__DockRegistry__clear(void *thisObj);
// KDDockWidgets::DockRegistry::containsDockWidget(const QString & uniqueName) const
bool c_KDDockWidgets__DockRegistry__containsDockWidget_QString(void *thisObj, const char *uniqueName_);
// KDDockWidgets::DockRegistry::containsMainWindow(const QString & uniqueName) const
bool c_KDDockWidgets__DockRegistry__containsMainWindow_QString(void *thisObj, const char *uniqueName_);
// KDDockWidgets::DockRegistry::dockByName(const QString & arg__1) const
void *c_KDDockWidgets__DockRegistry__dockByName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::DockRegistry::ensureAllFloatingWidgetsAreMorphed()
void c_KDDockWidgets__DockRegistry__ensureAllFloatingWidgetsAreMorphed(void *thisObj);
// KDDockWidgets::DockRegistry::focusedDockWidget() const
void *c_KDDockWidgets__DockRegistry__focusedDockWidget(void *thisObj);
// KDDockWidgets::DockRegistry::groupInMDIResize() const
void *c_KDDockWidgets__DockRegistry__groupInMDIResize(void *thisObj);
// KDDockWidgets::DockRegistry::hasFloatingWindows() const
bool c_KDDockWidgets__DockRegistry__hasFloatingWindows(void *thisObj);
// KDDockWidgets::DockRegistry::isEmpty(bool excludeBeingDeleted) const
bool c_KDDockWidgets__DockRegistry__isEmpty_bool(void *thisObj, bool excludeBeingDeleted);
// KDDockWidgets::DockRegistry::isSane() const
bool c_KDDockWidgets__DockRegistry__isSane(void *thisObj);
// KDDockWidgets::DockRegistry::itemIsInMainWindow(const KDDockWidgets::Core::Item * arg__1) const
bool c_KDDockWidgets__DockRegistry__itemIsInMainWindow_Item(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::layoutForItem(const KDDockWidgets::Core::Item * arg__1) const
void *c_KDDockWidgets__DockRegistry__layoutForItem_Item(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::mainWindowByName(const QString & arg__1) const
void *c_KDDockWidgets__DockRegistry__mainWindowByName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::DockRegistry::registerDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__DockRegistry__registerDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::registerFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__DockRegistry__registerFloatingWindow_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::registerGroup(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__DockRegistry__registerGroup_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::registerLayoutSaver()
void c_KDDockWidgets__DockRegistry__registerLayoutSaver(void *thisObj);
// KDDockWidgets::DockRegistry::registerMainWindow(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__DockRegistry__registerMainWindow_MainWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::self()
void *c_static_KDDockWidgets__DockRegistry__self();
// KDDockWidgets::DockRegistry::sideBarForDockWidget(const KDDockWidgets::Core::DockWidget * arg__1) const
void *c_KDDockWidgets__DockRegistry__sideBarForDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::unregisterDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__DockRegistry__unregisterDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::unregisterFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1)
void c_KDDockWidgets__DockRegistry__unregisterFloatingWindow_FloatingWindow(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::unregisterGroup(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__DockRegistry__unregisterGroup_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::DockRegistry::unregisterLayoutSaver()
void c_KDDockWidgets__DockRegistry__unregisterLayoutSaver(void *thisObj);
// KDDockWidgets::DockRegistry::unregisterMainWindow(KDDockWidgets::Core::MainWindow * arg__1)
void c_KDDockWidgets__DockRegistry__unregisterMainWindow_MainWindow(void *thisObj, void *arg__1_);
void c_KDDockWidgets__DockRegistry__destructor(void *thisObj);
void c_KDDockWidgets__DockRegistry__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__DockRegistry_Finalizer(void *cppObj); // KDDockWidgets::Core::ViewFactory::ViewFactory()
void *c_KDDockWidgets__Core__ViewFactory__constructor();
// KDDockWidgets::Core::ViewFactory::createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createClassicIndicatorWindow_ClassicDropIndicatorOverlay_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createDockWidget(const QString & uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions, Qt::WindowFlags windowFlags) const
void *c_KDDockWidgets__Core__ViewFactory__createDockWidget_QString_DockWidgetOptions_LayoutSaverOptions_WindowFlags(void *thisObj, const char *uniqueName_, int options_, int layoutSaverOptions_, int windowFlags);
// KDDockWidgets::Core::ViewFactory::createDropArea(KDDockWidgets::Core::DropArea * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createDropArea_DropArea_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createFloatingWindow(KDDockWidgets::Core::FloatingWindow * controller, KDDockWidgets::Core::MainWindow * parent, Qt::WindowFlags windowFlags) const
void *c_KDDockWidgets__Core__ViewFactory__createFloatingWindow_FloatingWindow_MainWindow_WindowFlags(void *thisObj, void *controller_, void *parent_, int windowFlags);
// KDDockWidgets::Core::ViewFactory::createGroup(KDDockWidgets::Core::Group * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createGroup_Group_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createRubberBand(KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createRubberBand_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::ViewFactory::createSeparator(KDDockWidgets::Core::Separator * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createSeparator_Separator_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createSideBar(KDDockWidgets::Core::SideBar * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createSideBar_SideBar_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createStack(KDDockWidgets::Core::Stack * stack, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createStack_Stack_View(void *thisObj, void *stack_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createTabBar(KDDockWidgets::Core::TabBar * tabBar, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createTabBar_TabBar_View(void *thisObj, void *tabBar_, void *parent_);
// KDDockWidgets::Core::ViewFactory::createTitleBar(KDDockWidgets::Core::TitleBar * controller, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__Core__ViewFactory__createTitleBar_TitleBar_View(void *thisObj, void *controller_, void *parent_);
void c_KDDockWidgets__Core__ViewFactory__destructor(void *thisObj);
void c_KDDockWidgets__Core__ViewFactory__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__ViewFactory_Finalizer(void *cppObj); // KDDockWidgets::flutter::ViewFactory::ViewFactory()
void *c_KDDockWidgets__flutter__ViewFactory__constructor();
// KDDockWidgets::flutter::ViewFactory::createClassicIndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createClassicIndicatorWindow_ClassicDropIndicatorOverlay_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createClassicIndicatorWindow_flutter(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createClassicIndicatorWindow_flutter_ClassicDropIndicatorOverlay_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createDockWidget(const QString & uniqueName, QFlags<KDDockWidgets::DockWidgetOption> arg__2, QFlags<KDDockWidgets::LayoutSaverOption> arg__3, Qt::WindowFlags arg__4) const
void *c_KDDockWidgets__flutter__ViewFactory__createDockWidget_QString_DockWidgetOptions_LayoutSaverOptions_WindowFlags(void *thisObj, const char *uniqueName_, int arg__2_, int arg__3_, int arg__4);
// KDDockWidgets::flutter::ViewFactory::createDropArea(KDDockWidgets::Core::DropArea * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createDropArea_DropArea_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createFloatingWindow(KDDockWidgets::Core::FloatingWindow * arg__1, KDDockWidgets::Core::MainWindow * parent, Qt::WindowFlags windowFlags) const
void *c_KDDockWidgets__flutter__ViewFactory__createFloatingWindow_FloatingWindow_MainWindow_WindowFlags(void *thisObj, void *arg__1_, void *parent_, int windowFlags);
// KDDockWidgets::flutter::ViewFactory::createGroup(KDDockWidgets::Core::Group * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createGroup_Group_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createRubberBand(KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createRubberBand_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createSeparator(KDDockWidgets::Core::Separator * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createSeparator_Separator_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createSideBar(KDDockWidgets::Core::SideBar * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createSideBar_SideBar_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createStack(KDDockWidgets::Core::Stack * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createStack_Stack_View(void *thisObj, void *arg__1_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createTabBar(KDDockWidgets::Core::TabBar * tabBar, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createTabBar_TabBar_View(void *thisObj, void *tabBar_, void *parent_);
// KDDockWidgets::flutter::ViewFactory::createTitleBar(KDDockWidgets::Core::TitleBar * arg__1, KDDockWidgets::Core::View * parent) const
void *c_KDDockWidgets__flutter__ViewFactory__createTitleBar_TitleBar_View(void *thisObj, void *arg__1_, void *parent_);
void c_KDDockWidgets__flutter__ViewFactory__destructor(void *thisObj);
void c_KDDockWidgets__flutter__ViewFactory__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__ViewFactory_Finalizer(void *cppObj); // KDDockWidgets::Core::Item::checkSanity()
bool c_KDDockWidgets__Core__Item__checkSanity(void *thisObj);
// KDDockWidgets::Core::Item::dumpLayout(int level, bool printSeparators)
void c_KDDockWidgets__Core__Item__dumpLayout_int_bool(void *thisObj, int level, bool printSeparators);
// KDDockWidgets::Core::Item::geometry() const
void *c_KDDockWidgets__Core__Item__geometry(void *thisObj);
// KDDockWidgets::Core::Item::height() const
int c_KDDockWidgets__Core__Item__height(void *thisObj);
// KDDockWidgets::Core::Item::inSetSize() const
bool c_KDDockWidgets__Core__Item__inSetSize(void *thisObj);
// KDDockWidgets::Core::Item::isBeingInserted() const
bool c_KDDockWidgets__Core__Item__isBeingInserted(void *thisObj);
// KDDockWidgets::Core::Item::isContainer() const
bool c_KDDockWidgets__Core__Item__isContainer(void *thisObj);
// KDDockWidgets::Core::Item::isMDI() const
bool c_KDDockWidgets__Core__Item__isMDI(void *thisObj);
// KDDockWidgets::Core::Item::isPlaceholder() const
bool c_KDDockWidgets__Core__Item__isPlaceholder(void *thisObj);
// KDDockWidgets::Core::Item::isRoot() const
bool c_KDDockWidgets__Core__Item__isRoot(void *thisObj);
// KDDockWidgets::Core::Item::isVisible(bool excludeBeingInserted) const
bool c_KDDockWidgets__Core__Item__isVisible_bool(void *thisObj, bool excludeBeingInserted);
// KDDockWidgets::Core::Item::mapFromParent(KDDockWidgets::Point arg__1) const
void *c_KDDockWidgets__Core__Item__mapFromParent_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::mapFromRoot(KDDockWidgets::Point arg__1) const
void *c_KDDockWidgets__Core__Item__mapFromRoot_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::mapFromRoot(KDDockWidgets::Rect arg__1) const
void *c_KDDockWidgets__Core__Item__mapFromRoot_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::mapToRoot(KDDockWidgets::Point arg__1) const
void *c_KDDockWidgets__Core__Item__mapToRoot_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::mapToRoot(KDDockWidgets::Rect arg__1) const
void *c_KDDockWidgets__Core__Item__mapToRoot_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::maxSizeHint() const
void *c_KDDockWidgets__Core__Item__maxSizeHint(void *thisObj);
// KDDockWidgets::Core::Item::minSize() const
void *c_KDDockWidgets__Core__Item__minSize(void *thisObj);
// KDDockWidgets::Core::Item::missingSize() const
void *c_KDDockWidgets__Core__Item__missingSize(void *thisObj);
// KDDockWidgets::Core::Item::outermostNeighbor(KDDockWidgets::Location arg__1, bool visibleOnly) const
void *c_KDDockWidgets__Core__Item__outermostNeighbor_Location_bool(void *thisObj, int arg__1, bool visibleOnly);
// KDDockWidgets::Core::Item::pos() const
void *c_KDDockWidgets__Core__Item__pos(void *thisObj);
// KDDockWidgets::Core::Item::rect() const
void *c_KDDockWidgets__Core__Item__rect(void *thisObj);
// KDDockWidgets::Core::Item::ref()
void c_KDDockWidgets__Core__Item__ref(void *thisObj);
// KDDockWidgets::Core::Item::refCount() const
int c_KDDockWidgets__Core__Item__refCount(void *thisObj);
// KDDockWidgets::Core::Item::requestResize(int left, int top, int right, int bottom)
void c_KDDockWidgets__Core__Item__requestResize_int_int_int_int(void *thisObj, int left, int top, int right, int bottom);
// KDDockWidgets::Core::Item::setBeingInserted(bool arg__1)
void c_KDDockWidgets__Core__Item__setBeingInserted_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Item::setGeometry(KDDockWidgets::Rect rect)
void c_KDDockWidgets__Core__Item__setGeometry_Rect(void *thisObj, void *rect_);
// KDDockWidgets::Core::Item::setGeometry_recursive(KDDockWidgets::Rect rect)
void c_KDDockWidgets__Core__Item__setGeometry_recursive_Rect(void *thisObj, void *rect_);
// KDDockWidgets::Core::Item::setIsVisible(bool arg__1)
void c_KDDockWidgets__Core__Item__setIsVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Item::setMaxSizeHint(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__Item__setMaxSizeHint_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::setMinSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__Item__setMinSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::setPos(KDDockWidgets::Point arg__1)
void c_KDDockWidgets__Core__Item__setPos_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::setSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__Item__setSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Item::size() const
void *c_KDDockWidgets__Core__Item__size(void *thisObj);
// KDDockWidgets::Core::Item::turnIntoPlaceholder()
void c_KDDockWidgets__Core__Item__turnIntoPlaceholder(void *thisObj);
// KDDockWidgets::Core::Item::unref()
void c_KDDockWidgets__Core__Item__unref(void *thisObj);
// KDDockWidgets::Core::Item::updateWidgetGeometries()
void c_KDDockWidgets__Core__Item__updateWidgetGeometries(void *thisObj);
// KDDockWidgets::Core::Item::visibleCount_recursive() const
int c_KDDockWidgets__Core__Item__visibleCount_recursive(void *thisObj);
// KDDockWidgets::Core::Item::width() const
int c_KDDockWidgets__Core__Item__width(void *thisObj);
// KDDockWidgets::Core::Item::x() const
int c_KDDockWidgets__Core__Item__x(void *thisObj);
// KDDockWidgets::Core::Item::y() const
int c_KDDockWidgets__Core__Item__y(void *thisObj);
void c_KDDockWidgets__Core__Item__destructor(void *thisObj);
int c_static_KDDockWidgets__Core__Item___get_separatorThickness();
int c_static_KDDockWidgets__Core__Item___get_layoutSpacing();
bool c_static_KDDockWidgets__Core__Item___get_s_silenceSanityChecks();
bool c_KDDockWidgets__Core__Item___get_m_isContainer(void *thisObj);
bool c_KDDockWidgets__Core__Item___get_m_isSettingGuest(void *thisObj);
bool c_KDDockWidgets__Core__Item___get_m_inDtor(void *thisObj);
void c_static_KDDockWidgets__Core__Item___set_separatorThickness_int(int separatorThickness_);
void c_static_KDDockWidgets__Core__Item___set_layoutSpacing_int(int layoutSpacing_);
void c_static_KDDockWidgets__Core__Item___set_s_silenceSanityChecks_bool(bool s_silenceSanityChecks_);
void c_KDDockWidgets__Core__Item___set_m_isSettingGuest_bool(void *thisObj, bool m_isSettingGuest_);
void c_KDDockWidgets__Core__Item___set_m_inDtor_bool(void *thisObj, bool m_inDtor_);
void c_KDDockWidgets__Core__Item__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__Item_Finalizer(void *cppObj); // KDDockWidgets::Core::DelayedCall::DelayedCall()
void *c_KDDockWidgets__Core__DelayedCall__constructor();
// KDDockWidgets::Core::DelayedCall::call()
void c_KDDockWidgets__Core__DelayedCall__call(void *thisObj);
void c_KDDockWidgets__Core__DelayedCall__destructor(void *thisObj);
void c_KDDockWidgets__Core__DelayedCall__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__DelayedCall_Finalizer(void *cppObj); // KDDockWidgets::Core::CreateViewOptions::CreateViewOptions()
void *c_KDDockWidgets__Core__CreateViewOptions__constructor();
// KDDockWidgets::Core::CreateViewOptions::getMaxSize() const
void *c_KDDockWidgets__Core__CreateViewOptions__getMaxSize(void *thisObj);
// KDDockWidgets::Core::CreateViewOptions::getMinSize() const
void *c_KDDockWidgets__Core__CreateViewOptions__getMinSize(void *thisObj);
// KDDockWidgets::Core::CreateViewOptions::getSize() const
void *c_KDDockWidgets__Core__CreateViewOptions__getSize(void *thisObj);
void c_KDDockWidgets__Core__CreateViewOptions__destructor(void *thisObj);
bool c_KDDockWidgets__Core__CreateViewOptions___get_isVisible(void *thisObj);
bool c_KDDockWidgets__Core__CreateViewOptions___get_createWindow(void *thisObj);
void c_KDDockWidgets__Core__CreateViewOptions___set_isVisible_bool(void *thisObj, bool isVisible_);
void c_KDDockWidgets__Core__CreateViewOptions___set_createWindow_bool(void *thisObj, bool createWindow_);
void c_KDDockWidgets__Core__CreateViewOptions_Finalizer(void *cppObj); // KDDockWidgets::Core::Controller::Controller(KDDockWidgets::Core::ViewType type, KDDockWidgets::Core::View * arg__2)
void *c_KDDockWidgets__Core__Controller__constructor_ViewType_View(int type, void *arg__2_);
// KDDockWidgets::Core::Controller::close()
bool c_KDDockWidgets__Core__Controller__close(void *thisObj);
// KDDockWidgets::Core::Controller::destroyLater()
void c_KDDockWidgets__Core__Controller__destroyLater(void *thisObj);
// KDDockWidgets::Core::Controller::geometry() const
void *c_KDDockWidgets__Core__Controller__geometry(void *thisObj);
// KDDockWidgets::Core::Controller::height() const
int c_KDDockWidgets__Core__Controller__height(void *thisObj);
// KDDockWidgets::Core::Controller::inDtor() const
bool c_KDDockWidgets__Core__Controller__inDtor(void *thisObj);
// KDDockWidgets::Core::Controller::isFixedHeight() const
bool c_KDDockWidgets__Core__Controller__isFixedHeight(void *thisObj);
// KDDockWidgets::Core::Controller::isFixedWidth() const
bool c_KDDockWidgets__Core__Controller__isFixedWidth(void *thisObj);
// KDDockWidgets::Core::Controller::isVisible() const
bool c_KDDockWidgets__Core__Controller__isVisible(void *thisObj);
// KDDockWidgets::Core::Controller::mapToGlobal(KDDockWidgets::Point arg__1) const
void *c_KDDockWidgets__Core__Controller__mapToGlobal_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Controller::pos() const
void *c_KDDockWidgets__Core__Controller__pos(void *thisObj);
// KDDockWidgets::Core::Controller::rect() const
void *c_KDDockWidgets__Core__Controller__rect(void *thisObj);
// KDDockWidgets::Core::Controller::setParentView(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__Controller__setParentView_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Controller::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__Controller__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Controller::setVisible(bool arg__1)
void c_KDDockWidgets__Core__Controller__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Controller::show() const
void c_KDDockWidgets__Core__Controller__show(void *thisObj);
// KDDockWidgets::Core::Controller::size() const
void *c_KDDockWidgets__Core__Controller__size(void *thisObj);
// KDDockWidgets::Core::Controller::type() const
int c_KDDockWidgets__Core__Controller__type(void *thisObj);
// KDDockWidgets::Core::Controller::view() const
void *c_KDDockWidgets__Core__Controller__view(void *thisObj);
// KDDockWidgets::Core::Controller::width() const
int c_KDDockWidgets__Core__Controller__width(void *thisObj);
// KDDockWidgets::Core::Controller::x() const
int c_KDDockWidgets__Core__Controller__x(void *thisObj);
// KDDockWidgets::Core::Controller::y() const
int c_KDDockWidgets__Core__Controller__y(void *thisObj);
void c_KDDockWidgets__Core__Controller__destructor(void *thisObj);
void c_KDDockWidgets__Core__Controller__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__Controller_Finalizer(void *cppObj); // KDDockWidgets::Core::View::activateWindow()
void c_KDDockWidgets__Core__View__activateWindow(void *thisObj);
// KDDockWidgets::Core::View::asDockWidgetController() const
void *c_KDDockWidgets__Core__View__asDockWidgetController(void *thisObj);
// KDDockWidgets::Core::View::asDropAreaController() const
void *c_KDDockWidgets__Core__View__asDropAreaController(void *thisObj);
// KDDockWidgets::Core::View::asFloatingWindowController() const
void *c_KDDockWidgets__Core__View__asFloatingWindowController(void *thisObj);
// KDDockWidgets::Core::View::asGroupController() const
void *c_KDDockWidgets__Core__View__asGroupController(void *thisObj);
// KDDockWidgets::Core::View::asLayout() const
void *c_KDDockWidgets__Core__View__asLayout(void *thisObj);
// KDDockWidgets::Core::View::asMainWindowController() const
void *c_KDDockWidgets__Core__View__asMainWindowController(void *thisObj);
// KDDockWidgets::Core::View::asStackController() const
void *c_KDDockWidgets__Core__View__asStackController(void *thisObj);
// KDDockWidgets::Core::View::asTabBarController() const
void *c_KDDockWidgets__Core__View__asTabBarController(void *thisObj);
// KDDockWidgets::Core::View::asTitleBarController() const
void *c_KDDockWidgets__Core__View__asTitleBarController(void *thisObj);
// KDDockWidgets::Core::View::close()
bool c_KDDockWidgets__Core__View__close(void *thisObj);
// KDDockWidgets::Core::View::controller() const
void *c_KDDockWidgets__Core__View__controller(void *thisObj);
// KDDockWidgets::Core::View::createPlatformWindow()
void c_KDDockWidgets__Core__View__createPlatformWindow(void *thisObj);
// KDDockWidgets::Core::View::deliverViewEventToFilters(KDDockWidgets::Event * e)
bool c_KDDockWidgets__Core__View__deliverViewEventToFilters_Event(void *thisObj, void *e_);
// KDDockWidgets::Core::View::dumpDebug()
void c_KDDockWidgets__Core__View__dumpDebug(void *thisObj);
// KDDockWidgets::Core::View::equals(const KDDockWidgets::Core::View * one, const KDDockWidgets::Core::View * two)
bool c_static_KDDockWidgets__Core__View__equals_View_View(void *one_, void *two_);
// KDDockWidgets::Core::View::equals(const KDDockWidgets::Core::View * other) const
bool c_KDDockWidgets__Core__View__equals_View(void *thisObj, void *other_);
// KDDockWidgets::Core::View::firstParentOfType(KDDockWidgets::Core::View * view, KDDockWidgets::Core::ViewType arg__2)
void *c_static_KDDockWidgets__Core__View__firstParentOfType_View_ViewType(void *view_, int arg__2);
// KDDockWidgets::Core::View::flags() const
int c_KDDockWidgets__Core__View__flags(void *thisObj);
// KDDockWidgets::Core::View::geometry() const
void *c_KDDockWidgets__Core__View__geometry(void *thisObj);
// KDDockWidgets::Core::View::grabMouse()
void c_KDDockWidgets__Core__View__grabMouse(void *thisObj);
// KDDockWidgets::Core::View::hardcodedMinimumSize()
void *c_static_KDDockWidgets__Core__View__hardcodedMinimumSize();
// KDDockWidgets::Core::View::hasFocus() const
bool c_KDDockWidgets__Core__View__hasFocus(void *thisObj);
// KDDockWidgets::Core::View::height() const
int c_KDDockWidgets__Core__View__height(void *thisObj);
// KDDockWidgets::Core::View::hide()
void c_KDDockWidgets__Core__View__hide(void *thisObj);
// KDDockWidgets::Core::View::inDtor() const
bool c_KDDockWidgets__Core__View__inDtor(void *thisObj);
// KDDockWidgets::Core::View::init()
void c_KDDockWidgets__Core__View__init(void *thisObj);
// KDDockWidgets::Core::View::isActiveWindow() const
bool c_KDDockWidgets__Core__View__isActiveWindow(void *thisObj);
// KDDockWidgets::Core::View::isExplicitlyHidden() const
bool c_KDDockWidgets__Core__View__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::Core::View::isFixedHeight() const
bool c_KDDockWidgets__Core__View__isFixedHeight(void *thisObj);
// KDDockWidgets::Core::View::isFixedWidth() const
bool c_KDDockWidgets__Core__View__isFixedWidth(void *thisObj);
// KDDockWidgets::Core::View::isMaximized() const
bool c_KDDockWidgets__Core__View__isMaximized(void *thisObj);
// KDDockWidgets::Core::View::isMinimized() const
bool c_KDDockWidgets__Core__View__isMinimized(void *thisObj);
// KDDockWidgets::Core::View::isNull() const
bool c_KDDockWidgets__Core__View__isNull(void *thisObj);
// KDDockWidgets::Core::View::isRootView() const
bool c_KDDockWidgets__Core__View__isRootView(void *thisObj);
// KDDockWidgets::Core::View::isVisible() const
bool c_KDDockWidgets__Core__View__isVisible(void *thisObj);
// KDDockWidgets::Core::View::mapFromGlobal(KDDockWidgets::Point arg__1) const
void *c_KDDockWidgets__Core__View__mapFromGlobal_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::mapTo(KDDockWidgets::Core::View * arg__1, KDDockWidgets::Point arg__2) const
void *c_KDDockWidgets__Core__View__mapTo_View_Point(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::Core::View::mapToGlobal(KDDockWidgets::Point arg__1) const
void *c_KDDockWidgets__Core__View__mapToGlobal_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::maxSizeHint() const
void *c_KDDockWidgets__Core__View__maxSizeHint(void *thisObj);
// KDDockWidgets::Core::View::minSize() const
void *c_KDDockWidgets__Core__View__minSize(void *thisObj);
// KDDockWidgets::Core::View::minimumHeight() const
int c_KDDockWidgets__Core__View__minimumHeight(void *thisObj);
// KDDockWidgets::Core::View::minimumWidth() const
int c_KDDockWidgets__Core__View__minimumWidth(void *thisObj);
// KDDockWidgets::Core::View::move(KDDockWidgets::Point arg__1)
void c_KDDockWidgets__Core__View__move_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::move(int x, int y)
void c_KDDockWidgets__Core__View__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::Core::View::normalGeometry() const
void *c_KDDockWidgets__Core__View__normalGeometry(void *thisObj);
// KDDockWidgets::Core::View::onResize(KDDockWidgets::Size arg__1)
bool c_KDDockWidgets__Core__View__onResize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::onResize(int h, int w)
bool c_KDDockWidgets__Core__View__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::Core::View::pos() const
void *c_KDDockWidgets__Core__View__pos(void *thisObj);
// KDDockWidgets::Core::View::raise()
void c_KDDockWidgets__Core__View__raise(void *thisObj);
// KDDockWidgets::Core::View::raiseAndActivate()
void c_KDDockWidgets__Core__View__raiseAndActivate(void *thisObj);
// KDDockWidgets::Core::View::rect() const
void *c_KDDockWidgets__Core__View__rect(void *thisObj);
// KDDockWidgets::Core::View::releaseKeyboard()
void c_KDDockWidgets__Core__View__releaseKeyboard(void *thisObj);
// KDDockWidgets::Core::View::releaseMouse()
void c_KDDockWidgets__Core__View__releaseMouse(void *thisObj);
// KDDockWidgets::Core::View::resize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__View__resize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::resize(int w, int h)
void c_KDDockWidgets__Core__View__resize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::Core::View::screenSize() const
void *c_KDDockWidgets__Core__View__screenSize(void *thisObj);
// KDDockWidgets::Core::View::setCursor(Qt::CursorShape arg__1)
void c_KDDockWidgets__Core__View__setCursor_CursorShape(void *thisObj, int arg__1);
// KDDockWidgets::Core::View::setFixedHeight(int arg__1)
void c_KDDockWidgets__Core__View__setFixedHeight_int(void *thisObj, int arg__1);
// KDDockWidgets::Core::View::setFixedWidth(int arg__1)
void c_KDDockWidgets__Core__View__setFixedWidth_int(void *thisObj, int arg__1);
// KDDockWidgets::Core::View::setGeometry(KDDockWidgets::Rect arg__1)
void c_KDDockWidgets__Core__View__setGeometry_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::setHeight(int height)
void c_KDDockWidgets__Core__View__setHeight_int(void *thisObj, int height);
// KDDockWidgets::Core::View::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__Core__View__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::Core::View::setMinimumSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__View__setMinimumSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::setMouseTracking(bool arg__1)
void c_KDDockWidgets__Core__View__setMouseTracking_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::View::setParent(KDDockWidgets::Core::View * arg__1)
void c_KDDockWidgets__Core__View__setParent_View(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::setSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__View__setSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::View::setSize(int width, int height)
void c_KDDockWidgets__Core__View__setSize_int_int(void *thisObj, int width, int height);
// KDDockWidgets::Core::View::setViewName(const QString & arg__1)
void c_KDDockWidgets__Core__View__setViewName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::Core::View::setVisible(bool arg__1)
void c_KDDockWidgets__Core__View__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::View::setWidth(int width)
void c_KDDockWidgets__Core__View__setWidth_int(void *thisObj, int width);
// KDDockWidgets::Core::View::setWindowOpacity(double arg__1)
void c_KDDockWidgets__Core__View__setWindowOpacity_double(void *thisObj, double arg__1);
// KDDockWidgets::Core::View::setWindowTitle(const QString & title)
void c_KDDockWidgets__Core__View__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::Core::View::setZOrder(int arg__1)
void c_KDDockWidgets__Core__View__setZOrder_int(void *thisObj, int arg__1);
// KDDockWidgets::Core::View::show()
void c_KDDockWidgets__Core__View__show(void *thisObj);
// KDDockWidgets::Core::View::showMaximized()
void c_KDDockWidgets__Core__View__showMaximized(void *thisObj);
// KDDockWidgets::Core::View::showMinimized()
void c_KDDockWidgets__Core__View__showMinimized(void *thisObj);
// KDDockWidgets::Core::View::showNormal()
void c_KDDockWidgets__Core__View__showNormal(void *thisObj);
// KDDockWidgets::Core::View::size() const
void *c_KDDockWidgets__Core__View__size(void *thisObj);
// KDDockWidgets::Core::View::update()
void c_KDDockWidgets__Core__View__update(void *thisObj);
// KDDockWidgets::Core::View::viewName() const
void *c_KDDockWidgets__Core__View__viewName(void *thisObj);
// KDDockWidgets::Core::View::width() const
int c_KDDockWidgets__Core__View__width(void *thisObj);
// KDDockWidgets::Core::View::x() const
int c_KDDockWidgets__Core__View__x(void *thisObj);
// KDDockWidgets::Core::View::y() const
int c_KDDockWidgets__Core__View__y(void *thisObj);
// KDDockWidgets::Core::View::zOrder() const
int c_KDDockWidgets__Core__View__zOrder(void *thisObj);
void c_KDDockWidgets__Core__View__destructor(void *thisObj);
void c_KDDockWidgets__Core__View__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__View_Finalizer(void *cppObj); // KDDockWidgets::flutter::View::View(KDDockWidgets::Core::Controller * controller, KDDockWidgets::Core::ViewType type, KDDockWidgets::Core::View * arg__3, Qt::WindowFlags windowFlags)
void *c_KDDockWidgets__flutter__View__constructor_Controller_ViewType_View_WindowFlags(void *controller_, int type, void *arg__3_, int windowFlags);
// KDDockWidgets::flutter::View::activateWindow()
void c_KDDockWidgets__flutter__View__activateWindow(void *thisObj);
// KDDockWidgets::flutter::View::close()
bool c_KDDockWidgets__flutter__View__close(void *thisObj);
// KDDockWidgets::flutter::View::createPlatformWindow()
void c_KDDockWidgets__flutter__View__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::View::flags() const
int c_KDDockWidgets__flutter__View__flags(void *thisObj);
// KDDockWidgets::flutter::View::geometry() const
void *c_KDDockWidgets__flutter__View__geometry(void *thisObj);
// KDDockWidgets::flutter::View::grabMouse()
void c_KDDockWidgets__flutter__View__grabMouse(void *thisObj);
// KDDockWidgets::flutter::View::hasFocus() const
bool c_KDDockWidgets__flutter__View__hasFocus(void *thisObj);
// KDDockWidgets::flutter::View::hide()
void c_KDDockWidgets__flutter__View__hide(void *thisObj);
// KDDockWidgets::flutter::View::init()
void c_KDDockWidgets__flutter__View__init(void *thisObj);
// KDDockWidgets::flutter::View::isActiveWindow() const
bool c_KDDockWidgets__flutter__View__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::View::isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__View__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::View::isMaximized() const
bool c_KDDockWidgets__flutter__View__isMaximized(void *thisObj);
// KDDockWidgets::flutter::View::isMinimized() const
bool c_KDDockWidgets__flutter__View__isMinimized(void *thisObj);
// KDDockWidgets::flutter::View::isMounted() const
bool c_KDDockWidgets__flutter__View__isMounted(void *thisObj);
// KDDockWidgets::flutter::View::isNull() const
bool c_KDDockWidgets__flutter__View__isNull(void *thisObj);
// KDDockWidgets::flutter::View::isRootView() const
bool c_KDDockWidgets__flutter__View__isRootView(void *thisObj);
// KDDockWidgets::flutter::View::isVisible() const
bool c_KDDockWidgets__flutter__View__isVisible(void *thisObj);
// KDDockWidgets::flutter::View::mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__View__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::View::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__View__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::View::mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__View__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::View::maxSizeHint() const
void *c_KDDockWidgets__flutter__View__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::View::minSize() const
void *c_KDDockWidgets__flutter__View__minSize(void *thisObj);
// KDDockWidgets::flutter::View::move(int x, int y)
void c_KDDockWidgets__flutter__View__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::View::normalGeometry() const
void *c_KDDockWidgets__flutter__View__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::View::onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__View__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::View::onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__View__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::View::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__View__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::View::onFlutterWidgetResized(int w, int h)
bool c_KDDockWidgets__flutter__View__onFlutterWidgetResized_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::View::onGeometryChanged()
void c_KDDockWidgets__flutter__View__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::View::onMouseEvent(KDDockWidgets::Event::Type eventType, KDDockWidgets::Point localPos, KDDockWidgets::Point globalPos, bool leftIsPressed)
void c_KDDockWidgets__flutter__View__onMouseEvent_Type_Point_Point_bool(void *thisObj, int eventType, void *localPos_, void *globalPos_, bool leftIsPressed);
// KDDockWidgets::flutter::View::onRebuildRequested()
void c_KDDockWidgets__flutter__View__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::View::onResize(int h, int w)
bool c_KDDockWidgets__flutter__View__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::View::raise()
void c_KDDockWidgets__flutter__View__raise(void *thisObj);
// KDDockWidgets::flutter::View::raiseAndActivate()
void c_KDDockWidgets__flutter__View__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::View::raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__View__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::View::raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__View__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::View::releaseKeyboard()
void c_KDDockWidgets__flutter__View__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::View::releaseMouse()
void c_KDDockWidgets__flutter__View__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::View::setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__View__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::View::setFixedHeight(int h)
void c_KDDockWidgets__flutter__View__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::View::setFixedWidth(int w)
void c_KDDockWidgets__flutter__View__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::View::setGeometry(KDDockWidgets::Rect geometry)
void c_KDDockWidgets__flutter__View__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::View::setHeight(int h)
void c_KDDockWidgets__flutter__View__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::View::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__View__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::View::setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__View__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::View::setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__View__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::View::setNormalGeometry(KDDockWidgets::Rect geo)
void c_KDDockWidgets__flutter__View__setNormalGeometry_Rect(void *thisObj, void *geo_);
// KDDockWidgets::flutter::View::setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__View__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::View::setSize(int w, int h)
void c_KDDockWidgets__flutter__View__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::View::setViewName(const QString & name)
void c_KDDockWidgets__flutter__View__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::View::setVisible(bool visible)
void c_KDDockWidgets__flutter__View__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::View::setWidth(int w)
void c_KDDockWidgets__flutter__View__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::View::setWindowOpacity(double v)
void c_KDDockWidgets__flutter__View__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::View::setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__View__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::View::setZOrder(int z)
void c_KDDockWidgets__flutter__View__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::View::show()
void c_KDDockWidgets__flutter__View__show(void *thisObj);
// KDDockWidgets::flutter::View::showMaximized()
void c_KDDockWidgets__flutter__View__showMaximized(void *thisObj);
// KDDockWidgets::flutter::View::showMinimized()
void c_KDDockWidgets__flutter__View__showMinimized(void *thisObj);
// KDDockWidgets::flutter::View::showNormal()
void c_KDDockWidgets__flutter__View__showNormal(void *thisObj);
// KDDockWidgets::flutter::View::update()
void c_KDDockWidgets__flutter__View__update(void *thisObj);
// KDDockWidgets::flutter::View::updateGeometry()
void c_KDDockWidgets__flutter__View__updateGeometry(void *thisObj);
// KDDockWidgets::flutter::View::viewName() const
void *c_KDDockWidgets__flutter__View__viewName(void *thisObj);
// KDDockWidgets::flutter::View::zOrder() const
int c_KDDockWidgets__flutter__View__zOrder(void *thisObj);
void c_KDDockWidgets__flutter__View__destructor(void *thisObj);
void c_KDDockWidgets__flutter__View__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__View_Finalizer(void *cppObj); // KDDockWidgets::flutter::TitleBar::TitleBar(KDDockWidgets::Core::TitleBar * controller, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__TitleBar__constructor_TitleBar_View(void *controller_, void *parent_);
// KDDockWidgets::flutter::TitleBar::activateWindow()
void c_KDDockWidgets__flutter__TitleBar__activateWindow(void *thisObj);
// KDDockWidgets::flutter::TitleBar::close()
bool c_KDDockWidgets__flutter__TitleBar__close(void *thisObj);
// KDDockWidgets::flutter::TitleBar::createPlatformWindow()
void c_KDDockWidgets__flutter__TitleBar__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::TitleBar::flags() const
int c_KDDockWidgets__flutter__TitleBar__flags(void *thisObj);
// KDDockWidgets::flutter::TitleBar::geometry() const
void *c_KDDockWidgets__flutter__TitleBar__geometry(void *thisObj);
// KDDockWidgets::flutter::TitleBar::grabMouse()
void c_KDDockWidgets__flutter__TitleBar__grabMouse(void *thisObj);
// KDDockWidgets::flutter::TitleBar::hasFocus() const
bool c_KDDockWidgets__flutter__TitleBar__hasFocus(void *thisObj);
// KDDockWidgets::flutter::TitleBar::hide()
void c_KDDockWidgets__flutter__TitleBar__hide(void *thisObj);
// KDDockWidgets::flutter::TitleBar::init()
void c_KDDockWidgets__flutter__TitleBar__init(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isActiveWindow() const
bool c_KDDockWidgets__flutter__TitleBar__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isCloseButtonEnabled() const
bool c_KDDockWidgets__flutter__TitleBar__isCloseButtonEnabled(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isCloseButtonVisible() const
bool c_KDDockWidgets__flutter__TitleBar__isCloseButtonVisible(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__TitleBar__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isFloatButtonVisible() const
bool c_KDDockWidgets__flutter__TitleBar__isFloatButtonVisible(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isMaximized() const
bool c_KDDockWidgets__flutter__TitleBar__isMaximized(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isMinimized() const
bool c_KDDockWidgets__flutter__TitleBar__isMinimized(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isMounted() const
bool c_KDDockWidgets__flutter__TitleBar__isMounted(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isNull() const
bool c_KDDockWidgets__flutter__TitleBar__isNull(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isRootView() const
bool c_KDDockWidgets__flutter__TitleBar__isRootView(void *thisObj);
// KDDockWidgets::flutter::TitleBar::isVisible() const
bool c_KDDockWidgets__flutter__TitleBar__isVisible(void *thisObj);
// KDDockWidgets::flutter::TitleBar::mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__TitleBar__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::TitleBar::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__TitleBar__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::TitleBar::mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__TitleBar__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::TitleBar::maxSizeHint() const
void *c_KDDockWidgets__flutter__TitleBar__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::TitleBar::minSize() const
void *c_KDDockWidgets__flutter__TitleBar__minSize(void *thisObj);
// KDDockWidgets::flutter::TitleBar::move(int x, int y)
void c_KDDockWidgets__flutter__TitleBar__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::TitleBar::normalGeometry() const
void *c_KDDockWidgets__flutter__TitleBar__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::TitleBar::onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__TitleBar__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TitleBar::onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__TitleBar__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TitleBar::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__TitleBar__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TitleBar::onGeometryChanged()
void c_KDDockWidgets__flutter__TitleBar__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::TitleBar::onRebuildRequested()
void c_KDDockWidgets__flutter__TitleBar__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::TitleBar::onResize(int h, int w)
bool c_KDDockWidgets__flutter__TitleBar__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::TitleBar::onTitleBarChanged(const QString & arg__1)
void c_KDDockWidgets__flutter__TitleBar__onTitleBarChanged_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::flutter::TitleBar::raise()
void c_KDDockWidgets__flutter__TitleBar__raise(void *thisObj);
// KDDockWidgets::flutter::TitleBar::raiseAndActivate()
void c_KDDockWidgets__flutter__TitleBar__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::TitleBar::raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__TitleBar__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TitleBar::raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__TitleBar__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::TitleBar::releaseKeyboard()
void c_KDDockWidgets__flutter__TitleBar__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::TitleBar::releaseMouse()
void c_KDDockWidgets__flutter__TitleBar__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::TitleBar::setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__TitleBar__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::TitleBar::setFixedHeight(int h)
void c_KDDockWidgets__flutter__TitleBar__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::TitleBar::setFixedWidth(int w)
void c_KDDockWidgets__flutter__TitleBar__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::TitleBar::setGeometry(KDDockWidgets::Rect geometry)
void c_KDDockWidgets__flutter__TitleBar__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::TitleBar::setHeight(int h)
void c_KDDockWidgets__flutter__TitleBar__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::TitleBar::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__TitleBar__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::TitleBar::setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__TitleBar__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::TitleBar::setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__TitleBar__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::TitleBar::setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__TitleBar__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::TitleBar::setSize(int w, int h)
void c_KDDockWidgets__flutter__TitleBar__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::TitleBar::setViewName(const QString & name)
void c_KDDockWidgets__flutter__TitleBar__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::TitleBar::setVisible(bool visible)
void c_KDDockWidgets__flutter__TitleBar__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::TitleBar::setWidth(int w)
void c_KDDockWidgets__flutter__TitleBar__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::TitleBar::setWindowOpacity(double v)
void c_KDDockWidgets__flutter__TitleBar__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::TitleBar::setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__TitleBar__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::TitleBar::setZOrder(int z)
void c_KDDockWidgets__flutter__TitleBar__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::TitleBar::show()
void c_KDDockWidgets__flutter__TitleBar__show(void *thisObj);
// KDDockWidgets::flutter::TitleBar::showMaximized()
void c_KDDockWidgets__flutter__TitleBar__showMaximized(void *thisObj);
// KDDockWidgets::flutter::TitleBar::showMinimized()
void c_KDDockWidgets__flutter__TitleBar__showMinimized(void *thisObj);
// KDDockWidgets::flutter::TitleBar::showNormal()
void c_KDDockWidgets__flutter__TitleBar__showNormal(void *thisObj);
// KDDockWidgets::flutter::TitleBar::update()
void c_KDDockWidgets__flutter__TitleBar__update(void *thisObj);
// KDDockWidgets::flutter::TitleBar::viewName() const
void *c_KDDockWidgets__flutter__TitleBar__viewName(void *thisObj);
// KDDockWidgets::flutter::TitleBar::zOrder() const
int c_KDDockWidgets__flutter__TitleBar__zOrder(void *thisObj);
void c_KDDockWidgets__flutter__TitleBar__destructor(void *thisObj);
void c_KDDockWidgets__flutter__TitleBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__TitleBar_Finalizer(void *cppObj); // KDDockWidgets::flutter::TabBar::TabBar(KDDockWidgets::Core::TabBar * controller, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__TabBar__constructor_TabBar_View(void *controller_, void *parent_);
// KDDockWidgets::flutter::TabBar::activateWindow()
void c_KDDockWidgets__flutter__TabBar__activateWindow(void *thisObj);
// KDDockWidgets::flutter::TabBar::close()
bool c_KDDockWidgets__flutter__TabBar__close(void *thisObj);
// KDDockWidgets::flutter::TabBar::createPlatformWindow()
void c_KDDockWidgets__flutter__TabBar__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::TabBar::flags() const
int c_KDDockWidgets__flutter__TabBar__flags(void *thisObj);
// KDDockWidgets::flutter::TabBar::geometry() const
void *c_KDDockWidgets__flutter__TabBar__geometry(void *thisObj);
// KDDockWidgets::flutter::TabBar::grabMouse()
void c_KDDockWidgets__flutter__TabBar__grabMouse(void *thisObj);
// KDDockWidgets::flutter::TabBar::hasFocus() const
bool c_KDDockWidgets__flutter__TabBar__hasFocus(void *thisObj);
// KDDockWidgets::flutter::TabBar::hide()
void c_KDDockWidgets__flutter__TabBar__hide(void *thisObj);
// KDDockWidgets::flutter::TabBar::init()
void c_KDDockWidgets__flutter__TabBar__init(void *thisObj);
// KDDockWidgets::flutter::TabBar::isActiveWindow() const
bool c_KDDockWidgets__flutter__TabBar__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::TabBar::isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__TabBar__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::TabBar::isMaximized() const
bool c_KDDockWidgets__flutter__TabBar__isMaximized(void *thisObj);
// KDDockWidgets::flutter::TabBar::isMinimized() const
bool c_KDDockWidgets__flutter__TabBar__isMinimized(void *thisObj);
// KDDockWidgets::flutter::TabBar::isMounted() const
bool c_KDDockWidgets__flutter__TabBar__isMounted(void *thisObj);
// KDDockWidgets::flutter::TabBar::isNull() const
bool c_KDDockWidgets__flutter__TabBar__isNull(void *thisObj);
// KDDockWidgets::flutter::TabBar::isRootView() const
bool c_KDDockWidgets__flutter__TabBar__isRootView(void *thisObj);
// KDDockWidgets::flutter::TabBar::isVisible() const
bool c_KDDockWidgets__flutter__TabBar__isVisible(void *thisObj);
// KDDockWidgets::flutter::TabBar::mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__TabBar__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::TabBar::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__TabBar__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::TabBar::mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__TabBar__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::TabBar::maxSizeHint() const
void *c_KDDockWidgets__flutter__TabBar__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::TabBar::minSize() const
void *c_KDDockWidgets__flutter__TabBar__minSize(void *thisObj);
// KDDockWidgets::flutter::TabBar::move(int x, int y)
void c_KDDockWidgets__flutter__TabBar__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::TabBar::moveTabTo(int from, int to)
void c_KDDockWidgets__flutter__TabBar__moveTabTo_int_int(void *thisObj, int from, int to);
// KDDockWidgets::flutter::TabBar::normalGeometry() const
void *c_KDDockWidgets__flutter__TabBar__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::TabBar::onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__TabBar__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TabBar::onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__TabBar__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TabBar::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__TabBar__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TabBar::onGeometryChanged()
void c_KDDockWidgets__flutter__TabBar__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::TabBar::onRebuildRequested()
void c_KDDockWidgets__flutter__TabBar__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::TabBar::onResize(int h, int w)
bool c_KDDockWidgets__flutter__TabBar__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::TabBar::raise()
void c_KDDockWidgets__flutter__TabBar__raise(void *thisObj);
// KDDockWidgets::flutter::TabBar::raiseAndActivate()
void c_KDDockWidgets__flutter__TabBar__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::TabBar::raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__TabBar__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::TabBar::raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__TabBar__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::TabBar::rectForTab(int index) const
void *c_KDDockWidgets__flutter__TabBar__rectForTab_int(void *thisObj, int index);
// KDDockWidgets::flutter::TabBar::releaseKeyboard()
void c_KDDockWidgets__flutter__TabBar__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::TabBar::releaseMouse()
void c_KDDockWidgets__flutter__TabBar__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::TabBar::removeDockWidget(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__flutter__TabBar__removeDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::flutter::TabBar::renameTab(int index, const QString & name)
void c_KDDockWidgets__flutter__TabBar__renameTab_int_QString(void *thisObj, int index, const char *name_);
// KDDockWidgets::flutter::TabBar::setCurrentIndex(int index)
void c_KDDockWidgets__flutter__TabBar__setCurrentIndex_int(void *thisObj, int index);
// KDDockWidgets::flutter::TabBar::setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__TabBar__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::TabBar::setFixedHeight(int h)
void c_KDDockWidgets__flutter__TabBar__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::TabBar::setFixedWidth(int w)
void c_KDDockWidgets__flutter__TabBar__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::TabBar::setGeometry(KDDockWidgets::Rect geometry)
void c_KDDockWidgets__flutter__TabBar__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::TabBar::setHeight(int h)
void c_KDDockWidgets__flutter__TabBar__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::TabBar::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__TabBar__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::TabBar::setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__TabBar__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::TabBar::setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__TabBar__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::TabBar::setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__TabBar__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::TabBar::setSize(int w, int h)
void c_KDDockWidgets__flutter__TabBar__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::TabBar::setViewName(const QString & name)
void c_KDDockWidgets__flutter__TabBar__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::TabBar::setVisible(bool visible)
void c_KDDockWidgets__flutter__TabBar__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::TabBar::setWidth(int w)
void c_KDDockWidgets__flutter__TabBar__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::TabBar::setWindowOpacity(double v)
void c_KDDockWidgets__flutter__TabBar__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::TabBar::setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__TabBar__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::TabBar::setZOrder(int z)
void c_KDDockWidgets__flutter__TabBar__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::TabBar::show()
void c_KDDockWidgets__flutter__TabBar__show(void *thisObj);
// KDDockWidgets::flutter::TabBar::showMaximized()
void c_KDDockWidgets__flutter__TabBar__showMaximized(void *thisObj);
// KDDockWidgets::flutter::TabBar::showMinimized()
void c_KDDockWidgets__flutter__TabBar__showMinimized(void *thisObj);
// KDDockWidgets::flutter::TabBar::showNormal()
void c_KDDockWidgets__flutter__TabBar__showNormal(void *thisObj);
// KDDockWidgets::flutter::TabBar::tabAt(KDDockWidgets::Point localPos) const
int c_KDDockWidgets__flutter__TabBar__tabAt_Point(void *thisObj, void *localPos_);
// KDDockWidgets::flutter::TabBar::text(int index) const
void *c_KDDockWidgets__flutter__TabBar__text_int(void *thisObj, int index);
// KDDockWidgets::flutter::TabBar::update()
void c_KDDockWidgets__flutter__TabBar__update(void *thisObj);
// KDDockWidgets::flutter::TabBar::viewName() const
void *c_KDDockWidgets__flutter__TabBar__viewName(void *thisObj);
// KDDockWidgets::flutter::TabBar::zOrder() const
int c_KDDockWidgets__flutter__TabBar__zOrder(void *thisObj);
void c_KDDockWidgets__flutter__TabBar__destructor(void *thisObj);
void c_KDDockWidgets__flutter__TabBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__TabBar_Finalizer(void *cppObj); // KDDockWidgets::flutter::Stack::Stack(KDDockWidgets::Core::Stack * controller, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__Stack__constructor_Stack_View(void *controller_, void *parent_);
// KDDockWidgets::flutter::Stack::activateWindow()
void c_KDDockWidgets__flutter__Stack__activateWindow(void *thisObj);
// KDDockWidgets::flutter::Stack::close()
bool c_KDDockWidgets__flutter__Stack__close(void *thisObj);
// KDDockWidgets::flutter::Stack::createPlatformWindow()
void c_KDDockWidgets__flutter__Stack__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::Stack::flags() const
int c_KDDockWidgets__flutter__Stack__flags(void *thisObj);
// KDDockWidgets::flutter::Stack::geometry() const
void *c_KDDockWidgets__flutter__Stack__geometry(void *thisObj);
// KDDockWidgets::flutter::Stack::grabMouse()
void c_KDDockWidgets__flutter__Stack__grabMouse(void *thisObj);
// KDDockWidgets::flutter::Stack::hasFocus() const
bool c_KDDockWidgets__flutter__Stack__hasFocus(void *thisObj);
// KDDockWidgets::flutter::Stack::hide()
void c_KDDockWidgets__flutter__Stack__hide(void *thisObj);
// KDDockWidgets::flutter::Stack::init()
void c_KDDockWidgets__flutter__Stack__init(void *thisObj);
// KDDockWidgets::flutter::Stack::isActiveWindow() const
bool c_KDDockWidgets__flutter__Stack__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::Stack::isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__Stack__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::Stack::isMaximized() const
bool c_KDDockWidgets__flutter__Stack__isMaximized(void *thisObj);
// KDDockWidgets::flutter::Stack::isMinimized() const
bool c_KDDockWidgets__flutter__Stack__isMinimized(void *thisObj);
// KDDockWidgets::flutter::Stack::isMounted() const
bool c_KDDockWidgets__flutter__Stack__isMounted(void *thisObj);
// KDDockWidgets::flutter::Stack::isNull() const
bool c_KDDockWidgets__flutter__Stack__isNull(void *thisObj);
// KDDockWidgets::flutter::Stack::isPositionDraggable(KDDockWidgets::Point p) const
bool c_KDDockWidgets__flutter__Stack__isPositionDraggable_Point(void *thisObj, void *p_);
// KDDockWidgets::flutter::Stack::isRootView() const
bool c_KDDockWidgets__flutter__Stack__isRootView(void *thisObj);
// KDDockWidgets::flutter::Stack::isVisible() const
bool c_KDDockWidgets__flutter__Stack__isVisible(void *thisObj);
// KDDockWidgets::flutter::Stack::mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__Stack__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::Stack::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__Stack__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::Stack::mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__Stack__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::Stack::maxSizeHint() const
void *c_KDDockWidgets__flutter__Stack__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::Stack::minSize() const
void *c_KDDockWidgets__flutter__Stack__minSize(void *thisObj);
// KDDockWidgets::flutter::Stack::move(int x, int y)
void c_KDDockWidgets__flutter__Stack__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::Stack::normalGeometry() const
void *c_KDDockWidgets__flutter__Stack__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::Stack::onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__Stack__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Stack::onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__Stack__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Stack::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__Stack__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Stack::onGeometryChanged()
void c_KDDockWidgets__flutter__Stack__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::Stack::onRebuildRequested()
void c_KDDockWidgets__flutter__Stack__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::Stack::onResize(int h, int w)
bool c_KDDockWidgets__flutter__Stack__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::Stack::raise()
void c_KDDockWidgets__flutter__Stack__raise(void *thisObj);
// KDDockWidgets::flutter::Stack::raiseAndActivate()
void c_KDDockWidgets__flutter__Stack__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::Stack::raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__Stack__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Stack::raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__Stack__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::Stack::releaseKeyboard()
void c_KDDockWidgets__flutter__Stack__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::Stack::releaseMouse()
void c_KDDockWidgets__flutter__Stack__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::Stack::setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__Stack__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::Stack::setDocumentMode(bool arg__1)
void c_KDDockWidgets__flutter__Stack__setDocumentMode_bool(void *thisObj, bool arg__1);
// KDDockWidgets::flutter::Stack::setFixedHeight(int h)
void c_KDDockWidgets__flutter__Stack__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::Stack::setFixedWidth(int w)
void c_KDDockWidgets__flutter__Stack__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::Stack::setGeometry(KDDockWidgets::Rect geometry)
void c_KDDockWidgets__flutter__Stack__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::Stack::setHeight(int h)
void c_KDDockWidgets__flutter__Stack__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::Stack::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__Stack__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::Stack::setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__Stack__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::Stack::setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__Stack__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::Stack::setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__Stack__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::Stack::setSize(int w, int h)
void c_KDDockWidgets__flutter__Stack__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::Stack::setViewName(const QString & name)
void c_KDDockWidgets__flutter__Stack__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::Stack::setVisible(bool visible)
void c_KDDockWidgets__flutter__Stack__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::Stack::setWidth(int w)
void c_KDDockWidgets__flutter__Stack__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::Stack::setWindowOpacity(double v)
void c_KDDockWidgets__flutter__Stack__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::Stack::setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__Stack__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::Stack::setZOrder(int z)
void c_KDDockWidgets__flutter__Stack__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::Stack::show()
void c_KDDockWidgets__flutter__Stack__show(void *thisObj);
// KDDockWidgets::flutter::Stack::showMaximized()
void c_KDDockWidgets__flutter__Stack__showMaximized(void *thisObj);
// KDDockWidgets::flutter::Stack::showMinimized()
void c_KDDockWidgets__flutter__Stack__showMinimized(void *thisObj);
// KDDockWidgets::flutter::Stack::showNormal()
void c_KDDockWidgets__flutter__Stack__showNormal(void *thisObj);
// KDDockWidgets::flutter::Stack::update()
void c_KDDockWidgets__flutter__Stack__update(void *thisObj);
// KDDockWidgets::flutter::Stack::viewName() const
void *c_KDDockWidgets__flutter__Stack__viewName(void *thisObj);
// KDDockWidgets::flutter::Stack::zOrder() const
int c_KDDockWidgets__flutter__Stack__zOrder(void *thisObj);
void c_KDDockWidgets__flutter__Stack__destructor(void *thisObj);
void c_KDDockWidgets__flutter__Stack__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__Stack_Finalizer(void *cppObj); // KDDockWidgets::flutter::MainWindow::MainWindow(const QString & uniqueName, QFlags<KDDockWidgets::MainWindowOption> options, KDDockWidgets::flutter::View * parent, Qt::WindowFlags flags)
void *c_KDDockWidgets__flutter__MainWindow__constructor_QString_MainWindowOptions_View_WindowFlags(const char *uniqueName_, int options_, void *parent_, int flags);
// KDDockWidgets::flutter::MainWindow::activateWindow()
void c_KDDockWidgets__flutter__MainWindow__activateWindow(void *thisObj);
// KDDockWidgets::flutter::MainWindow::centerWidgetMargins() const
void *c_KDDockWidgets__flutter__MainWindow__centerWidgetMargins(void *thisObj);
// KDDockWidgets::flutter::MainWindow::centralAreaGeometry() const
void *c_KDDockWidgets__flutter__MainWindow__centralAreaGeometry(void *thisObj);
// KDDockWidgets::flutter::MainWindow::close()
bool c_KDDockWidgets__flutter__MainWindow__close(void *thisObj);
// KDDockWidgets::flutter::MainWindow::createPlatformWindow()
void c_KDDockWidgets__flutter__MainWindow__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::MainWindow::flags() const
int c_KDDockWidgets__flutter__MainWindow__flags(void *thisObj);
// KDDockWidgets::flutter::MainWindow::geometry() const
void *c_KDDockWidgets__flutter__MainWindow__geometry(void *thisObj);
// KDDockWidgets::flutter::MainWindow::grabMouse()
void c_KDDockWidgets__flutter__MainWindow__grabMouse(void *thisObj);
// KDDockWidgets::flutter::MainWindow::hasFocus() const
bool c_KDDockWidgets__flutter__MainWindow__hasFocus(void *thisObj);
// KDDockWidgets::flutter::MainWindow::hide()
void c_KDDockWidgets__flutter__MainWindow__hide(void *thisObj);
// KDDockWidgets::flutter::MainWindow::init()
void c_KDDockWidgets__flutter__MainWindow__init(void *thisObj);
// KDDockWidgets::flutter::MainWindow::isActiveWindow() const
bool c_KDDockWidgets__flutter__MainWindow__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::MainWindow::isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__MainWindow__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::MainWindow::isMaximized() const
bool c_KDDockWidgets__flutter__MainWindow__isMaximized(void *thisObj);
// KDDockWidgets::flutter::MainWindow::isMinimized() const
bool c_KDDockWidgets__flutter__MainWindow__isMinimized(void *thisObj);
// KDDockWidgets::flutter::MainWindow::isMounted() const
bool c_KDDockWidgets__flutter__MainWindow__isMounted(void *thisObj);
// KDDockWidgets::flutter::MainWindow::isNull() const
bool c_KDDockWidgets__flutter__MainWindow__isNull(void *thisObj);
// KDDockWidgets::flutter::MainWindow::isRootView() const
bool c_KDDockWidgets__flutter__MainWindow__isRootView(void *thisObj);
// KDDockWidgets::flutter::MainWindow::isVisible() const
bool c_KDDockWidgets__flutter__MainWindow__isVisible(void *thisObj);
// KDDockWidgets::flutter::MainWindow::mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__MainWindow__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::MainWindow::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__MainWindow__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::MainWindow::mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__MainWindow__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::MainWindow::maxSizeHint() const
void *c_KDDockWidgets__flutter__MainWindow__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::MainWindow::minSize() const
void *c_KDDockWidgets__flutter__MainWindow__minSize(void *thisObj);
// KDDockWidgets::flutter::MainWindow::move(int x, int y)
void c_KDDockWidgets__flutter__MainWindow__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::MainWindow::normalGeometry() const
void *c_KDDockWidgets__flutter__MainWindow__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::MainWindow::onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__MainWindow__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::MainWindow::onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__MainWindow__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::MainWindow::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__MainWindow__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::MainWindow::onGeometryChanged()
void c_KDDockWidgets__flutter__MainWindow__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::MainWindow::onRebuildRequested()
void c_KDDockWidgets__flutter__MainWindow__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::MainWindow::onResize(int h, int w)
bool c_KDDockWidgets__flutter__MainWindow__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::MainWindow::raise()
void c_KDDockWidgets__flutter__MainWindow__raise(void *thisObj);
// KDDockWidgets::flutter::MainWindow::raiseAndActivate()
void c_KDDockWidgets__flutter__MainWindow__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::MainWindow::raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__MainWindow__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::MainWindow::raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__MainWindow__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::MainWindow::releaseKeyboard()
void c_KDDockWidgets__flutter__MainWindow__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::MainWindow::releaseMouse()
void c_KDDockWidgets__flutter__MainWindow__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::MainWindow::setContentsMargins(int left, int top, int right, int bottom)
void c_KDDockWidgets__flutter__MainWindow__setContentsMargins_int_int_int_int(void *thisObj, int left, int top, int right, int bottom);
// KDDockWidgets::flutter::MainWindow::setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__MainWindow__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::MainWindow::setFixedHeight(int h)
void c_KDDockWidgets__flutter__MainWindow__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::MainWindow::setFixedWidth(int w)
void c_KDDockWidgets__flutter__MainWindow__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::MainWindow::setGeometry(KDDockWidgets::Rect geometry)
void c_KDDockWidgets__flutter__MainWindow__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::MainWindow::setHeight(int h)
void c_KDDockWidgets__flutter__MainWindow__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::MainWindow::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__MainWindow__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::MainWindow::setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__MainWindow__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::MainWindow::setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__MainWindow__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::MainWindow::setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__MainWindow__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::MainWindow::setSize(int w, int h)
void c_KDDockWidgets__flutter__MainWindow__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::MainWindow::setViewName(const QString & name)
void c_KDDockWidgets__flutter__MainWindow__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::MainWindow::setVisible(bool visible)
void c_KDDockWidgets__flutter__MainWindow__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::MainWindow::setWidth(int w)
void c_KDDockWidgets__flutter__MainWindow__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::MainWindow::setWindowOpacity(double v)
void c_KDDockWidgets__flutter__MainWindow__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::MainWindow::setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__MainWindow__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::MainWindow::setZOrder(int z)
void c_KDDockWidgets__flutter__MainWindow__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::MainWindow::show()
void c_KDDockWidgets__flutter__MainWindow__show(void *thisObj);
// KDDockWidgets::flutter::MainWindow::showMaximized()
void c_KDDockWidgets__flutter__MainWindow__showMaximized(void *thisObj);
// KDDockWidgets::flutter::MainWindow::showMinimized()
void c_KDDockWidgets__flutter__MainWindow__showMinimized(void *thisObj);
// KDDockWidgets::flutter::MainWindow::showNormal()
void c_KDDockWidgets__flutter__MainWindow__showNormal(void *thisObj);
// KDDockWidgets::flutter::MainWindow::update()
void c_KDDockWidgets__flutter__MainWindow__update(void *thisObj);
// KDDockWidgets::flutter::MainWindow::viewName() const
void *c_KDDockWidgets__flutter__MainWindow__viewName(void *thisObj);
// KDDockWidgets::flutter::MainWindow::zOrder() const
int c_KDDockWidgets__flutter__MainWindow__zOrder(void *thisObj);
void c_KDDockWidgets__flutter__MainWindow__destructor(void *thisObj);
void c_KDDockWidgets__flutter__MainWindow__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__MainWindow_Finalizer(void *cppObj); // KDDockWidgets::flutter::Group::Group(KDDockWidgets::Core::Group * controller, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__Group__constructor_Group_View(void *controller_, void *parent_);
// KDDockWidgets::flutter::Group::activateWindow()
void c_KDDockWidgets__flutter__Group__activateWindow(void *thisObj);
// KDDockWidgets::flutter::Group::close()
bool c_KDDockWidgets__flutter__Group__close(void *thisObj);
// KDDockWidgets::flutter::Group::createPlatformWindow()
void c_KDDockWidgets__flutter__Group__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::Group::currentIndex() const
int c_KDDockWidgets__flutter__Group__currentIndex(void *thisObj);
// KDDockWidgets::flutter::Group::dragRect() const
void *c_KDDockWidgets__flutter__Group__dragRect(void *thisObj);
// KDDockWidgets::flutter::Group::flags() const
int c_KDDockWidgets__flutter__Group__flags(void *thisObj);
// KDDockWidgets::flutter::Group::geometry() const
void *c_KDDockWidgets__flutter__Group__geometry(void *thisObj);
// KDDockWidgets::flutter::Group::grabMouse()
void c_KDDockWidgets__flutter__Group__grabMouse(void *thisObj);
// KDDockWidgets::flutter::Group::hasFocus() const
bool c_KDDockWidgets__flutter__Group__hasFocus(void *thisObj);
// KDDockWidgets::flutter::Group::hide()
void c_KDDockWidgets__flutter__Group__hide(void *thisObj);
// KDDockWidgets::flutter::Group::init()
void c_KDDockWidgets__flutter__Group__init(void *thisObj);
// KDDockWidgets::flutter::Group::isActiveWindow() const
bool c_KDDockWidgets__flutter__Group__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::Group::isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__Group__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::Group::isMaximized() const
bool c_KDDockWidgets__flutter__Group__isMaximized(void *thisObj);
// KDDockWidgets::flutter::Group::isMinimized() const
bool c_KDDockWidgets__flutter__Group__isMinimized(void *thisObj);
// KDDockWidgets::flutter::Group::isMounted() const
bool c_KDDockWidgets__flutter__Group__isMounted(void *thisObj);
// KDDockWidgets::flutter::Group::isNull() const
bool c_KDDockWidgets__flutter__Group__isNull(void *thisObj);
// KDDockWidgets::flutter::Group::isRootView() const
bool c_KDDockWidgets__flutter__Group__isRootView(void *thisObj);
// KDDockWidgets::flutter::Group::isVisible() const
bool c_KDDockWidgets__flutter__Group__isVisible(void *thisObj);
// KDDockWidgets::flutter::Group::mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__Group__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::Group::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__Group__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::Group::mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__Group__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::Group::maxSizeHint() const
void *c_KDDockWidgets__flutter__Group__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::Group::minSize() const
void *c_KDDockWidgets__flutter__Group__minSize(void *thisObj);
// KDDockWidgets::flutter::Group::move(int x, int y)
void c_KDDockWidgets__flutter__Group__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::Group::nonContentsHeight() const
int c_KDDockWidgets__flutter__Group__nonContentsHeight(void *thisObj);
// KDDockWidgets::flutter::Group::normalGeometry() const
void *c_KDDockWidgets__flutter__Group__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::Group::onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__Group__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Group::onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__Group__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Group::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__Group__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Group::onGeometryChanged()
void c_KDDockWidgets__flutter__Group__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::Group::onRebuildRequested()
void c_KDDockWidgets__flutter__Group__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::Group::onResize(int h, int w)
bool c_KDDockWidgets__flutter__Group__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::Group::raise()
void c_KDDockWidgets__flutter__Group__raise(void *thisObj);
// KDDockWidgets::flutter::Group::raiseAndActivate()
void c_KDDockWidgets__flutter__Group__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::Group::raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__Group__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::Group::raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__Group__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::Group::releaseKeyboard()
void c_KDDockWidgets__flutter__Group__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::Group::releaseMouse()
void c_KDDockWidgets__flutter__Group__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::Group::setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__Group__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::Group::setFixedHeight(int h)
void c_KDDockWidgets__flutter__Group__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::Group::setFixedWidth(int w)
void c_KDDockWidgets__flutter__Group__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::Group::setGeometry(KDDockWidgets::Rect geometry)
void c_KDDockWidgets__flutter__Group__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::Group::setHeight(int h)
void c_KDDockWidgets__flutter__Group__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::Group::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__Group__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::Group::setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__Group__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::Group::setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__Group__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::Group::setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__Group__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::Group::setSize(int w, int h)
void c_KDDockWidgets__flutter__Group__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::Group::setViewName(const QString & name)
void c_KDDockWidgets__flutter__Group__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::Group::setVisible(bool visible)
void c_KDDockWidgets__flutter__Group__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::Group::setWidth(int w)
void c_KDDockWidgets__flutter__Group__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::Group::setWindowOpacity(double v)
void c_KDDockWidgets__flutter__Group__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::Group::setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__Group__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::Group::setZOrder(int z)
void c_KDDockWidgets__flutter__Group__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::Group::show()
void c_KDDockWidgets__flutter__Group__show(void *thisObj);
// KDDockWidgets::flutter::Group::showMaximized()
void c_KDDockWidgets__flutter__Group__showMaximized(void *thisObj);
// KDDockWidgets::flutter::Group::showMinimized()
void c_KDDockWidgets__flutter__Group__showMinimized(void *thisObj);
// KDDockWidgets::flutter::Group::showNormal()
void c_KDDockWidgets__flutter__Group__showNormal(void *thisObj);
// KDDockWidgets::flutter::Group::update()
void c_KDDockWidgets__flutter__Group__update(void *thisObj);
// KDDockWidgets::flutter::Group::viewName() const
void *c_KDDockWidgets__flutter__Group__viewName(void *thisObj);
// KDDockWidgets::flutter::Group::zOrder() const
int c_KDDockWidgets__flutter__Group__zOrder(void *thisObj);
void c_KDDockWidgets__flutter__Group__destructor(void *thisObj);
void c_KDDockWidgets__flutter__Group__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__Group_Finalizer(void *cppObj); // KDDockWidgets::flutter::DropArea::DropArea(KDDockWidgets::Core::DropArea * arg__1, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__DropArea__constructor_DropArea_View(void *arg__1_, void *parent_);
// KDDockWidgets::flutter::DropArea::activateWindow()
void c_KDDockWidgets__flutter__DropArea__activateWindow(void *thisObj);
// KDDockWidgets::flutter::DropArea::close()
bool c_KDDockWidgets__flutter__DropArea__close(void *thisObj);
// KDDockWidgets::flutter::DropArea::createPlatformWindow()
void c_KDDockWidgets__flutter__DropArea__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::DropArea::flags() const
int c_KDDockWidgets__flutter__DropArea__flags(void *thisObj);
// KDDockWidgets::flutter::DropArea::geometry() const
void *c_KDDockWidgets__flutter__DropArea__geometry(void *thisObj);
// KDDockWidgets::flutter::DropArea::grabMouse()
void c_KDDockWidgets__flutter__DropArea__grabMouse(void *thisObj);
// KDDockWidgets::flutter::DropArea::hasFocus() const
bool c_KDDockWidgets__flutter__DropArea__hasFocus(void *thisObj);
// KDDockWidgets::flutter::DropArea::hide()
void c_KDDockWidgets__flutter__DropArea__hide(void *thisObj);
// KDDockWidgets::flutter::DropArea::indicatorWindow() const
void *c_KDDockWidgets__flutter__DropArea__indicatorWindow(void *thisObj);
// KDDockWidgets::flutter::DropArea::init()
void c_KDDockWidgets__flutter__DropArea__init(void *thisObj);
// KDDockWidgets::flutter::DropArea::isActiveWindow() const
bool c_KDDockWidgets__flutter__DropArea__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::DropArea::isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__DropArea__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::DropArea::isMaximized() const
bool c_KDDockWidgets__flutter__DropArea__isMaximized(void *thisObj);
// KDDockWidgets::flutter::DropArea::isMinimized() const
bool c_KDDockWidgets__flutter__DropArea__isMinimized(void *thisObj);
// KDDockWidgets::flutter::DropArea::isMounted() const
bool c_KDDockWidgets__flutter__DropArea__isMounted(void *thisObj);
// KDDockWidgets::flutter::DropArea::isNull() const
bool c_KDDockWidgets__flutter__DropArea__isNull(void *thisObj);
// KDDockWidgets::flutter::DropArea::isRootView() const
bool c_KDDockWidgets__flutter__DropArea__isRootView(void *thisObj);
// KDDockWidgets::flutter::DropArea::isVisible() const
bool c_KDDockWidgets__flutter__DropArea__isVisible(void *thisObj);
// KDDockWidgets::flutter::DropArea::mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__DropArea__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::DropArea::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__DropArea__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::DropArea::mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__DropArea__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::DropArea::maxSizeHint() const
void *c_KDDockWidgets__flutter__DropArea__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::DropArea::minSize() const
void *c_KDDockWidgets__flutter__DropArea__minSize(void *thisObj);
// KDDockWidgets::flutter::DropArea::move(int x, int y)
void c_KDDockWidgets__flutter__DropArea__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::DropArea::normalGeometry() const
void *c_KDDockWidgets__flutter__DropArea__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::DropArea::onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DropArea__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::DropArea::onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DropArea__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::DropArea::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DropArea__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::DropArea::onGeometryChanged()
void c_KDDockWidgets__flutter__DropArea__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::DropArea::onRebuildRequested()
void c_KDDockWidgets__flutter__DropArea__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::DropArea::onResize(int h, int w)
bool c_KDDockWidgets__flutter__DropArea__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::DropArea::raise()
void c_KDDockWidgets__flutter__DropArea__raise(void *thisObj);
// KDDockWidgets::flutter::DropArea::raiseAndActivate()
void c_KDDockWidgets__flutter__DropArea__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::DropArea::raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DropArea__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::DropArea::raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__DropArea__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::DropArea::releaseKeyboard()
void c_KDDockWidgets__flutter__DropArea__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::DropArea::releaseMouse()
void c_KDDockWidgets__flutter__DropArea__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::DropArea::setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__DropArea__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::DropArea::setFixedHeight(int h)
void c_KDDockWidgets__flutter__DropArea__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::DropArea::setFixedWidth(int w)
void c_KDDockWidgets__flutter__DropArea__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::DropArea::setGeometry(KDDockWidgets::Rect geometry)
void c_KDDockWidgets__flutter__DropArea__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::DropArea::setHeight(int h)
void c_KDDockWidgets__flutter__DropArea__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::DropArea::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__DropArea__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::DropArea::setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__DropArea__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::DropArea::setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__DropArea__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::DropArea::setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__DropArea__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::DropArea::setSize(int w, int h)
void c_KDDockWidgets__flutter__DropArea__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::DropArea::setViewName(const QString & name)
void c_KDDockWidgets__flutter__DropArea__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::DropArea::setVisible(bool visible)
void c_KDDockWidgets__flutter__DropArea__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::DropArea::setWidth(int w)
void c_KDDockWidgets__flutter__DropArea__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::DropArea::setWindowOpacity(double v)
void c_KDDockWidgets__flutter__DropArea__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::DropArea::setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__DropArea__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::DropArea::setZOrder(int z)
void c_KDDockWidgets__flutter__DropArea__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::DropArea::show()
void c_KDDockWidgets__flutter__DropArea__show(void *thisObj);
// KDDockWidgets::flutter::DropArea::showMaximized()
void c_KDDockWidgets__flutter__DropArea__showMaximized(void *thisObj);
// KDDockWidgets::flutter::DropArea::showMinimized()
void c_KDDockWidgets__flutter__DropArea__showMinimized(void *thisObj);
// KDDockWidgets::flutter::DropArea::showNormal()
void c_KDDockWidgets__flutter__DropArea__showNormal(void *thisObj);
// KDDockWidgets::flutter::DropArea::update()
void c_KDDockWidgets__flutter__DropArea__update(void *thisObj);
// KDDockWidgets::flutter::DropArea::viewName() const
void *c_KDDockWidgets__flutter__DropArea__viewName(void *thisObj);
// KDDockWidgets::flutter::DropArea::zOrder() const
int c_KDDockWidgets__flutter__DropArea__zOrder(void *thisObj);
void c_KDDockWidgets__flutter__DropArea__destructor(void *thisObj);
void c_KDDockWidgets__flutter__DropArea__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__DropArea_Finalizer(void *cppObj); // KDDockWidgets::flutter::DockWidget::DockWidget(const QString & uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions)
void *c_KDDockWidgets__flutter__DockWidget__constructor_QString_DockWidgetOptions_LayoutSaverOptions(const char *uniqueName_, int options_, int layoutSaverOptions_);
// KDDockWidgets::flutter::DockWidget::activateWindow()
void c_KDDockWidgets__flutter__DockWidget__activateWindow(void *thisObj);
// KDDockWidgets::flutter::DockWidget::close()
bool c_KDDockWidgets__flutter__DockWidget__close(void *thisObj);
// KDDockWidgets::flutter::DockWidget::createPlatformWindow()
void c_KDDockWidgets__flutter__DockWidget__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::DockWidget::dockWidget() const
void *c_KDDockWidgets__flutter__DockWidget__dockWidget(void *thisObj);
// KDDockWidgets::flutter::DockWidget::flags() const
int c_KDDockWidgets__flutter__DockWidget__flags(void *thisObj);
// KDDockWidgets::flutter::DockWidget::geometry() const
void *c_KDDockWidgets__flutter__DockWidget__geometry(void *thisObj);
// KDDockWidgets::flutter::DockWidget::grabMouse()
void c_KDDockWidgets__flutter__DockWidget__grabMouse(void *thisObj);
// KDDockWidgets::flutter::DockWidget::hasFocus() const
bool c_KDDockWidgets__flutter__DockWidget__hasFocus(void *thisObj);
// KDDockWidgets::flutter::DockWidget::hide()
void c_KDDockWidgets__flutter__DockWidget__hide(void *thisObj);
// KDDockWidgets::flutter::DockWidget::init()
void c_KDDockWidgets__flutter__DockWidget__init(void *thisObj);
// KDDockWidgets::flutter::DockWidget::isActiveWindow() const
bool c_KDDockWidgets__flutter__DockWidget__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::DockWidget::isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__DockWidget__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::DockWidget::isMaximized() const
bool c_KDDockWidgets__flutter__DockWidget__isMaximized(void *thisObj);
// KDDockWidgets::flutter::DockWidget::isMinimized() const
bool c_KDDockWidgets__flutter__DockWidget__isMinimized(void *thisObj);
// KDDockWidgets::flutter::DockWidget::isMounted() const
bool c_KDDockWidgets__flutter__DockWidget__isMounted(void *thisObj);
// KDDockWidgets::flutter::DockWidget::isNull() const
bool c_KDDockWidgets__flutter__DockWidget__isNull(void *thisObj);
// KDDockWidgets::flutter::DockWidget::isRootView() const
bool c_KDDockWidgets__flutter__DockWidget__isRootView(void *thisObj);
// KDDockWidgets::flutter::DockWidget::isVisible() const
bool c_KDDockWidgets__flutter__DockWidget__isVisible(void *thisObj);
// KDDockWidgets::flutter::DockWidget::mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__DockWidget__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::DockWidget::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__DockWidget__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::DockWidget::mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__DockWidget__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::DockWidget::maxSizeHint() const
void *c_KDDockWidgets__flutter__DockWidget__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::DockWidget::minSize() const
void *c_KDDockWidgets__flutter__DockWidget__minSize(void *thisObj);
// KDDockWidgets::flutter::DockWidget::move(int x, int y)
void c_KDDockWidgets__flutter__DockWidget__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::DockWidget::normalGeometry() const
void *c_KDDockWidgets__flutter__DockWidget__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::DockWidget::onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DockWidget__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::DockWidget::onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DockWidget__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::DockWidget::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DockWidget__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::DockWidget::onGeometryChanged()
void c_KDDockWidgets__flutter__DockWidget__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::DockWidget::onRebuildRequested()
void c_KDDockWidgets__flutter__DockWidget__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::DockWidget::onResize(int h, int w)
bool c_KDDockWidgets__flutter__DockWidget__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::DockWidget::raise()
void c_KDDockWidgets__flutter__DockWidget__raise(void *thisObj);
// KDDockWidgets::flutter::DockWidget::raiseAndActivate()
void c_KDDockWidgets__flutter__DockWidget__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::DockWidget::raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__DockWidget__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::DockWidget::raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__DockWidget__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::DockWidget::releaseKeyboard()
void c_KDDockWidgets__flutter__DockWidget__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::DockWidget::releaseMouse()
void c_KDDockWidgets__flutter__DockWidget__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::DockWidget::setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__DockWidget__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::DockWidget::setFixedHeight(int h)
void c_KDDockWidgets__flutter__DockWidget__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::DockWidget::setFixedWidth(int w)
void c_KDDockWidgets__flutter__DockWidget__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::DockWidget::setGeometry(KDDockWidgets::Rect geometry)
void c_KDDockWidgets__flutter__DockWidget__setGeometry_Rect(void *thisObj, void *geometry_);
// KDDockWidgets::flutter::DockWidget::setHeight(int h)
void c_KDDockWidgets__flutter__DockWidget__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::DockWidget::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__DockWidget__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::DockWidget::setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__DockWidget__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::DockWidget::setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__DockWidget__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::DockWidget::setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__DockWidget__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::DockWidget::setSize(int w, int h)
void c_KDDockWidgets__flutter__DockWidget__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::DockWidget::setViewName(const QString & name)
void c_KDDockWidgets__flutter__DockWidget__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::DockWidget::setVisible(bool visible)
void c_KDDockWidgets__flutter__DockWidget__setVisible_bool(void *thisObj, bool visible);
// KDDockWidgets::flutter::DockWidget::setWidth(int w)
void c_KDDockWidgets__flutter__DockWidget__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::DockWidget::setWindowOpacity(double v)
void c_KDDockWidgets__flutter__DockWidget__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::DockWidget::setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__DockWidget__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::DockWidget::setZOrder(int z)
void c_KDDockWidgets__flutter__DockWidget__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::DockWidget::show()
void c_KDDockWidgets__flutter__DockWidget__show(void *thisObj);
// KDDockWidgets::flutter::DockWidget::showMaximized()
void c_KDDockWidgets__flutter__DockWidget__showMaximized(void *thisObj);
// KDDockWidgets::flutter::DockWidget::showMinimized()
void c_KDDockWidgets__flutter__DockWidget__showMinimized(void *thisObj);
// KDDockWidgets::flutter::DockWidget::showNormal()
void c_KDDockWidgets__flutter__DockWidget__showNormal(void *thisObj);
// KDDockWidgets::flutter::DockWidget::update()
void c_KDDockWidgets__flutter__DockWidget__update(void *thisObj);
// KDDockWidgets::flutter::DockWidget::viewName() const
void *c_KDDockWidgets__flutter__DockWidget__viewName(void *thisObj);
// KDDockWidgets::flutter::DockWidget::zOrder() const
int c_KDDockWidgets__flutter__DockWidget__zOrder(void *thisObj);
void c_KDDockWidgets__flutter__DockWidget__destructor(void *thisObj);
void c_KDDockWidgets__flutter__DockWidget__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__DockWidget_Finalizer(void *cppObj); // KDDockWidgets::Core::TitleBar::TitleBar(KDDockWidgets::Core::FloatingWindow * parent)
void *c_KDDockWidgets__Core__TitleBar__constructor_FloatingWindow(void *parent_);
// KDDockWidgets::Core::TitleBar::TitleBar(KDDockWidgets::Core::Group * parent)
void *c_KDDockWidgets__Core__TitleBar__constructor_Group(void *parent_);
// KDDockWidgets::Core::TitleBar::TitleBar(KDDockWidgets::Core::View * arg__1)
void *c_KDDockWidgets__Core__TitleBar__constructor_View(void *arg__1_);
// KDDockWidgets::Core::TitleBar::closeButtonEnabled() const
bool c_KDDockWidgets__Core__TitleBar__closeButtonEnabled(void *thisObj);
// KDDockWidgets::Core::TitleBar::floatButtonToolTip() const
void *c_KDDockWidgets__Core__TitleBar__floatButtonToolTip(void *thisObj);
// KDDockWidgets::Core::TitleBar::floatButtonVisible() const
bool c_KDDockWidgets__Core__TitleBar__floatButtonVisible(void *thisObj);
// KDDockWidgets::Core::TitleBar::floatingWindow() const
void *c_KDDockWidgets__Core__TitleBar__floatingWindow(void *thisObj);
// KDDockWidgets::Core::TitleBar::group() const
void *c_KDDockWidgets__Core__TitleBar__group(void *thisObj);
// KDDockWidgets::Core::TitleBar::hasIcon() const
bool c_KDDockWidgets__Core__TitleBar__hasIcon(void *thisObj);
// KDDockWidgets::Core::TitleBar::isCloseButtonEnabled() const
bool c_KDDockWidgets__Core__TitleBar__isCloseButtonEnabled(void *thisObj);
// KDDockWidgets::Core::TitleBar::isCloseButtonVisible() const
bool c_KDDockWidgets__Core__TitleBar__isCloseButtonVisible(void *thisObj);
// KDDockWidgets::Core::TitleBar::isFloatButtonVisible() const
bool c_KDDockWidgets__Core__TitleBar__isFloatButtonVisible(void *thisObj);
// KDDockWidgets::Core::TitleBar::isFloating() const
bool c_KDDockWidgets__Core__TitleBar__isFloating(void *thisObj);
// KDDockWidgets::Core::TitleBar::isFocused() const
bool c_KDDockWidgets__Core__TitleBar__isFocused(void *thisObj);
// KDDockWidgets::Core::TitleBar::isMDI() const
bool c_KDDockWidgets__Core__TitleBar__isMDI(void *thisObj);
// KDDockWidgets::Core::TitleBar::isOverlayed() const
bool c_KDDockWidgets__Core__TitleBar__isOverlayed(void *thisObj);
// KDDockWidgets::Core::TitleBar::isStandalone() const
bool c_KDDockWidgets__Core__TitleBar__isStandalone(void *thisObj);
// KDDockWidgets::Core::TitleBar::isWindow() const
bool c_KDDockWidgets__Core__TitleBar__isWindow(void *thisObj);
// KDDockWidgets::Core::TitleBar::mainWindow() const
void *c_KDDockWidgets__Core__TitleBar__mainWindow(void *thisObj);
// KDDockWidgets::Core::TitleBar::maximizeButtonVisible() const
bool c_KDDockWidgets__Core__TitleBar__maximizeButtonVisible(void *thisObj);
// KDDockWidgets::Core::TitleBar::onAutoHideClicked()
void c_KDDockWidgets__Core__TitleBar__onAutoHideClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onCloseClicked()
void c_KDDockWidgets__Core__TitleBar__onCloseClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onDoubleClicked()
bool c_KDDockWidgets__Core__TitleBar__onDoubleClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onFloatClicked()
void c_KDDockWidgets__Core__TitleBar__onFloatClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onMaximizeClicked()
void c_KDDockWidgets__Core__TitleBar__onMaximizeClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::onMinimizeClicked()
void c_KDDockWidgets__Core__TitleBar__onMinimizeClicked(void *thisObj);
// KDDockWidgets::Core::TitleBar::setCloseButtonEnabled(bool arg__1)
void c_KDDockWidgets__Core__TitleBar__setCloseButtonEnabled_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::TitleBar::setCloseButtonVisible(bool arg__1)
void c_KDDockWidgets__Core__TitleBar__setCloseButtonVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::TitleBar::setFloatButtonVisible(bool arg__1)
void c_KDDockWidgets__Core__TitleBar__setFloatButtonVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::TitleBar::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__TitleBar__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::TitleBar::setTitle(const QString & title)
void c_KDDockWidgets__Core__TitleBar__setTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::Core::TitleBar::singleDockWidget() const
void *c_KDDockWidgets__Core__TitleBar__singleDockWidget(void *thisObj);
// KDDockWidgets::Core::TitleBar::supportsAutoHideButton() const
bool c_KDDockWidgets__Core__TitleBar__supportsAutoHideButton(void *thisObj);
// KDDockWidgets::Core::TitleBar::supportsFloatingButton() const
bool c_KDDockWidgets__Core__TitleBar__supportsFloatingButton(void *thisObj);
// KDDockWidgets::Core::TitleBar::supportsMaximizeButton() const
bool c_KDDockWidgets__Core__TitleBar__supportsMaximizeButton(void *thisObj);
// KDDockWidgets::Core::TitleBar::supportsMinimizeButton() const
bool c_KDDockWidgets__Core__TitleBar__supportsMinimizeButton(void *thisObj);
// KDDockWidgets::Core::TitleBar::tabBar() const
void *c_KDDockWidgets__Core__TitleBar__tabBar(void *thisObj);
// KDDockWidgets::Core::TitleBar::title() const
void *c_KDDockWidgets__Core__TitleBar__title(void *thisObj);
// KDDockWidgets::Core::TitleBar::titleBarIsFocusable() const
bool c_KDDockWidgets__Core__TitleBar__titleBarIsFocusable(void *thisObj);
// KDDockWidgets::Core::TitleBar::toggleMaximized()
void c_KDDockWidgets__Core__TitleBar__toggleMaximized(void *thisObj);
// KDDockWidgets::Core::TitleBar::updateButtons()
void c_KDDockWidgets__Core__TitleBar__updateButtons(void *thisObj);
void c_KDDockWidgets__Core__TitleBar__destructor(void *thisObj);
void c_KDDockWidgets__Core__TitleBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__TitleBar_Finalizer(void *cppObj); // KDDockWidgets::Core::TabBar::TabBar(KDDockWidgets::Core::Stack * tabWidget)
void *c_KDDockWidgets__Core__TabBar__constructor_Stack(void *tabWidget_);
// KDDockWidgets::Core::TabBar::currentDockWidget() const
void *c_KDDockWidgets__Core__TabBar__currentDockWidget(void *thisObj);
// KDDockWidgets::Core::TabBar::currentIndex() const
int c_KDDockWidgets__Core__TabBar__currentIndex(void *thisObj);
// KDDockWidgets::Core::TabBar::dockWidgetAt(KDDockWidgets::Point localPos) const
void *c_KDDockWidgets__Core__TabBar__dockWidgetAt_Point(void *thisObj, void *localPos_);
// KDDockWidgets::Core::TabBar::dockWidgetAt(int index) const
void *c_KDDockWidgets__Core__TabBar__dockWidgetAt_int(void *thisObj, int index);
// KDDockWidgets::Core::TabBar::dragCanStart(KDDockWidgets::Point pressPos, KDDockWidgets::Point pos) const
bool c_KDDockWidgets__Core__TabBar__dragCanStart_Point_Point(void *thisObj, void *pressPos_, void *pos_);
// KDDockWidgets::Core::TabBar::group() const
void *c_KDDockWidgets__Core__TabBar__group(void *thisObj);
// KDDockWidgets::Core::TabBar::hasSingleDockWidget() const
bool c_KDDockWidgets__Core__TabBar__hasSingleDockWidget(void *thisObj);
// KDDockWidgets::Core::TabBar::indexOfDockWidget(const KDDockWidgets::Core::DockWidget * dw) const
int c_KDDockWidgets__Core__TabBar__indexOfDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::TabBar::isMDI() const
bool c_KDDockWidgets__Core__TabBar__isMDI(void *thisObj);
// KDDockWidgets::Core::TabBar::isMovingTab() const
bool c_KDDockWidgets__Core__TabBar__isMovingTab(void *thisObj);
// KDDockWidgets::Core::TabBar::isWindow() const
bool c_KDDockWidgets__Core__TabBar__isWindow(void *thisObj);
// KDDockWidgets::Core::TabBar::moveTabTo(int from, int to)
void c_KDDockWidgets__Core__TabBar__moveTabTo_int_int(void *thisObj, int from, int to);
// KDDockWidgets::Core::TabBar::numDockWidgets() const
int c_KDDockWidgets__Core__TabBar__numDockWidgets(void *thisObj);
// KDDockWidgets::Core::TabBar::onMouseDoubleClick(KDDockWidgets::Point localPos)
void c_KDDockWidgets__Core__TabBar__onMouseDoubleClick_Point(void *thisObj, void *localPos_);
// KDDockWidgets::Core::TabBar::onMousePress(KDDockWidgets::Point localPos)
void c_KDDockWidgets__Core__TabBar__onMousePress_Point(void *thisObj, void *localPos_);
// KDDockWidgets::Core::TabBar::rectForTab(int index) const
void *c_KDDockWidgets__Core__TabBar__rectForTab_int(void *thisObj, int index);
// KDDockWidgets::Core::TabBar::removeDockWidget(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__TabBar__removeDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::TabBar::renameTab(int index, const QString & arg__2)
void c_KDDockWidgets__Core__TabBar__renameTab_int_QString(void *thisObj, int index, const char *arg__2_);
// KDDockWidgets::Core::TabBar::setCurrentDockWidget(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__TabBar__setCurrentDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::TabBar::setCurrentIndex(int index)
void c_KDDockWidgets__Core__TabBar__setCurrentIndex_int(void *thisObj, int index);
// KDDockWidgets::Core::TabBar::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__TabBar__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::TabBar::singleDockWidget() const
void *c_KDDockWidgets__Core__TabBar__singleDockWidget(void *thisObj);
// KDDockWidgets::Core::TabBar::stack() const
void *c_KDDockWidgets__Core__TabBar__stack(void *thisObj);
// KDDockWidgets::Core::TabBar::tabsAreMovable() const
bool c_KDDockWidgets__Core__TabBar__tabsAreMovable(void *thisObj);
// KDDockWidgets::Core::TabBar::text(int index) const
void *c_KDDockWidgets__Core__TabBar__text_int(void *thisObj, int index);
void c_KDDockWidgets__Core__TabBar__destructor(void *thisObj);
void c_KDDockWidgets__Core__TabBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__TabBar_Finalizer(void *cppObj); // KDDockWidgets::Core::Stack::addDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__Core__Stack__addDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Stack::contains(KDDockWidgets::Core::DockWidget * dw) const
bool c_KDDockWidgets__Core__Stack__contains_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::Stack::group() const
void *c_KDDockWidgets__Core__Stack__group(void *thisObj);
// KDDockWidgets::Core::Stack::insertDockWidget(KDDockWidgets::Core::DockWidget * dockwidget, int index)
bool c_KDDockWidgets__Core__Stack__insertDockWidget_DockWidget_int(void *thisObj, void *dockwidget_, int index);
// KDDockWidgets::Core::Stack::isMDI() const
bool c_KDDockWidgets__Core__Stack__isMDI(void *thisObj);
// KDDockWidgets::Core::Stack::isPositionDraggable(KDDockWidgets::Point p) const
bool c_KDDockWidgets__Core__Stack__isPositionDraggable_Point(void *thisObj, void *p_);
// KDDockWidgets::Core::Stack::isWindow() const
bool c_KDDockWidgets__Core__Stack__isWindow(void *thisObj);
// KDDockWidgets::Core::Stack::numDockWidgets() const
int c_KDDockWidgets__Core__Stack__numDockWidgets(void *thisObj);
// KDDockWidgets::Core::Stack::onMouseDoubleClick(KDDockWidgets::Point localPos)
bool c_KDDockWidgets__Core__Stack__onMouseDoubleClick_Point(void *thisObj, void *localPos_);
// KDDockWidgets::Core::Stack::setDocumentMode(bool arg__1)
void c_KDDockWidgets__Core__Stack__setDocumentMode_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Stack::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__Stack__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Stack::setTabBarAutoHide(bool arg__1)
void c_KDDockWidgets__Core__Stack__setTabBarAutoHide_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::Stack::singleDockWidget() const
void *c_KDDockWidgets__Core__Stack__singleDockWidget(void *thisObj);
// KDDockWidgets::Core::Stack::tabBar() const
void *c_KDDockWidgets__Core__Stack__tabBar(void *thisObj);
// KDDockWidgets::Core::Stack::tabBarAutoHide() const
bool c_KDDockWidgets__Core__Stack__tabBarAutoHide(void *thisObj);
void c_KDDockWidgets__Core__Stack__destructor(void *thisObj);
void c_KDDockWidgets__Core__Stack__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__Stack_Finalizer(void *cppObj); // KDDockWidgets::Core::SideBar::addDockWidget(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__SideBar__addDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::SideBar::clear()
void c_KDDockWidgets__Core__SideBar__clear(void *thisObj);
// KDDockWidgets::Core::SideBar::containsDockWidget(KDDockWidgets::Core::DockWidget * arg__1) const
bool c_KDDockWidgets__Core__SideBar__containsDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::SideBar::isEmpty() const
bool c_KDDockWidgets__Core__SideBar__isEmpty(void *thisObj);
// KDDockWidgets::Core::SideBar::isVertical() const
bool c_KDDockWidgets__Core__SideBar__isVertical(void *thisObj);
// KDDockWidgets::Core::SideBar::mainWindow() const
void *c_KDDockWidgets__Core__SideBar__mainWindow(void *thisObj);
// KDDockWidgets::Core::SideBar::onButtonClicked(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__SideBar__onButtonClicked_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::SideBar::removeDockWidget(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__SideBar__removeDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::SideBar::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__SideBar__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::SideBar::toggleOverlay(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__Core__SideBar__toggleOverlay_DockWidget(void *thisObj, void *arg__1_);
void c_KDDockWidgets__Core__SideBar__destructor(void *thisObj);
void c_KDDockWidgets__Core__SideBar__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__SideBar_Finalizer(void *cppObj); // KDDockWidgets::Core::Separator::isResizing()
bool c_static_KDDockWidgets__Core__Separator__isResizing();
// KDDockWidgets::Core::Separator::isVertical() const
bool c_KDDockWidgets__Core__Separator__isVertical(void *thisObj);
// KDDockWidgets::Core::Separator::numSeparators()
int c_static_KDDockWidgets__Core__Separator__numSeparators();
// KDDockWidgets::Core::Separator::onMouseDoubleClick()
void c_KDDockWidgets__Core__Separator__onMouseDoubleClick(void *thisObj);
// KDDockWidgets::Core::Separator::onMouseMove(KDDockWidgets::Point pos)
void c_KDDockWidgets__Core__Separator__onMouseMove_Point(void *thisObj, void *pos_);
// KDDockWidgets::Core::Separator::onMousePress()
void c_KDDockWidgets__Core__Separator__onMousePress(void *thisObj);
// KDDockWidgets::Core::Separator::onMouseReleased()
void c_KDDockWidgets__Core__Separator__onMouseReleased(void *thisObj);
// KDDockWidgets::Core::Separator::position() const
int c_KDDockWidgets__Core__Separator__position(void *thisObj);
// KDDockWidgets::Core::Separator::setGeometry(KDDockWidgets::Rect r)
void c_KDDockWidgets__Core__Separator__setGeometry_Rect(void *thisObj, void *r_);
// KDDockWidgets::Core::Separator::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__Separator__setParentView_impl_View(void *thisObj, void *parent_);
void c_KDDockWidgets__Core__Separator__destructor(void *thisObj);
void c_KDDockWidgets__Core__Separator__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__Separator_Finalizer(void *cppObj); // KDDockWidgets::Core::MainWindow::MainWindow(KDDockWidgets::Core::View * view, const QString & uniqueName, QFlags<KDDockWidgets::MainWindowOption> options)
void *c_KDDockWidgets__Core__MainWindow__constructor_View_QString_MainWindowOptions(void *view_, const char *uniqueName_, int options_);
// KDDockWidgets::Core::MainWindow::addDockWidget(KDDockWidgets::Core::DockWidget * dockWidget, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__MainWindow__addDockWidget_DockWidget_Location_DockWidget_InitialOption(void *thisObj, void *dockWidget_, int location, void *relativeTo_, void *initialOption_);
// KDDockWidgets::Core::MainWindow::addDockWidgetAsTab(KDDockWidgets::Core::DockWidget * dockwidget)
void c_KDDockWidgets__Core__MainWindow__addDockWidgetAsTab_DockWidget(void *thisObj, void *dockwidget_);
// KDDockWidgets::Core::MainWindow::addDockWidgetToSide(KDDockWidgets::Core::DockWidget * dockWidget, KDDockWidgets::Location location, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__MainWindow__addDockWidgetToSide_DockWidget_Location_InitialOption(void *thisObj, void *dockWidget_, int location, void *initialOption_);
// KDDockWidgets::Core::MainWindow::anySideBarIsVisible() const
bool c_KDDockWidgets__Core__MainWindow__anySideBarIsVisible(void *thisObj);
// KDDockWidgets::Core::MainWindow::centerWidgetMargins() const
void *c_KDDockWidgets__Core__MainWindow__centerWidgetMargins(void *thisObj);
// KDDockWidgets::Core::MainWindow::centralAreaGeometry() const
void *c_KDDockWidgets__Core__MainWindow__centralAreaGeometry(void *thisObj);
// KDDockWidgets::Core::MainWindow::clearSideBarOverlay(bool deleteGroup)
void c_KDDockWidgets__Core__MainWindow__clearSideBarOverlay_bool(void *thisObj, bool deleteGroup);
// KDDockWidgets::Core::MainWindow::closeDockWidgets(bool force)
bool c_KDDockWidgets__Core__MainWindow__closeDockWidgets_bool(void *thisObj, bool force);
// KDDockWidgets::Core::MainWindow::dropArea() const
void *c_KDDockWidgets__Core__MainWindow__dropArea(void *thisObj);
// KDDockWidgets::Core::MainWindow::init(const QString & name)
void c_KDDockWidgets__Core__MainWindow__init_QString(void *thisObj, const char *name_);
// KDDockWidgets::Core::MainWindow::isMDI() const
bool c_KDDockWidgets__Core__MainWindow__isMDI(void *thisObj);
// KDDockWidgets::Core::MainWindow::layout() const
void *c_KDDockWidgets__Core__MainWindow__layout(void *thisObj);
// KDDockWidgets::Core::MainWindow::layoutEqually()
void c_KDDockWidgets__Core__MainWindow__layoutEqually(void *thisObj);
// KDDockWidgets::Core::MainWindow::layoutParentContainerEqually(KDDockWidgets::Core::DockWidget * dockWidget)
void c_KDDockWidgets__Core__MainWindow__layoutParentContainerEqually_DockWidget(void *thisObj, void *dockWidget_);
// KDDockWidgets::Core::MainWindow::moveToSideBar(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__MainWindow__moveToSideBar_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::MainWindow::multiSplitter() const
void *c_KDDockWidgets__Core__MainWindow__multiSplitter(void *thisObj);
// KDDockWidgets::Core::MainWindow::options() const
int c_KDDockWidgets__Core__MainWindow__options(void *thisObj);
// KDDockWidgets::Core::MainWindow::overlayMargin() const
int c_KDDockWidgets__Core__MainWindow__overlayMargin(void *thisObj);
// KDDockWidgets::Core::MainWindow::overlayOnSideBar(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__MainWindow__overlayOnSideBar_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::MainWindow::overlayedDockWidget() const
void *c_KDDockWidgets__Core__MainWindow__overlayedDockWidget(void *thisObj);
// KDDockWidgets::Core::MainWindow::restoreFromSideBar(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__MainWindow__restoreFromSideBar_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::MainWindow::setContentsMargins(int l, int t, int r, int b)
void c_KDDockWidgets__Core__MainWindow__setContentsMargins_int_int_int_int(void *thisObj, int l, int t, int r, int b);
// KDDockWidgets::Core::MainWindow::setOverlayMargin(int margin)
void c_KDDockWidgets__Core__MainWindow__setOverlayMargin_int(void *thisObj, int margin);
// KDDockWidgets::Core::MainWindow::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__MainWindow__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::MainWindow::setUniqueName(const QString & uniqueName)
void c_KDDockWidgets__Core__MainWindow__setUniqueName_QString(void *thisObj, const char *uniqueName_);
// KDDockWidgets::Core::MainWindow::sideBarForDockWidget(const KDDockWidgets::Core::DockWidget * dw) const
void *c_KDDockWidgets__Core__MainWindow__sideBarForDockWidget_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::MainWindow::toggleOverlayOnSideBar(KDDockWidgets::Core::DockWidget * dw)
void c_KDDockWidgets__Core__MainWindow__toggleOverlayOnSideBar_DockWidget(void *thisObj, void *dw_);
// KDDockWidgets::Core::MainWindow::uniqueName() const
void *c_KDDockWidgets__Core__MainWindow__uniqueName(void *thisObj);
void c_KDDockWidgets__Core__MainWindow__destructor(void *thisObj);
void c_KDDockWidgets__Core__MainWindow__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__MainWindow_Finalizer(void *cppObj); // KDDockWidgets::Core::Layout::Layout(KDDockWidgets::Core::ViewType arg__1, KDDockWidgets::Core::View * arg__2)
void *c_KDDockWidgets__Core__Layout__constructor_ViewType_View(int arg__1, void *arg__2_);
// KDDockWidgets::Core::Layout::asDropArea() const
void *c_KDDockWidgets__Core__Layout__asDropArea(void *thisObj);
// KDDockWidgets::Core::Layout::checkSanity() const
bool c_KDDockWidgets__Core__Layout__checkSanity(void *thisObj);
// KDDockWidgets::Core::Layout::clearLayout()
void c_KDDockWidgets__Core__Layout__clearLayout(void *thisObj);
// KDDockWidgets::Core::Layout::containsGroup(const KDDockWidgets::Core::Group * arg__1) const
bool c_KDDockWidgets__Core__Layout__containsGroup_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Layout::containsItem(const KDDockWidgets::Core::Item * arg__1) const
bool c_KDDockWidgets__Core__Layout__containsItem_Item(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Layout::count() const
int c_KDDockWidgets__Core__Layout__count(void *thisObj);
// KDDockWidgets::Core::Layout::dumpLayout() const
void c_KDDockWidgets__Core__Layout__dumpLayout(void *thisObj);
// KDDockWidgets::Core::Layout::floatingWindow() const
void *c_KDDockWidgets__Core__Layout__floatingWindow(void *thisObj);
// KDDockWidgets::Core::Layout::isInMainWindow(bool honourNesting) const
bool c_KDDockWidgets__Core__Layout__isInMainWindow_bool(void *thisObj, bool honourNesting);
// KDDockWidgets::Core::Layout::itemForGroup(const KDDockWidgets::Core::Group * group) const
void *c_KDDockWidgets__Core__Layout__itemForGroup_Group(void *thisObj, void *group_);
// KDDockWidgets::Core::Layout::layoutHeight() const
int c_KDDockWidgets__Core__Layout__layoutHeight(void *thisObj);
// KDDockWidgets::Core::Layout::layoutMaximumSizeHint() const
void *c_KDDockWidgets__Core__Layout__layoutMaximumSizeHint(void *thisObj);
// KDDockWidgets::Core::Layout::layoutMinimumSize() const
void *c_KDDockWidgets__Core__Layout__layoutMinimumSize(void *thisObj);
// KDDockWidgets::Core::Layout::layoutSize() const
void *c_KDDockWidgets__Core__Layout__layoutSize(void *thisObj);
// KDDockWidgets::Core::Layout::layoutWidth() const
int c_KDDockWidgets__Core__Layout__layoutWidth(void *thisObj);
// KDDockWidgets::Core::Layout::mainWindow(bool honourNesting) const
void *c_KDDockWidgets__Core__Layout__mainWindow_bool(void *thisObj, bool honourNesting);
// KDDockWidgets::Core::Layout::placeholderCount() const
int c_KDDockWidgets__Core__Layout__placeholderCount(void *thisObj);
// KDDockWidgets::Core::Layout::removeItem(KDDockWidgets::Core::Item * item)
void c_KDDockWidgets__Core__Layout__removeItem_Item(void *thisObj, void *item_);
// KDDockWidgets::Core::Layout::restorePlaceholder(KDDockWidgets::Core::DockWidget * dw, KDDockWidgets::Core::Item * arg__2, int tabIndex)
void c_KDDockWidgets__Core__Layout__restorePlaceholder_DockWidget_Item_int(void *thisObj, void *dw_, void *arg__2_, int tabIndex);
// KDDockWidgets::Core::Layout::setLayoutMinimumSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__Layout__setLayoutMinimumSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Layout::setLayoutSize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__Layout__setLayoutSize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Layout::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__Layout__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Layout::updateSizeConstraints()
void c_KDDockWidgets__Core__Layout__updateSizeConstraints(void *thisObj);
// KDDockWidgets::Core::Layout::viewAboutToBeDeleted()
void c_KDDockWidgets__Core__Layout__viewAboutToBeDeleted(void *thisObj);
// KDDockWidgets::Core::Layout::visibleCount() const
int c_KDDockWidgets__Core__Layout__visibleCount(void *thisObj);
void c_KDDockWidgets__Core__Layout__destructor(void *thisObj);
void c_KDDockWidgets__Core__Layout__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__Layout_Finalizer(void *cppObj); // KDDockWidgets::Core::DropArea::DropArea(KDDockWidgets::Core::View * parent, QFlags<KDDockWidgets::MainWindowOption> options, bool isMDIWrapper)
void *c_KDDockWidgets__Core__DropArea__constructor_View_MainWindowOptions_bool(void *parent_, int options_, bool isMDIWrapper);
// KDDockWidgets::Core::DropArea::_addDockWidget(KDDockWidgets::Core::DockWidget * dw, KDDockWidgets::Location location, KDDockWidgets::Core::Item * relativeTo, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__DropArea___addDockWidget_DockWidget_Location_Item_InitialOption(void *thisObj, void *dw_, int location, void *relativeTo_, void *initialOption_);
// KDDockWidgets::Core::DropArea::addDockWidget(KDDockWidgets::Core::DockWidget * dw, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__DropArea__addDockWidget_DockWidget_Location_DockWidget_InitialOption(void *thisObj, void *dw_, int location, void *relativeTo_, void *initialOption_);
// KDDockWidgets::Core::DropArea::addMultiSplitter(KDDockWidgets::Core::DropArea * splitter, KDDockWidgets::Location location, KDDockWidgets::Core::Group * relativeToGroup, KDDockWidgets::InitialOption option)
void c_KDDockWidgets__Core__DropArea__addMultiSplitter_DropArea_Location_Group_InitialOption(void *thisObj, void *splitter_, int location, void *relativeToGroup_, void *option_);
// KDDockWidgets::Core::DropArea::addWidget(KDDockWidgets::Core::View * widget, KDDockWidgets::Location location, KDDockWidgets::Core::Item * relativeToItem, KDDockWidgets::InitialOption option)
void c_KDDockWidgets__Core__DropArea__addWidget_View_Location_Item_InitialOption(void *thisObj, void *widget_, int location, void *relativeToItem_, void *option_);
// KDDockWidgets::Core::DropArea::centralFrame() const
void *c_KDDockWidgets__Core__DropArea__centralFrame(void *thisObj);
// KDDockWidgets::Core::DropArea::containsDockWidget(KDDockWidgets::Core::DockWidget * arg__1) const
bool c_KDDockWidgets__Core__DropArea__containsDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::DropArea::createCentralGroup(QFlags<KDDockWidgets::MainWindowOption> options)
void *c_static_KDDockWidgets__Core__DropArea__createCentralGroup_MainWindowOptions(int options_);
// KDDockWidgets::Core::DropArea::currentDropLocation() const
int c_KDDockWidgets__Core__DropArea__currentDropLocation(void *thisObj);
// KDDockWidgets::Core::DropArea::dropIndicatorOverlay() const
void *c_KDDockWidgets__Core__DropArea__dropIndicatorOverlay(void *thisObj);
// KDDockWidgets::Core::DropArea::hasSingleFloatingGroup() const
bool c_KDDockWidgets__Core__DropArea__hasSingleFloatingGroup(void *thisObj);
// KDDockWidgets::Core::DropArea::hasSingleGroup() const
bool c_KDDockWidgets__Core__DropArea__hasSingleGroup(void *thisObj);
// KDDockWidgets::Core::DropArea::isMDIWrapper() const
bool c_KDDockWidgets__Core__DropArea__isMDIWrapper(void *thisObj);
// KDDockWidgets::Core::DropArea::layoutEqually()
void c_KDDockWidgets__Core__DropArea__layoutEqually(void *thisObj);
// KDDockWidgets::Core::DropArea::layoutParentContainerEqually(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__Core__DropArea__layoutParentContainerEqually_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::DropArea::mdiDockWidgetWrapper() const
void *c_KDDockWidgets__Core__DropArea__mdiDockWidgetWrapper(void *thisObj);
// KDDockWidgets::Core::DropArea::removeHover()
void c_KDDockWidgets__Core__DropArea__removeHover(void *thisObj);
// KDDockWidgets::Core::DropArea::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__DropArea__setParentView_impl_View(void *thisObj, void *parent_);
void c_KDDockWidgets__Core__DropArea__destructor(void *thisObj);
void c_KDDockWidgets__Core__DropArea__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__DropArea_Finalizer(void *cppObj); // KDDockWidgets::Core::Group::Group(KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__Core__Group__constructor_View(void *parent_);
// KDDockWidgets::Core::Group::actualTitleBar() const
void *c_KDDockWidgets__Core__Group__actualTitleBar(void *thisObj);
// KDDockWidgets::Core::Group::addTab(KDDockWidgets::Core::DockWidget * arg__1, KDDockWidgets::InitialOption arg__2)
void c_KDDockWidgets__Core__Group__addTab_DockWidget_InitialOption(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::Core::Group::addTab(KDDockWidgets::Core::FloatingWindow * floatingWindow, KDDockWidgets::InitialOption arg__2)
void c_KDDockWidgets__Core__Group__addTab_FloatingWindow_InitialOption(void *thisObj, void *floatingWindow_, void *arg__2_);
// KDDockWidgets::Core::Group::addTab(KDDockWidgets::Core::Group * arg__1, KDDockWidgets::InitialOption arg__2)
void c_KDDockWidgets__Core__Group__addTab_Group_InitialOption(void *thisObj, void *arg__1_, void *arg__2_);
// KDDockWidgets::Core::Group::allDockWidgetsHave(KDDockWidgets::DockWidgetOption arg__1) const
bool c_KDDockWidgets__Core__Group__allDockWidgetsHave_DockWidgetOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::Group::allDockWidgetsHave(KDDockWidgets::LayoutSaverOption arg__1) const
bool c_KDDockWidgets__Core__Group__allDockWidgetsHave_LayoutSaverOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::Group::alwaysShowsTabs() const
bool c_KDDockWidgets__Core__Group__alwaysShowsTabs(void *thisObj);
// KDDockWidgets::Core::Group::anyDockWidgetsHas(KDDockWidgets::DockWidgetOption arg__1) const
bool c_KDDockWidgets__Core__Group__anyDockWidgetsHas_DockWidgetOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::Group::anyDockWidgetsHas(KDDockWidgets::LayoutSaverOption arg__1) const
bool c_KDDockWidgets__Core__Group__anyDockWidgetsHas_LayoutSaverOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::Group::anyNonClosable() const
bool c_KDDockWidgets__Core__Group__anyNonClosable(void *thisObj);
// KDDockWidgets::Core::Group::anyNonDockable() const
bool c_KDDockWidgets__Core__Group__anyNonDockable(void *thisObj);
// KDDockWidgets::Core::Group::beingDeletedLater() const
bool c_KDDockWidgets__Core__Group__beingDeletedLater(void *thisObj);
// KDDockWidgets::Core::Group::biggestDockWidgetMaxSize() const
void *c_KDDockWidgets__Core__Group__biggestDockWidgetMaxSize(void *thisObj);
// KDDockWidgets::Core::Group::containsDockWidget(KDDockWidgets::Core::DockWidget * w) const
bool c_KDDockWidgets__Core__Group__containsDockWidget_DockWidget(void *thisObj, void *w_);
// KDDockWidgets::Core::Group::containsMouse(KDDockWidgets::Point globalPos) const
bool c_KDDockWidgets__Core__Group__containsMouse_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::Group::currentDockWidget() const
void *c_KDDockWidgets__Core__Group__currentDockWidget(void *thisObj);
// KDDockWidgets::Core::Group::currentIndex() const
int c_KDDockWidgets__Core__Group__currentIndex(void *thisObj);
// KDDockWidgets::Core::Group::currentTabIndex() const
int c_KDDockWidgets__Core__Group__currentTabIndex(void *thisObj);
// KDDockWidgets::Core::Group::dbg_numFrames()
int c_static_KDDockWidgets__Core__Group__dbg_numFrames();
// KDDockWidgets::Core::Group::detachTab(KDDockWidgets::Core::DockWidget * arg__1)
void *c_KDDockWidgets__Core__Group__detachTab_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::dockWidgetAt(int index) const
void *c_KDDockWidgets__Core__Group__dockWidgetAt_int(void *thisObj, int index);
// KDDockWidgets::Core::Group::dockWidgetCount() const
int c_KDDockWidgets__Core__Group__dockWidgetCount(void *thisObj);
// KDDockWidgets::Core::Group::dockWidgetsMinSize() const
void *c_KDDockWidgets__Core__Group__dockWidgetsMinSize(void *thisObj);
// KDDockWidgets::Core::Group::dragRect() const
void *c_KDDockWidgets__Core__Group__dragRect(void *thisObj);
// KDDockWidgets::Core::Group::floatingWindow() const
void *c_KDDockWidgets__Core__Group__floatingWindow(void *thisObj);
// KDDockWidgets::Core::Group::focusedWidgetChangedCallback()
void c_KDDockWidgets__Core__Group__focusedWidgetChangedCallback(void *thisObj);
// KDDockWidgets::Core::Group::fromItem(const KDDockWidgets::Core::Item * arg__1)
void *c_static_KDDockWidgets__Core__Group__fromItem_Item(void *arg__1_);
// KDDockWidgets::Core::Group::hasNestedMDIDockWidgets() const
bool c_KDDockWidgets__Core__Group__hasNestedMDIDockWidgets(void *thisObj);
// KDDockWidgets::Core::Group::hasSingleDockWidget() const
bool c_KDDockWidgets__Core__Group__hasSingleDockWidget(void *thisObj);
// KDDockWidgets::Core::Group::hasTabsVisible() const
bool c_KDDockWidgets__Core__Group__hasTabsVisible(void *thisObj);
// KDDockWidgets::Core::Group::indexOfDockWidget(const KDDockWidgets::Core::DockWidget * arg__1)
int c_KDDockWidgets__Core__Group__indexOfDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::insertDockWidget(KDDockWidgets::Core::DockWidget * arg__1, int index)
void c_KDDockWidgets__Core__Group__insertDockWidget_DockWidget_int(void *thisObj, void *arg__1_, int index);
// KDDockWidgets::Core::Group::insertWidget(KDDockWidgets::Core::DockWidget * arg__1, int index, KDDockWidgets::InitialOption arg__3)
void c_KDDockWidgets__Core__Group__insertWidget_DockWidget_int_InitialOption(void *thisObj, void *arg__1_, int index, void *arg__3_);
// KDDockWidgets::Core::Group::isCentralGroup() const
bool c_KDDockWidgets__Core__Group__isCentralGroup(void *thisObj);
// KDDockWidgets::Core::Group::isDockable() const
bool c_KDDockWidgets__Core__Group__isDockable(void *thisObj);
// KDDockWidgets::Core::Group::isEmpty() const
bool c_KDDockWidgets__Core__Group__isEmpty(void *thisObj);
// KDDockWidgets::Core::Group::isFloating() const
bool c_KDDockWidgets__Core__Group__isFloating(void *thisObj);
// KDDockWidgets::Core::Group::isFocusedChangedCallback()
void c_KDDockWidgets__Core__Group__isFocusedChangedCallback(void *thisObj);
// KDDockWidgets::Core::Group::isInFloatingWindow() const
bool c_KDDockWidgets__Core__Group__isInFloatingWindow(void *thisObj);
// KDDockWidgets::Core::Group::isInMainWindow() const
bool c_KDDockWidgets__Core__Group__isInMainWindow(void *thisObj);
// KDDockWidgets::Core::Group::isMDI() const
bool c_KDDockWidgets__Core__Group__isMDI(void *thisObj);
// KDDockWidgets::Core::Group::isMDIWrapper() const
bool c_KDDockWidgets__Core__Group__isMDIWrapper(void *thisObj);
// KDDockWidgets::Core::Group::isOverlayed() const
bool c_KDDockWidgets__Core__Group__isOverlayed(void *thisObj);
// KDDockWidgets::Core::Group::isTheOnlyGroup() const
bool c_KDDockWidgets__Core__Group__isTheOnlyGroup(void *thisObj);
// KDDockWidgets::Core::Group::layoutItem() const
void *c_KDDockWidgets__Core__Group__layoutItem(void *thisObj);
// KDDockWidgets::Core::Group::mainWindow() const
void *c_KDDockWidgets__Core__Group__mainWindow(void *thisObj);
// KDDockWidgets::Core::Group::mdiDockWidgetWrapper() const
void *c_KDDockWidgets__Core__Group__mdiDockWidgetWrapper(void *thisObj);
// KDDockWidgets::Core::Group::mdiDropAreaWrapper() const
void *c_KDDockWidgets__Core__Group__mdiDropAreaWrapper(void *thisObj);
// KDDockWidgets::Core::Group::mdiFrame() const
void *c_KDDockWidgets__Core__Group__mdiFrame(void *thisObj);
// KDDockWidgets::Core::Group::nonContentsHeight() const
int c_KDDockWidgets__Core__Group__nonContentsHeight(void *thisObj);
// KDDockWidgets::Core::Group::onDockWidgetCountChanged()
void c_KDDockWidgets__Core__Group__onDockWidgetCountChanged(void *thisObj);
// KDDockWidgets::Core::Group::onDockWidgetTitleChanged(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__Core__Group__onDockWidgetTitleChanged_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::removeWidget(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__Core__Group__removeWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::renameTab(int index, const QString & arg__2)
void c_KDDockWidgets__Core__Group__renameTab_int_QString(void *thisObj, int index, const char *arg__2_);
// KDDockWidgets::Core::Group::restoreToPreviousPosition()
void c_KDDockWidgets__Core__Group__restoreToPreviousPosition(void *thisObj);
// KDDockWidgets::Core::Group::setCurrentDockWidget(KDDockWidgets::Core::DockWidget * arg__1)
void c_KDDockWidgets__Core__Group__setCurrentDockWidget_DockWidget(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::setCurrentTabIndex(int index)
void c_KDDockWidgets__Core__Group__setCurrentTabIndex_int(void *thisObj, int index);
// KDDockWidgets::Core::Group::setLayout(KDDockWidgets::Core::Layout * arg__1)
void c_KDDockWidgets__Core__Group__setLayout_Layout(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::Group::setLayoutItem(KDDockWidgets::Core::Item * item)
void c_KDDockWidgets__Core__Group__setLayoutItem_Item(void *thisObj, void *item_);
// KDDockWidgets::Core::Group::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__Group__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::Group::stack() const
void *c_KDDockWidgets__Core__Group__stack(void *thisObj);
// KDDockWidgets::Core::Group::tabBar() const
void *c_KDDockWidgets__Core__Group__tabBar(void *thisObj);
// KDDockWidgets::Core::Group::title() const
void *c_KDDockWidgets__Core__Group__title(void *thisObj);
// KDDockWidgets::Core::Group::titleBar() const
void *c_KDDockWidgets__Core__Group__titleBar(void *thisObj);
// KDDockWidgets::Core::Group::unoverlay()
void c_KDDockWidgets__Core__Group__unoverlay(void *thisObj);
// KDDockWidgets::Core::Group::updateFloatingActions()
void c_KDDockWidgets__Core__Group__updateFloatingActions(void *thisObj);
// KDDockWidgets::Core::Group::updateTitleAndIcon()
void c_KDDockWidgets__Core__Group__updateTitleAndIcon(void *thisObj);
// KDDockWidgets::Core::Group::updateTitleBarVisibility()
void c_KDDockWidgets__Core__Group__updateTitleBarVisibility(void *thisObj);
// KDDockWidgets::Core::Group::userType() const
int c_KDDockWidgets__Core__Group__userType(void *thisObj);
void c_KDDockWidgets__Core__Group__destructor(void *thisObj);
void c_KDDockWidgets__Core__Group__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__Group_Finalizer(void *cppObj); // KDDockWidgets::Core::FloatingWindow::FloatingWindow(KDDockWidgets::Core::Group * group, KDDockWidgets::Rect suggestedGeometry, KDDockWidgets::Core::MainWindow * parent)
void *c_KDDockWidgets__Core__FloatingWindow__constructor_Group_Rect_MainWindow(void *group_, void *suggestedGeometry_, void *parent_);
// KDDockWidgets::Core::FloatingWindow::FloatingWindow(KDDockWidgets::Rect suggestedGeometry, KDDockWidgets::Core::MainWindow * parent)
void *c_KDDockWidgets__Core__FloatingWindow__constructor_Rect_MainWindow(void *suggestedGeometry_, void *parent_);
// KDDockWidgets::Core::FloatingWindow::addDockWidget(KDDockWidgets::Core::DockWidget * arg__1, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption arg__4)
void c_KDDockWidgets__Core__FloatingWindow__addDockWidget_DockWidget_Location_DockWidget_InitialOption(void *thisObj, void *arg__1_, int location, void *relativeTo_, void *arg__4_);
// KDDockWidgets::Core::FloatingWindow::allDockWidgetsHave(KDDockWidgets::DockWidgetOption arg__1) const
bool c_KDDockWidgets__Core__FloatingWindow__allDockWidgetsHave_DockWidgetOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::FloatingWindow::allDockWidgetsHave(KDDockWidgets::LayoutSaverOption arg__1) const
bool c_KDDockWidgets__Core__FloatingWindow__allDockWidgetsHave_LayoutSaverOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::FloatingWindow::anyDockWidgetsHas(KDDockWidgets::DockWidgetOption arg__1) const
bool c_KDDockWidgets__Core__FloatingWindow__anyDockWidgetsHas_DockWidgetOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::FloatingWindow::anyDockWidgetsHas(KDDockWidgets::LayoutSaverOption arg__1) const
bool c_KDDockWidgets__Core__FloatingWindow__anyDockWidgetsHas_LayoutSaverOption(void *thisObj, int arg__1);
// KDDockWidgets::Core::FloatingWindow::anyNonClosable() const
bool c_KDDockWidgets__Core__FloatingWindow__anyNonClosable(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::anyNonDockable() const
bool c_KDDockWidgets__Core__FloatingWindow__anyNonDockable(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::beingDeleted() const
bool c_KDDockWidgets__Core__FloatingWindow__beingDeleted(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::contentMargins() const
void *c_KDDockWidgets__Core__FloatingWindow__contentMargins(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::dragRect() const
void *c_KDDockWidgets__Core__FloatingWindow__dragRect(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::dropArea() const
void *c_KDDockWidgets__Core__FloatingWindow__dropArea(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::ensureRectIsOnScreen(KDDockWidgets::Rect & geometry)
void c_static_KDDockWidgets__Core__FloatingWindow__ensureRectIsOnScreen_Rect(void *geometry_);
// KDDockWidgets::Core::FloatingWindow::hasSingleDockWidget() const
bool c_KDDockWidgets__Core__FloatingWindow__hasSingleDockWidget(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::hasSingleGroup() const
bool c_KDDockWidgets__Core__FloatingWindow__hasSingleGroup(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::isInDragArea(KDDockWidgets::Point globalPoint) const
bool c_KDDockWidgets__Core__FloatingWindow__isInDragArea_Point(void *thisObj, void *globalPoint_);
// KDDockWidgets::Core::FloatingWindow::isMDI() const
bool c_KDDockWidgets__Core__FloatingWindow__isMDI(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::isUtilityWindow() const
bool c_KDDockWidgets__Core__FloatingWindow__isUtilityWindow(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::isWindow() const
bool c_KDDockWidgets__Core__FloatingWindow__isWindow(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::layout() const
void *c_KDDockWidgets__Core__FloatingWindow__layout(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::mainWindow() const
void *c_KDDockWidgets__Core__FloatingWindow__mainWindow(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::maybeCreateResizeHandler()
void c_KDDockWidgets__Core__FloatingWindow__maybeCreateResizeHandler(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::multiSplitter() const
void *c_KDDockWidgets__Core__FloatingWindow__multiSplitter(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::scheduleDeleteLater()
void c_KDDockWidgets__Core__FloatingWindow__scheduleDeleteLater(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__FloatingWindow__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::FloatingWindow::setSuggestedGeometry(KDDockWidgets::Rect suggestedRect)
void c_KDDockWidgets__Core__FloatingWindow__setSuggestedGeometry_Rect(void *thisObj, void *suggestedRect_);
// KDDockWidgets::Core::FloatingWindow::singleDockWidget() const
void *c_KDDockWidgets__Core__FloatingWindow__singleDockWidget(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::singleFrame() const
void *c_KDDockWidgets__Core__FloatingWindow__singleFrame(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::supportsMaximizeButton() const
bool c_KDDockWidgets__Core__FloatingWindow__supportsMaximizeButton(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::supportsMinimizeButton() const
bool c_KDDockWidgets__Core__FloatingWindow__supportsMinimizeButton(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::titleBar() const
void *c_KDDockWidgets__Core__FloatingWindow__titleBar(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::updateTitleAndIcon()
void c_KDDockWidgets__Core__FloatingWindow__updateTitleAndIcon(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::updateTitleBarVisibility()
void c_KDDockWidgets__Core__FloatingWindow__updateTitleBarVisibility(void *thisObj);
// KDDockWidgets::Core::FloatingWindow::userType() const
int c_KDDockWidgets__Core__FloatingWindow__userType(void *thisObj);
void c_KDDockWidgets__Core__FloatingWindow__destructor(void *thisObj);
void c_KDDockWidgets__Core__FloatingWindow__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__FloatingWindow_Finalizer(void *cppObj); // KDDockWidgets::Core::DropIndicatorOverlay::DropIndicatorOverlay(KDDockWidgets::Core::DropArea * dropArea)
void *c_KDDockWidgets__Core__DropIndicatorOverlay__constructor_DropArea(void *dropArea_);
// KDDockWidgets::Core::DropIndicatorOverlay::DropIndicatorOverlay(KDDockWidgets::Core::DropArea * dropArea, KDDockWidgets::Core::View * view)
void *c_KDDockWidgets__Core__DropIndicatorOverlay__constructor_DropArea_View(void *dropArea_, void *view_);
// KDDockWidgets::Core::DropIndicatorOverlay::currentDropLocation() const
int c_KDDockWidgets__Core__DropIndicatorOverlay__currentDropLocation(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const
bool c_KDDockWidgets__Core__DropIndicatorOverlay__dropIndicatorVisible_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::hover(KDDockWidgets::Point globalPos)
int c_KDDockWidgets__Core__DropIndicatorOverlay__hover_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::DropIndicatorOverlay::hover_impl(KDDockWidgets::Point globalPos)
int c_KDDockWidgets__Core__DropIndicatorOverlay__hover_impl_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::DropIndicatorOverlay::hoveredGroup() const
void *c_KDDockWidgets__Core__DropIndicatorOverlay__hoveredGroup(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::hoveredGroupRect() const
void *c_KDDockWidgets__Core__DropIndicatorOverlay__hoveredGroupRect(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::isHovered() const
bool c_KDDockWidgets__Core__DropIndicatorOverlay__isHovered(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::multisplitterLocationFor(KDDockWidgets::DropLocation arg__1)
int c_static_KDDockWidgets__Core__DropIndicatorOverlay__multisplitterLocationFor_DropLocation(int arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::onHoveredGroupChanged(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__Core__DropIndicatorOverlay__onHoveredGroupChanged_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::DropIndicatorOverlay::posForIndicator(KDDockWidgets::DropLocation arg__1) const
void *c_KDDockWidgets__Core__DropIndicatorOverlay__posForIndicator_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::removeHover()
void c_KDDockWidgets__Core__DropIndicatorOverlay__removeHover(void *thisObj);
// KDDockWidgets::Core::DropIndicatorOverlay::setCurrentDropLocation(KDDockWidgets::DropLocation arg__1)
void c_KDDockWidgets__Core__DropIndicatorOverlay__setCurrentDropLocation_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::setHoveredGroup(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__Core__DropIndicatorOverlay__setHoveredGroup_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::DropIndicatorOverlay::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__DropIndicatorOverlay__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::DropIndicatorOverlay::setWindowBeingDragged(bool arg__1)
void c_KDDockWidgets__Core__DropIndicatorOverlay__setWindowBeingDragged_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::DropIndicatorOverlay::updateVisibility()
void c_KDDockWidgets__Core__DropIndicatorOverlay__updateVisibility(void *thisObj);
void c_KDDockWidgets__Core__DropIndicatorOverlay__destructor(void *thisObj);
void c_KDDockWidgets__Core__DropIndicatorOverlay__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__DropIndicatorOverlay_Finalizer(void *cppObj); // KDDockWidgets::Core::DockWidget::DockWidget(KDDockWidgets::Core::View * view, const QString & uniqueName, QFlags<KDDockWidgets::DockWidgetOption> options, QFlags<KDDockWidgets::LayoutSaverOption> layoutSaverOptions)
void *c_KDDockWidgets__Core__DockWidget__constructor_View_QString_DockWidgetOptions_LayoutSaverOptions(void *view_, const char *uniqueName_, int options_, int layoutSaverOptions_);
// KDDockWidgets::Core::DockWidget::addDockWidgetAsTab(KDDockWidgets::Core::DockWidget * other, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__DockWidget__addDockWidgetAsTab_DockWidget_InitialOption(void *thisObj, void *other_, void *initialOption_);
// KDDockWidgets::Core::DockWidget::addDockWidgetToContainingWindow(KDDockWidgets::Core::DockWidget * other, KDDockWidgets::Location location, KDDockWidgets::Core::DockWidget * relativeTo, KDDockWidgets::InitialOption initialOption)
void c_KDDockWidgets__Core__DockWidget__addDockWidgetToContainingWindow_DockWidget_Location_DockWidget_InitialOption(void *thisObj, void *other_, int location, void *relativeTo_, void *initialOption_);
// KDDockWidgets::Core::DockWidget::byName(const QString & uniqueName)
void *c_static_KDDockWidgets__Core__DockWidget__byName_QString(const char *uniqueName_);
// KDDockWidgets::Core::DockWidget::currentTabIndex() const
int c_KDDockWidgets__Core__DockWidget__currentTabIndex(void *thisObj);
// KDDockWidgets::Core::DockWidget::floatingWindow() const
void *c_KDDockWidgets__Core__DockWidget__floatingWindow(void *thisObj);
// KDDockWidgets::Core::DockWidget::forceClose()
void c_KDDockWidgets__Core__DockWidget__forceClose(void *thisObj);
// KDDockWidgets::Core::DockWidget::groupGeometry() const
void *c_KDDockWidgets__Core__DockWidget__groupGeometry(void *thisObj);
// KDDockWidgets::Core::DockWidget::hasPreviousDockedLocation() const
bool c_KDDockWidgets__Core__DockWidget__hasPreviousDockedLocation(void *thisObj);
// KDDockWidgets::Core::DockWidget::init()
void c_KDDockWidgets__Core__DockWidget__init(void *thisObj);
// KDDockWidgets::Core::DockWidget::isCurrentTab() const
bool c_KDDockWidgets__Core__DockWidget__isCurrentTab(void *thisObj);
// KDDockWidgets::Core::DockWidget::isFloating() const
bool c_KDDockWidgets__Core__DockWidget__isFloating(void *thisObj);
// KDDockWidgets::Core::DockWidget::isFocused() const
bool c_KDDockWidgets__Core__DockWidget__isFocused(void *thisObj);
// KDDockWidgets::Core::DockWidget::isInMainWindow() const
bool c_KDDockWidgets__Core__DockWidget__isInMainWindow(void *thisObj);
// KDDockWidgets::Core::DockWidget::isInSideBar() const
bool c_KDDockWidgets__Core__DockWidget__isInSideBar(void *thisObj);
// KDDockWidgets::Core::DockWidget::isMainWindow() const
bool c_KDDockWidgets__Core__DockWidget__isMainWindow(void *thisObj);
// KDDockWidgets::Core::DockWidget::isOpen() const
bool c_KDDockWidgets__Core__DockWidget__isOpen(void *thisObj);
// KDDockWidgets::Core::DockWidget::isOverlayed() const
bool c_KDDockWidgets__Core__DockWidget__isOverlayed(void *thisObj);
// KDDockWidgets::Core::DockWidget::isPersistentCentralDockWidget() const
bool c_KDDockWidgets__Core__DockWidget__isPersistentCentralDockWidget(void *thisObj);
// KDDockWidgets::Core::DockWidget::isTabbed() const
bool c_KDDockWidgets__Core__DockWidget__isTabbed(void *thisObj);
// KDDockWidgets::Core::DockWidget::lastOverlayedSize() const
void *c_KDDockWidgets__Core__DockWidget__lastOverlayedSize(void *thisObj);
// KDDockWidgets::Core::DockWidget::layoutSaverOptions() const
int c_KDDockWidgets__Core__DockWidget__layoutSaverOptions(void *thisObj);
// KDDockWidgets::Core::DockWidget::mainWindow() const
void *c_KDDockWidgets__Core__DockWidget__mainWindow(void *thisObj);
// KDDockWidgets::Core::DockWidget::mdiZ() const
int c_KDDockWidgets__Core__DockWidget__mdiZ(void *thisObj);
// KDDockWidgets::Core::DockWidget::moveToSideBar()
void c_KDDockWidgets__Core__DockWidget__moveToSideBar(void *thisObj);
// KDDockWidgets::Core::DockWidget::onResize(KDDockWidgets::Size newSize)
void c_KDDockWidgets__Core__DockWidget__onResize_Size(void *thisObj, void *newSize_);
// KDDockWidgets::Core::DockWidget::open()
void c_KDDockWidgets__Core__DockWidget__open(void *thisObj);
// KDDockWidgets::Core::DockWidget::options() const
int c_KDDockWidgets__Core__DockWidget__options(void *thisObj);
// KDDockWidgets::Core::DockWidget::raise()
void c_KDDockWidgets__Core__DockWidget__raise(void *thisObj);
// KDDockWidgets::Core::DockWidget::removeFromSideBar()
void c_KDDockWidgets__Core__DockWidget__removeFromSideBar(void *thisObj);
// KDDockWidgets::Core::DockWidget::resizeInLayout(int left, int top, int right, int bottom)
void c_KDDockWidgets__Core__DockWidget__resizeInLayout_int_int_int_int(void *thisObj, int left, int top, int right, int bottom);
// KDDockWidgets::Core::DockWidget::setAffinityName(const QString & name)
void c_KDDockWidgets__Core__DockWidget__setAffinityName_QString(void *thisObj, const char *name_);
// KDDockWidgets::Core::DockWidget::setAsCurrentTab()
void c_KDDockWidgets__Core__DockWidget__setAsCurrentTab(void *thisObj);
// KDDockWidgets::Core::DockWidget::setFloating(bool floats)
bool c_KDDockWidgets__Core__DockWidget__setFloating_bool(void *thisObj, bool floats);
// KDDockWidgets::Core::DockWidget::setFloatingGeometry(KDDockWidgets::Rect geo)
void c_KDDockWidgets__Core__DockWidget__setFloatingGeometry_Rect(void *thisObj, void *geo_);
// KDDockWidgets::Core::DockWidget::setMDIPosition(KDDockWidgets::Point pos)
void c_KDDockWidgets__Core__DockWidget__setMDIPosition_Point(void *thisObj, void *pos_);
// KDDockWidgets::Core::DockWidget::setMDISize(KDDockWidgets::Size size)
void c_KDDockWidgets__Core__DockWidget__setMDISize_Size(void *thisObj, void *size_);
// KDDockWidgets::Core::DockWidget::setMDIZ(int z)
void c_KDDockWidgets__Core__DockWidget__setMDIZ_int(void *thisObj, int z);
// KDDockWidgets::Core::DockWidget::setOptions(QFlags<KDDockWidgets::DockWidgetOption> arg__1)
void c_KDDockWidgets__Core__DockWidget__setOptions_DockWidgetOptions(void *thisObj, int arg__1_);
// KDDockWidgets::Core::DockWidget::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__DockWidget__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::DockWidget::setTitle(const QString & title)
void c_KDDockWidgets__Core__DockWidget__setTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::Core::DockWidget::setUniqueName(const QString & arg__1)
void c_KDDockWidgets__Core__DockWidget__setUniqueName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::Core::DockWidget::setUserType(int userType)
void c_KDDockWidgets__Core__DockWidget__setUserType_int(void *thisObj, int userType);
// KDDockWidgets::Core::DockWidget::show()
void c_KDDockWidgets__Core__DockWidget__show(void *thisObj);
// KDDockWidgets::Core::DockWidget::sizeInLayout() const
void *c_KDDockWidgets__Core__DockWidget__sizeInLayout(void *thisObj);
// KDDockWidgets::Core::DockWidget::skipsRestore() const
bool c_KDDockWidgets__Core__DockWidget__skipsRestore(void *thisObj);
// KDDockWidgets::Core::DockWidget::startDragging(bool singleTab)
bool c_KDDockWidgets__Core__DockWidget__startDragging_bool(void *thisObj, bool singleTab);
// KDDockWidgets::Core::DockWidget::tabIndex() const
int c_KDDockWidgets__Core__DockWidget__tabIndex(void *thisObj);
// KDDockWidgets::Core::DockWidget::title() const
void *c_KDDockWidgets__Core__DockWidget__title(void *thisObj);
// KDDockWidgets::Core::DockWidget::titleBar() const
void *c_KDDockWidgets__Core__DockWidget__titleBar(void *thisObj);
// KDDockWidgets::Core::DockWidget::uniqueName() const
void *c_KDDockWidgets__Core__DockWidget__uniqueName(void *thisObj);
// KDDockWidgets::Core::DockWidget::userType() const
int c_KDDockWidgets__Core__DockWidget__userType(void *thisObj);
// KDDockWidgets::Core::DockWidget::wasRestored() const
bool c_KDDockWidgets__Core__DockWidget__wasRestored(void *thisObj);
void c_KDDockWidgets__Core__DockWidget__destructor(void *thisObj);
void c_KDDockWidgets__Core__DockWidget__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__DockWidget_Finalizer(void *cppObj); // KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::ClassicIndicatorWindowViewInterface()
void *c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__constructor();
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::hover(KDDockWidgets::Point arg__1)
int c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__hover_Point(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::isWindow() const
bool c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__isWindow(void *thisObj);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::posForIndicator(KDDockWidgets::DropLocation arg__1) const
void *c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__posForIndicator_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::raise()
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__raise(void *thisObj);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::resize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__resize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::setGeometry(KDDockWidgets::Rect arg__1)
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__setGeometry_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::setObjectName(const QString & arg__1)
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__setObjectName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::setVisible(bool arg__1)
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::updateIndicatorVisibility()
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__updateIndicatorVisibility(void *thisObj);
// KDDockWidgets::Core::ClassicIndicatorWindowViewInterface::updatePositions()
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__updatePositions(void *thisObj);
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__destructor(void *thisObj);
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__ClassicIndicatorWindowViewInterface_Finalizer(void *cppObj); // KDDockWidgets::flutter::IndicatorWindow::IndicatorWindow(KDDockWidgets::Core::ClassicDropIndicatorOverlay * arg__1, KDDockWidgets::Core::View * parent)
void *c_KDDockWidgets__flutter__IndicatorWindow__constructor_ClassicDropIndicatorOverlay_View(void *arg__1_, void *parent_);
// KDDockWidgets::flutter::IndicatorWindow::activateWindow()
void c_KDDockWidgets__flutter__IndicatorWindow__activateWindow(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::close()
bool c_KDDockWidgets__flutter__IndicatorWindow__close(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::createPlatformWindow()
void c_KDDockWidgets__flutter__IndicatorWindow__createPlatformWindow(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::flags() const
int c_KDDockWidgets__flutter__IndicatorWindow__flags(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::geometry() const
void *c_KDDockWidgets__flutter__IndicatorWindow__geometry(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::grabMouse()
void c_KDDockWidgets__flutter__IndicatorWindow__grabMouse(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::hasFocus() const
bool c_KDDockWidgets__flutter__IndicatorWindow__hasFocus(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::hide()
void c_KDDockWidgets__flutter__IndicatorWindow__hide(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::hover(KDDockWidgets::Point globalPos)
int c_KDDockWidgets__flutter__IndicatorWindow__hover_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::flutter::IndicatorWindow::hover_flutter(KDDockWidgets::Point globalPos)
int c_KDDockWidgets__flutter__IndicatorWindow__hover_flutter_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::flutter::IndicatorWindow::hoveredGroup() const
void *c_KDDockWidgets__flutter__IndicatorWindow__hoveredGroup(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::init()
void c_KDDockWidgets__flutter__IndicatorWindow__init(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isActiveWindow() const
bool c_KDDockWidgets__flutter__IndicatorWindow__isActiveWindow(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isExplicitlyHidden() const
bool c_KDDockWidgets__flutter__IndicatorWindow__isExplicitlyHidden(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isMaximized() const
bool c_KDDockWidgets__flutter__IndicatorWindow__isMaximized(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isMinimized() const
bool c_KDDockWidgets__flutter__IndicatorWindow__isMinimized(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isMounted() const
bool c_KDDockWidgets__flutter__IndicatorWindow__isMounted(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isNull() const
bool c_KDDockWidgets__flutter__IndicatorWindow__isNull(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isRootView() const
bool c_KDDockWidgets__flutter__IndicatorWindow__isRootView(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isVisible() const
bool c_KDDockWidgets__flutter__IndicatorWindow__isVisible(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::isWindow() const
bool c_KDDockWidgets__flutter__IndicatorWindow__isWindow(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::mapFromGlobal(KDDockWidgets::Point globalPt) const
void *c_KDDockWidgets__flutter__IndicatorWindow__mapFromGlobal_Point(void *thisObj, void *globalPt_);
// KDDockWidgets::flutter::IndicatorWindow::mapTo(KDDockWidgets::Core::View * parent, KDDockWidgets::Point pos) const
void *c_KDDockWidgets__flutter__IndicatorWindow__mapTo_View_Point(void *thisObj, void *parent_, void *pos_);
// KDDockWidgets::flutter::IndicatorWindow::mapToGlobal(KDDockWidgets::Point localPt) const
void *c_KDDockWidgets__flutter__IndicatorWindow__mapToGlobal_Point(void *thisObj, void *localPt_);
// KDDockWidgets::flutter::IndicatorWindow::maxSizeHint() const
void *c_KDDockWidgets__flutter__IndicatorWindow__maxSizeHint(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::minSize() const
void *c_KDDockWidgets__flutter__IndicatorWindow__minSize(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::move(int x, int y)
void c_KDDockWidgets__flutter__IndicatorWindow__move_int_int(void *thisObj, int x, int y);
// KDDockWidgets::flutter::IndicatorWindow::normalGeometry() const
void *c_KDDockWidgets__flutter__IndicatorWindow__normalGeometry(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::onChildAdded(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__IndicatorWindow__onChildAdded_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::IndicatorWindow::onChildRemoved(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__IndicatorWindow__onChildRemoved_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::IndicatorWindow::onChildVisibilityChanged(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__IndicatorWindow__onChildVisibilityChanged_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::IndicatorWindow::onGeometryChanged()
void c_KDDockWidgets__flutter__IndicatorWindow__onGeometryChanged(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::onRebuildRequested()
void c_KDDockWidgets__flutter__IndicatorWindow__onRebuildRequested(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::onResize(int h, int w)
bool c_KDDockWidgets__flutter__IndicatorWindow__onResize_int_int(void *thisObj, int h, int w);
// KDDockWidgets::flutter::IndicatorWindow::posForIndicator(KDDockWidgets::DropLocation arg__1) const
void *c_KDDockWidgets__flutter__IndicatorWindow__posForIndicator_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::flutter::IndicatorWindow::posForIndicator_flutter(KDDockWidgets::DropLocation arg__1) const
void *c_KDDockWidgets__flutter__IndicatorWindow__posForIndicator_flutter_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::flutter::IndicatorWindow::raise()
void c_KDDockWidgets__flutter__IndicatorWindow__raise(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::raiseAndActivate()
void c_KDDockWidgets__flutter__IndicatorWindow__raiseAndActivate(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::raiseChild(KDDockWidgets::Core::View * childView)
void c_KDDockWidgets__flutter__IndicatorWindow__raiseChild_View(void *thisObj, void *childView_);
// KDDockWidgets::flutter::IndicatorWindow::raiseWindow(KDDockWidgets::Core::View * rootView)
void c_KDDockWidgets__flutter__IndicatorWindow__raiseWindow_View(void *thisObj, void *rootView_);
// KDDockWidgets::flutter::IndicatorWindow::releaseKeyboard()
void c_KDDockWidgets__flutter__IndicatorWindow__releaseKeyboard(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::releaseMouse()
void c_KDDockWidgets__flutter__IndicatorWindow__releaseMouse(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::resize(KDDockWidgets::Size arg__1)
void c_KDDockWidgets__flutter__IndicatorWindow__resize_Size(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::IndicatorWindow::rubberBand() const
void *c_KDDockWidgets__flutter__IndicatorWindow__rubberBand(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::setCursor(Qt::CursorShape shape)
void c_KDDockWidgets__flutter__IndicatorWindow__setCursor_CursorShape(void *thisObj, int shape);
// KDDockWidgets::flutter::IndicatorWindow::setFixedHeight(int h)
void c_KDDockWidgets__flutter__IndicatorWindow__setFixedHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::IndicatorWindow::setFixedWidth(int w)
void c_KDDockWidgets__flutter__IndicatorWindow__setFixedWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::IndicatorWindow::setGeometry(KDDockWidgets::Rect arg__1)
void c_KDDockWidgets__flutter__IndicatorWindow__setGeometry_Rect(void *thisObj, void *arg__1_);
// KDDockWidgets::flutter::IndicatorWindow::setHeight(int h)
void c_KDDockWidgets__flutter__IndicatorWindow__setHeight_int(void *thisObj, int h);
// KDDockWidgets::flutter::IndicatorWindow::setMaximumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__IndicatorWindow__setMaximumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::IndicatorWindow::setMinimumSize(KDDockWidgets::Size sz)
void c_KDDockWidgets__flutter__IndicatorWindow__setMinimumSize_Size(void *thisObj, void *sz_);
// KDDockWidgets::flutter::IndicatorWindow::setMouseTracking(bool enable)
void c_KDDockWidgets__flutter__IndicatorWindow__setMouseTracking_bool(void *thisObj, bool enable);
// KDDockWidgets::flutter::IndicatorWindow::setObjectName(const QString & arg__1)
void c_KDDockWidgets__flutter__IndicatorWindow__setObjectName_QString(void *thisObj, const char *arg__1_);
// KDDockWidgets::flutter::IndicatorWindow::setParent(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__flutter__IndicatorWindow__setParent_View(void *thisObj, void *parent_);
// KDDockWidgets::flutter::IndicatorWindow::setSize(int w, int h)
void c_KDDockWidgets__flutter__IndicatorWindow__setSize_int_int(void *thisObj, int w, int h);
// KDDockWidgets::flutter::IndicatorWindow::setViewName(const QString & name)
void c_KDDockWidgets__flutter__IndicatorWindow__setViewName_QString(void *thisObj, const char *name_);
// KDDockWidgets::flutter::IndicatorWindow::setVisible(bool arg__1)
void c_KDDockWidgets__flutter__IndicatorWindow__setVisible_bool(void *thisObj, bool arg__1);
// KDDockWidgets::flutter::IndicatorWindow::setWidth(int w)
void c_KDDockWidgets__flutter__IndicatorWindow__setWidth_int(void *thisObj, int w);
// KDDockWidgets::flutter::IndicatorWindow::setWindowOpacity(double v)
void c_KDDockWidgets__flutter__IndicatorWindow__setWindowOpacity_double(void *thisObj, double v);
// KDDockWidgets::flutter::IndicatorWindow::setWindowTitle(const QString & title)
void c_KDDockWidgets__flutter__IndicatorWindow__setWindowTitle_QString(void *thisObj, const char *title_);
// KDDockWidgets::flutter::IndicatorWindow::setZOrder(int z)
void c_KDDockWidgets__flutter__IndicatorWindow__setZOrder_int(void *thisObj, int z);
// KDDockWidgets::flutter::IndicatorWindow::show()
void c_KDDockWidgets__flutter__IndicatorWindow__show(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::showMaximized()
void c_KDDockWidgets__flutter__IndicatorWindow__showMaximized(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::showMinimized()
void c_KDDockWidgets__flutter__IndicatorWindow__showMinimized(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::showNormal()
void c_KDDockWidgets__flutter__IndicatorWindow__showNormal(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::update()
void c_KDDockWidgets__flutter__IndicatorWindow__update(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::updateIndicatorVisibility()
void c_KDDockWidgets__flutter__IndicatorWindow__updateIndicatorVisibility(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::updatePositions()
void c_KDDockWidgets__flutter__IndicatorWindow__updatePositions(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::updatePositions_flutter(int overlayWidth, int overlayHeight, KDDockWidgets::Core::Group * hoveredGroup, int visibleLocations)
bool c_KDDockWidgets__flutter__IndicatorWindow__updatePositions_flutter_int_int_Group_int(void *thisObj, int overlayWidth, int overlayHeight, void *hoveredGroup_, int visibleLocations);
// KDDockWidgets::flutter::IndicatorWindow::viewName() const
void *c_KDDockWidgets__flutter__IndicatorWindow__viewName(void *thisObj);
// KDDockWidgets::flutter::IndicatorWindow::zOrder() const
int c_KDDockWidgets__flutter__IndicatorWindow__zOrder(void *thisObj);
void c_KDDockWidgets__flutter__IndicatorWindow__destructor(void *thisObj);
void c_KDDockWidgets__flutter__IndicatorWindow__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__flutter__IndicatorWindow_Finalizer(void *cppObj); // KDDockWidgets::Core::ClassicDropIndicatorOverlay::ClassicDropIndicatorOverlay(KDDockWidgets::Core::DropArea * dropArea)
void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__constructor_DropArea(void *dropArea_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::dropIndicatorVisible(KDDockWidgets::DropLocation arg__1) const
bool c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__dropIndicatorVisible_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::hover_impl(KDDockWidgets::Point globalPos)
int c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__hover_impl_Point(void *thisObj, void *globalPos_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::indicatorWindow() const
void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__indicatorWindow(void *thisObj);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::onHoveredGroupChanged(KDDockWidgets::Core::Group * arg__1)
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__onHoveredGroupChanged_Group(void *thisObj, void *arg__1_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::onResize(KDDockWidgets::Size newSize)
bool c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__onResize_Size(void *thisObj, void *newSize_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::posForIndicator(KDDockWidgets::DropLocation arg__1) const
void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__posForIndicator_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::rubberBand() const
void *c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__rubberBand(void *thisObj);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::setCurrentDropLocation(KDDockWidgets::DropLocation arg__1)
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__setCurrentDropLocation_DropLocation(void *thisObj, int arg__1);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::setParentView_impl(KDDockWidgets::Core::View * parent)
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__setParentView_impl_View(void *thisObj, void *parent_);
// KDDockWidgets::Core::ClassicDropIndicatorOverlay::updateVisibility()
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__updateVisibility(void *thisObj);
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__destructor(void *thisObj);
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay__registerVirtualMethodCallback(void *ptr, void *callback, int methodId);
void c_KDDockWidgets__Core__ClassicDropIndicatorOverlay_Finalizer(void *cppObj); // KDDockWidgets::Config::absoluteWidgetMaxSize() const
void *c_KDDockWidgets__Config__absoluteWidgetMaxSize(void *thisObj);
// KDDockWidgets::Config::absoluteWidgetMinSize() const
void *c_KDDockWidgets__Config__absoluteWidgetMinSize(void *thisObj);
// KDDockWidgets::Config::draggedWindowOpacity() const
double c_KDDockWidgets__Config__draggedWindowOpacity(void *thisObj);
// KDDockWidgets::Config::dropIndicatorsInhibited() const
bool c_KDDockWidgets__Config__dropIndicatorsInhibited(void *thisObj);
// KDDockWidgets::Config::layoutSaverUsesStrictMode() const
bool c_KDDockWidgets__Config__layoutSaverUsesStrictMode(void *thisObj);
// KDDockWidgets::Config::layoutSpacing() const
int c_KDDockWidgets__Config__layoutSpacing(void *thisObj);
// KDDockWidgets::Config::mdiPopupThreshold() const
int c_KDDockWidgets__Config__mdiPopupThreshold(void *thisObj);
// KDDockWidgets::Config::onlyProgrammaticDrag() const
bool c_KDDockWidgets__Config__onlyProgrammaticDrag(void *thisObj);
// KDDockWidgets::Config::printDebug()
void c_KDDockWidgets__Config__printDebug(void *thisObj);
// KDDockWidgets::Config::self()
void *c_static_KDDockWidgets__Config__self();
// KDDockWidgets::Config::separatorThickness() const
int c_KDDockWidgets__Config__separatorThickness(void *thisObj);
// KDDockWidgets::Config::setAbsoluteWidgetMaxSize(KDDockWidgets::Size size)
void c_KDDockWidgets__Config__setAbsoluteWidgetMaxSize_Size(void *thisObj, void *size_);
// KDDockWidgets::Config::setAbsoluteWidgetMinSize(KDDockWidgets::Size size)
void c_KDDockWidgets__Config__setAbsoluteWidgetMinSize_Size(void *thisObj, void *size_);
// KDDockWidgets::Config::setDraggedWindowOpacity(double opacity)
void c_KDDockWidgets__Config__setDraggedWindowOpacity_double(void *thisObj, double opacity);
// KDDockWidgets::Config::setDropIndicatorsInhibited(bool inhibit) const
void c_KDDockWidgets__Config__setDropIndicatorsInhibited_bool(void *thisObj, bool inhibit);
// KDDockWidgets::Config::setLayoutSaverStrictMode(bool arg__1)
void c_KDDockWidgets__Config__setLayoutSaverStrictMode_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Config::setLayoutSpacing(int arg__1)
void c_KDDockWidgets__Config__setLayoutSpacing_int(void *thisObj, int arg__1);
// KDDockWidgets::Config::setMDIPopupThreshold(int arg__1)
void c_KDDockWidgets__Config__setMDIPopupThreshold_int(void *thisObj, int arg__1);
// KDDockWidgets::Config::setOnlyProgrammaticDrag(bool arg__1)
void c_KDDockWidgets__Config__setOnlyProgrammaticDrag_bool(void *thisObj, bool arg__1);
// KDDockWidgets::Config::setSeparatorThickness(int value)
void c_KDDockWidgets__Config__setSeparatorThickness_int(void *thisObj, int value);
// KDDockWidgets::Config::setStartDragDistance(int arg__1)
void c_KDDockWidgets__Config__setStartDragDistance_int(void *thisObj, int arg__1);
// KDDockWidgets::Config::setTransparencyOnlyOverDropIndicator(bool only)
void c_KDDockWidgets__Config__setTransparencyOnlyOverDropIndicator_bool(void *thisObj, bool only);
// KDDockWidgets::Config::setViewFactory(KDDockWidgets::Core::ViewFactory * arg__1)
void c_KDDockWidgets__Config__setViewFactory_ViewFactory(void *thisObj, void *arg__1_);
// KDDockWidgets::Config::startDragDistance() const
int c_KDDockWidgets__Config__startDragDistance(void *thisObj);
// KDDockWidgets::Config::transparencyOnlyOverDropIndicator() const
bool c_KDDockWidgets__Config__transparencyOnlyOverDropIndicator(void *thisObj);
// KDDockWidgets::Config::viewFactory() const
void *c_KDDockWidgets__Config__viewFactory(void *thisObj);
void c_KDDockWidgets__Config__destructor(void *thisObj);
void c_KDDockWidgets__Config_Finalizer(void *cppObj);
