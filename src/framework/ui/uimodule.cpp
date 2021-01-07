#include "uimodule.h"

#include <QtQml>

#include "modularity/ioc.h"

#include "internal/uiengine.h"
#include "internal/uiconfiguration.h"
#include "internal/interactiveuriregister.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosplatformtheme.h"
#else
#include "internal/platform/stub/stubplatformtheme.h"
#endif

#include "view/qmltooltip.h"
#include "view/iconcodes.h"
#include "view/musicalsymbolcodes.h"
#include "view/qmldialog.h"

#include "dev/interactivetestsmodel.h"
#include "dev/testdialog.h"

using namespace mu::framework;

static std::shared_ptr<UiConfiguration> s_configuration = std::make_shared<UiConfiguration>();

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
    ioc()->registerExportNoDelete<ITheme>(moduleName(), UiEngine::instance()->theme());
    ioc()->registerExport<IInteractiveProvider>(moduleName(), UiEngine::instance()->interactiveProvider());
    ioc()->registerExport<IInteractiveUriRegister>(moduleName(), new InteractiveUriRegister());

#ifdef Q_OS_MAC
    ioc()->registerExport<IPlatformTheme>(moduleName(), new MacOSPlatformTheme());
#else
    ioc()->registerExport<IPlatformTheme>(moduleName(), new StubPlatformTheme());
#endif
}

void UiModule::resolveImports()
{
    auto ir = framework::ioc()->resolve<framework::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://devtools/interactive/testdialog"),
                        ContainerMeta(ContainerType::QWidgetDialog, TestDialog::metaTypeId()));
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
    qmlRegisterUncreatableType<Theme>("MuseScore.Ui", 1, 0, "QmlTheme", "Cannot create a QmlTheme");
    qmlRegisterUncreatableType<QmlToolTip>("MuseScore.Ui", 1, 0, "QmlToolTip", "Cannot create a QmlToolTip");
    qmlRegisterUncreatableType<IconCode>("MuseScore.Ui", 1, 0, "IconCode", "Cannot create an IconCode");
    qmlRegisterUncreatableType<MusicalSymbolCodes>("MuseScore.Ui", 1, 0, "MusicalSymbolCodes",
                                                   "Cannot create an MusicalSymbolCodes");
    qmlRegisterUncreatableType<InteractiveProvider>("MuseScore.Ui", 1, 0, "QmlInteractiveProvider", "Cannot create");
    qmlRegisterUncreatableType<ContainerType>("MuseScore.Ui", 1, 0, "ContainerType", "Cannot create a ContainerType");

    qmlRegisterType<QmlDialog>("MuseScore.Ui", 1, 0, "QmlDialog");
    qmlRegisterType<InteractiveTestsModel>("MuseScore.Ui", 1, 0, "InteractiveTestsModel");

    qRegisterMetaType<TestDialog>("TestDialog");

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(ui_QML_IMPORT);
}

void UiModule::onInit(const IApplication::RunMode&)
{
    //! NOTE It is also needed in converter mode, because it defines the notation rendering settings
    s_configuration->init();
}
