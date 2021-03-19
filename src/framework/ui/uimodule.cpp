#include "uimodule.h"

#include <QtQml>

#include "modularity/ioc.h"

#include "internal/uiengine.h"
#include "internal/uiconfiguration.h"
#include "internal/interactiveuriregister.h"

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

#include "dev/interactivetestsmodel.h"
#include "dev/testdialog.h"

using namespace mu::ui;
using namespace mu::framework;

static std::shared_ptr<UiConfiguration> s_configuration = std::make_shared<UiConfiguration>();

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
}

void UiModule::resolveImports()
{
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
    qmlRegisterType<InteractiveTestsModel>("MuseScore.Ui", 1, 0, "InteractiveTestsModel");

    qRegisterMetaType<TestDialog>("TestDialog");

    framework::ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(ui_QML_IMPORT);
}

void UiModule::onInit(const IApplication::RunMode&)
{
    s_configuration->init();
}

void UiModule::onDeinit()
{
    s_configuration->deinit();
}
