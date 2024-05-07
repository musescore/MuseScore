/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "autobot.h"

#include <QTimer>
#include <QWindow>

#include "global/io/file.h"
#include "global/serialization/json.h"

#include "draw/types/font.h"

#include "testcasecontext.h"
#include "../autobotutils.h"

#include "log.h"

using namespace muse;
using namespace muse::autobot;

void Autobot::init()
{
    m_autobotInteractive = std::make_shared<AutobotInteractive>();

    m_runner.stepStatusChanged().onReceive(this, [this](const StepInfo& stepInfo, const Ret& ret) {
        if (stepInfo.status == StepStatus::Started) {
            m_context->addStep(stepInfo.name);
        }

        m_report.onStepStatusChanged(stepInfo, m_context);
        m_stepStatusChanged.send(stepInfo, ret);

        if (stepInfo.status == StepStatus::Aborted) {
            setStatus(Status::Aborted);
        } else if (stepInfo.status == StepStatus::Error) {
            setStatus(Status::Error);
        }
    });

    m_runner.allFinished().onReceive(this, [this](bool aborted) {
        m_report.endReport(aborted);
    });

    setStatus(Status::Undefined);
    setSpeedMode(SpeedMode::Default);
}

void Autobot::affectOnServices()
{
    IApplication::RunMode runMode = application()->runMode();
    if (runMode == IApplication::RunMode::GuiApp) {
        //! NOTE Move focus to main window
        mainWindow()->qWindow()->requestActivate();

        //! NOTE Disable reset on mouse press for testing purpose
        navigation()->setIsResetOnMousePress(false);

        //! NOTE Set navigation highlight
        navigation()->setIsHighlight(true);

        //! NOTE Only defaults shortcuts
        shortcutsRegister()->reload(true);

        //! NOTE Change Interactive implementation
        modularity::ModulesIoC* ioc = application()->ioc();
        auto realInteractive = ioc->resolve<IInteractive>("autobot");
        m_autobotInteractive->setRealInteractive(realInteractive);
        ioc->unregister<IInteractive>("autobot");
        ioc->registerExport<IInteractive>("autobot", m_autobotInteractive);
    }

    m_affectedServiceState.fontDisabledMerging = muse::draw::Font::g_disableFontMerging;
    muse::draw::Font::g_disableFontMerging = true;
}

void Autobot::restoreAffectOnServices()
{
    IApplication::RunMode runMode = application()->runMode();
    if (runMode == IApplication::RunMode::GuiApp) {
        navigation()->setIsResetOnMousePress(true);
        shortcutsRegister()->reload(false);

        modularity::ModulesIoC* ioc = application()->ioc();
        auto realInteractive = m_autobotInteractive->realInteractive();
        ioc->unregister<IInteractive>("autobot");
        ioc->registerExport<IInteractive>("autobot", realInteractive);
    }

    muse::draw::Font::g_disableFontMerging = m_affectedServiceState.fontDisabledMerging;
}

void Autobot::loadContext(ITestCaseContextPtr ctx, const io::path_t& context, const std::string& contextVal, ScriptEngine* e)
{
    auto loadFromJson = [ctx, e](const ByteArray& json) {
        std::string err;
        JsonObject ctxObj = JsonDocument::fromJson(json, &err).rootObject();
        if (!err.empty()) {
            LOGE() << "failed parse context value, err: " << err;
            return;
        }

        std::vector<std::string> keys = ctxObj.keys();
        for (const std::string& k : keys) {
            ctx->setGlobalVal(QString::fromStdString(k), toQJSValue(ctxObj.value(k), e));
        }
    };

    // from file
    if (!context.empty()) {
        ctx->setGlobalVal("context_path", context.toQString());

        ByteArray json;
        Ret ret = io::File::readFile(context, json);
        if (!ret) {
            LOGE() << "failed read file: " << context;
            return;
        }
        loadFromJson(json);
    }

    // from value (maybe override)
    if (!contextVal.empty()) {
        ByteArray json = ByteArray::fromRawData(contextVal.c_str(), contextVal.size());
        loadFromJson(json);
    }
}

QJSValueList Autobot::parseFuncArgs(const std::string& funcArgs, ScriptEngine* e) const
{
    ByteArray json = ByteArray::fromRawData(funcArgs.c_str(), funcArgs.size());
    std::string err;
    JsonArray arr = JsonDocument::fromJson(json, &err).rootArray();
    if (!err.empty()) {
        LOGE() << "failed parse context value, err: " << err;
        return QJSValueList();
    }

    QJSValueList args;
    for (size_t i = 0; i < arr.size(); ++i) {
        const JsonValue& a = arr.at(i);
        args.append(toQJSValue(a, e));
    }

    return args;
}

void Autobot::execScript(const io::path_t& path, const Options& opt)
{
    LOGD() << "path: " << path
           << ", context: " << opt.context
           << ", contextVal: " << opt.contextVal
           << ", func: " << opt.func
           << ", funcArgs: " << opt.funcArgs;

    if (status() == Status::Running || status() == Status::Paused) {
        abort();
    }

    //! NOTE If an error occurred during the execution of TestCase and a dialog was opened at that time,
    //! the TestCase loop does not exit because of the open dialog,
    //! so we need to close all dialogs in order to complete the execution of the previous script.
    if (m_engine) {
        std::vector<Uri> stack = interactive()->stack();
        if (stack.size() > 1) {
            const Uri& uri = stack.back();
            interactive()->close(uri);
            QTimer::singleShot(1000, [this, path, opt]() {
                execScript(path, opt);
            });
            return;
        }
    }

    if (m_engine) {
        LOGE() << "unknown internal error with prev execScript";
        delete m_engine;
        m_engine = nullptr;
    }

    IF_ASSERT_FAILED(!m_engine) {
        delete m_engine;
        m_engine = nullptr;
    }

    affectOnServices();

    m_engine = new ScriptEngine(iocContext());
    m_engine->setScriptPath(path);

    m_context = std::make_shared<TestCaseContext>();
    m_context->setGlobalVal("script_path", path.toQString());
    loadContext(m_context, opt.context, opt.contextVal, m_engine);

    setStatus(Status::Running);
    QString func = opt.func.empty() ? QString("main") : QString::fromStdString(opt.func);
    QJSValueList args = opt.funcArgs.empty() ? QJSValueList() : parseFuncArgs(opt.funcArgs, m_engine);
    Ret ret = m_engine->call(func, args);

    //! NOTE Also maybe abort or error
    if (status() == Status::Running) {
        setStatus(Status::Finished);
    }

    delete m_engine;
    m_engine = nullptr;
    if (!ret) {
        LOGE() << ret.toString();
    }

    restoreAffectOnServices();
}

SpeedMode Autobot::speedMode() const
{
    return m_runner.speedMode();
}

void Autobot::setSpeedMode(SpeedMode mode)
{
    m_runner.setSpeedMode(mode);
}

async::Channel<SpeedMode> Autobot::speedModeChanged() const
{
    return m_runner.speedModeChanged();
}

void Autobot::setDefaultIntervalMsec(int msec)
{
    m_runner.setDefaultInterval(msec);
}

int Autobot::defaultIntervalMsec() const
{
    return m_runner.defaultInterval();
}

int Autobot::intervalMsec() const
{
    return m_runner.intervalMsec();
}

void Autobot::runTestCase(const TestCase& testCase)
{
    m_report.beginReport(testCase);
    m_runner.run(testCase);
}

void Autobot::abort()
{
    fatal("abort");
}

void Autobot::fatal(const QString& msg)
{
    if (status() == Status::Paused) {
        unpause();
    }

    if (status() != Status::Running) {
        return;
    }

    if (m_engine) {
        m_engine->throwError(msg);
    }
    m_runner.abort();
}

void Autobot::sleep(int msec)
{
    //! NOTE If pause state, then we sleep until to unpause
    //! It's allowing to do pause during step execution
    if (status() == IAutobot::Status::Paused) {
        m_sleepLoop.exec();
        return;
    }

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &m_sleepLoop, &QEventLoop::quit);
    timer.start(msec);
    m_sleepLoop.exec();
}

void Autobot::pause()
{
    setStatus(Status::Paused);
    m_runner.pause();
}

void Autobot::unpause()
{
    bool isNextStep = true;

    //! NOTE If pause did on sleep (during step execution),
    //! then unpause current step (without perform next step)
    if (m_sleepLoop.isRunning()) {
        m_sleepLoop.quit();
        isNextStep = false;
    }

    m_runner.unpause(isNextStep);
    setStatus(Status::Running);
}

ITestCaseContextPtr Autobot::context() const
{
    return m_context;
}

AutobotInteractivePtr Autobot::autobotInteractive() const
{
    return m_autobotInteractive;
}

void Autobot::setStatus(Status st)
{
    if (m_status == st) {
        return;
    }

    if (m_sleepLoop.isRunning()) {
        m_sleepLoop.quit();
    }

    m_status = st;

    io::path_t path;
    if (m_engine) {
        path = m_engine->scriptPath();
    }
    m_statusChanged.send(path, st);
}

IAutobot::Status Autobot::status() const
{
    return m_status;
}

async::Channel<io::path_t, IAutobot::Status> Autobot::statusChanged() const
{
    return m_statusChanged;
}

async::Channel<StepInfo, Ret> Autobot::stepStatusChanged() const
{
    return m_stepStatusChanged;
}
