#include "uimodule.h"

#include <QtQml>

#include "modularity/ioc.h"

#include "internal/uiengine.h"
#include "internal/uiconfiguration.h"
#include "internal/interactiveuriregister.h"
#include "internal/uiactionsregister.h"
#include "internal/keynavigationcontroller.h"
#include "internal/keynavigationuiactions.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosplatformtheme.h"
#elif defined(Q_OS_WIN)
#include "internal/platform/windows/windowsplatformtheme.h"
#else
#include "internal/platform/stub/stubplatformtheme.h"
#endif

#include "view/qmltooltip.h"
#include "view/iconcodes.h"
#include "view/musicalsymbolcodes.h"
#include "view/qmldialog.h"
#include "view/keynavigationsection.h"
#include "view/keynavigationsubsection.h"
#include "view/keynavigationcontrol.h"

#include "dev/interactivetestsmodel.h"
#include "dev/testdialog.h"

using namespace mu::ui;
using namespace mu::framework;

static std::shared_ptr<UiConfiguration> s_configuration = std::make_shared<UiConfiguration>();
static std::shared_ptr<UiActionsRegister> s_uiactionsRegister = std::make_shared<UiActionsRegister>();
static std::shared_ptr<KeyNavigationController> s_keyNavigationController = std::make_shared<KeyNavigationController>();
static std::shared_ptr<KeyNavigationUiActions> s_keyNavigationUiActions = std::make_shared<KeyNavigationUiActions>();

#ifdef Q_OS_MAC
static std::shared_ptr<MacOSPlatformTheme> s_platformTheme = std::make_shared<MacOSPlatformTheme>();
#elif defined(Q_OS_WIN)
static std::shared_ptr<WindowsPlatformTheme> s_platformTheme = std::make_shared<WindowsPlatformTheme>();
#else
static std::shared_ptr<StubPlatformTheme> s_platformTheme = std::make_shared<StubPlatformTheme>();
#endif

static void ui_init_qrc()
{
    Q_INIT_RESOURCE(ui);
}

std::string UiModule::moduleName() const
{
    return "ui";
}

void UiModule::registerExports()
{
    ioc()->registerExport<IUiConfiguration>(moduleName(), s_configuration);
    ioc()->registerExportNoDelete<IUiEngine>(moduleName(), UiEngine::instance());
    ioc()->registerExport<IInteractiveProvider>(moduleName(), UiEngine::instance()->interactiveProvider());
    ioc()->registerExport<IInteractiveUriRegister>(moduleName(), new InteractiveUriRegister());
    ioc()->registerExport<IPlatformTheme>(moduleName(), s_platformTheme);
    ioc()->registerExport<IUiActionsRegister>(moduleName(), s_uiactionsRegister);
    ioc()->registerExport<IKeyNavigationController>(moduleName(), s_keyNavigationController);
}

void UiModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(s_keyNavigationUiActions);
    }

    auto ir = framework::ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://devtools/interactive/testdialog"),
                        ContainerMeta(ContainerType::QWidgetDialog, TestDialog::static_metaTypeId()));
        ir->registerUri(Uri("musescore://devtools/interactive/sample"),
                        ContainerMeta(ContainerType::QmlDialog, "DevTools/Interactive/SampleDialog.qml"));
    }
}

void UiModule::registerResources()
{
    ui_init_qrc();
}

void UiModule::registerUiTypes()
{
    qmlRegisterUncreatableType<UiEngine>("MuseScore.Ui", 1, 0, "UiEngine", "Cannot create an UiEngine");
    qmlRegisterUncreatableType<UiTheme>("MuseScore.Ui", 1, 0, "QmlTheme", "Cannot create a QmlTheme");
    qmlRegisterUncreatableType<QmlToolTip>("MuseScore.Ui", 1, 0, "QmlToolTip", "Cannot create a QmlToolTip");
    qmlRegisterUncreatableType<IconCode>("MuseScore.Ui", 1, 0, "IconCode", "Cannot create an IconCode");
    qmlRegisterUncreatableType<MusicalSymbolCodes>("MuseScore.Ui", 1, 0, "MusicalSymbolCodes",
                                                   "Cannot create an MusicalSymbolCodes");
    qmlRegisterUncreatableType<InteractiveProvider>("MuseScore.Ui", 1, 0, "QmlInteractiveProvider", "Cannot create");
    qmlRegisterUncreatableType<ContainerType>("MuseScore.Ui", 1, 0, "ContainerType", "Cannot create a ContainerType");

    qmlRegisterType<QmlDialog>("MuseScore.Ui", 1, 0, "QmlDialog");

    qmlRegisterUncreatableType<AbstractKeyNavigation>("MuseScore.Ui", 1, 0, "AbstractKeyNavigation", "Cannot create a AbstractType");
    qmlRegisterType<KeyNavigationSection>("MuseScore.Ui", 1, 0, "KeyNavigationSection");
    qmlRegisterType<KeyNavigationSubSection>("MuseScore.Ui", 1, 0, "KeyNavigationSubSection");
    qmlRegisterType<KeyNavigationControl>("MuseScore.Ui", 1, 0, "KeyNavigationControl");

    qmlRegisterType<InteractiveTestsModel>("MuseScore.Ui", 1, 0, "InteractiveTestsModel");
    qRegisterMetaType<TestDialog>("TestDialog");

    framework::ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(ui_QML_IMPORT);
}

void UiModule::onInit(const IApplication::RunMode&)
{
    s_configuration->init();
    s_uiactionsRegister->init();
    s_keyNavigationController->init();
}

void UiModule::onDeinit()
{
    s_configuration->deinit();
}
