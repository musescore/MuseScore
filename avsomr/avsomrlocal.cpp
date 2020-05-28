//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "avsomrlocal.h"

#include <QFileInfo>
#include <QEventLoop>
#include <QProcess>
#include <QtConcurrent>
#include <QFutureWatcher>

#include "mscore/preferences.h"

#include "avslog.h"
#include "avsomrlocalinstaller.h"

namespace {
static const QString AVS_EXT_NAME("avsomr_local");

#ifdef Q_OS_WIN

static const QString SHELL("cmd");
static const QStringList SHELL_ARGS("/c");
static const QString AVS_MAKE_SCRIPT("make_avs_win.bat");

#elif defined(Q_OS_MAC)

static const QString SHELL("sh");
static const QStringList SHELL_ARGS;
static const QString AVS_MAKE_SCRIPT("make_avs_mac.sh");

#else

static const QString SHELL("sh");
static const QStringList SHELL_ARGS;
static const QString AVS_MAKE_SCRIPT("make_avs_lin.sh");

#endif
}

using namespace Ms::Avs;

AvsOmrLocal::AvsOmrLocal()
{
}

AvsOmrLocal::~AvsOmrLocal()
{
    delete _installer;
}

//---------------------------------------------------------
//   installer
//---------------------------------------------------------

AvsOmrLocalInstaller* AvsOmrLocal::installer() const
{
    if (!_installer) {
        _installer = new AvsOmrLocalInstaller(avsHomePath());
    }

    return _installer;
}

//---------------------------------------------------------
//   avsHomePath
//---------------------------------------------------------

QString AvsOmrLocal::avsHomePath() const
{
    QString path = preferences.getString(PREF_APP_PATHS_MYEXTENSIONS) + "/" + AVS_EXT_NAME;
    return path;
}

//---------------------------------------------------------
//   makeAvsFilePath
//---------------------------------------------------------

QString AvsOmrLocal::makeAvsFilePath(const QString& buildDir, const QString& baseName) const
{
    return buildDir + "/" + baseName + "/" + baseName + ".msmr";
}

//---------------------------------------------------------
//   version
//---------------------------------------------------------

QString AvsOmrLocal::version() const
{
    QString versionPath = avsHomePath() + "/VERSION";
    QFile versionFile(versionPath);
    bool ok = versionFile.open(QIODevice::ReadOnly);
    if (!ok) {
        return QString();
    }

    QString ver = versionFile.readAll();
    return ver.trimmed();
}

//---------------------------------------------------------
//   state
//---------------------------------------------------------

const AvsOmrLocal::State& AvsOmrLocal::state() const
{
    if (State::Undefined == _state) {
        bool ok = execAvs("check", QString(), QString());
        _state = ok ? State::Ready : State::NotInstalled;
    }
    return _state;
}

//---------------------------------------------------------
//   stateString
//---------------------------------------------------------

QString AvsOmrLocal::stateString() const
{
    switch (state()) {
    case State::Undefined:    return QStringLiteral("Undefined");
    case State::NotInstalled: return QStringLiteral("NotInstalled");
    case State::Instaling:    return QStringLiteral("Instaling");
    case State::Ready:        return QStringLiteral("Ready");
    case State::Building:     return QStringLiteral("Building");
    }
    Q_UNREACHABLE();
    return QString();
}

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

Ret AvsOmrLocal::stateToRet(const State& st) const
{
    switch (st) {
    case State::Undefined:    return Ret::UnknownError;
    case State::NotInstalled: return Ret::LocalNotInstalled;
    case State::Instaling:    return Ret::LocalInstaling;
    case State::Ready:        return Ret::Ok;
    case State::Building:     return Ret::LocalAlreadyBuilding;
    }
    Q_UNREACHABLE();
    return Ret::Undefined;
}

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void AvsOmrLocal::setState(State st)
{
    _state = st;
}

//---------------------------------------------------------
//   isUseLocal
//---------------------------------------------------------

bool AvsOmrLocal::isUseLocal() const
{
    bool useLocal = preferences.getBool(PREF_IMPORT_AVSOMR_USELOCAL);
    return useLocal;
}

//---------------------------------------------------------
//   isInstalled
//---------------------------------------------------------

bool AvsOmrLocal::isInstalled() const
{
    return state() != State::NotInstalled;
}

//---------------------------------------------------------
//   isInstalledAsync
//---------------------------------------------------------

void AvsOmrLocal::isInstalledAsync(const std::function<void(bool)>& callback) const
{
    if (_state != State::Undefined) {
        callback(_state != State::NotInstalled);
        return;
    }

    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>();
    QObject::connect(watcher, &QFutureWatcher<bool>::finished, [watcher, callback]() {
        bool isInstl = watcher->result();
        callback(isInstl);
        watcher->deleteLater();
    });

    QFuture<bool> future = QtConcurrent::run([this]() { return isInstalled(); });
    watcher->setFuture(future);
}

//---------------------------------------------------------
//   checkInstallOrUpdate
//---------------------------------------------------------

Ret AvsOmrLocal::checkInstallOrUpdate(bool isWait)
{
    const State& st = state();
    if (State::NotInstalled == st) {
        installBackground();
    } else if (State::Instaling == st) {
        // noop, only waiting
    } else {
        if (isNeedUpdate()) {
            installBackground();
        }
    }

    if (isWait) {
        waitInstallOrUpdate();
    }

    if (State::Ready != state()) {
        return stateToRet(state());
    }

    return Ret::Ok;
}

//---------------------------------------------------------
//   isNeedUpdate
//---------------------------------------------------------

bool AvsOmrLocal::isNeedUpdate() const
{
    const AvsOmrLocalInstaller::ReleaseInfo& info = installer()->loadReleaseInfo();
    if (!info.isValid()) {
        LOGW() << "failed load release info";
        return false;
    }

    QString localVer = version();
    if (localVer.isEmpty()) {
        LOGW() << "failed read version";
        return true;
    }

    if (localVer != info.tag) {
        LOGI() << "avs need update, installed version: " << localVer << ", latest: " << info.tag;
        return true;
    }

    return false;
}

//---------------------------------------------------------
//   installBackground
//---------------------------------------------------------

void AvsOmrLocal::installBackground()
{
    setState(State::Instaling);
    installer()->installBackground();
}

//---------------------------------------------------------
//   waitInstallOrUpdate
//---------------------------------------------------------

void AvsOmrLocal::waitInstallOrUpdate()
{
    installer()->waitForFinished();
    setState(State::Undefined);
}

//---------------------------------------------------------
//   build
//---------------------------------------------------------

Ret AvsOmrLocal::build(const QString& filePath, const QString& buildDir)
{
    if (State::Ready != state()) {
        LOGE() << "unable start build, state: " << stateString();
        return stateToRet(state());
    }

    setState(State::Building);

    Ret ret = execAvs("build", filePath, buildDir);

    setState(State::Instaling);

    return ret;
}

//---------------------------------------------------------
//   execAvs
//---------------------------------------------------------

Ret AvsOmrLocal::execAvs(const QString& cmd, const QString& filePath, const QString& buildDir) const
{
    QString home = avsHomePath();
    if (home.isEmpty()) {
        LOGE() << "not found avs plugin";
        return Ret::LocalNotInstalled;
    }

    QString make_avs = avsHomePath() + "/" + AVS_MAKE_SCRIPT;
    if (!QFileInfo(make_avs).exists()) {
        LOGE() << "not found make script: " << make_avs;
        return Ret::LocalNotInstalled;
    }

    QStringList arguments;
    if (!SHELL_ARGS.isEmpty()) {
        arguments << SHELL_ARGS;
    }

    arguments << make_avs << cmd << filePath << buildDir;

    QEventLoop loop;
    QProcess proc;

    int exitCode = -1;
    auto onFinished = [&loop, &exitCode](int code, QProcess::ExitStatus exitStatus) {
                          LOGI() << "onFinished exitCode: " << code << ", exitStatus: " << exitStatus;
                          exitCode = code;
                          loop.quit();
                      };

    QObject::connect(&proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     onFinished);

    LOGI() << "try start: " << SHELL << arguments;
    proc.start(SHELL, arguments);
    loop.exec();

    LOGD() << QString::fromLatin1(proc.readAll());

    if (exitCode != 0) {
        return Ret::LocalFailedExec;
    }

    return Ret::Ok;
}
