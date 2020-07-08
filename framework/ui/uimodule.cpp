#include "uimodule.h"

#include <QtQml>

#include "modularity/ioc.h"

#include "internal/uiengine.h"
#include "view/qmltheme.h"
#include "view/qmltooltip.h"
#include "view/iconcodes.h"
#include "view/qmldialog.h"

#include "dev/launchertestsmodel.h"

using namespace mu::framework;

static void ui_init_qrc()
{
    Q_INIT_RESOURCE(ui);
}

std::string UiModule::moduleName() const
{
    static std::string name = "ui";
    return name;
}

void UiModule::registerExports()
{
    ioc()->registerExportNoDelete<IUiEngine>(moduleName(), UiEngine::instance());
    ioc()->registerExport<IQmlLaunchProvider>(moduleName(), UiEngine::instance()->launchProvider());
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
}
