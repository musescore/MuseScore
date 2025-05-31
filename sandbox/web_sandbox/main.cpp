#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

#include "global/globalmodule.h"
#include "actions/actionsmodule.h"
#include "ui/uimodule.h"
#include "uicomponents/uicomponentsmodule.h"

#include "global/modularity/ioc.h"
#include "ui/iuiengine.h"
#include "ui/iinteractiveuriregister.h"

#include "interactivetestmodel.h"

static void app_init_qrc()
{
    Q_INIT_RESOURCE(app);
}

using namespace muse;

int main(int argc, char* argv[])
{
    qputenv("QT_STYLE_OVERRIDE", "Fusion");
    qputenv("QML_DISABLE_DISK_CACHE", "true");

    app_init_qrc();

    QGuiApplication app(argc, argv);

    // === Setup ===
    QQuickWindow::setDefaultAlphaBuffer(true);

    QList<muse::modularity::IModuleSetup*> modules = {
        new muse::GlobalModule(),
        new muse::actions::ActionsModule(),
        new muse::ui::UiModule(),
        new muse::uicomponents::UiComponentsModule()
    };

    for (auto* m : modules) {
        m->registerResources();
        m->registerExports();
    }

    muse::IApplication::RunMode mode = muse::IApplication::RunMode::GuiApp;
    for (auto* m : modules) {
        m->registerUiTypes();
        m->resolveImports();
    }

    for (auto* m : modules) {
        m->onPreInit(mode);
    }

    for (auto* m : modules) {
        m->onInit(mode);
    }

    for (auto* m : modules) {
        m->onAllInited(mode);
    }

    auto ir = muse::modularity::globalIoc()->resolve<muse::ui::IInteractiveUriRegister>("app");
    if (ir) {
        ir->registerQmlUri(Uri("muse://interactive/sample"), "SampleDialog.qml");
    }

    qmlRegisterType<InteractiveTestModel>("Muse.WasmTest", 1, 0, "InteractiveTestModel");

    QQmlApplicationEngine* engine = muse::modularity::globalIoc()->resolve<muse::ui::IUiEngine>("app")->qmlAppEngine();
    const QUrl url(QStringLiteral("qrc:/web_sandbox/Main.qml"));
    QObject::connect(
        engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    engine->load(url);

    LOGDA() << "run";

    return app.exec();
}
