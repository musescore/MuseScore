#include "uicomponentsmodule.h"

#include <QtQml>
#include "cpp/sampleobject.h"

using namespace mu::framework;

static void uicomponents_init_qrc()
{
    Q_INIT_RESOURCE(uicomponents);
}

std::string UiComponentsModule::moduleName() const
{
    return "uicomponents";
}

void UiComponentsModule::registerResources()
{
    uicomponents_init_qrc();
}

void UiComponentsModule::registerUiTypes()
{
    qmlRegisterType<SampleObject>("MuseScore.UiComponents", 1, 0, "SampleObject");
}
