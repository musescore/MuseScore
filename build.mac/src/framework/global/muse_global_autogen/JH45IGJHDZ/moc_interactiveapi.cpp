/****************************************************************************
** Meta object code from reading C++ file 'interactiveapi.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../src/framework/global/api/interactiveapi.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'interactiveapi.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS = QtMocHelpers::stringData(
    "muse::api::InteractiveApi",
    "info",
    "",
    "contentTitle",
    "text",
    "openUrl",
    "url"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS_t {
    uint offsetsAndSizes[14];
    char stringdata0[26];
    char stringdata1[5];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[5];
    char stringdata5[8];
    char stringdata6[4];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS_t qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS = {
    {
        QT_MOC_LITERAL(0, 25),  // "muse::api::InteractiveApi"
        QT_MOC_LITERAL(26, 4),  // "info"
        QT_MOC_LITERAL(31, 0),  // ""
        QT_MOC_LITERAL(32, 12),  // "contentTitle"
        QT_MOC_LITERAL(45, 4),  // "text"
        QT_MOC_LITERAL(50, 7),  // "openUrl"
        QT_MOC_LITERAL(58, 3)   // "url"
    },
    "muse::api::InteractiveApi",
    "info",
    "",
    "contentTitle",
    "text",
    "openUrl",
    "url"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS[] = {

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
       1,    2,   26,    2, 0x02,    1 /* Public */,
       5,    1,   31,    2, 0x02,    4 /* Public */,

 // methods: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    4,
    QMetaType::Void, QMetaType::QString,    6,

       0        // eod
};

Q_CONSTINIT const QMetaObject muse::api::InteractiveApi::staticMetaObject = { {
    QMetaObject::SuperData::link<ApiObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<InteractiveApi, std::true_type>,
        // method 'info'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'openUrl'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void muse::api::InteractiveApi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<InteractiveApi *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->info((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->openUrl((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *muse::api::InteractiveApi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *muse::api::InteractiveApi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSmuseSCOPEapiSCOPEInteractiveApiENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return ApiObject::qt_metacast(_clname);
}

int muse::api::InteractiveApi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ApiObject::qt_metacall(_c, _id, _a);
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
