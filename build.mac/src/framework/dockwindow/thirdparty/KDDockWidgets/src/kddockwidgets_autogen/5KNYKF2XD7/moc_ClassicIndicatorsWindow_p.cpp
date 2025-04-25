/****************************************************************************
** Meta object code from reading C++ file 'ClassicIndicatorsWindow_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/indicators/ClassicIndicatorsWindow_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ClassicIndicatorsWindow_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::IndicatorWindow",
    "iconName",
    "",
    "loc",
    "active",
    "classicIndicators",
    "KDDockWidgets::ClassicIndicators*"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS_t {
    uint offsetsAndSizes[14];
    char stringdata0[31];
    char stringdata1[9];
    char stringdata2[1];
    char stringdata3[4];
    char stringdata4[7];
    char stringdata5[18];
    char stringdata6[34];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS = {
    {
        QT_MOC_LITERAL(0, 30),  // "KDDockWidgets::IndicatorWindow"
        QT_MOC_LITERAL(31, 8),  // "iconName"
        QT_MOC_LITERAL(40, 0),  // ""
        QT_MOC_LITERAL(41, 3),  // "loc"
        QT_MOC_LITERAL(45, 6),  // "active"
        QT_MOC_LITERAL(52, 17),  // "classicIndicators"
        QT_MOC_LITERAL(70, 33)   // "KDDockWidgets::ClassicIndicat..."
    },
    "KDDockWidgets::IndicatorWindow",
    "iconName",
    "",
    "loc",
    "active",
    "classicIndicators",
    "KDDockWidgets::ClassicIndicators*"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       1,   25, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   20,    2, 0x102,    2 /* Public | MethodIsConst  */,

 // methods: parameters
    QMetaType::QString, QMetaType::Int, QMetaType::Bool,    3,    4,

 // properties: name, type, flags
       5, 0x80000000 | 6, 0x00015409, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::IndicatorWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickView::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS_t,
        // property 'classicIndicators'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::ClassicIndicators*, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<IndicatorWindow, std::true_type>,
        // method 'iconName'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::IndicatorWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IndicatorWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: { QString _r = _t->iconName((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< KDDockWidgets::ClassicIndicators* >(); break;
        }
    } else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<IndicatorWindow *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< KDDockWidgets::ClassicIndicators**>(_v) = _t->classicIndicators(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::IndicatorWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::IndicatorWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEIndicatorWindowENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QQuickView::qt_metacast(_clname);
}

int KDDockWidgets::IndicatorWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickView::qt_metacall(_c, _id, _a);
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
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
