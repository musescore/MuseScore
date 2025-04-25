/****************************************************************************
** Meta object code from reading C++ file 'DockWidgetInstantiator_p.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/private/quick/DockWidgetInstantiator_p.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DockWidgetInstantiator_p.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::DockWidgetInstantiator",
    "uniqueNameChanged",
    "",
    "sourceChanged",
    "dockWidgetChanged",
    "actualTitleBarChanged",
    "titleChanged",
    "title",
    "shown",
    "hidden",
    "iconChanged",
    "widgetChanged",
    "KDDockWidgets::QWidgetOrQuick*",
    "optionsChanged",
    "KDDockWidgets::DockWidgetBase::Options",
    "isFocusedChanged",
    "isOverlayedChanged",
    "isFloatingChanged",
    "removedFromSideBar",
    "windowActiveAboutToChange",
    "activated",
    "addDockWidgetAsTab",
    "KDDockWidgets::DockWidgetInstantiator*",
    "other",
    "KDDockWidgets::InitialVisibilityOption",
    "KDDockWidgets::DockWidgetBase*",
    "addDockWidgetToContainingWindow",
    "KDDockWidgets::Location",
    "location",
    "relativeTo",
    "initialSize",
    "setAsCurrentTab",
    "forceClose",
    "close",
    "show",
    "raise",
    "moveToSideBar",
    "uniqueName",
    "source",
    "dockWidget",
    "KDDockWidgets::DockWidgetQuick*",
    "actualTitleBar",
    "KDDockWidgets::TitleBar*",
    "isFocused",
    "isFloating"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS_t {
    uint offsetsAndSizes[90];
    char stringdata0[38];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[18];
    char stringdata5[22];
    char stringdata6[13];
    char stringdata7[6];
    char stringdata8[6];
    char stringdata9[7];
    char stringdata10[12];
    char stringdata11[14];
    char stringdata12[31];
    char stringdata13[15];
    char stringdata14[39];
    char stringdata15[17];
    char stringdata16[19];
    char stringdata17[18];
    char stringdata18[19];
    char stringdata19[26];
    char stringdata20[10];
    char stringdata21[19];
    char stringdata22[39];
    char stringdata23[6];
    char stringdata24[39];
    char stringdata25[31];
    char stringdata26[32];
    char stringdata27[24];
    char stringdata28[9];
    char stringdata29[11];
    char stringdata30[12];
    char stringdata31[16];
    char stringdata32[11];
    char stringdata33[6];
    char stringdata34[5];
    char stringdata35[6];
    char stringdata36[14];
    char stringdata37[11];
    char stringdata38[7];
    char stringdata39[11];
    char stringdata40[32];
    char stringdata41[15];
    char stringdata42[25];
    char stringdata43[10];
    char stringdata44[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS = {
    {
        QT_MOC_LITERAL(0, 37),  // "KDDockWidgets::DockWidgetInst..."
        QT_MOC_LITERAL(38, 17),  // "uniqueNameChanged"
        QT_MOC_LITERAL(56, 0),  // ""
        QT_MOC_LITERAL(57, 13),  // "sourceChanged"
        QT_MOC_LITERAL(71, 17),  // "dockWidgetChanged"
        QT_MOC_LITERAL(89, 21),  // "actualTitleBarChanged"
        QT_MOC_LITERAL(111, 12),  // "titleChanged"
        QT_MOC_LITERAL(124, 5),  // "title"
        QT_MOC_LITERAL(130, 5),  // "shown"
        QT_MOC_LITERAL(136, 6),  // "hidden"
        QT_MOC_LITERAL(143, 11),  // "iconChanged"
        QT_MOC_LITERAL(155, 13),  // "widgetChanged"
        QT_MOC_LITERAL(169, 30),  // "KDDockWidgets::QWidgetOrQuick*"
        QT_MOC_LITERAL(200, 14),  // "optionsChanged"
        QT_MOC_LITERAL(215, 38),  // "KDDockWidgets::DockWidgetBase..."
        QT_MOC_LITERAL(254, 16),  // "isFocusedChanged"
        QT_MOC_LITERAL(271, 18),  // "isOverlayedChanged"
        QT_MOC_LITERAL(290, 17),  // "isFloatingChanged"
        QT_MOC_LITERAL(308, 18),  // "removedFromSideBar"
        QT_MOC_LITERAL(327, 25),  // "windowActiveAboutToChange"
        QT_MOC_LITERAL(353, 9),  // "activated"
        QT_MOC_LITERAL(363, 18),  // "addDockWidgetAsTab"
        QT_MOC_LITERAL(382, 38),  // "KDDockWidgets::DockWidgetInst..."
        QT_MOC_LITERAL(421, 5),  // "other"
        QT_MOC_LITERAL(427, 38),  // "KDDockWidgets::InitialVisibil..."
        QT_MOC_LITERAL(466, 30),  // "KDDockWidgets::DockWidgetBase*"
        QT_MOC_LITERAL(497, 31),  // "addDockWidgetToContainingWindow"
        QT_MOC_LITERAL(529, 23),  // "KDDockWidgets::Location"
        QT_MOC_LITERAL(553, 8),  // "location"
        QT_MOC_LITERAL(562, 10),  // "relativeTo"
        QT_MOC_LITERAL(573, 11),  // "initialSize"
        QT_MOC_LITERAL(585, 15),  // "setAsCurrentTab"
        QT_MOC_LITERAL(601, 10),  // "forceClose"
        QT_MOC_LITERAL(612, 5),  // "close"
        QT_MOC_LITERAL(618, 4),  // "show"
        QT_MOC_LITERAL(623, 5),  // "raise"
        QT_MOC_LITERAL(629, 13),  // "moveToSideBar"
        QT_MOC_LITERAL(643, 10),  // "uniqueName"
        QT_MOC_LITERAL(654, 6),  // "source"
        QT_MOC_LITERAL(661, 10),  // "dockWidget"
        QT_MOC_LITERAL(672, 31),  // "KDDockWidgets::DockWidgetQuick*"
        QT_MOC_LITERAL(704, 14),  // "actualTitleBar"
        QT_MOC_LITERAL(719, 24),  // "KDDockWidgets::TitleBar*"
        QT_MOC_LITERAL(744, 9),  // "isFocused"
        QT_MOC_LITERAL(754, 10)   // "isFloating"
    },
    "KDDockWidgets::DockWidgetInstantiator",
    "uniqueNameChanged",
    "",
    "sourceChanged",
    "dockWidgetChanged",
    "actualTitleBarChanged",
    "titleChanged",
    "title",
    "shown",
    "hidden",
    "iconChanged",
    "widgetChanged",
    "KDDockWidgets::QWidgetOrQuick*",
    "optionsChanged",
    "KDDockWidgets::DockWidgetBase::Options",
    "isFocusedChanged",
    "isOverlayedChanged",
    "isFloatingChanged",
    "removedFromSideBar",
    "windowActiveAboutToChange",
    "activated",
    "addDockWidgetAsTab",
    "KDDockWidgets::DockWidgetInstantiator*",
    "other",
    "KDDockWidgets::InitialVisibilityOption",
    "KDDockWidgets::DockWidgetBase*",
    "addDockWidgetToContainingWindow",
    "KDDockWidgets::Location",
    "location",
    "relativeTo",
    "initialSize",
    "setAsCurrentTab",
    "forceClose",
    "close",
    "show",
    "raise",
    "moveToSideBar",
    "uniqueName",
    "source",
    "dockWidget",
    "KDDockWidgets::DockWidgetQuick*",
    "actualTitleBar",
    "KDDockWidgets::TitleBar*",
    "isFocused",
    "isFloating"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      33,   14, // methods
       7,  327, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      15,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  212,    2, 0x06,    8 /* Public */,
       3,    0,  213,    2, 0x06,    9 /* Public */,
       4,    0,  214,    2, 0x06,   10 /* Public */,
       5,    0,  215,    2, 0x06,   11 /* Public */,
       6,    1,  216,    2, 0x06,   12 /* Public */,
       8,    0,  219,    2, 0x06,   14 /* Public */,
       9,    0,  220,    2, 0x06,   15 /* Public */,
      10,    0,  221,    2, 0x06,   16 /* Public */,
      11,    1,  222,    2, 0x06,   17 /* Public */,
      13,    1,  225,    2, 0x06,   19 /* Public */,
      15,    1,  228,    2, 0x06,   21 /* Public */,
      16,    1,  231,    2, 0x06,   23 /* Public */,
      17,    1,  234,    2, 0x06,   25 /* Public */,
      18,    0,  237,    2, 0x06,   27 /* Public */,
      19,    1,  238,    2, 0x06,   28 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      21,    2,  241,    2, 0x02,   30 /* Public */,
      21,    1,  246,    2, 0x22,   33 /* Public | MethodCloned */,
      21,    2,  249,    2, 0x02,   35 /* Public */,
      21,    1,  254,    2, 0x22,   38 /* Public | MethodCloned */,
      26,    5,  257,    2, 0x02,   40 /* Public */,
      26,    4,  268,    2, 0x22,   46 /* Public | MethodCloned */,
      26,    3,  277,    2, 0x22,   51 /* Public | MethodCloned */,
      26,    2,  284,    2, 0x22,   55 /* Public | MethodCloned */,
      26,    5,  289,    2, 0x02,   58 /* Public */,
      26,    4,  300,    2, 0x22,   64 /* Public | MethodCloned */,
      26,    3,  309,    2, 0x22,   69 /* Public | MethodCloned */,
      26,    2,  316,    2, 0x22,   73 /* Public | MethodCloned */,
      31,    0,  321,    2, 0x02,   76 /* Public */,
      32,    0,  322,    2, 0x02,   77 /* Public */,
      33,    0,  323,    2, 0x02,   78 /* Public */,
      34,    0,  324,    2, 0x02,   79 /* Public */,
      35,    0,  325,    2, 0x02,   80 /* Public */,
      36,    0,  326,    2, 0x02,   81 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 12,    2,
    QMetaType::Void, 0x80000000 | 14,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   20,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 22, 0x80000000 | 24,   23,    2,
    QMetaType::Void, 0x80000000 | 22,   23,
    QMetaType::Void, 0x80000000 | 25, 0x80000000 | 24,   23,    2,
    QMetaType::Void, 0x80000000 | 25,   23,
    QMetaType::Void, 0x80000000 | 25, 0x80000000 | 27, 0x80000000 | 25, QMetaType::QSize, 0x80000000 | 24,   23,   28,   29,   30,    2,
    QMetaType::Void, 0x80000000 | 25, 0x80000000 | 27, 0x80000000 | 25, QMetaType::QSize,   23,   28,   29,   30,
    QMetaType::Void, 0x80000000 | 25, 0x80000000 | 27, 0x80000000 | 25,   23,   28,   29,
    QMetaType::Void, 0x80000000 | 25, 0x80000000 | 27,   23,   28,
    QMetaType::Void, 0x80000000 | 22, 0x80000000 | 27, 0x80000000 | 22, QMetaType::QSize, 0x80000000 | 24,   23,   28,   29,   30,    2,
    QMetaType::Void, 0x80000000 | 22, 0x80000000 | 27, 0x80000000 | 22, QMetaType::QSize,   23,   28,   29,   30,
    QMetaType::Void, 0x80000000 | 22, 0x80000000 | 27, 0x80000000 | 22,   23,   28,   29,
    QMetaType::Void, 0x80000000 | 22, 0x80000000 | 27,   23,   28,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      37, QMetaType::QString, 0x00015103, uint(0), 0,
      38, QMetaType::QString, 0x00015103, uint(1), 0,
      39, 0x80000000 | 40, 0x00015009, uint(2), 0,
      41, 0x80000000 | 42, 0x00015009, uint(3), 0,
       7, QMetaType::QString, 0x00015103, uint(4), 0,
      43, QMetaType::Bool, 0x00015001, uint(10), 0,
      44, QMetaType::Bool, 0x00015003, uint(12), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::DockWidgetInstantiator::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickItem::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS_t,
        // property 'uniqueName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'source'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'dockWidget'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetQuick*, std::true_type>,
        // property 'actualTitleBar'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::TitleBar*, std::true_type>,
        // property 'title'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'isFocused'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'isFloating'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DockWidgetInstantiator, std::true_type>,
        // method 'uniqueNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sourceChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'dockWidgetChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'actualTitleBarChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'titleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'shown'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'hidden'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'iconChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'widgetChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::QWidgetOrQuick *, std::false_type>,
        // method 'optionsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase::Options, std::false_type>,
        // method 'isFocusedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'isOverlayedChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'isFloatingChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'removedFromSideBar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'windowActiveAboutToChange'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'addDockWidgetAsTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::InitialVisibilityOption, std::false_type>,
        // method 'addDockWidgetAsTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        // method 'addDockWidgetAsTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::InitialVisibilityOption, std::false_type>,
        // method 'addDockWidgetAsTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSize, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::InitialVisibilityOption, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSize, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSize, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::InitialVisibilityOption, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QSize, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetInstantiator *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        // method 'setAsCurrentTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'forceClose'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'close'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'show'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'raise'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'moveToSideBar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void KDDockWidgets::DockWidgetInstantiator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DockWidgetInstantiator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->uniqueNameChanged(); break;
        case 1: _t->sourceChanged(); break;
        case 2: _t->dockWidgetChanged(); break;
        case 3: _t->actualTitleBarChanged(); break;
        case 4: _t->titleChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->shown(); break;
        case 6: _t->hidden(); break;
        case 7: _t->iconChanged(); break;
        case 8: _t->widgetChanged((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::QWidgetOrQuick*>>(_a[1]))); break;
        case 9: _t->optionsChanged((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase::Options>>(_a[1]))); break;
        case 10: _t->isFocusedChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 11: _t->isOverlayedChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->isFloatingChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 13: _t->removedFromSideBar(); break;
        case 14: _t->windowActiveAboutToChange((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->addDockWidgetAsTab((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::InitialVisibilityOption>>(_a[2]))); break;
        case 16: _t->addDockWidgetAsTab((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1]))); break;
        case 17: _t->addDockWidgetAsTab((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::InitialVisibilityOption>>(_a[2]))); break;
        case 18: _t->addDockWidgetAsTab((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 19: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSize>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::InitialVisibilityOption>>(_a[5]))); break;
        case 20: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSize>>(_a[4]))); break;
        case 21: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3]))); break;
        case 22: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2]))); break;
        case 23: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSize>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::InitialVisibilityOption>>(_a[5]))); break;
        case 24: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QSize>>(_a[4]))); break;
        case 25: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[3]))); break;
        case 26: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetInstantiator*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2]))); break;
        case 27: _t->setAsCurrentTab(); break;
        case 28: _t->forceClose(); break;
        case 29: { bool _r = _t->close();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 30: _t->show(); break;
        case 31: _t->raise(); break;
        case 32: _t->moveToSideBar(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 15:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetInstantiator* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::InitialVisibilityOption >(); break;
            }
            break;
        case 16:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetInstantiator* >(); break;
            }
            break;
        case 17:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::InitialVisibilityOption >(); break;
            }
            break;
        case 18:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            }
            break;
        case 19:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            case 4:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::InitialVisibilityOption >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        case 20:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        case 21:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        case 22:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        case 23:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetInstantiator* >(); break;
            case 4:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::InitialVisibilityOption >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        case 24:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetInstantiator* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        case 25:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetInstantiator* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        case 26:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetInstantiator* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DockWidgetInstantiator::*)();
            if (_t _q_method = &DockWidgetInstantiator::uniqueNameChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)();
            if (_t _q_method = &DockWidgetInstantiator::sourceChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)();
            if (_t _q_method = &DockWidgetInstantiator::dockWidgetChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)();
            if (_t _q_method = &DockWidgetInstantiator::actualTitleBarChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)(const QString & );
            if (_t _q_method = &DockWidgetInstantiator::titleChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)();
            if (_t _q_method = &DockWidgetInstantiator::shown; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)();
            if (_t _q_method = &DockWidgetInstantiator::hidden; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)();
            if (_t _q_method = &DockWidgetInstantiator::iconChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)(KDDockWidgets::QWidgetOrQuick * );
            if (_t _q_method = &DockWidgetInstantiator::widgetChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)(KDDockWidgets::DockWidgetBase::Options );
            if (_t _q_method = &DockWidgetInstantiator::optionsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)(bool );
            if (_t _q_method = &DockWidgetInstantiator::isFocusedChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)(bool );
            if (_t _q_method = &DockWidgetInstantiator::isOverlayedChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)(bool );
            if (_t _q_method = &DockWidgetInstantiator::isFloatingChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)();
            if (_t _q_method = &DockWidgetInstantiator::removedFromSideBar; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (DockWidgetInstantiator::*)(bool );
            if (_t _q_method = &DockWidgetInstantiator::windowActiveAboutToChange; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< KDDockWidgets::DockWidgetQuick* >(); break;
        }
    } else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<DockWidgetInstantiator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->uniqueName(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->source(); break;
        case 2: *reinterpret_cast< KDDockWidgets::DockWidgetQuick**>(_v) = _t->dockWidget(); break;
        case 3: *reinterpret_cast< KDDockWidgets::TitleBar**>(_v) = _t->actualTitleBar(); break;
        case 4: *reinterpret_cast< QString*>(_v) = _t->title(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->isFocused(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->isFloating(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<DockWidgetInstantiator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setUniqueName(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->setSource(*reinterpret_cast< QString*>(_v)); break;
        case 4: _t->setTitle(*reinterpret_cast< QString*>(_v)); break;
        case 6: _t->setFloating(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::DockWidgetInstantiator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::DockWidgetInstantiator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetInstantiatorENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QQuickItem::qt_metacast(_clname);
}

int KDDockWidgets::DockWidgetInstantiator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 33)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 33;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 33)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 33;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::DockWidgetInstantiator::uniqueNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void KDDockWidgets::DockWidgetInstantiator::sourceChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void KDDockWidgets::DockWidgetInstantiator::dockWidgetChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void KDDockWidgets::DockWidgetInstantiator::actualTitleBarChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void KDDockWidgets::DockWidgetInstantiator::titleChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void KDDockWidgets::DockWidgetInstantiator::shown()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void KDDockWidgets::DockWidgetInstantiator::hidden()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void KDDockWidgets::DockWidgetInstantiator::iconChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void KDDockWidgets::DockWidgetInstantiator::widgetChanged(KDDockWidgets::QWidgetOrQuick * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void KDDockWidgets::DockWidgetInstantiator::optionsChanged(KDDockWidgets::DockWidgetBase::Options _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void KDDockWidgets::DockWidgetInstantiator::isFocusedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void KDDockWidgets::DockWidgetInstantiator::isOverlayedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void KDDockWidgets::DockWidgetInstantiator::isFloatingChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void KDDockWidgets::DockWidgetInstantiator::removedFromSideBar()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void KDDockWidgets::DockWidgetInstantiator::windowActiveAboutToChange(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}
QT_WARNING_POP
