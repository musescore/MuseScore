#include "uimodule.h"

#include <QtQml>

#include "modularity/ioc.h"

#include "uiengine.h"
#include "qmltheme.h"
#include "iconcodes.h"

#include "uiinteractive.h"

using namespace mu::framework;

std::string UiModule::moduleName() const
{
    static std::string name = "ui";
    return name;
}

void UiModule::registerExports()
{
    ioc()->registerExport<IUiEngine>(moduleName(), UiEngine::instance());
    ioc()->registerExport<IInteractive>(moduleName(), new UiInteractive());
}

void UiModule::registerUiTypes()
{
    qmlRegisterUncreatableType<UiEngine>("MuseScore.Ui", 1, 0, "UiEngine", "Cannot create an UiEngine");
    qmlRegisterUncreatableType<QmlTheme>("MuseScore.Ui", 1, 0, "QmlTheme", "Cannot create a QmlTheme");
    qmlRegisterUncreatableType<IconCode>("MuseScore.Ui", 1, 0, "IconCode", "Cannot create an IconCode");
}
