/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "graphicsapiprovider.h"

#include <QFile>
#include <QQuickWindow>
#include <QSGRendererInterface>

#include "global/internal/globalconfiguration.h"
#include "global/logger.h"

#include "log.h"

using namespace muse::ui;
using namespace muse::logger;

#ifdef Q_OS_WIN
static const std::vector<GraphicsApi > ALLOWED_APIS = {
    GraphicsApi::Direct3D11,
    GraphicsApi::OpenGL,
    GraphicsApi::Software
};
#endif

#ifdef Q_OS_MACOS
static const std::vector<GraphicsApi > ALLOWED_APIS = {
    GraphicsApi::Metal,
    GraphicsApi::OpenGL,
    GraphicsApi::Software
};
#endif

#ifdef Q_OS_LINUX
static const std::vector<GraphicsApi > ALLOWED_APIS = {
    GraphicsApi::OpenGL,
    GraphicsApi::Software
};
#endif

#ifdef Q_OS_FREEBSD
static const std::vector<GraphicsApi > ALLOWED_APIS = {
    GraphicsApi::OpenGL,
    GraphicsApi::Software
};
#endif

#ifdef Q_OS_WASM
static const std::vector<GraphicsApi > ALLOWED_APIS = {
    GraphicsApi::OpenGL,
    GraphicsApi::Software
};
#endif

static const std::vector<std::string> BAD_MESSAGES = {
    "Failed to build graphics pipeline state",
    "Failed to create RHI",
    "Failed to build texture render target for layer",
    "Framebuffer incomplete:"
};

GraphicsTestObject* GraphicsApiProvider::graphicsTestObject = nullptr;

namespace muse::ui {
class GraphicsProblemsDetectorLogDest : public LogDest
{
public:

    GraphicsProblemsDetectorLogDest()
        : LogDest(LogLayout("")) {}

    std::string name() const { return "GraphicsProblemsDetectorLogDest"; }
    void write(const LogMsg& logMsg)
    {
        if (isProblemDetected) {
            return;
        }

        for (const std::string& m : BAD_MESSAGES) {
            if (logMsg.message.find(m) != std::string::npos) {
                isProblemDetected = true;
                std::cout << "found message: " << m << std::endl;
                break;
            }
        }
    }

    bool isProblemDetected = false;
};
}

GraphicsApiProvider::GraphicsApiProvider(const Version& appVersion)
    : m_appVersion(appVersion)
{
    Logger* logger = Logger::instance();

    m_logDest = new GraphicsProblemsDetectorLogDest();
    logger->addDest(m_logDest);

    QObject::connect(&m_timer, &QTimer::timeout, [this]() {
        // fail case
        if (m_logDest->isProblemDetected) {
            if (m_onResult) {
                m_onResult(false);
            }
            m_timer.stop();
        } else
        // success case
        if (graphicsTestObject && graphicsTestObject->painted) {
            if (m_onResult) {
                m_onResult(true);
            }
            m_timer.stop();
        }
    });

    qmlRegisterType<GraphicsTestObject>("Muse.Ui", 1, 0, "GraphicsTestObject");
}

GraphicsApiProvider::~GraphicsApiProvider()
{
    m_timer.stop();
    Logger* logger = Logger::instance();
    logger->removeDest(m_logDest);
    delete m_logDest;
}

void GraphicsApiProvider::setGraphicsApi(GraphicsApi api)
{
    QQuickWindow::setGraphicsApi(static_cast<QSGRendererInterface::GraphicsApi>(api));
}

GraphicsApi GraphicsApiProvider::graphicsApi()
{
    //! NOTE QQuickWindow::graphicsApi returns a not valid value,
    //! it does not take into account the current backend

    QString backend = QQuickWindow::sceneGraphBackend();
    if (backend == "software") {
        return GraphicsApi::Software;
    } else if (backend == "openvg") {
        return GraphicsApi::OpenVG;
    }

    return static_cast<GraphicsApi>(QQuickWindow::graphicsApi());
}

QString GraphicsApiProvider::graphicsApiName()
{
    return apiName(graphicsApi());
}

GraphicsApi GraphicsApiProvider::apiByName(const QString& name)
{
    for (const GraphicsApi& a : ALLOWED_APIS) {
        if (apiName(a) == name) {
            return a;
        }
    }
    return GraphicsApi::Default;
}

QString GraphicsApiProvider::apiName(GraphicsApi api)
{
    switch (api) {
    case GraphicsApi::Default: return "default";
    case GraphicsApi::Software: return "software";
    case GraphicsApi::OpenVG: return "openvg";
    case GraphicsApi::OpenGL: return "opengl";
    case GraphicsApi::Direct3D11: return "d3d11";
    case GraphicsApi::Vulkan: return "vulkan";
    case GraphicsApi::Metal: return "metal";
    case GraphicsApi::Null: return "null";
    }

    return QString();
}

QString GraphicsApiProvider::dataFilePath() const
{
    GlobalConfiguration conf(modularity::globalCtx());
    return conf.userAppDataPath().toQString() + "/required_graphicsapi.dat";
}

GraphicsApiProvider::Data GraphicsApiProvider::readData() const
{
    QFile file(dataFilePath());
    if (!file.exists()) {
        return Data();
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return Data();
    }

    QString str = QString::fromUtf8(file.readAll());

    // format:  appversion|api|status
    QStringList vals = str.split('|');

    //! NOTE Check format
    if (vals.size() < 3) {
        LOGE() << "Graphics GraphicsApi data has bad format: " << str << ", file: " << file.fileName();
        return Data();
    }

    Data d;
    d.appVersion = vals.at(0).trimmed();
    d.apiName = vals.at(1).trimmed();
    d.status = static_cast<Status>(vals.at(2).trimmed().toInt());

    return d;
}

void GraphicsApiProvider::writeData(const Data& d)
{
    QStringList vals = { d.appVersion, d.apiName, QString::number(int(d.status)) };
    QString str = vals.join('|');

    QFile file(dataFilePath());
    file.open(QIODevice::WriteOnly);
    file.write(str.toUtf8());
}

GraphicsApi GraphicsApiProvider::requiredGraphicsApi()
{
    Data data = readData();
    if (!data.isValid()) {
        return GraphicsApi::Default;
    }

    //! NOTE For the new version, we will repeat the check starting with the default API
    if (m_appVersion.toString() != data.appVersion) {
        LOGI() << "Graphics GraphicsApi data from another version, the check will be repeated starting from the default API,"
               << " data app version: " << data.appVersion
               << " current app version: " << m_appVersion.toString();
        return GraphicsApi::Default;
    }

    GraphicsApi api = apiByName(data.apiName);
    IF_ASSERT_FAILED(api != GraphicsApi::Default) {
        return switchToNextGraphicsApi(api);
    }

    //! NOTE If there is a required API, but it status is checking,
    //! then something bad happened with its use (for example, a crash),
    //! so we switch to the next one.
    if (data.status == Status::Checking) {
        return switchToNextGraphicsApi(api);
    }

    return api;
}

void GraphicsApiProvider::setGraphicsApiStatus(GraphicsApi api, Status status)
{
    if (api == GraphicsApi::Default) {
        return;
    }

    Data d;
    d.appVersion = m_appVersion.toString();
    d.apiName = apiName(api);
    d.status = status;
    writeData(d);
}

GraphicsApi GraphicsApiProvider::nextGraphicsApi(GraphicsApi api) const
{
    //! NOTE If this is the default value, then it means the current one, since we shouldn't have changed it yet.
    if (api == GraphicsApi::Default) {
        api = graphicsApi();
    }

    auto it = std::find(ALLOWED_APIS.begin(), ALLOWED_APIS.end(), api);
    IF_ASSERT_FAILED(it != ALLOWED_APIS.end()) {
        //! NOTE Something strange happened that shouldn't happen.
        //! Let's switch to software to avoid an infinite loop in which there is an error
        return GraphicsApi::Software;
    }

    ++it;

    //! NOTE This was the last api
    if (it == ALLOWED_APIS.end()) {
        return GraphicsApi::Software;
    }

    return *it;
}

GraphicsApi GraphicsApiProvider::switchToNextGraphicsApi(GraphicsApi current)
{
    GraphicsApi api = nextGraphicsApi(current);

    Data d;
    d.appVersion = m_appVersion.toString();
    d.apiName = apiName(api);
    d.status = Status::NeedCheck;
    writeData(d);

    return api;
}

void GraphicsApiProvider::listen(const OnResult& f)
{
    m_onResult = f;
    m_timer.start(10);
}

void GraphicsApiProvider::destroy()
{
    GraphicsApiProvider* self = this;
    QTimer::singleShot(1, [self]() {
        delete self;
    });
}

// GraphicsTestObject

GraphicsTestObject::GraphicsTestObject()
{
    setWidth(1);
    setHeight(1);

    GraphicsApiProvider::graphicsTestObject = this;
}

GraphicsTestObject::~GraphicsTestObject()
{
    GraphicsApiProvider::graphicsTestObject = nullptr;
}

void GraphicsTestObject::paint(QPainter*)
{
    LOGD() << "painted, graphics api: " << GraphicsApiProvider::graphicsApiName();

    // just for test
    // {
    //     if (GraphicsApiProvider::graphicsApi() == GraphicsApiProvider::Direct3D11) {
    //         LOGDA() << "Failed to build graphics pipeline state";
    //     }

    //     if (GraphicsApiProvider::graphicsApi() == GraphicsApiProvider::OpenGL) {
    //         LOGDA() << "Failed to build graphics pipeline state";
    //     }
    // }

    painted = true;
}
