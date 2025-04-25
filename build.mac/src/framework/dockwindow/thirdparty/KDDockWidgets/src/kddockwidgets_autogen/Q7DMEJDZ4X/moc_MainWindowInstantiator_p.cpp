/****************************************************************************
** Meta object code from reading C++ file 'MainWindowInstantiator_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/quick/MainWindowInstantiator_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindowInstantiator_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::MainWindowInstantiator",
    "uniqueNameChanged",
    "",
    "optionsChanged",
    "addDockWidget",
    "KDDockWidgets::DockWidgetBase*",
    "dockWidget",
    "KDDockWidgets::Location",
    "location",
    "relativeTo",
    "initialSize",
    "KDDockWidgets::InitialVisibilityOption",
    "KDDockWidgets::DockWidgetInstantiator*",
    "layoutEqually",
    "layoutParentContainerEqually",
    "moveToSideBar",
    "KDDockWidgets::SideBarLocation",
    "restoreFromSideBar",
    "overlayOnSideBar",
    "toggleOverlayOnSideBar",
    "clearSideBarOverlay",
    "deleteFrame",
    "sideBarForDockWidget",
    "KDDockWidgets::SideBar*",
    "const KDDockWidgets::DockWidgetBase*",
    "sideBarIsVisible",
    "closeDockWidgets",
    "force",
    "uniqueName",
    "options",
    "KDDockWidgets::MainWindowOptions",
    "isMDI",
    "affinities"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS_t {
    uint offsetsAndSizes[66];
    char stringdata0[38];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[15];
    char stringdata4[14];
    char stringdata5[31];
    char stringdata6[11];
    char stringdata7[24];
    char stringdata8[9];
    char stringdata9[11];
    char stringdata10[12];
    char stringdata11[39];
    char stringdata12[39];
    char stringdata13[14];
    char stringdata14[29];
    char stringdata15[14];
    char stringdata16[31];
    char stringdata17[19];
    char stringdata18[17];
    char stringdata19[23];
    char stringdata20[20];
    char stringdata21[12];
    char stringdata22[21];
    char stringdata23[24];
    char stringdata24[37];
    char stringdata25[17];
    char stringdata26[17];
    char stringdata27[6];
    char stringdata28[11];
    char stringdata29[8];
    char stringdata30[33];
    char stringdata31[6];
    char stringdata32[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS = {
    {
        QT_MOC_LITERAL(0, 37),  // "KDDockWidgets::MainWindowInst..."
        QT_MOC_LITERAL(38, 17),  // "uniqueNameChanged"
        QT_MOC_LITERAL(56, 0),  // ""
        QT_MOC_LITERAL(57, 14),  // "optionsChanged"
        QT_MOC_LITERAL(72, 13),  // "addDockWidget"
        QT_MOC_LITERAL(86, 30),  // "KDDockWidgets::DockWidgetBase*"
        QT_MOC_LITERAL(117, 10),  // "dockWidget"
        QT_MOC_LITERAL(128, 23),  // "KDDockWidgets::Location"
        QT_MOC_LITERAL(152, 8),  // "location"
        QT_MOC_LITERAL(161, 10),  // "relativeTo"
        QT_MOC_LITERAL(172, 11),  // "initialSize"
        QT_MOC_LITERAL(184, 38),  // "KDDockWidgets::InitialVisibil..."
        QT_MOC_LITERAL(223, 38),  // "KDDockWidgets::DockWidgetInst..."
        QT_MOC_LITERAL(262, 13),  // "layoutEqually"
        QT_MOC_LITERAL(276, 28),  // "layoutParentContainerEqually"
        QT_MOC_LITERAL(305, 13),  // "moveToSideBar"
        QT_MOC_LITERAL(319, 30),  // "KDDockWidgets::SideBarLocation"
        QT_MOC_LITERAL(350, 18),  // "restoreFromSideBar"
        QT_MOC_LITERAL(369, 16),  // "overlayOnSideBar"
        QT_MOC_LITERAL(386, 22),  // "toggleOverlayOnSideBar"
        QT_MOC_LITERAL(409, 19),  // "clearSideBarOverlay"
        QT_MOC_LITERAL(429, 11),  // "deleteFrame"
        QT_MOC_LITERAL(441, 20),  // "sideBarForDockWidget"
        QT_MOC_LITERAL(462, 23),  // "KDDockWidgets::SideBar*"
        QT_MOC_LITERAL(486, 36),  // "const KDDockWidgets::DockWidg..."
        QT_MOC_LITERAL(523, 16),  // "sideBarIsVisible"
        QT_MOC_LITERAL(540, 16),  // "closeDockWidgets"
        QT_MOC_LITERAL(557, 5),  // "force"
        QT_MOC_LITERAL(563, 10),  // "uniqueName"
        QT_MOC_LITERAL(574, 7),  // "options"
        QT_MOC_LITERAL(582, 32),  // "KDDockWidgets::MainWindowOptions"
        QT_MOC_LITERAL(615, 5),  // "isMDI"
        QT_MOC_LITERAL(621, 10)   // "affinities"
    },
    "KDDockWidgets::MainWindowInstantiator",
    "uniqueNameChanged",
    "",
    "optionsChanged",
    "addDockWidget",
    "KDDockWidgets::DockWidgetBase*",
    "dockWidget",
    "KDDockWidgets::Location",
    "location",
    "relativeTo",
    "initialSize",
    "KDDockWidgets::InitialVisibilityOption",
    "KDDockWidgets::DockWidgetInstantiator*",
    "layoutEqually",
    "layoutParentContainerEqually",
    "moveToSideBar",
    "KDDockWidgets::SideBarLocation",
    "restoreFromSideBar",
    "overlayOnSideBar",
    "toggleOverlayOnSideBar",
    "clearSideBarOverlay",
    "deleteFrame",
    "sideBarForDockWidget",
    "KDDockWidgets::SideBar*",
    "const KDDockWidgets::DockWidgetBase*",
    "sideBarIsVisible",
    "closeDockWidgets",
    "force",
    "uniqueName",
    "options",
    "KDDockWidgets::MainWindowOptions",
    "isMDI",
    "affinities"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       4,  253, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  152,    2, 0x06,    5 /* Public */,
       3,    0,  153,    2, 0x06,    6 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       4,    5,  154,    2, 0x02,    7 /* Public */,
       4,    4,  165,    2, 0x22,   13 /* Public | MethodCloned */,
       4,    3,  174,    2, 0x22,   18 /* Public | MethodCloned */,
       4,    2,  181,    2, 0x22,   22 /* Public | MethodCloned */,
       4,    5,  186,    2, 0x02,   25 /* Public */,
       4,    4,  197,    2, 0x22,   31 /* Public | MethodCloned */,
       4,    3,  206,    2, 0x22,   36 /* Public | MethodCloned */,
       4,    2,  213,    2, 0x22,   40 /* Public | MethodCloned */,
      13,    0,  218,    2, 0x02,   43 /* Public */,
      14,    1,  219,    2, 0x02,   44 /* Public */,
      15,    1,  222,    2, 0x02,   46 /* Public */,
      15,    2,  225,    2, 0x02,   48 /* Public */,
      17,    1,  230,    2, 0x02,   51 /* Public */,
      18,    1,  233,    2, 0x02,   53 /* Public */,
      19,    1,  236,    2, 0x02,   55 /* Public */,
      20,    1,  239,    2, 0x02,   57 /* Public */,
      20,    0,  242,    2, 0x22,   59 /* Public | MethodCloned */,
      22,    1,  243,    2, 0x102,   60 /* Public | MethodIsConst  */,
      25,    1,  246,    2, 0x102,   62 /* Public | MethodIsConst  */,
      26,    1,  249,    2, 0x02,   64 /* Public */,
      26,    0,  252,    2, 0x22,   66 /* Public | MethodCloned */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 7, 0x80000000 | 5, QMetaType::QSize, 0x80000000 | 11,    6,    8,    9,   10,    2,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 7, 0x80000000 | 5, QMetaType::QSize,    6,    8,    9,   10,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 7, 0x80000000 | 5,    6,    8,    9,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 7,    6,    8,
    QMetaType::Void, 0x80000000 | 12, 0x80000000 | 7, 0x80000000 | 12, QMetaType::QSize, 0x80000000 | 11,    6,    8,    9,   10,    2,
    QMetaType::Void, 0x80000000 | 12, 0x80000000 | 7, 0x80000000 | 12, QMetaType::QSize,    6,    8,    9,   10,
    QMetaType::Void, 0x80000000 | 12, 0x80000000 | 7, 0x80000000 | 12,    6,    8,    9,
    QMetaType::Void, 0x80000000 | 12, 0x80000000 | 7,    6,    8,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 16,    2,    2,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, QMetaType::Bool,   21,
    QMetaType::Void,
    0x80000000 | 23, 0x80000000 | 24,    2,
    QMetaType::Bool, 0x80000000 | 16,    2,
    QMetaType::Bool, QMetaType::Bool,   27,
    QMetaType::Bool,

 // properties: name, type, flags
      28, QMetaType::QString, 0x00015103, uint(0), 0,
      29, 0x80000000 | 30, 0x0001510b, uint(1), 0,
      31, QMetaType::Bool, 0x00015401, uint(-1), 0,
      32, QMetaType::QStringList, 0x00015401, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT static const QMetaObject::SuperData qt_meta_extradata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS[] = {
    QMetaObject::SuperData::link<KDDockWidgets::staticMetaObject>(),
    nullptr
};

Q_CONSTINIT const QMetaObject KDDockWidgets::MainWindowInstantiator::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickItem::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS,
    qt_static_metacall,
    qt_meta_extradata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS_t,
        // property 'uniqueName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'options'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::MainWindowOptions, std::true_type>,
        // property 'isMDI'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'affinities'
        QtPrivate::TypeAndForceComplete<QStringList, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindowInstantiator, std::true_type>,
        // method 'uniqueNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'optionsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSize, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::InitialVisibilityOption, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSize, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSize, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::InitialVisibilityOption, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSize, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        // method 'layoutEqually'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'layoutParentContainerEqually'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'moveToSideBar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'moveToSideBar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::SideBarLocation, std::false_type>,
        // method 'restoreFromSideBar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'overlayOnSideBar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'toggleOverlayOnSideBar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'clearSideBarOverlay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'clearSideBarOverlay'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sideBarForDockWidget'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::SideBar *, std::false_type>,
        QtPrivate::TypeAndForceComplete<const KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'sideBarIsVisible'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::SideBarLocation, std::false_type>,
        // method 'closeDockWidgets'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'closeDockWidgets'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::MainWindowInstantiator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindowInstantiator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->uniqueNameChanged(); break;
        case 1: _t->optionsChanged(); break;
        case 2: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSize>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::InitialVisibilityOption>>(_a[5]))); break;
        case 3: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSize>>(_a[4]))); break;
        case 4: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3]))); break;
        case 5: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2]))); break;
        case 6: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSize>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::InitialVisibilityOption>>(_a[5]))); break;
        case 7: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSize>>(_a[4]))); break;
        case 8: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[3]))); break;
        case 9: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2]))); break;
        case 10: _t->layoutEqually(); break;
        case 11: _t->layoutParentContainerEqually((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 12: _t->moveToSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 13: _t->moveToSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::SideBarLocation>>(_a[2]))); break;
        case 14: _t->restoreFromSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 15: _t->overlayOnSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 16: _t->toggleOverlayOnSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 17: _t->clearSideBarOverlay((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 18: _t->clearSideBarOverlay(); break;
        case 19: { KDDockWidgets::SideBar* _r = _t->sideBarForDockWidget((*reinterpret_cast< std::add_pointer_t<const KDDockWidgets::DockWidgetBase*>>(_a[1])));
            if (_a[0]) *reinterpret_cast< KDDockWidgets::SideBar**>(_a[0]) = std::move(_r); }  break;
        case 20: { bool _r = _t->sideBarIsVisible((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::SideBarLocation>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 21: { bool _r = _t->closeDockWidgets((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 22: { bool _r = _t->closeDockWidgets();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 4:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::InitialVisibilityOption >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 4:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::InitialVisibilityOption >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainWindowInstantiator::*)();
            if (_t _q_method = &MainWindowInstantiator::uniqueNameChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MainWindowInstantiator::*)();
            if (_t _q_method = &MainWindowInstantiator::optionsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<MainWindowInstantiator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->uniqueName(); break;
        case 1: *reinterpret_cast< KDDockWidgets::MainWindowOptions*>(_v) = _t->options(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->isMDI(); break;
        case 3: *reinterpret_cast< QStringList*>(_v) = _t->affinities(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<MainWindowInstantiator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setUniqueName(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->setOptions(*reinterpret_cast< KDDockWidgets::MainWindowOptions*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::MainWindowInstantiator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::MainWindowInstantiator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowInstantiatorENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QQuickItem::qt_metacast(_clname);
}

int KDDockWidgets::MainWindowInstantiator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::MainWindowInstantiator::uniqueNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void KDDockWidgets::MainWindowInstantiator::optionsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
