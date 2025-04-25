/****************************************************************************
** Meta object code from reading C++ file 'Helpers_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/quick/Helpers_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Helpers_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::QtQuickHelpers",
    "logicalDpiFactor",
    "",
    "const QQuickItem*",
    "item"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS_t {
    uint offsetsAndSizes[10];
    char stringdata0[30];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[18];
    char stringdata4[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS = {
    {
        QT_MOC_LITERAL(0, 29),  // "KDDockWidgets::QtQuickHelpers"
        QT_MOC_LITERAL(30, 16),  // "logicalDpiFactor"
        QT_MOC_LITERAL(47, 0),  // ""
        QT_MOC_LITERAL(48, 17),  // "const QQuickItem*"
        QT_MOC_LITERAL(66, 4)   // "item"
    },
    "KDDockWidgets::QtQuickHelpers",
    "logicalDpiFactor",
    "",
    "const QQuickItem*",
    "item"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   20,    2, 0x102,    1 /* Public | MethodIsConst  */,

 // methods: parameters
    QMetaType::QReal, 0x80000000 | 3,    4,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::QtQuickHelpers::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<QtQuickHelpers, std::true_type>,
        // method 'logicalDpiFactor'
        QtPrivate::TypeAndForceComplete<qreal, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QQuickItem *, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::QtQuickHelpers::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QtQuickHelpers *>(_o);
        (void)_t;
        switch (_id) {
        case 0: { qreal _r = _t->logicalDpiFactor((*reinterpret_cast< std::add_pointer_t<const QQuickItem*>>(_a[1])));
            if (_a[0]) *reinterpret_cast< qreal*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

const QMetaObject *KDDockWidgets::QtQuickHelpers::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::QtQuickHelpers::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEQtQuickHelpersENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int KDDockWidgets::QtQuickHelpers::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
