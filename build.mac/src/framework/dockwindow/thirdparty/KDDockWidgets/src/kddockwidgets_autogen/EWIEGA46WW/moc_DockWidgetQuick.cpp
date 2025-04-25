/****************************************************************************
** Meta object code from reading C++ file 'DockWidgetQuick.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DockWidgetQuick.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::DockWidgetQuick",
    "frameGeometryChanged",
    "",
    "setWidget",
    "QQuickItem*",
    "widget",
    "frame",
    "KDDockWidgets::Frame*",
    "onGeometryUpdated",
    "actualTitleBarChanged",
    "actualTitleBar"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS_t {
    uint offsetsAndSizes[22];
    char stringdata0[31];
    char stringdata1[21];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[12];
    char stringdata5[7];
    char stringdata6[6];
    char stringdata7[22];
    char stringdata8[18];
    char stringdata9[22];
    char stringdata10[15];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS = {
    {
        QT_MOC_LITERAL(0, 30),  // "KDDockWidgets::DockWidgetQuick"
        QT_MOC_LITERAL(31, 20),  // "frameGeometryChanged"
        QT_MOC_LITERAL(52, 0),  // ""
        QT_MOC_LITERAL(53, 9),  // "setWidget"
        QT_MOC_LITERAL(63, 11),  // "QQuickItem*"
        QT_MOC_LITERAL(75, 6),  // "widget"
        QT_MOC_LITERAL(82, 5),  // "frame"
        QT_MOC_LITERAL(88, 21),  // "KDDockWidgets::Frame*"
        QT_MOC_LITERAL(110, 17),  // "onGeometryUpdated"
        QT_MOC_LITERAL(128, 21),  // "actualTitleBarChanged"
        QT_MOC_LITERAL(150, 14)   // "actualTitleBar"
    },
    "KDDockWidgets::DockWidgetQuick",
    "frameGeometryChanged",
    "",
    "setWidget",
    "QQuickItem*",
    "widget",
    "frame",
    "KDDockWidgets::Frame*",
    "onGeometryUpdated",
    "actualTitleBarChanged",
    "actualTitleBar"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       1,   46, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x06,    2 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       3,    1,   41,    2, 0x02,    4 /* Public */,
       6,    0,   44,    2, 0x102,    6 /* Public | MethodIsConst  */,
       8,    0,   45,    2, 0x02,    7 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QRect,    2,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 4,    5,
    0x80000000 | 7,
    QMetaType::Void,

 // properties: name, type, flags
      10, QMetaType::QObjectStar, 0x00015001, uint(1879048201), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::DockWidgetQuick::staticMetaObject = { {
    QMetaObject::SuperData::link<DockWidgetBase::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS_t,
        // property 'actualTitleBar'
        QtPrivate::TypeAndForceComplete<QObject*, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DockWidgetQuick, std::true_type>,
        // method 'frameGeometryChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QRect, std::false_type>,
        // method 'setWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QQuickItem *, std::false_type>,
        // method 'frame'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Frame *, std::false_type>,
        // method 'onGeometryUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::DockWidgetQuick::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DockWidgetQuick *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->frameGeometryChanged((*reinterpret_cast< std::add_pointer_t<QRect>>(_a[1]))); break;
        case 1: _t->setWidget((*reinterpret_cast< std::add_pointer_t<QQuickItem*>>(_a[1]))); break;
        case 2: { KDDockWidgets::Frame* _r = _t->frame();
            if (_a[0]) *reinterpret_cast< KDDockWidgets::Frame**>(_a[0]) = std::move(_r); }  break;
        case 3: _t->onGeometryUpdated(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
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
            using _t = void (DockWidgetQuick::*)(QRect );
            if (_t _q_method = &DockWidgetQuick::frameGeometryChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<DockWidgetQuick *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QObject**>(_v) = _t->actualTitleBarObj(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::DockWidgetQuick::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::DockWidgetQuick::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return DockWidgetBase::qt_metacast(_clname);
}

int KDDockWidgets::DockWidgetQuick::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = DockWidgetBase::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::DockWidgetQuick::frameGeometryChanged(QRect _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
// If you get a compile error in this function it can be because either
//     a) You are using a NOTIFY signal that does not exist. Fix it.
//     b) You are using a NOTIFY signal that does exist (in a parent class) but has a non-empty parameter list. This is a moc limitation.
[[maybe_unused]] static void checkNotifySignalValidity_CLASSKDDockWidgetsSCOPEDockWidgetQuickENDCLASS(KDDockWidgets::DockWidgetQuick *t) {
    t->actualTitleBarChanged();
}
QT_WARNING_POP
