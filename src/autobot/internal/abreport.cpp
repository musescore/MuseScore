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
#include "abreport.h"

#include <QDateTime>

#include "io/path.h"
#include "log.h"

using namespace mu::autobot;

inline QTextStream& operator<<(QTextStream& s, const std::string& v)
{
    s << v.c_str();
    return s;
}

static QString formatRet(const mu::Ret& ret)
{
    if (ret) {
        return QString("success");
    } else {
        return QString("error, code: %1, text: %2").arg(ret.code()).arg(QString::fromStdString(ret.text()));
    }
}

static QString formatVal(IAbContext::Key key, const IAbContext::Val& val)
{
    switch (key) {
    case IAbContext::Key::Undefined:    return QString("undefined: undefined");
    case IAbContext::Key::FilePath:     return QString("file path: %1").arg(std::any_cast<mu::io::path>(val).toQString());
    case IAbContext::Key::FileIndex:    return QString("file index: %1").arg(std::any_cast<size_t>(val));
    case IAbContext::Key::ViewZoom:     return QString("view zoom: %1").arg(std::any_cast<int>(val));
    case IAbContext::Key::CurDrawData:  return QString("current drawdata: %1").arg(val.has_value() ? "has" : "none");
    case IAbContext::Key::RefDrawData:  return QString("ref drawdata: %1").arg(val.has_value() ? "has" : "none");
    case IAbContext::Key::DiffDrawData: return QString("diff drawdata: %1").arg(val.has_value() ? "has" : "none");
    }

    return QString("undefined: undefined");
}

mu::Ret AbReport::beginReport(const ITestCasePtr& testCase)
{
    IF_ASSERT_FAILED(testCase) {
        return make_ret(Ret::Code::InternalError);
    }

    io::path reportsPath = configuration()->reportsPath();
    Ret ret = fileSystem()->makePath(reportsPath);
    if (!ret) {
        return ret;
    }

    IF_ASSERT_FAILED(!m_file.isOpen()) {
        m_file.close();
    }

    QString tcname = QString::fromStdString(testCase->name());
    QDateTime now = QDateTime::currentDateTime();
    QString reportPath = reportsPath.toQString()
                         + "/" + tcname
                         + "_" + now.toString("yyyyMMddhhmm")
                         + ".txt";

    m_file.setFileName(reportPath);
    if (!m_file.open(QIODevice::WriteOnly)) {
        return make_ret(Ret::Code::UnknownError);
    }

    m_stream.setDevice(&m_file);

    m_stream << "Test: " << tcname << Qt::endl;
    m_stream << "date: " << now.toString("yyyy.MM.dd hh:mm") << Qt::endl;
    m_stream << "steps: ";
    for (const ITestStepPtr& step : testCase->steps()) {
        m_stream << step->name() << " ";
    }
    m_stream << Qt::endl;
    m_stream << Qt::endl;

    m_opened = true;
    return make_ret(Ret::Code::Ok);
}

void AbReport::endReport()
{
    if (!m_opened) {
        return;
    }

    m_stream.flush();
    m_file.close();
}

void AbReport::beginFile(const File& file)
{
    if (!m_opened) {
        return;
    }
    m_stream << "File: " << io::filename(file.path).toQString() << Qt::endl;
}

void AbReport::endFile(const IAbContextPtr& ctx)
{
    if (!m_opened) {
        return;
    }
    m_stream << "end file: " << formatRet(ctx->completeRet()) << Qt::endl;
    m_stream << "===============================================" << Qt::endl;
    m_stream << Qt::endl;
}

void AbReport::beginStep(const IAbContextPtr& ctx)
{
    if (!m_opened) {
        return;
    }
    m_stream << "  begin step: " << ctx->currentStep().name << Qt::endl;
    m_stream.flush();
}

void AbReport::endStep(const IAbContextPtr& ctx)
{
    if (!m_opened) {
        return;
    }

    const IAbContext::StepContext& step = ctx->currentStep();
    for (auto it = step.vals.cbegin(); it != step.vals.cend(); ++it) {
        m_stream << "    " << formatVal(it->first, it->second) << Qt::endl;
    }

    m_stream << "  end: " << formatRet(ctx->currentStep().ret) << Qt::endl;
}
