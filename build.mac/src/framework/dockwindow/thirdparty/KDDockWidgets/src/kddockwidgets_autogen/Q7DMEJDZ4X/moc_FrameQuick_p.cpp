/****************************************************************************
** Meta object code from reading C++ file 'FrameQuick_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/quick/FrameQuick_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FrameQuick_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::FrameQuick",
    "tabTitlesChanged",
    "",
    "updateConstriants",
    "setStackLayout",
    "QQuickItem*",
    "tabWidget"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS_t {
    uint offsetsAndSizes[14];
    char stringdata0[26];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[18];
    char stringdata4[15];
    char stringdata5[12];
    char stringdata6[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS = {
    {
        QT_MOC_LITERAL(0, 25),  // "KDDockWidgets::FrameQuick"
        QT_MOC_LITERAL(26, 16),  // "tabTitlesChanged"
        QT_MOC_LITERAL(43, 0),  // ""
        QT_MOC_LITERAL(44, 17),  // "updateConstriants"
        QT_MOC_LITERAL(62, 14),  // "setStackLayout"
        QT_MOC_LITERAL(77, 11),  // "QQuickItem*"
        QT_MOC_LITERAL(89, 9)   // "tabWidget"
    },
    "KDDockWidgets::FrameQuick",
    "tabTitlesChanged",
    "",
    "updateConstriants",
    "setStackLayout",
    "QQuickItem*",
    "tabWidget"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       1,   37, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   32,    2, 0x06,    2 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    0,   33,    2, 0x0a,    3 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       4,    1,   34,    2, 0x01,    4 /* Protected */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 5,    2,

 // properties: name, type, flags
       6, QMetaType::QObjectStar, 0x00015401, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::FrameQuick::staticMetaObject = { {
    QMetaObject::SuperData::link<Frame::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS_t,
        // property 'tabWidget'
        QtPrivate::TypeAndForceComplete<QObject*, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FrameQuick, std::true_type>,
        // method 'tabTitlesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateConstriants'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setStackLayout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QQuickItem *, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::FrameQuick::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FrameQuick *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->tabTitlesChanged(); break;
        case 1: _t->updateConstriants(); break;
        case 2: _t->setStackLayout((*reinterpret_cast< std::add_pointer_t<QQuickItem*>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QQuickItem* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FrameQuick::*)();
            if (_t _q_method = &FrameQuick::tabTitlesChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<FrameQuick *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QObject**>(_v) = _t->tabWidgetObj(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::FrameQuick::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::FrameQuick::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFrameQuickENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return Frame::qt_metacast(_clname);
}

int KDDockWidgets::FrameQuick::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Frame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::FrameQuick::tabTitlesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
