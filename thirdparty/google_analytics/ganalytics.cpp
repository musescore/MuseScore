#include "ganalytics.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQueue>
#include <QSettings>
#include <QTimer>
#include <QUrlQuery>
#include <QUuid>

#ifdef QT_GUI_LIB
#include <QScreen>
#include <QGuiApplication>
#endif // QT_GUI_LIB

#ifdef QT_QML_LIB
#include <QQmlEngine>
#include <QQmlContext>
#endif // QT_QML_LIB

struct QueryBuffer
{
    QUrlQuery postQuery;
    QDateTime time;
};

/**
 * Class Private
 * Private members and functions.
 */
class GAnalytics::Private : public QObject
{
    Q_OBJECT

public:
    explicit Private(GAnalytics *parent = 0);
    ~Private();

    GAnalytics *q;

    QNetworkAccessManager *networkManager;

    QQueue<QueryBuffer> messageQueue;
    QTimer timer;
    QNetworkRequest request;
    GAnalytics::LogLevel logLevel;

    QString trackingID;
    QString clientID;
    QString userID;
    QString appName;
    QString appVersion;
    QString language;
    QString screenResolution;
    QString viewportSize;

    bool isSending;

    const static int fourHours = 4 * 60 * 60 * 1000;
    const static QString dateTimeFormat;

public:
    void logMessage(GAnalytics::LogLevel level, const QString &message);

    QUrlQuery buildStandardPostQuery(const QString &type);
#ifdef QT_GUI_LIB
    QString getScreenResolution();
#endif // QT_GUI_LIB
    QString getUserAgent();
    QString getSystemInfo();
    QList<QString> persistMessageQueue();
    void readMessagesFromFile(const QList<QString> &dataList);
    QString getClientID();
    QString getUserID();
    void setUserID(const QString &userID);
    void enqueQueryWithCurrentTime(const QUrlQuery &query);
    void setIsSending(bool doSend);

signals:
    void postNextMessage();

public slots:
    void postMessage();
    void postMessageFinished();
};

const QString GAnalytics::Private::dateTimeFormat  = "yyyy,MM,dd-hh:mm::ss:zzz";

/**
 * Constructor
 * Constructs an object of class Private.
 * @param parent
 */
GAnalytics::Private::Private(GAnalytics *parent)
: QObject(parent)
, q(parent)
, networkManager(NULL)
, request(QUrl("http://www.google-analytics.com/collect"))
, logLevel(GAnalytics::Error)
, isSending(false)
{
    clientID = getClientID();
    userID = getUserID();
    language = QLocale::system().name().toLower().replace("_", "-");
#ifdef QT_GUI_LIB
    screenResolution = getScreenResolution();
#endif // QT_GUI_LIB
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    appName = QCoreApplication::instance()->applicationName();
    appVersion = QCoreApplication::instance()->applicationVersion();
    request.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent());
    connect(this, SIGNAL(postNextMessage()), this, SLOT(postMessage()));
    timer.start(30000);
    connect(&timer, SIGNAL(timeout()), this, SLOT(postMessage()));
}

/**
 * Destructor
 * Delete an object of class Private.
 */
GAnalytics::Private::~Private()
{
}

void GAnalytics::Private::logMessage(LogLevel level, const QString &message)
{
    if (logLevel > level)
    {
        return;
    }

    qDebug() << "[Analytics]" << message;
}

/**
 * Build the POST query. Adds all parameter to the query
 * which are used in every POST.
 * @param type      Type of POST message. The event which is to post.
 * @return query    Most used parameter in a query for a POST.
 */
QUrlQuery GAnalytics::Private::buildStandardPostQuery(const QString &type)
{
    QUrlQuery query;
    query.addQueryItem("v", "1");
    query.addQueryItem("tid", trackingID);
    query.addQueryItem("cid", clientID);
    if(!userID.isEmpty())
    {
        query.addQueryItem("uid", userID);
    }
    query.addQueryItem("t", type);
    query.addQueryItem("ul", language);

#ifdef QT_GUI_LIB
    query.addQueryItem("vp", viewportSize);
    query.addQueryItem("sr", screenResolution);
#endif // QT_GUI_LIB

    return query;
}

#ifdef QT_GUI_LIB
/**
 * Get devicese screen resolution.
 * @return      A QString like "800x600".
 */
QString GAnalytics::Private::getScreenResolution()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QSize size = screen->size();

    return QString("%1x%2").arg(size.width()).arg(size.height());
}
#endif // QT_GUI_LIB


/**
 * Try to gain information about the system where this application
 * is running. It needs to get the name and version of the operating
 * system, the language and screen resolution.
 * All this information will be send in POST messages.
 * @return agent        A QString with all the information formatted for a POST message.
 */
QString GAnalytics::Private::getUserAgent()
{
    QString locale = QLocale::system().name();
    QString system = getSystemInfo();

    return QString("%1/%2 (%3; %4) GAnalytics/1.0 (Qt/%5)").arg(appName).arg(appVersion).arg(system).arg(locale).arg(QT_VERSION_STR);
}


#ifdef Q_OS_MAC
/**
 * Only on Mac OS X
 * Get the Operating system name and version.
 * @return os   The operating system name and version in a string.
 */
QString GAnalytics::Private::getSystemInfo()
{
    QSysInfo::MacVersion version = QSysInfo::macVersion();
    QString os;
    switch (version)
    {
    case QSysInfo::MV_9:
        os = "Macintosh; Mac OS 9";
        break;
    case QSysInfo::MV_10_0:
        os = "Macintosh; Mac OS 10.0";
        break;
    case QSysInfo::MV_10_1:
        os = "Macintosh; Mac OS 10.1";
        break;
    case QSysInfo::MV_10_2:
        os = "Macintosh; Mac OS 10.2";
        break;
    case QSysInfo::MV_10_3:
        os = "Macintosh; Mac OS 10.3";
        break;
    case QSysInfo::MV_10_4:
        os = "Macintosh; Mac OS 10.4";
        break;
    case QSysInfo::MV_10_5:
        os = "Macintosh; Mac OS 10.5";
        break;
    case QSysInfo::MV_10_6:
        os = "Macintosh; Mac OS 10.6";
        break;
    case QSysInfo::MV_10_7:
        os = "Macintosh; Mac OS 10.7";
        break;
    case QSysInfo::MV_10_8:
        os = "Macintosh; Mac OS 10.8";
        break;
    case QSysInfo::MV_10_9:
        os = "Macintosh; Mac OS 10.9";
        break;
    case QSysInfo::MV_10_10:
        os = "Macintosh; Mac OS 10.10";
        break;
    case QSysInfo::MV_10_11:
        os = "Macintosh; Mac OS 10.11";
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    case QSysInfo::MV_10_12:
        os = "Macintosh; Mac OS 10.12";
        break;
#endif
    case QSysInfo::MV_Unknown:
        os = "Macintosh; Mac OS unknown";
        break;
    case QSysInfo::MV_IOS_5_0:
        os = "iPhone; iOS 5.0";
        break;
    case QSysInfo::MV_IOS_5_1:
        os = "iPhone; iOS 5.1";
        break;
    case QSysInfo::MV_IOS_6_0:
        os = "iPhone; iOS 6.0";
        break;
    case QSysInfo::MV_IOS_6_1:
        os = "iPhone; iOS 6.1";
        break;
    case QSysInfo::MV_IOS_7_0:
        os = "iPhone; iOS 7.0";
        break;
    case QSysInfo::MV_IOS_7_1:
        os = "iPhone; iOS 7.1";
        break;
    case QSysInfo::MV_IOS_8_0:
        os = "iPhone; iOS 8.0";
        break;
    case QSysInfo::MV_IOS_8_1:
        os = "iPhone; iOS 8.1";
        break;
    case QSysInfo::MV_IOS_8_2:
        os = "iPhone; iOS 8.2";
        break;
    case QSysInfo::MV_IOS_8_3:
        os = "iPhone; iOS 8.3";
        break;
    case QSysInfo::MV_IOS_8_4:
        os = "iPhone; iOS 8.4";
        break;
    case QSysInfo::MV_IOS_9_0:
        os = "iPhone; iOS 9.0";
        break;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    case QSysInfo::MV_IOS_9_1:
        os = "iPhone; iOS 9.1";
        break;
    case QSysInfo::MV_IOS_9_2:
        os = "iPhone; iOS 9.2";
        break;
    case QSysInfo::MV_IOS_9_3:
        os = "iPhone; iOS 9.3";
        break;
    case QSysInfo::MV_IOS_10_0:
        os = "iPhone; iOS 10.0";
        break;
#endif
    case QSysInfo::MV_IOS:
        os = "iPhone; iOS unknown";
        break;
    default:
        os = "Macintosh";
        break;
    }
    return os;
}
#endif

#ifdef Q_OS_WIN
/**
 * Only on Windows
 * Get operating system and its version.
 * @return os   A QString containing the oprating systems name and version.
 */
QString GAnalytics::Private::getSystemInfo()
{
    QSysInfo::WinVersion version = QSysInfo::windowsVersion();
    QString os("Windows; ");
    switch (version)
    {
    case QSysInfo::WV_95:
        os += "Win 95";
        break;
    case QSysInfo::WV_98:
        os += "Win 98";
        break;
    case QSysInfo::WV_Me:
        os += "Win ME";
        break;
    case QSysInfo::WV_NT:
        os += "Win NT";
        break;
    case QSysInfo::WV_2000:
        os += "Win 2000";
        break;
    case QSysInfo::WV_2003:
        os += "Win Server 2003";
        break;
    case QSysInfo::WV_VISTA:
        os += "Win Vista";
        break;
    case QSysInfo::WV_WINDOWS7:
        os += "Win 7";
        break;
    case QSysInfo::WV_WINDOWS8:
        os += "Win 8";
        break;
    case QSysInfo::WV_WINDOWS8_1:
        os += "Win 8.1";
        break;
      case QSysInfo::WV_WINDOWS10:
        os += "Win 10";
        break;
    default:
        os = "Windows; unknown";
        break;
    }
    return os;
}
#endif

#if defined(Q_OS_ANDROID)
#include <QAndroidJniObject>

QString GAnalytics::Private::getSystemInfo()
{
    return QString("Linux; U; Android %1; %2 %3 Build/%4; %5")
            .arg(QAndroidJniObject::getStaticObjectField<jstring>("android/os/Build$VERSION", "RELEASE").toString())
            .arg(QAndroidJniObject::getStaticObjectField<jstring>("android/os/Build", "MANUFACTURER").toString())
            .arg(QAndroidJniObject::getStaticObjectField<jstring>("android/os/Build", "MODEL").toString())
            .arg(QAndroidJniObject::getStaticObjectField<jstring>("android/os/Build", "ID").toString())
            .arg(QAndroidJniObject::getStaticObjectField<jstring>("android/os/Build", "BRAND").toString());
}
#elif defined(Q_OS_LINUX)
#include <sys/utsname.h>

/**
 * Only on Unix systems.
 * Get operation system name and version.
 * @return os       A QString with the name and version of the operating system.
 */
QString GAnalytics::Private::getSystemInfo()
{
    struct utsname buf;
    uname(&buf);
    QString system(buf.sysname);
    QString release(buf.release);

    return system + "; " + release;
}
#endif


/**
 * The message queue contains a list of QueryBuffer object.
 * QueryBuffer holds a QUrlQuery object and a QDateTime object.
 * These both object are freed from the buffer object and
 * inserted as QString objects in a QList.
 * @return dataList     The list with concartinated queue data.
 */
QList<QString> GAnalytics::Private::persistMessageQueue()
{
    QList<QString> dataList;
    foreach (QueryBuffer buffer, messageQueue)
    {
        dataList << buffer.postQuery.toString();
        dataList << buffer.time.toString(dateTimeFormat);
    }

    return dataList;
}

/**
 * Reads persistent messages from a file.
 * Gets all message data as a QList<QString>.
 * Two lines in the list build a QueryBuffer object.
 */
void GAnalytics::Private::readMessagesFromFile(const QList<QString> &dataList)
{
    QListIterator<QString> iter(dataList);
    while (iter.hasNext())
    {
        QString queryString = iter.next();
        if(!iter.hasNext())
            break;
        QString dateString = iter.next();
        if(queryString.isEmpty() || dateString.isEmpty())
            break;
        QUrlQuery query;
        query.setQuery(queryString);
        QDateTime dateTime = QDateTime::fromString(dateString, dateTimeFormat);
        QueryBuffer buffer;
        buffer.postQuery = query;
        buffer.time = dateTime;
        messageQueue.enqueue(buffer);
    }
}

/**
 * Change the user id.
 * @param userID         A string with the user id.
 */
void GAnalytics::Private::setUserID(const QString &userID)
{
    this->userID = userID;
    QSettings settings;
    settings.setValue("GAnalytics-uid", userID);
}

/**
 * Get the user id.
 * User id once created is stored in application settings.
 * @return userID         A string with the user id.
 */
QString GAnalytics::Private::getUserID()
{    
    QSettings settings;
    QString userID = settings.value("GAnalytics-uid", QString("")).toString();

    return userID;
}

/**
 * Get the client id.
 * Client id once created is stored in application settings.
 * @return clientID         A string with the client id.
 */
QString GAnalytics::Private::getClientID()
{
    QSettings settings;
    QString clientID;
    if (!settings.contains("GAnalytics-cid"))
    {
        clientID = QUuid::createUuid().toString();
        settings.setValue("GAnalytics-cid", clientID);
    }
    else
    {
        clientID = settings.value("GAnalytics-cid").toString();
    }

    return clientID;
}

/**
 * Takes a QUrlQuery object and wrapp it together with
 * a QTime object into a QueryBuffer struct. These struct
 * will be stored in the message queue.
 * @param query
 */
void GAnalytics::Private::enqueQueryWithCurrentTime(const QUrlQuery &query)
{
    QueryBuffer buffer;
    buffer.postQuery = query;
    buffer.time = QDateTime::currentDateTime();

    messageQueue.enqueue(buffer);
}

/**
 * Change status of class. Emit signal that status was changed.
 * @param doSend
 */
void GAnalytics::Private::setIsSending(bool doSend)
{
    if (doSend)
    {
        timer.stop();
    }
    else
    {
        timer.start();
    }

    bool changed = (isSending != doSend);

    isSending = doSend;

    if (changed)
    {
        emit q->isSendingChanged(isSending);
    }
}


/**
 * CONSTRUCTOR  GAnalytics
 * ------------------------------------------------------------------------------------------------------------
 * Constructs the GAnalytics Object.
 * @param parent        The application which uses this object.
 * @param trackingID
 * @param clientID
 * @param withGet       Determines wheather the messages are send with GET or POST.
 */
GAnalytics::GAnalytics(QObject *parent)
: QObject(parent)
, d(new Private(this))
{
}

GAnalytics::GAnalytics(const QString &trackingID, QObject *parent)
: QObject(parent)
, d(new Private(this))
{
    setTrackingID(trackingID);
}

/**
 * Destructor of class GAnalytics.
 */
GAnalytics::~GAnalytics()
{
    delete d;
}

void GAnalytics::setLogLevel(GAnalytics::LogLevel logLevel)
{
    if (d->logLevel != logLevel)
    {
        d->logLevel = logLevel;
        emit logLevelChanged();
    }
}

GAnalytics::LogLevel GAnalytics::logLevel() const
{
    return d->logLevel;
}

// SETTER and GETTER
void GAnalytics::setViewportSize(const QString &viewportSize)
{
    if (d->viewportSize != viewportSize)
    {
        d->viewportSize = viewportSize;
        emit viewportSizeChanged();
    }
}

QString GAnalytics::viewportSize() const
{
    return d->viewportSize;
}

void GAnalytics::setLanguage(const QString &language)
{
    if (d->language != language)
    {
        d->language = language;
        emit languageChanged();
    }
}

QString GAnalytics::language() const
{
    return d->language;
}

void GAnalytics::setTrackingID(const QString &trackingID)
{
    if (d->trackingID != trackingID)
    {
        d->trackingID = trackingID;
        emit trackingIDChanged();
    }
}

QString GAnalytics::trackingID() const
{
    return d->trackingID;
}

void GAnalytics::setSendInterval(int milliseconds)
{
    if (d->timer.interval() != milliseconds)
    {
        d->timer.setInterval(milliseconds);
        emit sendIntervalChanged();
    }
}

void GAnalytics::setUserID(const QString &userID)
{
    if(d->userID != userID)
    {
        d->setUserID(userID);
        emit userIDChanged();
    }
}

QString GAnalytics::userID() const
{
    return d->getUserID();
}

int GAnalytics::sendInterval() const
{
    return (d->timer.interval());
}

void GAnalytics::startSending()
{
    if (!isSending())
      emit d->postNextMessage();
}

bool GAnalytics::isSending() const
{
    return d->isSending;
}

void GAnalytics::setNetworkAccessManager(QNetworkAccessManager *networkAccessManager)
{
    if (d->networkManager != networkAccessManager)
    {
        // Delete the old network manager if it was our child
        if (d->networkManager && d->networkManager->parent() == this)
        {
            d->networkManager->deleteLater();
        }

        d->networkManager = networkAccessManager;
    }
}

QNetworkAccessManager *GAnalytics::networkAccessManager() const
{
    return d->networkManager;
}

static void appendCustomValues(QUrlQuery &query, const QVariantMap &customValues) {
  for(QVariantMap::const_iterator iter = customValues.begin(); iter != customValues.end(); ++iter) {
    query.addQueryItem(iter.key(), iter.value().toString());
  }
}


/**
* SentAppview is called when the user changed the applications view.
* Deprecated because after SDK Version 3.08 and up no more "appview" event:
* Use sendScreenView() instead
* @param appName
* @param appVersion
* @param screenName
*/
void GAnalytics::sendAppView(const QString &screenName,
                             const QVariantMap &customValues)
{
    sendScreenView(screenName, customValues);
}

/**
 * Sent screen view is called when the user changed the applications view.
 * These action of the user should be noticed and reported. Therefore
 * a QUrlQuery is build in this method. It holts all the parameter for
 * a http POST. The UrlQuery will be stored in a message Queue.
 * @param appName
 * @param appVersion
 * @param screenName
 */
void GAnalytics::sendScreenView(const QString &screenName,
                                const QVariantMap &customValues)
{
    d->logMessage(Info, QString("ScreenView: %1").arg(screenName));

    QUrlQuery query = d->buildStandardPostQuery("screenview");
    query.addQueryItem("cd", screenName);
    query.addQueryItem("an", d->appName);
    query.addQueryItem("av", d->appVersion);
    appendCustomValues(query, customValues);

    d->enqueQueryWithCurrentTime(query);
}

/**
 * This method is called whenever a button was pressed in the application.
 * A query for a POST message will be created to report this event. The
 * created query will be stored in a message queue.
 * @param eventCategory
 * @param eventAction
 * @param eventLabel
 * @param eventValue
 */
void GAnalytics::sendEvent(const QString &category, const QString &action,
                           const QString &label, const QVariant &value,
                           const QVariantMap &customValues)
{
    QUrlQuery query = d->buildStandardPostQuery("event");
    query.addQueryItem("an", d->appName);
    query.addQueryItem("av", d->appVersion);
    query.addQueryItem("ec", category);
    query.addQueryItem("ea", action);
    if (! label.isEmpty())
        query.addQueryItem("el", label);
    if (value.isValid())
        query.addQueryItem("ev", value.toString());

    appendCustomValues(query, customValues);

    d->enqueQueryWithCurrentTime(query);
}

/**
 * Method is called after an exception was raised. It builds a
 * query for a POST message. These query will be stored in a
 * message queue.
 * @param exceptionDescription
 * @param exceptionFatal
 */
void GAnalytics::sendException(const QString &exceptionDescription,
                               bool exceptionFatal,
                               const QVariantMap &customValues)
{
    QUrlQuery query = d->buildStandardPostQuery("exception");
    query.addQueryItem("an", d->appName);
    query.addQueryItem("av", d->appVersion);

    query.addQueryItem("exd", exceptionDescription);

    if (exceptionFatal)
    {
        query.addQueryItem("exf", "1");
    }
    else
    {
        query.addQueryItem("exf", "0");
    }
    appendCustomValues(query, customValues);

    d->enqueQueryWithCurrentTime(query);
}

/**
 * Session starts. This event will be sent by a POST message.
 * Query is setup in this method and stored in the message
 * queue.
 */
void GAnalytics::startSession()
{
	QVariantMap customValues;
	customValues.insert("sc", "start");
	sendEvent("Session", "Start", QString(), QVariant(), customValues);
}

/**
 * Session ends. This event will be sent by a POST message.
 * Query is setup in this method and stored in the message
 * queue.
 */
void GAnalytics::endSession()
{
	QVariantMap customValues;
	customValues.insert("sc", "end");
	sendEvent("Session", "End", QString(), QVariant(), customValues);
}

/**
 * This function is called by a timer interval.
 * The function tries to send a messages from the queue.
 * If message was successfully send then this function
 * will be called back to send next message.
 * If message queue contains more than one message then
 * the connection will kept open.
 * The message POST is asyncroniously when the server
 * answered a signal will be emitted.
 */
void GAnalytics::Private::postMessage()
{
    if (messageQueue.isEmpty())
    {
        setIsSending(false);
        return;
    }
    else
    {
        setIsSending(true);
    }

    QString connection = "close";
    if (messageQueue.count() > 1)
    {
        connection = "keep-alive";
    }

    QueryBuffer buffer = messageQueue.head();
    QDateTime sendTime = QDateTime::currentDateTime();
    qint64 timeDiff = buffer.time.msecsTo(sendTime);

    if(timeDiff > fourHours)
    {
        // too old.
        messageQueue.dequeue();
        emit postNextMessage();
        return;
    }

    buffer.postQuery.addQueryItem("qt", QString::number(timeDiff));
    request.setRawHeader("Connection", connection.toUtf8());
    QByteArray ba;
    ba = buffer.postQuery.query(QUrl::FullyEncoded).toUtf8();
    request.setHeader(QNetworkRequest::ContentLengthHeader, ba.length());

    // Create a new network access manager if we don't have one yet
    if (networkManager == NULL)
    {
        networkManager = new QNetworkAccessManager(this);
    }

    QNetworkReply *reply = networkManager->post(request, ba);
    connect(reply, SIGNAL(finished()), this, SLOT(postMessageFinished()));
}

/**
 * NetworkAccsessManager has finished to POST a message.
 * If POST message was successfully send then the message
 * query should be removed from queue.
 * SIGNAL "postMessage" will be emitted to send next message
 * if there is any.
 * If message couldn't be send then next try is when the
 * timer emits its signal.
 */
void GAnalytics::Private::postMessageFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    reply->deleteLater();

    int httpStausCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (httpStausCode < 200 || httpStausCode > 299)
    {
        logMessage(GAnalytics::Error, QString("Error posting message: %1").arg(reply->errorString()));

        // An error ocurred.
        setIsSending(false);
        return;
    }
    else
    {
        logMessage(GAnalytics::Debug, "Message sent");
    }

    messageQueue.dequeue();
    emit postNextMessage();
}


/**
 * Qut stream to persist class GAnalytics.
 * @param outStream
 * @param analytics
 * @return
 */
QDataStream &operator<<(QDataStream &outStream, const GAnalytics &analytics)
{
    outStream << analytics.d->persistMessageQueue();

    return outStream;
}


/**
 * In stream to read GAnalytics from file.
 * @param inStream
 * @param analytics
 * @return
 */
QDataStream &operator >>(QDataStream &inStream, GAnalytics &analytics)
{
    QList<QString> dataList;
    inStream >> dataList;
    analytics.d->readMessagesFromFile(dataList);

    return inStream;
}

#ifdef QT_QML_LIB
void GAnalytics::classBegin()
{
    // Get the network access manager from the QmlEngine
    QQmlContext *context = QQmlEngine::contextForObject(this);
    if (context)
    {
        QQmlEngine *engine = context->engine();
        setNetworkAccessManager(engine->networkAccessManager());
    }
}

void GAnalytics::componentComplete()
{
}
#endif // QT_QML_LIB

#include "ganalytics.moc"
