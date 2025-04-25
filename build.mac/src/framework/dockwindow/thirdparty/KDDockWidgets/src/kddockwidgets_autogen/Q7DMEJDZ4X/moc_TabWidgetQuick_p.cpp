/****************************************************************************
** Meta object code from reading C++ file 'TabWidgetQuick_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/quick/TabWidgetQuick_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TabWidgetQuick_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::TabWidgetQuick",
    "currentTabChanged",
    "",
    "index",
    "currentDockWidgetChanged",
    "KDDockWidgets::DockWidgetBase*",
    "dw",
    "countChanged",
    "setCurrentDockWidget",
    "dockWidgetModel",
    "DockWidgetModel*",
    "tabBar"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS_t {
    uint offsetsAndSizes[24];
    char stringdata0[30];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[6];
    char stringdata4[25];
    char stringdata5[31];
    char stringdata6[3];
    char stringdata7[13];
    char stringdata8[21];
    char stringdata9[16];
    char stringdata10[17];
    char stringdata11[7];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS = {
    {
        QT_MOC_LITERAL(0, 29),  // "KDDockWidgets::TabWidgetQuick"
        QT_MOC_LITERAL(30, 17),  // "currentTabChanged"
        QT_MOC_LITERAL(48, 0),  // ""
        QT_MOC_LITERAL(49, 5),  // "index"
        QT_MOC_LITERAL(55, 24),  // "currentDockWidgetChanged"
        QT_MOC_LITERAL(80, 30),  // "KDDockWidgets::DockWidgetBase*"
        QT_MOC_LITERAL(111, 2),  // "dw"
        QT_MOC_LITERAL(114, 12),  // "countChanged"
        QT_MOC_LITERAL(127, 20),  // "setCurrentDockWidget"
        QT_MOC_LITERAL(148, 15),  // "dockWidgetModel"
        QT_MOC_LITERAL(164, 16),  // "DockWidgetModel*"
        QT_MOC_LITERAL(181, 6)   // "tabBar"
    },
    "KDDockWidgets::TabWidgetQuick",
    "currentTabChanged",
    "",
    "index",
    "currentDockWidgetChanged",
    "KDDockWidgets::DockWidgetBase*",
    "dw",
    "countChanged",
    "setCurrentDockWidget",
    "dockWidgetModel",
    "DockWidgetModel*",
    "tabBar"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       2,   48, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   38,    2, 0x06,    3 /* Public */,
       4,    1,   41,    2, 0x06,    5 /* Public */,
       7,    0,   44,    2, 0x06,    7 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       8,    1,   45,    2, 0x02,    8 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::Int,    3,

 // properties: name, type, flags
       9, 0x80000000 | 10, 0x00015409, uint(-1), 0,
      11, QMetaType::QObjectStar, 0x00015401, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::TabWidgetQuick::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidgetAdapter::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS_t,
        // property 'dockWidgetModel'
        QtPrivate::TypeAndForceComplete<DockWidgetModel*, std::true_type>,
        // property 'tabBar'
        QtPrivate::TypeAndForceComplete<QObject*, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TabWidgetQuick, std::true_type>,
        // method 'currentTabChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'currentDockWidgetChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'countChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setCurrentDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::TabWidgetQuick::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TabWidgetQuick *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->currentTabChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->currentDockWidgetChanged((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 2: _t->countChanged(); break;
        case 3: _t->setCurrentDockWidget((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TabWidgetQuick::*)(int );
            if (_t _q_method = &TabWidgetQuick::currentTabChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TabWidgetQuick::*)(KDDockWidgets::DockWidgetBase * );
            if (_t _q_method = &TabWidgetQuick::currentDockWidgetChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TabWidgetQuick::*)();
            if (_t _q_method = &TabWidgetQuick::countChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< DockWidgetModel* >(); break;
        }
    } else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<TabWidgetQuick *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< DockWidgetModel**>(_v) = _t->dockWidgetModel(); break;
        case 1: *reinterpret_cast< QObject**>(_v) = _t->tabBarObj(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::TabWidgetQuick::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::TabWidgetQuick::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPETabWidgetQuickENDCLASS.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "TabWidget"))
        return static_cast< TabWidget*>(this);
    return QWidgetAdapter::qt_metacast(_clname);
}

int KDDockWidgets::TabWidgetQuick::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidgetAdapter::qt_metacall(_c, _id, _a);
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
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::TabWidgetQuick::currentTabChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void KDDockWidgets::TabWidgetQuick::currentDockWidgetChanged(KDDockWidgets::DockWidgetBase * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void KDDockWidgets::TabWidgetQuick::countChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::DockWidgetModel",
    "countChanged",
    ""
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS_t {
    uint offsetsAndSizes[6];
    char stringdata0[31];
    char stringdata1[13];
    char stringdata2[1];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS = {
    {
        QT_MOC_LITERAL(0, 30),  // "KDDockWidgets::DockWidgetModel"
        QT_MOC_LITERAL(31, 12),  // "countChanged"
        QT_MOC_LITERAL(44, 0)   // ""
    },
    "KDDockWidgets::DockWidgetModel",
    "countChanged",
    ""
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   20,    2, 0x06,    1 /* Public */,

 // signals: parameters
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::DockWidgetModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DockWidgetModel, std::true_type>,
        // method 'countChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::DockWidgetModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DockWidgetModel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->countChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DockWidgetModel::*)();
            if (_t _q_method = &DockWidgetModel::countChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

const QMetaObject *KDDockWidgets::DockWidgetModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::DockWidgetModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetModelENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int KDDockWidgets::DockWidgetModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void KDDockWidgets::DockWidgetModel::countChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
