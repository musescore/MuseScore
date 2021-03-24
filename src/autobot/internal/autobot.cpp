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

#include "abscorelist.h"
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
        f.ret = Ret();
    }
    m_files.notification.notify();
}

const mu::ValCh<ITestCasePtr>& Autobot::currentTestCase() const
{
    return m_currentTestCase;
}

void Autobot::init()
{
    m_status.val = Status::Stoped;

    m_runner.finished().onReceive(this, [this](const AbContext& ctx) {
        onFileFinished(ctx);
    });

    m_currentTestCase.set(m_testCases.front());

    RetVal<io::paths> scores = AbScoreList().scoreList();
    if (!scores.ret) {
        LOGE() << "failed get score list, err: " << scores.ret.toString();
        return;
    }

    m_files.val.clear();
    for (const io::path& p : scores.val) {
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

    nextFile();
}

void Autobot::stop()
{
    m_status.set(Status::Stoped);
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

    if (m_status.val == Status::Stoped) {
        return;
    }

    m_fileIndex.val += 1;
    m_fileIndex.ch.send(m_fileIndex.val);

    if (size_t(m_fileIndex.val) > (m_files.val.size() - 1)) {
        return;
    }

    const File& file = m_files.val.at(size_t(m_fileIndex.val));

    AbContext ctx;
    ctx.setVal<io::path>(AbContext::Key::FilePath, file.path);
    ctx.setVal<size_t>(AbContext::Key::FileIndex, size_t(m_fileIndex.val));

    m_runner.run(m_currentTestCase.val, ctx);
}

void Autobot::onFileFinished(const AbContext& ctx)
{
    if (ctx.ret) {
        LOGI() << "success finished, score: " << ctx.val<io::path>(AbContext::Key::FilePath);
    } else {
        LOGE() << "failed finished, score: " << ctx.val<io::path>(AbContext::Key::FilePath);
    }

    size_t fileIndex = ctx.val<size_t>(AbContext::Key::FileIndex);
    IF_ASSERT_FAILED(fileIndex < m_files.val.size()) {
        return;
    }

    File& file = m_files.val.at(fileIndex);
    file.ret = ctx.ret;

    m_fileFinished.send(file);

    if (m_status.val == Status::RunningAll) {
        QTimer::singleShot(10, [this]() {
            nextFile();
        });
    } else {
        stop();
    }
}
