/****************************************************************************
** Meta object code from reading C++ file 'FloatingWindow_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/FloatingWindow_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FloatingWindow_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::FloatingWindow",
    "activatedChanged",
    "",
    "numFramesChanged",
    "windowStateChanged",
    "QWindowStateChangeEvent*",
    "titleBar",
    "KDDockWidgets::TitleBar*",
    "dropArea",
    "KDDockWidgets::DropArea*"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS_t {
    uint offsetsAndSizes[20];
    char stringdata0[30];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[17];
    char stringdata4[19];
    char stringdata5[25];
    char stringdata6[9];
    char stringdata7[25];
    char stringdata8[9];
    char stringdata9[25];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS = {
    {
        QT_MOC_LITERAL(0, 29),  // "KDDockWidgets::FloatingWindow"
        QT_MOC_LITERAL(30, 16),  // "activatedChanged"
        QT_MOC_LITERAL(47, 0),  // ""
        QT_MOC_LITERAL(48, 16),  // "numFramesChanged"
        QT_MOC_LITERAL(65, 18),  // "windowStateChanged"
        QT_MOC_LITERAL(84, 24),  // "QWindowStateChangeEvent*"
        QT_MOC_LITERAL(109, 8),  // "titleBar"
        QT_MOC_LITERAL(118, 24),  // "KDDockWidgets::TitleBar*"
        QT_MOC_LITERAL(143, 8),  // "dropArea"
        QT_MOC_LITERAL(152, 24)   // "KDDockWidgets::DropArea*"
    },
    "KDDockWidgets::FloatingWindow",
    "activatedChanged",
    "",
    "numFramesChanged",
    "windowStateChanged",
    "QWindowStateChangeEvent*",
    "titleBar",
    "KDDockWidgets::TitleBar*",
    "dropArea",
    "KDDockWidgets::DropArea*"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       2,   37, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   32,    2, 0x06,    3 /* Public */,
       3,    0,   33,    2, 0x06,    4 /* Public */,
       4,    1,   34,    2, 0x06,    5 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    2,

 // properties: name, type, flags
       6, 0x80000000 | 7, 0x00015409, uint(-1), 0,
       8, 0x80000000 | 9, 0x00015409, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::FloatingWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidgetAdapter::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS_t,
        // property 'titleBar'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::TitleBar*, std::true_type>,
        // property 'dropArea'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DropArea*, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FloatingWindow, std::true_type>,
        // method 'activatedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'numFramesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'windowStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QWindowStateChangeEvent *, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::FloatingWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FloatingWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->activatedChanged(); break;
        case 1: _t->numFramesChanged(); break;
        case 2: _t->windowStateChanged((*reinterpret_cast< std::add_pointer_t<QWindowStateChangeEvent*>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FloatingWindow::*)();
            if (_t _q_method = &FloatingWindow::activatedChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FloatingWindow::*)();
            if (_t _q_method = &FloatingWindow::numFramesChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (FloatingWindow::*)(QWindowStateChangeEvent * );
            if (_t _q_method = &FloatingWindow::windowStateChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< KDDockWidgets::DropArea* >(); break;
        }
    } else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<FloatingWindow *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< KDDockWidgets::TitleBar**>(_v) = _t->titleBar(); break;
        case 1: *reinterpret_cast< KDDockWidgets::DropArea**>(_v) = _t->dropArea(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::FloatingWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::FloatingWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEFloatingWindowENDCLASS.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "Draggable"))
        return static_cast< Draggable*>(this);
    return QWidgetAdapter::qt_metacast(_clname);
}

int KDDockWidgets::FloatingWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidgetAdapter::qt_metacall(_c, _id, _a);
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
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::FloatingWindow::activatedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void KDDockWidgets::FloatingWindow::numFramesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void KDDockWidgets::FloatingWindow::windowStateChanged(QWindowStateChangeEvent * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
