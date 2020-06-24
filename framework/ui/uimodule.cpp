#include "uimodule.h"

#include <QtQml>

#include "modularity/ioc.h"

#include "internal/uiengine.h"
#include "view/qmltheme.h"
#include "view/iconcodes.h"

#include "internal/uiinteractive.h"
#include "internal/launcher.h"

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
    ioc()->registerExport<IUiEngine>(moduleName(), UiEngine::instance());
    ioc()->registerExport<IQmlLaunchProvider>(moduleName(), UiEngine::instance()->launchProvider());
    ioc()->registerExport<IInteractive>(moduleName(), new UiInteractive());
    ioc()->registerExport<ILauncher>(moduleName(), new Launcher());
}

void UiModule::registerResources()
{
    ui_init_qrc();
}

void UiModule::registerUiTypes()
{
    qmlRegisterUncreatableType<UiEngine>("MuseScore.Ui", 1, 0, "UiEngine", "Cannot create an UiEngine");
    qmlRegisterUncreatableType<QmlTheme>("MuseScore.Ui", 1, 0, "QmlTheme", "Cannot create a QmlTheme");
    qmlRegisterUncreatableType<IconCode>("MuseScore.Ui", 1, 0, "IconCode", "Cannot create an IconCode");
    qmlRegisterUncreatableType<QmlLaunchProvider>("MuseScore.Ui", 1, 0, "QmlLaunchProvider", "Cannot create");
}
