/****************************************************************************
** Meta object code from reading C++ file 'processapi.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../src/framework/global/api/processapi.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'processapi.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS = QtMocHelpers::stringData(
    "muse::api::ProcessApi",
    "execute",
    "",
    "program",
    "arguments",
    "startDetached"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS_t {
    uint offsetsAndSizes[12];
    char stringdata0[22];
    char stringdata1[8];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[10];
    char stringdata5[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS_t qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS = {
    {
        QT_MOC_LITERAL(0, 21),  // "muse::api::ProcessApi"
        QT_MOC_LITERAL(22, 7),  // "execute"
        QT_MOC_LITERAL(30, 0),  // ""
        QT_MOC_LITERAL(31, 7),  // "program"
        QT_MOC_LITERAL(39, 9),  // "arguments"
        QT_MOC_LITERAL(49, 13)   // "startDetached"
    },
    "muse::api::ProcessApi",
    "execute",
    "",
    "program",
    "arguments",
    "startDetached"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   38,    2, 0x02,    1 /* Public */,
       1,    1,   43,    2, 0x22,    4 /* Public | MethodCloned */,
       5,    2,   46,    2, 0x02,    6 /* Public */,
       5,    1,   51,    2, 0x22,    9 /* Public | MethodCloned */,

 // methods: parameters
    QMetaType::Int, QMetaType::QString, QMetaType::QStringList,    3,    4,
    QMetaType::Int, QMetaType::QString,    3,
    QMetaType::Bool, QMetaType::QString, QMetaType::QStringList,    3,    4,
    QMetaType::Bool, QMetaType::QString,    3,

       0        // eod
};

Q_CONSTINIT const QMetaObject muse::api::ProcessApi::staticMetaObject = { {
    QMetaObject::SuperData::link<ApiObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ProcessApi, std::true_type>,
        // method 'execute'
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QStringList &, std::false_type>,
        // method 'execute'
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'startDetached'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QStringList &, std::false_type>,
        // method 'startDetached'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void muse::api::ProcessApi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ProcessApi *>(_o);
        (void)_t;
        switch (_id) {
        case 0: { int _r = _t->execute((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QStringList>>(_a[2])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 1: { int _r = _t->execute((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 2: { bool _r = _t->startDetached((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QStringList>>(_a[2])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: { bool _r = _t->startDetached((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

const QMetaObject *muse::api::ProcessApi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *muse::api::ProcessApi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEProcessApiENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return ApiObject::qt_metacast(_clname);
}

int muse::api::ProcessApi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ApiObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
