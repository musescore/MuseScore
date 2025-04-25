/****************************************************************************
** Meta object code from reading C++ file 'Separator_quick.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/multisplitter/Separator_quick.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Separator_quick.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSLayoutingSCOPESeparatorQuickENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSLayoutingSCOPESeparatorQuickENDCLASS = QtMocHelpers::stringData(
    "Layouting::SeparatorQuick",
    "isVerticalChanged",
    "",
    "onMousePressed",
    "onMouseMoved",
    "localPos",
    "onMouseReleased",
    "onMouseDoubleClicked",
    "isVertical"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSLayoutingSCOPESeparatorQuickENDCLASS_t {
    uint offsetsAndSizes[18];
    char stringdata0[26];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[15];
    char stringdata4[13];
    char stringdata5[9];
    char stringdata6[16];
    char stringdata7[21];
    char stringdata8[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSLayoutingSCOPESeparatorQuickENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSLayoutingSCOPESeparatorQuickENDCLASS_t qt_meta_stringdata_CLASSLayoutingSCOPESeparatorQuickENDCLASS = {
    {
        QT_MOC_LITERAL(0, 25),  // "Layouting::SeparatorQuick"
        QT_MOC_LITERAL(26, 17),  // "isVerticalChanged"
        QT_MOC_LITERAL(44, 0),  // ""
        QT_MOC_LITERAL(45, 14),  // "onMousePressed"
        QT_MOC_LITERAL(60, 12),  // "onMouseMoved"
        QT_MOC_LITERAL(73, 8),  // "localPos"
        QT_MOC_LITERAL(82, 15),  // "onMouseReleased"
        QT_MOC_LITERAL(98, 20),  // "onMouseDoubleClicked"
        QT_MOC_LITERAL(119, 10)   // "isVertical"
    },
    "Layouting::SeparatorQuick",
    "isVerticalChanged",
    "",
    "onMousePressed",
    "onMouseMoved",
    "localPos",
    "onMouseReleased",
    "onMouseDoubleClicked",
    "isVertical"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSLayoutingSCOPESeparatorQuickENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       1,   51, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   44,    2, 0x06,    2 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,   45,    2, 0x02,    3 /* Public */,
       4,    1,   46,    2, 0x02,    4 /* Public */,
       6,    0,   49,    2, 0x02,    6 /* Public */,
       7,    0,   50,    2, 0x02,    7 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPointF,    5,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       8, QMetaType::Bool, 0x00015001, uint(0), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject Layouting::SeparatorQuick::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickItem::staticMetaObject>(),
    qt_meta_stringdata_CLASSLayoutingSCOPESeparatorQuickENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSLayoutingSCOPESeparatorQuickENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSLayoutingSCOPESeparatorQuickENDCLASS_t,
        // property 'isVertical'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SeparatorQuick, std::true_type>,
        // method 'isVerticalChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMousePressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMouseMoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QPointF, std::false_type>,
        // method 'onMouseReleased'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMouseDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Layouting::SeparatorQuick::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SeparatorQuick *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->isVerticalChanged(); break;
        case 1: _t->onMousePressed(); break;
        case 2: _t->onMouseMoved((*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 3: _t->onMouseReleased(); break;
        case 4: _t->onMouseDoubleClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SeparatorQuick::*)();
            if (_t _q_method = &SeparatorQuick::isVerticalChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<SeparatorQuick *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->isVertical(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *Layouting::SeparatorQuick::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Layouting::SeparatorQuick::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSLayoutingSCOPESeparatorQuickENDCLASS.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Layouting::Separator"))
        return static_cast< Layouting::Separator*>(this);
    if (!strcmp(_clname, "Layouting::Widget_quick"))
        return static_cast< Layouting::Widget_quick*>(this);
    return QQuickItem::qt_metacast(_clname);
}

int Layouting::SeparatorQuick::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void Layouting::SeparatorQuick::isVerticalChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
