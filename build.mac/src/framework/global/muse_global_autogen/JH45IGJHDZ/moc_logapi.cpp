/****************************************************************************
** Meta object code from reading C++ file 'logapi.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../src/framework/global/api/logapi.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'logapi.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSmuseSCOPEapiSCOPELogApiENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSmuseSCOPEapiSCOPELogApiENDCLASS = QtMocHelpers::stringData(
    "muse::api::LogApi",
    "error",
    "",
    "message",
    "tag",
    "warn",
    "info",
    "debug"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSmuseSCOPEapiSCOPELogApiENDCLASS_t {
    uint offsetsAndSizes[16];
    char stringdata0[18];
    char stringdata1[6];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[4];
    char stringdata5[5];
    char stringdata6[5];
    char stringdata7[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSmuseSCOPEapiSCOPELogApiENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSmuseSCOPEapiSCOPELogApiENDCLASS_t qt_meta_stringdata_CLASSmuseSCOPEapiSCOPELogApiENDCLASS = {
    {
        QT_MOC_LITERAL(0, 17),  // "muse::api::LogApi"
        QT_MOC_LITERAL(18, 5),  // "error"
        QT_MOC_LITERAL(24, 0),  // ""
        QT_MOC_LITERAL(25, 7),  // "message"
        QT_MOC_LITERAL(33, 3),  // "tag"
        QT_MOC_LITERAL(37, 4),  // "warn"
        QT_MOC_LITERAL(42, 4),  // "info"
        QT_MOC_LITERAL(47, 5)   // "debug"
    },
    "muse::api::LogApi",
    "error",
    "",
    "message",
    "tag",
    "warn",
    "info",
    "debug"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSmuseSCOPEapiSCOPELogApiENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   62,    2, 0x02,    1 /* Public */,
       1,    2,   65,    2, 0x02,    3 /* Public */,
       5,    1,   70,    2, 0x02,    6 /* Public */,
       5,    2,   73,    2, 0x02,    8 /* Public */,
       6,    1,   78,    2, 0x02,   11 /* Public */,
       6,    2,   81,    2, 0x02,   13 /* Public */,
       7,    1,   86,    2, 0x02,   16 /* Public */,
       7,    2,   89,    2, 0x02,   18 /* Public */,

 // methods: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    4,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    4,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    4,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    4,    3,

       0        // eod
};

Q_CONSTINIT const QMetaObject muse::api::LogApi::staticMetaObject = { {
    QMetaObject::SuperData::link<api::ApiObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSmuseSCOPEapiSCOPELogApiENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSmuseSCOPEapiSCOPELogApiENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSmuseSCOPEapiSCOPELogApiENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LogApi, std::true_type>,
        // method 'error'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'error'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'warn'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'warn'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'info'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'info'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'debug'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'debug'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void muse::api::LogApi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LogApi *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->error((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->error((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: _t->warn((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->warn((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->info((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->info((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 6: _t->debug((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->debug((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObject *muse::api::LogApi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *muse::api::LogApi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSmuseSCOPEapiSCOPELogApiENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return api::ApiObject::qt_metacast(_clname);
}

int muse::api::LogApi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = api::ApiObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
