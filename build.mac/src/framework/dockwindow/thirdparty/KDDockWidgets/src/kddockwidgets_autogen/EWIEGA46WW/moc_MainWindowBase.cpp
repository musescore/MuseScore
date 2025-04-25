/****************************************************************************
** Meta object code from reading C++ file 'MainWindowBase.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../../../../src/framework/dockwindow/thirdparty/KDDockWidgets/src/MainWindowBase.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindowBase.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS = QtMocHelpers::stringData(
    "KDDockWidgets::MainWindowBase",
    "uniqueNameChanged",
    "",
    "frameCountChanged",
    "addDockWidgetAsTab",
    "KDDockWidgets::DockWidgetBase*",
    "dockwidget",
    "addDockWidget",
    "dockWidget",
    "KDDockWidgets::Location",
    "location",
    "relativeTo",
    "KDDockWidgets::InitialOption",
    "initialOption",
    "setPersistentCentralWidget",
    "QWidgetOrQuick*",
    "widget",
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
    "affinities",
    "uniqueName",
    "options",
    "KDDockWidgets::MainWindowOptions",
    "isMDI"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS_t {
    uint offsetsAndSizes[74];
    char stringdata0[30];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[18];
    char stringdata4[19];
    char stringdata5[31];
    char stringdata6[11];
    char stringdata7[14];
    char stringdata8[11];
    char stringdata9[24];
    char stringdata10[9];
    char stringdata11[11];
    char stringdata12[29];
    char stringdata13[14];
    char stringdata14[27];
    char stringdata15[16];
    char stringdata16[7];
    char stringdata17[14];
    char stringdata18[29];
    char stringdata19[14];
    char stringdata20[31];
    char stringdata21[19];
    char stringdata22[17];
    char stringdata23[23];
    char stringdata24[20];
    char stringdata25[12];
    char stringdata26[21];
    char stringdata27[24];
    char stringdata28[37];
    char stringdata29[17];
    char stringdata30[17];
    char stringdata31[6];
    char stringdata32[11];
    char stringdata33[11];
    char stringdata34[8];
    char stringdata35[33];
    char stringdata36[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS_t qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS = {
    {
        QT_MOC_LITERAL(0, 29),  // "KDDockWidgets::MainWindowBase"
        QT_MOC_LITERAL(30, 17),  // "uniqueNameChanged"
        QT_MOC_LITERAL(48, 0),  // ""
        QT_MOC_LITERAL(49, 17),  // "frameCountChanged"
        QT_MOC_LITERAL(67, 18),  // "addDockWidgetAsTab"
        QT_MOC_LITERAL(86, 30),  // "KDDockWidgets::DockWidgetBase*"
        QT_MOC_LITERAL(117, 10),  // "dockwidget"
        QT_MOC_LITERAL(128, 13),  // "addDockWidget"
        QT_MOC_LITERAL(142, 10),  // "dockWidget"
        QT_MOC_LITERAL(153, 23),  // "KDDockWidgets::Location"
        QT_MOC_LITERAL(177, 8),  // "location"
        QT_MOC_LITERAL(186, 10),  // "relativeTo"
        QT_MOC_LITERAL(197, 28),  // "KDDockWidgets::InitialOption"
        QT_MOC_LITERAL(226, 13),  // "initialOption"
        QT_MOC_LITERAL(240, 26),  // "setPersistentCentralWidget"
        QT_MOC_LITERAL(267, 15),  // "QWidgetOrQuick*"
        QT_MOC_LITERAL(283, 6),  // "widget"
        QT_MOC_LITERAL(290, 13),  // "layoutEqually"
        QT_MOC_LITERAL(304, 28),  // "layoutParentContainerEqually"
        QT_MOC_LITERAL(333, 13),  // "moveToSideBar"
        QT_MOC_LITERAL(347, 30),  // "KDDockWidgets::SideBarLocation"
        QT_MOC_LITERAL(378, 18),  // "restoreFromSideBar"
        QT_MOC_LITERAL(397, 16),  // "overlayOnSideBar"
        QT_MOC_LITERAL(414, 22),  // "toggleOverlayOnSideBar"
        QT_MOC_LITERAL(437, 19),  // "clearSideBarOverlay"
        QT_MOC_LITERAL(457, 11),  // "deleteFrame"
        QT_MOC_LITERAL(469, 20),  // "sideBarForDockWidget"
        QT_MOC_LITERAL(490, 23),  // "KDDockWidgets::SideBar*"
        QT_MOC_LITERAL(514, 36),  // "const KDDockWidgets::DockWidg..."
        QT_MOC_LITERAL(551, 16),  // "sideBarIsVisible"
        QT_MOC_LITERAL(568, 16),  // "closeDockWidgets"
        QT_MOC_LITERAL(585, 5),  // "force"
        QT_MOC_LITERAL(591, 10),  // "affinities"
        QT_MOC_LITERAL(602, 10),  // "uniqueName"
        QT_MOC_LITERAL(613, 7),  // "options"
        QT_MOC_LITERAL(621, 32),  // "KDDockWidgets::MainWindowOptions"
        QT_MOC_LITERAL(654, 5)   // "isMDI"
    },
    "KDDockWidgets::MainWindowBase",
    "uniqueNameChanged",
    "",
    "frameCountChanged",
    "addDockWidgetAsTab",
    "KDDockWidgets::DockWidgetBase*",
    "dockwidget",
    "addDockWidget",
    "dockWidget",
    "KDDockWidgets::Location",
    "location",
    "relativeTo",
    "KDDockWidgets::InitialOption",
    "initialOption",
    "setPersistentCentralWidget",
    "QWidgetOrQuick*",
    "widget",
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
    "affinities",
    "uniqueName",
    "options",
    "KDDockWidgets::MainWindowOptions",
    "isMDI"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       4,  200, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  134,    2, 0x06,    5 /* Public */,
       3,    1,  135,    2, 0x06,    6 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       4,    1,  138,    2, 0x02,    8 /* Public */,
       7,    4,  141,    2, 0x02,   10 /* Public */,
       7,    3,  150,    2, 0x22,   15 /* Public | MethodCloned */,
       7,    2,  157,    2, 0x22,   19 /* Public | MethodCloned */,
      14,    1,  162,    2, 0x02,   22 /* Public */,
      17,    0,  165,    2, 0x02,   24 /* Public */,
      18,    1,  166,    2, 0x02,   25 /* Public */,
      19,    1,  169,    2, 0x02,   27 /* Public */,
      19,    2,  172,    2, 0x02,   29 /* Public */,
      21,    1,  177,    2, 0x02,   32 /* Public */,
      22,    1,  180,    2, 0x02,   34 /* Public */,
      23,    1,  183,    2, 0x02,   36 /* Public */,
      24,    1,  186,    2, 0x02,   38 /* Public */,
      24,    0,  189,    2, 0x22,   40 /* Public | MethodCloned */,
      26,    1,  190,    2, 0x102,   41 /* Public | MethodIsConst  */,
      29,    1,  193,    2, 0x102,   43 /* Public | MethodIsConst  */,
      30,    1,  196,    2, 0x02,   45 /* Public */,
      30,    0,  199,    2, 0x22,   47 /* Public | MethodCloned */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 9, 0x80000000 | 5, 0x80000000 | 12,    8,   10,   11,   13,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 9, 0x80000000 | 5,    8,   10,   11,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 9,    8,   10,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5,    8,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 20,    2,    2,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, 0x80000000 | 5,    2,
    QMetaType::Void, QMetaType::Bool,   25,
    QMetaType::Void,
    0x80000000 | 27, 0x80000000 | 28,    2,
    QMetaType::Bool, 0x80000000 | 20,    2,
    QMetaType::Bool, QMetaType::Bool,   31,
    QMetaType::Bool,

 // properties: name, type, flags
      32, QMetaType::QStringList, 0x00015401, uint(-1), 0,
      33, QMetaType::QString, 0x00015401, uint(-1), 0,
      34, 0x80000000 | 35, 0x00015409, uint(-1), 0,
      36, QMetaType::Bool, 0x00015401, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT static const QMetaObject::SuperData qt_meta_extradata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS[] = {
    QMetaObject::SuperData::link<KDDockWidgets::staticMetaObject>(),
    nullptr
};

Q_CONSTINIT const QMetaObject KDDockWidgets::MainWindowBase::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindowOrQuick::staticMetaObject>(),
    qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS,
    qt_static_metacall,
    qt_meta_extradata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS_t,
        // property 'affinities'
        QtPrivate::TypeAndForceComplete<QStringList, std::true_type>,
        // property 'uniqueName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'options'
        QtPrivate::TypeAndForceComplete<KDDockWidgets::MainWindowOptions, std::true_type>,
        // property 'isMDI'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindowBase, std::true_type>,
        // method 'uniqueNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'frameCountChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'addDockWidgetAsTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::InitialOption, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        // method 'addDockWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::DockWidgetBase *, std::false_type>,
        QtPrivate::TypeAndForceComplete<KDDockWidgets::Location, std::false_type>,
        // method 'setPersistentCentralWidget'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QWidgetOrQuick *, std::false_type>,
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

void KDDockWidgets::MainWindowBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindowBase *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->uniqueNameChanged(); break;
        case 1: _t->frameCountChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->addDockWidgetAsTab((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 3: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::InitialOption>>(_a[4]))); break;
        case 4: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[3]))); break;
        case 5: _t->addDockWidget((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::Location>>(_a[2]))); break;
        case 6: _t->setPersistentCentralWidget((*reinterpret_cast< std::add_pointer_t<QWidgetOrQuick*>>(_a[1]))); break;
        case 7: _t->layoutEqually(); break;
        case 8: _t->layoutParentContainerEqually((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 9: _t->moveToSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 10: _t->moveToSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<KDDockWidgets::SideBarLocation>>(_a[2]))); break;
        case 11: _t->restoreFromSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 12: _t->overlayOnSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 13: _t->toggleOverlayOnSideBar((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::DockWidgetBase*>>(_a[1]))); break;
        case 14: _t->clearSideBarOverlay((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->clearSideBarOverlay(); break;
        case 16: { KDDockWidgets::SideBar* _r = _t->sideBarForDockWidget((*reinterpret_cast< std::add_pointer_t<const KDDockWidgets::DockWidgetBase*>>(_a[1])));
            if (_a[0]) *reinterpret_cast< KDDockWidgets::SideBar**>(_a[0]) = std::move(_r); }  break;
        case 17: { bool _r = _t->sideBarIsVisible((*reinterpret_cast< std::add_pointer_t<KDDockWidgets::SideBarLocation>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 18: { bool _r = _t->closeDockWidgets((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 19: { bool _r = _t->closeDockWidgets();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MainWindowBase::*)();
            if (_t _q_method = &MainWindowBase::uniqueNameChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MainWindowBase::*)(int );
            if (_t _q_method = &MainWindowBase::frameCountChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<MainWindowBase *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QStringList*>(_v) = _t->affinities(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->uniqueName(); break;
        case 2: *reinterpret_cast< KDDockWidgets::MainWindowOptions*>(_v) = _t->options(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->isMDI(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *KDDockWidgets::MainWindowBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KDDockWidgets::MainWindowBase::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSKDDockWidgetsSCOPEMainWindowBaseENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QMainWindowOrQuick::qt_metacast(_clname);
}

int KDDockWidgets::MainWindowBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindowOrQuick::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 20)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 20;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void KDDockWidgets::MainWindowBase::uniqueNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void KDDockWidgets::MainWindowBase::frameCountChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
