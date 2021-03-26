//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "autobot.h"

#include <QTimer>

#include "log.h"

#include "abcontext.h"
#include "typicaltc.h"

#include "steps/abscoreloadstep.h"
#include "steps/abscorezoom.h"
#include "steps/abscoreclosestep.h"
#include "steps/abdrawcurrentstep.h"
#include "steps/abdrawrefstep.h"
#include "steps/abdrawcompstep.h"
#include "steps/abdiffdrawstep.h"

using namespace mu::autobot;

Autobot::Autobot()
{
    auto makeTypicalTC = [](const std::string& name, std::vector<ITestStep*> steps) {
        return std::make_shared<TypicalTC>(name, steps);
    };

    m_testCases = {
        makeTypicalTC("Zoom", {
            new AbScoreLoadStep(),
            new AbScoreZoom(100),
            new AbScoreZoom(50, ITestStep::Delay::Long),
            new AbScoreZoom(100, ITestStep::Delay::Long),
            new AbScoreCloseStep(ITestStep::Delay::Long)
        }),
        makeTypicalTC("Create Draw Ref", {
            new AbScoreLoadStep(),
            new AbScoreZoom(100),
            new AbDrawCurrentStep(true),
            new AbScoreCloseStep()
        }),
        makeTypicalTC("Comp Draw Data", {
            new AbScoreLoadStep(),
            new AbScoreZoom(100),
            new AbDrawCurrentStep(false),
            new AbDrawRefStep(),
            new AbDrawCompStep(),
            new AbDiffDrawStep(),
            new AbScoreCloseStep()
        }),
    };
}

std::vector<ITestCasePtr> Autobot::testCases() const
{
    return m_testCases;
}

ITestCasePtr Autobot::testCase(const std::string& name) const
{
    auto it = std::find_if(m_testCases.cbegin(), m_testCases.cend(), [name](const ITestCasePtr& t) {
        return t->name() == name;
    });

    if (it != m_testCases.cend()) {
        return *it;
    }
    return nullptr;
}

void Autobot::setCurrentTestCase(const std::string& name)
{
    m_currentTestCase.set(testCase(name));

    for (File& f : m_files.val) {
        f.completeRet = Ret();
    }
    m_files.notification.notify();
}

const mu::ValCh<ITestCasePtr>& Autobot::currentTestCase() const
{
    return m_currentTestCase;
}

mu::RetVal<mu::io::paths> Autobot::filesList() const
{
    using namespace mu::system;

    io::path filesPath = configuration()->filesPath();
    LOGI() << "filesPath: " << filesPath;
    RetVal<io::paths> paths = fileSystem()->scanFiles(filesPath, QStringList(), IFileSystem::ScanMode::OnlyCurrentDir);
    return paths;
}

void Autobot::init()
{
    m_status.val = Status::Stoped;

    m_runner.allFinished().onReceive(this, [this](const IAbContextPtr& ctx) {
        onFileFinished(ctx);
    });

    m_runner.stepStarted().onReceive(this, [this](const IAbContextPtr& ctx) {
        m_report.beginStep(ctx);
    });

    m_runner.stepFinished().onReceive(this, [this](const IAbContextPtr& ctx) {
        m_report.endStep(ctx);
    });

    m_currentTestCase.set(m_testCases.front());

    RetVal<io::paths> files = filesList();
    if (!files.ret) {
        LOGE() << "failed get score list, err: " << files.ret.toString();
        return;
    }

    m_files.val.clear();
    for (const io::path& p : files.val) {
        File f;
        f.path = p;
        m_files.val.push_back(std::move(f));
    }

    m_files.notification.notify();
}

void Autobot::runAllFiles()
{
    if (m_status.val != Status::Stoped) {
        LOGW() << "already running";
        return;
    }

    IF_ASSERT_FAILED(m_currentTestCase.val) {
        return;
    }

    m_fileIndex.val = -1;
    m_status.set(Status::RunningAll);

    m_report.beginReport(m_currentTestCase.val);

    nextFile();
}

void Autobot::runFile(int fileIndex)
{
    if (m_status.val != Status::Stoped) {
        LOGW() << "already running";
        return;
    }

    IF_ASSERT_FAILED(m_currentTestCase.val) {
        return;
    }

    m_fileIndex.val = fileIndex - 1;
    m_status.set(Status::RunningFile);

    m_report.beginReport(m_currentTestCase.val);

    nextFile();
}

void Autobot::stop()
{
    m_status.set(Status::Stoped);
}

void Autobot::doStop()
{
    m_report.endReport();

    if (m_status.val != Status::Stoped) {
        m_status.set(Status::Stoped);
    }
}

const mu::ValCh<IAutobot::Status>& Autobot::status() const
{
    return m_status;
}

const mu::ValNt<Files>& Autobot::files() const
{
    return m_files;
}

mu::async::Channel<File> Autobot::fileFinished() const
{
    return m_fileFinished;
}

const mu::ValCh<int>& Autobot::currentFileIndex() const
{
    return m_fileIndex;
}

void Autobot::nextFile()
{
    IF_ASSERT_FAILED(m_currentTestCase.val) {
        return;
    }

    IF_ASSERT_FAILED(!m_files.val.empty()) {
        return;
    }

    if (m_status.val == Status::Stoped) {
        doStop();
        return;
    }

    m_fileIndex.val += 1;
    m_fileIndex.ch.send(m_fileIndex.val);

    if (size_t(m_fileIndex.val) > (m_files.val.size() - 1)) {
        return;
    }

    const File& file = m_files.val.at(size_t(m_fileIndex.val));

    IAbContextPtr ctx = std::make_shared<AbContext>();
    ctx->setGlobalVal(AbContext::Key::FilePath, file.path);
    ctx->setGlobalVal(AbContext::Key::FileIndex, size_t(m_fileIndex.val));

    m_report.beginFile(file);
    m_runner.run(m_currentTestCase.val, ctx);
}

void Autobot::onFileFinished(const IAbContextPtr& ctx)
{
    Ret completeRet = ctx->completeRet();
    if (completeRet) {
        LOGI() << "success finished, score: " << ctx->globalVal<io::path>(IAbContext::Key::FilePath);
    } else {
        LOGE() << "failed finished, score: " << ctx->globalVal<io::path>(IAbContext::Key::FilePath);
    }

    m_report.endFile(ctx);

    size_t fileIndex = ctx->globalVal<size_t>(IAbContext::Key::FileIndex);
    IF_ASSERT_FAILED(fileIndex < m_files.val.size()) {
        return;
    }

    File& file = m_files.val.at(fileIndex);
    file.completeRet = completeRet;

    m_fileFinished.send(file);

    if (m_status.val == Status::RunningAll) {
        QTimer::singleShot(10, [this]() {
            nextFile();
        });
    } else {
        doStop();
    }
}
