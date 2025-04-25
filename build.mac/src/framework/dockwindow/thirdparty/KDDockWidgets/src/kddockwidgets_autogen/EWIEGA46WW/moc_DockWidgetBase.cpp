/****************************************************************************
** Meta object code from reading C++ file 'DockWidgetBase.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/DockWidgetBase.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DockWidgetBase.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::DockWidgetBase",
    "shown",
    "",
    "hidden",
    "iconChanged",
    "titleChanged",
    "title",
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
    "actualTitleBarChanged",
    "aboutToDeleteOnClose",
    "addDockWidgetAsTab",
    "KDDockWidgets::DockWidgetBase*",
    "other",
    "KDDockWidgets::InitialOption",
    "initialOption",
    "addDockWidgetToContainingWindow",
    "KDDockWidgets::Location",
    "location",
    "relativeTo",
    "toggleAction",
    "QAction*",
    "floatAction",
    "setAsCurrentTab",
    "forceClose",
    "isOpen",
    "show",
    "raise",
    "moveToSideBar",
    "isFocused",
    "isFloating",
    "uniqueName",
    "widget",
    "options",
    "Options",
    "Option",
    "Option_None",
    "Option_NotClosable",
    "Option_NotDockable",
    "Option_DeleteOnClose",
    "IconPlace",
    "TitleBar",
    "TabBar",
    "ToggleAction",
    "All"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS_t {
    uint offsetsAndSizes[106];
    char stringdata0[30];
    char stringdata1[6];
    char stringdata2[1];
    char stringdata3[7];
    char stringdata4[12];
    char stringdata5[13];
    char stringdata6[6];
    char stringdata7[14];
    char stringdata8[31];
    char stringdata9[15];
    char stringdata10[39];
    char stringdata11[17];
    char stringdata12[19];
    char stringdata13[18];
    char stringdata14[19];
    char stringdata15[26];
    char stringdata16[10];
    char stringdata17[22];
    char stringdata18[21];
    char stringdata19[19];
    char stringdata20[31];
    char stringdata21[6];
    char stringdata22[29];
    char stringdata23[14];
    char stringdata24[32];
    char stringdata25[24];
    char stringdata26[9];
    char stringdata27[11];
    char stringdata28[13];
    char stringdata29[9];
    char stringdata30[12];
    char stringdata31[16];
    char stringdata32[11];
    char stringdata33[7];
    char stringdata34[5];
    char stringdata35[6];
    char stringdata36[14];
    char stringdata37[10];
    char stringdata38[11];
    char stringdata39[11];
    char stringdata40[7];
    char stringdata41[8];
    char stringdata42[8];
    char stringdata43[7];
    char stringdata44[12];
    char stringdata45[19];
    char stringdata46[19];
    char stringdata47[21];
    char stringdata48[10];
    char stringdata49[9];
    char stringdata50[7];
    char stringdata51[13];
    char stringdata52[4];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS = {
    {
        QT_MOC_LITERAL(0, 29),  // "KDDockWidgets::DockWidgetBase"
        QT_MOC_LITERAL(30, 5),  // "shown"
        QT_MOC_LITERAL(36, 0),  // ""
        QT_MOC_LITERAL(37, 6),  // "hidden"
        QT_MOC_LITERAL(44, 11),  // "iconChanged"
        QT_MOC_LITERAL(56, 12),  // "titleChanged"
        QT_MOC_LITERAL(69, 5),  // "title"
        QT_MOC_LITERAL(75, 13),  // "widgetChanged"
        QT_MOC_LITERAL(89, 30),  // "KDDockWidgets::QWidgetOrQuick*"
        QT_MOC_LITERAL(120, 14),  // "optionsChanged"
        QT_MOC_LITERAL(135, 38),  // "KDDockWidgets::DockWidgetBase..."
        QT_MOC_LITERAL(174, 16),  // "isFocusedChanged"
        QT_MOC_LITERAL(191, 18),  // "isOverlayedChanged"
        QT_MOC_LITERAL(210, 17),  // "isFloatingChanged"
        QT_MOC_LITERAL(228, 18),  // "removedFromSideBar"
        QT_MOC_LITERAL(247, 25),  // "windowActiveAboutToChange"
        QT_MOC_LITERAL(273, 9),  // "activated"
        QT_MOC_LITERAL(283, 21),  // "actualTitleBarChanged"
        QT_MOC_LITERAL(305, 20),  // "aboutToDeleteOnClose"
        QT_MOC_LITERAL(326, 18),  // "addDockWidgetAsTab"
        QT_MOC_LITERAL(345, 30),  // "KDDockWidgets::DockWidgetBase*"
        QT_MOC_LITERAL(376, 5),  // "other"
        QT_MOC_LITERAL(382, 28),  // "KDDockWidgets::InitialOption"
        QT_MOC_LITERAL(411, 13),  // "initialOption"
        QT_MOC_LITERAL(425, 31),  // "addDockWidgetToContainingWindow"
        QT_MOC_LITERAL(457, 23),  // "KDDockWidgets::Location"
        QT_MOC_LITERAL(481, 8),  // "location"
        QT_MOC_LITERAL(490, 10),  // "relativeTo"
        QT_MOC_LITERAL(501, 12),  // "toggleAction"
        QT_MOC_LITERAL(514, 8),  // "QAction*"
        QT_MOC_LITERAL(523, 11),  // "floatAction"
        QT_MOC_LITERAL(535, 15),  // "setAsCurrentTab"
        QT_MOC_LITERAL(551, 10),  // "forceClose"
        QT_MOC_LITERAL(562, 6),  // "isOpen"
        QT_MOC_LITERAL(569, 4),  // "show"
        QT_MOC_LITERAL(574, 5),  // "raise"
        QT_MOC_LITERAL(580, 13),  // "moveToSideBar"
        QT_MOC_LITERAL(594, 9),  // "isFocused"
        QT_MOC_LITERAL(604, 10),  // "isFloating"
        QT_MOC_LITERAL(615, 10),  // "uniqueName"
        QT_MOC_LITERAL(626, 6),  // "widget"
        QT_MOC_LITERAL(633, 7),  // "options"
        QT_MOC_LITERAL(641, 7),  // "Options"
        QT_MOC_LITERAL(649, 6),  // "Option"
        QT_MOC_LITERAL(656, 11),  // "Option_None"
        QT_MOC_LITERAL(668, 18),  // "Option_NotClosable"
        QT_MOC_LITERAL(687, 18),  // "Option_NotDockable"
        QT_MOC_LITERAL(706, 20),  // "Option_DeleteOnClose"
        QT_MOC_LITERAL(727, 9),  // "IconPlace"
        QT_MOC_LITERAL(737, 8),  // "TitleBar"
        QT_MOC_LITERAL(746, 6),  // "TabBar"
        QT_MOC_LITERAL(753, 12),  // "ToggleAction"
        QT_MOC_LITERAL(766, 3)   // "All"
    },
    "KDDockWidgets::DockWidgetBase",
    "shown",
    "",
    "hidden",
    "iconChanged",
    "titleChanged",
    "title",
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
    "actualTitleBarChanged",
    "aboutToDeleteOnClose",
    "addDockWidgetAsTab",
    "KDDockWidgets::DockWidgetBase*",
    "other",
    "KDDockWidgets::InitialOption",
    "initialOption",
    "addDockWidgetToContainingWindow",
    "KDDockWidgets::Location",
    "location",
    "relativeTo",
    "toggleAction",
    "QAction*",
    "floatAction",
    "setAsCurrentTab",
    "forceClose",
    "isOpen",
    "show",
    "raise",
    "moveToSideBar",
    "isFocused",
    "isFloating",
    "uniqueName",
    "widget",
    "options",
    "Options",
    "Option",
    "Option_None",
    "Option_NotClosable",
    "Option_NotDockable",
    "Option_DeleteOnClose",
    "IconPlace",
    "TitleBar",
    "TabBar",
    "ToggleAction",
    "All"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       6,  234, // properties
       2,  264, // enums/sets
       0,    0, // constructors
       0,       // flags
      13,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  170,    2, 0x06,    7 /* Public */,
       3,    0,  171,    2, 0x06,    8 /* Public */,
       4,    0,  172,    2, 0x06,    9 /* Public */,
       5,    1,  173,    2, 0x06,   10 /* Public */,
       7,    1,  176,    2, 0x06,   12 /* Public */,
       9,    1,  179,    2, 0x06,   14 /* Public */,
      11,    1,  182,    2, 0x06,   16 /* Public */,
      12,    1,  185,    2, 0x06,   18 /* Public */,
      13,    1,  188,    2, 0x06,   20 /* Public */,
      14,    0,  191,    2, 0x06,   22 /* Public */,
      15,    1,  192,    2, 0x06,   23 /* Public */,
      17,    0,  195,    2, 0x06,   25 /* Public */,
      18,    0,  196,    2, 0x06,   26 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      19,    2,  197,    2, 0x02,   27 /* Public */,
      19,    1,  202,    2, 0x22,   30 /* Public | MethodCloned */,
      24,    4,  205,    2, 0x02,   32 /* Public */,
      24,    3,  214,    2, 0x22,   37 /* Public | MethodCloned */,
      24,    2,  221,    2, 0x22,   41 /* Public | MethodCloned */,
      28,    0,  226,    2, 0x102,   44 /* Public | MethodIsConst  */,
      30,    0,  227,    2, 0x102,   45 /* Public | MethodIsConst  */,
      31,    0,  228,    2, 0x02,   46 /* Public */,
      32,    0,  229,    2, 0x02,   47 /* Public */,
      33,    0,  230,    2, 0x102,   48 /* Public | MethodIsConst  */,
      34,    0,  231,    2, 0x02,   49 /* Public */,
      35,    0,  232,    2, 0x02,   50 /* Public */,
      36,    0,  233,    2, 0x02,   51 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, 0x80000000 | 8,    2,
    QMetaType::Void, 0x80000000 | 10,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   16,
    QMetaType::Void,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 20, 0x80000000 | 22,   21,   23,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void, 0x80000000 | 20, 0x80000000 | 25, 0x80000000 | 20, 0x80000000 | 22,   21,   26,   27,   23,
    QMetaType::Void, 0x80000000 | 20, 0x80000000 | 25, 0x80000000 | 20,   21,   26,   27,
    QMetaType::Void, 0x80000000 | 20, 0x80000000 | 25,   21,   26,
    0x80000000 | 29,
    0x80000000 | 29,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      37, QMetaType::Bool, 0x00015001, uint(6), 0,
      38, QMetaType::Bool, 0x00015003, uint(8), 0,
      39, QMetaType::QString, 0x00015401, uint(-1), 0,
       6, QMetaType::QString, 0x00015103, uint(3), 0,
      40, QMetaType::QObjectStar, 0x00015001, uint(4), 0,
      41, 0x80000000 | 10, 0x0001510b, uint(5), 0,

 // enums: name, alias, flags, count, data
      42,   43, 0x0,    4,  274,
      48,   48, 0x2,    4,  282,

 // enum data: key, value
      44, uint(KDDockWidgets::DockWidgetBase::Option_None),
      45, uint(KDDockWidgets::DockWidgetBase::Option_NotClosable),
      46, uint(KDDockWidgets::DockWidgetBase::Option_NotDockable),
      47, uint(KDDockWidgets::DockWidgetBase::Option_DeleteOnClose),
      49, uint(KDDockWidgets::DockWidgetBase::IconPlace::TitleBar),
      50, uint(KDDockWidgets::DockWidgetBase::IconPlace::TabBar),
      51, uint(KDDockWidgets::DockWidgetBase::IconPlace::ToggleAction),
      52, uint(KDDockWidgets::DockWidgetBase::IconPlace::All),

       0        // eod
};

Q_CONSTINIT const QMetaObject KDDockWidgets::DockWidgetBase::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidgetAdapter::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS_t,
        // property 'isFocused'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'isFloating'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'uniqueName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'title'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'widget'
        QtPrivate::TypeAndForceComplete<QObject*, std::true_type>,
        // property 'options'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase::Options, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DockWidgetBase, std::true_type>,
        // method 'shown'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'hidden'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'iconChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'titleChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
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
        // method 'actualTitleBarChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'aboutToDeleteOnClose'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'addDockWidgetAsTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::InitialOption, std::false_type>,
        // method 'addDockWidgetAsTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::InitialOption, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'addDockWidgetToContainingWindow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        // method 'toggleAction'
        QtPrivate::TypeAndForceComplete<QAction *, std::false_type>,
        // method 'floatAction'
        QtPrivate::TypeAndForceComplete<QAction *, std::false_type>,
        // method 'setAsCurrentTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'forceClose'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'isOpen'
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

void KDDockWidgets::DockWidgetBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DockWidgetBase *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->shown(); break;
        case 1: _t->hidden(); break;
        case 2: _t->iconChanged(); break;
        case 3: _t->titleChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->widgetChanged((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::QWidgetOrQuick*>>(_a[1]))); break;
        case 5: _t->optionsChanged((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase::Options>>(_a[1]))); break;
        case 6: _t->isFocusedChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->isOverlayedChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 8: _t->isFloatingChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->removedFromSideBar(); break;
        case 10: _t->windowActiveAboutToChange((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 11: _t->actualTitleBarChanged(); break;
        case 12: _t->aboutToDeleteOnClose(); break;
        case 13: _t->addDockWidgetAsTab((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::InitialOption>>(_a[2]))); break;
        case 14: _t->addDockWidgetAsTab((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 15: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::InitialOption>>(_a[4]))); break;
        case 16: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3]))); break;
        case 17: _t->addDockWidgetToContainingWindow((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2]))); break;
        case 18: { QAction* _r = _t->toggleAction();
            if (_a[0]) *reinterpret_cast< QAction**>(_a[0]) = std::move(_r); }  break;
        case 19: { QAction* _r = _t->floatAction();
            if (_a[0]) *reinterpret_cast< QAction**>(_a[0]) = std::move(_r); }  break;
        case 20: _t->setAsCurrentTab(); break;
        case 21: _t->forceClose(); break;
        case 22: { bool _r = _t->isOpen();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 23: _t->show(); break;
        case 24: _t->raise(); break;
        case 25: _t->moveToSideBar(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 13:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            }
            break;
        case 14:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            }
            break;
        case 15:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        case 16:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 2:
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        case 17:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::DockWidgetBase* >(); break;
            case 1:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< KDDockWidgets::Location >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DockWidgetBase::*)();
            if (_t _q_method = &DockWidgetBase::shown; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)();
            if (_t _q_method = &DockWidgetBase::hidden; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)();
            if (_t _q_method = &DockWidgetBase::iconChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)(const QString & );
            if (_t _q_method = &DockWidgetBase::titleChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)(KDDockWidgets::QWidgetOrQuick * );
            if (_t _q_method = &DockWidgetBase::widgetChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)(KDDockWidgets::DockWidgetBase::Options );
            if (_t _q_method = &DockWidgetBase::optionsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)(bool );
            if (_t _q_method = &DockWidgetBase::isFocusedChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)(bool );
            if (_t _q_method = &DockWidgetBase::isOverlayedChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)(bool );
            if (_t _q_method = &DockWidgetBase::isFloatingChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)();
            if (_t _q_method = &DockWidgetBase::removedFromSideBar; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)(bool );
            if (_t _q_method = &DockWidgetBase::windowActiveAboutToChange; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)();
            if (_t _q_method = &DockWidgetBase::actualTitleBarChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (DockWidgetBase::*)();
            if (_t _q_method = &DockWidgetBase::aboutToDeleteOnClose; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<DockWidgetBase *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->isFocused(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->isFloating(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->uniqueName(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->title(); break;
        case 4: *reinterpret_cast< QObject**>(_v) = _t->widget(); break;
        case 5: *reinterpret_cast< KDDockWidgets::DockWidgetBase::Options*>(_v) = _t->options(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<DockWidgetBase *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 1: _t->setFloating(*reinterpret_cast< bool*>(_v)); break;
        case 3: _t->setTitle(*reinterpret_cast< QString*>(_v)); break;
        case 5: _t->setOptions(*reinterpret_cast< KDDockWidgets::DockWidgetBase::Options*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::DockWidgetBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::DockWidgetBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEDockWidgetBaseENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidgetAdapter::qt_metacast(_clname);
}

int KDDockWidgets::DockWidgetBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidgetAdapter::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::DockWidgetBase::shown()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void KDDockWidgets::DockWidgetBase::hidden()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void KDDockWidgets::DockWidgetBase::iconChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void KDDockWidgets::DockWidgetBase::titleChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void KDDockWidgets::DockWidgetBase::widgetChanged(KDDockWidgets::QWidgetOrQuick * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void KDDockWidgets::DockWidgetBase::optionsChanged(KDDockWidgets::DockWidgetBase::Options _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void KDDockWidgets::DockWidgetBase::isFocusedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void KDDockWidgets::DockWidgetBase::isOverlayedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void KDDockWidgets::DockWidgetBase::isFloatingChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void KDDockWidgets::DockWidgetBase::removedFromSideBar()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void KDDockWidgets::DockWidgetBase::windowActiveAboutToChange(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void KDDockWidgets::DockWidgetBase::actualTitleBarChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void KDDockWidgets::DockWidgetBase::aboutToDeleteOnClose()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}
QT_WARNING_POP
