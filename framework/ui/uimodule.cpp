#include "uimodule.h"

#include <QtQml>

#include "modularity/ioc.h"

#include "internal/uiengine.h"
#include "internal/uiconfiguration.h"
#include "internal/launcheruriregister.h"
#include "view/qmltheme.h"
#include "view/qmltooltip.h"
#include "view/iconcodes.h"
#include "view/qmldialog.h"

#include "dev/launchertestsmodel.h"
#include "dev/testdialog.h"

#include "mscore/globals.h"

using namespace mu::framework;

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
    ioc()->registerExport<IUiConfiguration>(moduleName(), new UiConfiguration());
    ioc()->registerExportNoDelete<IUiEngine>(moduleName(), UiEngine::instance());
    ioc()->registerExport<IQmlLaunchProvider>(moduleName(), UiEngine::instance()->launchProvider());
    ioc()->registerExport<ILauncherUriRegister>(moduleName(), new LauncherUriRegister());
}

void UiModule::resolveImports()
{
    auto lr = framework::ioc()->resolve<framework::ILauncherUriRegister>(moduleName());
    if (lr) {
        lr->registerUri("musescore://devtools/launcher/testdialog",
                        ContainerMeta { ContainerType::QWidget, QMetaType::type("TestDialog") });
    }
}

void UiModule::registerResources()
{
    ui_init_qrc();
}

void UiModule::registerUiTypes()
{
    qmlRegisterUncreatableType<UiEngine>("MuseScore.Ui", 1, 0, "UiEngine", "Cannot create an UiEngine");
    qmlRegisterUncreatableType<QmlTheme>("MuseScore.Ui", 1, 0, "QmlTheme", "Cannot create a QmlTheme");
    qmlRegisterUncreatableType<QmlToolTip>("MuseScore.Ui", 1, 0, "QmlToolTip", "Cannot create a QmlToolTip");
    qmlRegisterUncreatableType<IconCode>("MuseScore.Ui", 1, 0, "IconCode", "Cannot create an IconCode");
    qmlRegisterUncreatableType<QmlLaunchProvider>("MuseScore.Ui", 1, 0, "QmlLaunchProvider", "Cannot create");

    qmlRegisterType<QmlDialog>("MuseScore.Ui", 1, 0, "QmlDialog");
    qmlRegisterType<LauncherTestsModel>("MuseScore.Ui", 1, 0, "LauncherTestsModel");

    qRegisterMetaType<TestDialog>("TestDialog");
}

void UiModule::onInit()
{
    //! TODO
    Ms::guiScaling = 1.0;
}
