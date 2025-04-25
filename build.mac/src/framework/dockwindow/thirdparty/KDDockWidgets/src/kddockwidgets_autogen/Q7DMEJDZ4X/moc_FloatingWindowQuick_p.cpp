/****************************************************************************
** Meta object code from reading C++ file 'FloatingWindowQuick_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/quick/FloatingWindowQuick_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FloatingWindowQuick_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::FloatingWindowQuick"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS_t {
    uint offsetsAndSizes[2];
    char stringdata0[35];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS = {
    {
        QT_MOC_LITERAL(0, 34)   // "KDDockWidgets::FloatingWindow..."
    },
    "KDDockWidgets::FloatingWindowQuick"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::FloatingWindowQuick::staticMetaObject = { {
    QMetaObject::SuperData::link<FloatingWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FloatingWindowQuick, std::true_type>
    >,
    nullptr
} };

void KDDockWidgets::FloatingWindowQuick::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *KDDockWidgets::FloatingWindowQuick::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::FloatingWindowQuick::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowQuickENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return FloatingWindow::qt_metacast(_clname);
}

int KDDockWidgets::FloatingWindowQuick::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = FloatingWindow::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
