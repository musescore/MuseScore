#include "testnativedialogs.h"

#include <QDebug>

#include <QPluginLoader>

#include <QtWidgets/private/qfiledialog_p.h>
#include <QtGui/private/qguiapplication_p.h>
#include <qpa/qplatformthemefactory_p.h>
#include <qpa/qplatformtheme.h>
#include <qpa/qplatformintegration.h>

TestNativeDialogs::TestNativeDialogs()
    : QFileDialog()
{
}

void TestNativeDialogs::test()
{
    QObjectData* p = d_ptr.get();
    QFileDialogPrivate* dp = static_cast<QFileDialogPrivate*>(p);

    QLatin1String staticName(QFileDialog::staticMetaObject.className());
    QLatin1String dynamicName(this->metaObject()->className());

    qDebug() << "canBeNativeDialog: " << dp->canBeNativeDialog();
    qDebug() << "  AA_DontUseNativeDialogs: " << QCoreApplication::testAttribute(Qt::AA_DontUseNativeDialogs);
    qDebug() << "  WA_DontShowOnScreen: " << testAttribute(Qt::WA_DontShowOnScreen);
    qDebug() << "  DontUseNativeDialog: " << bool(options() & QFileDialog::DontUseNativeDialog);
    qDebug() << "  staticName == dynamicName: " <<
        (staticName == dynamicName) << ", staticName: " << staticName << ", dynamicName: " << dynamicName;
    qDebug() << "platformFileDialogHelper: " << (dp->platformFileDialogHelper() != nullptr);

    QPlatformTheme* theme = QGuiApplicationPrivate::platformTheme();

    qDebug() << "theme: " << typeid(*theme).name();
    QPlatformDialogHelper* platformHelper = theme->createPlatformDialogHelper(QPlatformTheme::DialogType::FileDialog);
    qDebug() << "platformHelper: " << (platformHelper != nullptr);

    QStringList themes = QGuiApplicationPrivate::platform_integration->themeNames();
    qDebug() << "themes: " << themes;

    qDebug() << "libraryPaths: " << QCoreApplication::libraryPaths();
    qDebug() << "applicationDirPath: " << QCoreApplication::applicationDirPath();
    QString platformPluginPath = QCoreApplication::applicationDirPath() + "/../plugins/platformthemes";
    platformPluginPath = QDir::cleanPath(platformPluginPath);
    qDebug() << "platformPluginPath: " << platformPluginPath;

    QStringList themeNames = { "ubuntu", "gtk3", "gnome", "generic" };
    QPlatformTheme* themeIns = nullptr;
    // 4) Look for a theme plugin.
    for (const QString& themeName : qAsConst(themeNames)) {
        themeIns = QPlatformThemeFactory::create(themeName, platformPluginPath);
        if (themeIns) {
            qDebug() << "1 created " << themeName << " " << typeid(*themeIns).name();
            break;
        }
    }

    // 5) If no theme plugin was found ask the platform integration to
    // create a theme
    if (!themeIns) {
        for (const QString& themeName : qAsConst(themeNames)) {
            themeIns = QGuiApplicationPrivate::platform_integration->createPlatformTheme(themeName);
            if (themeIns) {
                qDebug() << "2 created " << themeName << " " << typeid(*themeIns).name();
                break;
            }
        }
        // No error message; not having a theme plugin is allowed.
    }

    show();
}
