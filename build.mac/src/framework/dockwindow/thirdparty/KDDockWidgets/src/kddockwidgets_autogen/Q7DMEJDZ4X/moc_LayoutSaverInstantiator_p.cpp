/****************************************************************************
** Meta object code from reading C++ file 'LayoutSaverInstantiator_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/quick/LayoutSaverInstantiator_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LayoutSaverInstantiator_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::LayoutSaverInstantiator",
    "saveToFile",
    "",
    "jsonFilename",
    "restoreFromFile"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS_t {
    uint offsetsAndSizes[10];
    char stringdata0[39];
    char stringdata1[11];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[16];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS = {
    {
        QT_MOC_LITERAL(0, 38),  // "KDDockWidgets::LayoutSaverIns..."
        QT_MOC_LITERAL(39, 10),  // "saveToFile"
        QT_MOC_LITERAL(50, 0),  // ""
        QT_MOC_LITERAL(51, 12),  // "jsonFilename"
        QT_MOC_LITERAL(64, 15)   // "restoreFromFile"
    },
    "KDDockWidgets::LayoutSaverInstantiator",
    "saveToFile",
    "",
    "jsonFilename",
    "restoreFromFile"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   26,    2, 0x02,    1 /* Public */,
       4,    1,   29,    2, 0x02,    3 /* Public */,

 // methods: parameters
    QMetaType::Bool, QMetaType::QString,    3,
    QMetaType::Bool, QMetaType::QString,    3,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::LayoutSaverInstantiator::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LayoutSaverInstantiator, std::true_type>,
        // method 'saveToFile'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'restoreFromFile'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::LayoutSaverInstantiator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LayoutSaverInstantiator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: { bool _r = _t->saveToFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 1: { bool _r = _t->restoreFromFile((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

const QMetaObject *KDDockWidgets::LayoutSaverInstantiator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::LayoutSaverInstantiator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPELayoutSaverInstantiatorENDCLASS.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "LayoutSaver"))
        return static_cast< LayoutSaver*>(this);
    return QObject::qt_metacast(_clname);
}

int KDDockWidgets::LayoutSaverInstantiator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
