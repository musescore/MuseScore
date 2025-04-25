/****************************************************************************
** Meta object code from reading C++ file 'TitleBarQuick_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/quick/TitleBarQuick_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TitleBarQuick_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::TitleBarQuick",
    "titleBarQmlItemChanged",
    "",
    "titleBarQmlItem",
    "QQuickItem*",
    "titleBarMouseArea"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS_t {
    uint offsetsAndSizes[12];
    char stringdata0[29];
    char stringdata1[23];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[12];
    char stringdata5[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS = {
    {
        QT_MOC_LITERAL(0, 28),  // "KDDockWidgets::TitleBarQuick"
        QT_MOC_LITERAL(29, 22),  // "titleBarQmlItemChanged"
        QT_MOC_LITERAL(52, 0),  // ""
        QT_MOC_LITERAL(53, 15),  // "titleBarQmlItem"
        QT_MOC_LITERAL(69, 11),  // "QQuickItem*"
        QT_MOC_LITERAL(81, 17)   // "titleBarMouseArea"
    },
    "KDDockWidgets::TitleBarQuick",
    "titleBarQmlItemChanged",
    "",
    "titleBarQmlItem",
    "QQuickItem*",
    "titleBarMouseArea"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       2,   21, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   20,    2, 0x06,    3 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // properties: name, type, flags
       3, 0x80000000 | 4, 0x0001510b, uint(0), 0,
       5, 0x80000000 | 4, 0x00015409, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::TitleBarQuick::staticMetaObject = { {
    QMetaObject::SuperData::link<TitleBar::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS_t,
        // property 'titleBarQmlItem'
        QtPrivate::TypeAndForceComplete<QQuickItem*, std::true_type>,
        // property 'titleBarMouseArea'
        QtPrivate::TypeAndForceComplete<QQuickItem*, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TitleBarQuick, std::true_type>,
        // method 'titleBarQmlItemChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::TitleBarQuick::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TitleBarQuick *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->titleBarQmlItemChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TitleBarQuick::*)();
            if (_t _q_method = &TitleBarQuick::titleBarQmlItemChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QQuickItem* >(); break;
        }
    } else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<TitleBarQuick *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QQuickItem**>(_v) = _t->titleBarQmlItem(); break;
        case 1: *reinterpret_cast< QQuickItem**>(_v) = _t->titleBarMouseArea(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<TitleBarQuick *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setTitleBarQmlItem(*reinterpret_cast< QQuickItem**>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::TitleBarQuick::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::TitleBarQuick::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPETitleBarQuickENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return TitleBar::qt_metacast(_clname);
}

int KDDockWidgets::TitleBarQuick::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = TitleBar::qt_metacall(_c, _id, _a);
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
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::TitleBarQuick::titleBarQmlItemChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
