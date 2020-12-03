#include "uicomponentsmodule.h"

#include <QtQml>
#include "view/sampleobject.h"
#include "view/validators/doubleinputvalidator.h"
#include "view/validators/intinputvalidator.h"
#include "view/iconview.h"
#include "view/filterproxymodel.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

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

    qmlRegisterUncreatableType<QAbstractItemModel>("MuseScore.UiComponents", 1, 0, "AbstractItemModel", "Cannot ...");

    qmlRegisterType<DoubleInputValidator>("MuseScore.UiComponents", 1, 0, "DoubleInputValidator");
    qmlRegisterType<IntInputValidator>("MuseScore.UiComponents", 1, 0, "IntInputValidator");
    qmlRegisterType<IntInputValidator>("MuseScore.UiComponents", 1, 0, "IntInputValidator");

    qmlRegisterType<IconView>("MuseScore.UiComponents", 1, 0, "IconView");

    qmlRegisterType<FilterProxyModel>("MuseScore.UiComponents", 1, 0, "FilterProxyModel");
    qmlRegisterType<FilterValue>("MuseScore.UiComponents", 1, 0, "FilterValue");
    qmlRegisterUncreatableType<CompareType>("MuseScore.UiComponents", 1, 0, "CompareType", "Cannot create a CompareType");

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(uicomponents_QML_IMPORT);
}
