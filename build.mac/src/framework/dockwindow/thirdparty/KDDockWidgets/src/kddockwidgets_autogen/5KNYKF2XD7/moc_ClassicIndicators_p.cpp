/****************************************************************************
** Meta object code from reading C++ file 'ClassicIndicators_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/indicators/ClassicIndicators_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ClassicIndicators_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::ClassicIndicators",
    "innerIndicatorsVisibleChanged",
    "",
    "outterIndicatorsVisibleChanged",
    "tabIndicatorVisibleChanged",
    "innerIndicatorsVisible",
    "outterIndicatorsVisible",
    "tabIndicatorVisible"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS_t {
    uint offsetsAndSizes[16];
    char stringdata0[33];
    char stringdata1[30];
    char stringdata2[1];
    char stringdata3[31];
    char stringdata4[27];
    char stringdata5[23];
    char stringdata6[24];
    char stringdata7[20];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS = {
    {
        QT_MOC_LITERAL(0, 32),  // "KDDockWidgets::ClassicIndicators"
        QT_MOC_LITERAL(33, 29),  // "innerIndicatorsVisibleChanged"
        QT_MOC_LITERAL(63, 0),  // ""
        QT_MOC_LITERAL(64, 30),  // "outterIndicatorsVisibleChanged"
        QT_MOC_LITERAL(95, 26),  // "tabIndicatorVisibleChanged"
        QT_MOC_LITERAL(122, 22),  // "innerIndicatorsVisible"
        QT_MOC_LITERAL(145, 23),  // "outterIndicatorsVisible"
        QT_MOC_LITERAL(169, 19)   // "tabIndicatorVisible"
    },
    "KDDockWidgets::ClassicIndicators",
    "innerIndicatorsVisibleChanged",
    "",
    "outterIndicatorsVisibleChanged",
    "tabIndicatorVisibleChanged",
    "innerIndicatorsVisible",
    "outterIndicatorsVisible",
    "tabIndicatorVisible"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       3,   35, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   32,    2, 0x06,    4 /* Public */,
       3,    0,   33,    2, 0x06,    5 /* Public */,
       4,    0,   34,    2, 0x06,    6 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       5, QMetaType::Bool, 0x00015001, uint(0), 0,
       6, QMetaType::Bool, 0x00015001, uint(1), 0,
       7, QMetaType::Bool, 0x00015001, uint(2), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::ClassicIndicators::staticMetaObject = { {
    QMetaObject::SuperData::link<DropIndicatorOverlayInterface::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS_t,
        // property 'innerIndicatorsVisible'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'outterIndicatorsVisible'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'tabIndicatorVisible'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ClassicIndicators, std::true_type>,
        // method 'innerIndicatorsVisibleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'outterIndicatorsVisibleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'tabIndicatorVisibleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::ClassicIndicators::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ClassicIndicators *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->innerIndicatorsVisibleChanged(); break;
        case 1: _t->outterIndicatorsVisibleChanged(); break;
        case 2: _t->tabIndicatorVisibleChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ClassicIndicators::*)();
            if (_t _q_method = &ClassicIndicators::innerIndicatorsVisibleChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ClassicIndicators::*)();
            if (_t _q_method = &ClassicIndicators::outterIndicatorsVisibleChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ClassicIndicators::*)();
            if (_t _q_method = &ClassicIndicators::tabIndicatorVisibleChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<ClassicIndicators *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->innerIndicatorsVisible(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->outterIndicatorsVisible(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->tabIndicatorVisible(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
    (void)_a;
}

const QMetaObject *KDDockWidgets::ClassicIndicators::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::ClassicIndicators::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEClassicIndicatorsENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return DropIndicatorOverlayInterface::qt_metacast(_clname);
}

int KDDockWidgets::ClassicIndicators::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DropIndicatorOverlayInterface::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::ClassicIndicators::innerIndicatorsVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void KDDockWidgets::ClassicIndicators::outterIndicatorsVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void KDDockWidgets::ClassicIndicators::tabIndicatorVisibleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
