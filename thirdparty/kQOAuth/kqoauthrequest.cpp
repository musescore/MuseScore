/**
 * KQOAuth - An OAuth authentication library for Qt.
 *
 * Author: Johan Paul (johan.paul@gmail.com)
 *         http://www.johanpaul.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  KQOAuth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with KQOAuth.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QByteArray>
#include <QDateTime>
#include <QCryptographicHash>
#include <QPair>
#include <QStringList>

#include <QtDebug>
#include <QtAlgorithms>

#include "kqoauthrequest.h"
#include "kqoauthrequest_p.h"
#include "kqoauthutils.h"
#include "kqoauthglobals.h"


//////////// Private d_ptr implementation /////////

KQOAuthRequestPrivate::KQOAuthRequestPrivate() :
    timeout(0)
{

}

KQOAuthRequestPrivate::~KQOAuthRequestPrivate()
{

}

// This method will not include the "oauthSignature" paramater, since it is calculated from these parameters.
void KQOAuthRequestPrivate::prepareRequest() {

    // If parameter list is not empty, we don't want to insert these values by
    // accident a second time. So giving up.
    if( !requestParameters.isEmpty() ) {
        return;
    }

    switch ( requestType ) {
    case KQOAuthRequest::TemporaryCredentials:
        requestParameters.append( qMakePair( OAUTH_KEY_CALLBACK, oauthCallbackUrl.toString()) );  // This is so ugly that it is almost beautiful.
        requestParameters.append( qMakePair( OAUTH_KEY_SIGNATURE_METHOD, oauthSignatureMethod) );
        requestParameters.append( qMakePair( OAUTH_KEY_CONSUMER_KEY, oauthConsumerKey ));
        requestParameters.append( qMakePair( OAUTH_KEY_VERSION, oauthVersion ));
        requestParameters.append( qMakePair( OAUTH_KEY_TIMESTAMP, this->oauthTimestamp() ));
        requestParameters.append( qMakePair( OAUTH_KEY_NONCE, this->oauthNonce() ));
        break;

    case KQOAuthRequest::AccessToken:
        requestParameters.append( qMakePair( OAUTH_KEY_SIGNATURE_METHOD, oauthSignatureMethod ));
        requestParameters.append( qMakePair( OAUTH_KEY_CONSUMER_KEY, oauthConsumerKey ));
        requestParameters.append( qMakePair( OAUTH_KEY_VERSION, oauthVersion ));
        requestParameters.append( qMakePair( OAUTH_KEY_TIMESTAMP, this->oauthTimestamp() ));
        requestParameters.append( qMakePair( OAUTH_KEY_NONCE, this->oauthNonce() ));
        requestParameters.append( qMakePair( OAUTH_KEY_VERIFIER, oauthVerifier ));
        requestParameters.append( qMakePair( OAUTH_KEY_TOKEN, oauthToken ));
        break;

    case KQOAuthRequest::AuthorizedRequest:
        requestParameters.append( qMakePair( OAUTH_KEY_SIGNATURE_METHOD, oauthSignatureMethod ));
        requestParameters.append( qMakePair( OAUTH_KEY_CONSUMER_KEY, oauthConsumerKey ));
        requestParameters.append( qMakePair( OAUTH_KEY_VERSION, oauthVersion ));
        requestParameters.append( qMakePair( OAUTH_KEY_TIMESTAMP, this->oauthTimestamp() ));
        requestParameters.append( qMakePair( OAUTH_KEY_NONCE, this->oauthNonce() ));
        requestParameters.append( qMakePair( OAUTH_KEY_TOKEN, oauthToken ));
        break;

    default:
        break;
    }
}

void KQOAuthRequestPrivate::signRequest() {
    QString signature = this->oauthSignature();
    requestParameters.append( qMakePair( OAUTH_KEY_SIGNATURE, signature) );
}

QString KQOAuthRequestPrivate::oauthSignature()  {
    /**
     * http://oauth.net/core/1.0/#anchor16
     * The HMAC-SHA1 signature method uses the HMAC-SHA1 signature algorithm as defined in [RFC2104] where the
     * Signature Base String is the text and the key is the concatenated values (each first encoded per Parameter
     * Encoding) of the Consumer Secret and Token Secret, separated by an ‘&’ character (ASCII code 38) even if empty.
     *
     **/
    QByteArray baseString = this->requestBaseString();


    // Default: Use HMAC-SHA1
    QString secret = QString(QUrl::toPercentEncoding(oauthConsumerSecretKey)) + "&" + QString(QUrl::toPercentEncoding(oauthTokenSecret));
    QString signature = KQOAuthUtils::hmac_sha1(baseString, secret);


    if (debugOutput) {
        qDebug() << "========== KQOAuthRequest has the following signature:";
        qDebug() << " * Signature : " << QUrl::toPercentEncoding(signature) << "\n";
    }
    return QString( QUrl::toPercentEncoding(signature) );
}

bool normalizedParameterSort(const QPair<QString, QString> &left, const QPair<QString, QString> &right) {
    QString keyLeft = left.first;
    QString valueLeft = left.second;
    QString keyRight = right.first;
    QString valueRight = right.second;

    if(keyLeft == keyRight) {
        return (valueLeft < valueRight);
    } else {
        return (keyLeft < keyRight);
    }
}
QByteArray KQOAuthRequestPrivate::requestBaseString() {
    QByteArray baseString;

    // Every request has these as the commont parameters.
    baseString.append( oauthHttpMethodString.toUtf8() + "&");                                                     // HTTP method
    baseString.append( QUrl::toPercentEncoding( oauthRequestEndpoint.toString(QUrl::RemoveQuery) ) + "&" ); // The path and query components

    QList< QPair<QString, QString> > baseStringParameters;
    baseStringParameters.append(requestParameters);
    baseStringParameters.append(additionalParameters);

    // Sort the request parameters. These parameters have been
    // initialized earlier.
    qSort(baseStringParameters.begin(),
          baseStringParameters.end(),
          normalizedParameterSort
          );

    // Last append the request parameters correctly encoded.
    baseString.append( encodedParamaterList(baseStringParameters) );

    if (debugOutput) {
        qDebug() << "========== KQOAuthRequest has the following base string:";
        qDebug() << baseString << "\n";
    }

    return baseString;
}

QByteArray KQOAuthRequestPrivate::encodedParamaterList(const QList< QPair<QString, QString> > &parameters) {
    QByteArray resultList;

    bool first = true;
    QPair<QString, QString> parameter;

    // Do the debug output.
    if (debugOutput) {
        qDebug() << "========== KQOAuthRequest has the following parameters:";
    }
    foreach (parameter, parameters) {
        if(!first) {
            resultList.append( "&" );
        } else {
            first = false;
        }

        // Here we don't need to explicitely encode the strings to UTF-8 since
        // QUrl::toPercentEncoding() takes care of that for us.
        resultList.append( QUrl::toPercentEncoding(parameter.first)     // Parameter key
                           + "="
                           + QUrl::toPercentEncoding(parameter.second)  // Parameter value
                          );
        if (debugOutput) {
            qDebug() << " * "
                     << parameter.first
                     << " : "
                     << parameter.second;
        }
    }
    if (debugOutput) {
        qDebug() << "\n";
    }

    return QUrl::toPercentEncoding(resultList);
}

QString KQOAuthRequestPrivate::oauthTimestamp() const {
    // This is basically for unit tests only. In most cases we don't set the nonce beforehand.
    if (!oauthTimestamp_.isEmpty()) {
        return oauthTimestamp_;
    }

#if QT_VERSION >= 0x040700
    return QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#else
   return QString::number(QDateTime::currentDateTime().toUTC().toTime_t());
#endif

}

QString KQOAuthRequestPrivate::oauthNonce() const {
    // This is basically for unit tests only. In most cases we don't set the nonce beforehand.
    if (!oauthNonce_.isEmpty()) {
        return oauthNonce_;
    }

    return QString::number(qrand());
}

bool KQOAuthRequestPrivate::validateRequest() const {
    switch ( requestType ) {
    case KQOAuthRequest::TemporaryCredentials:
        if (oauthRequestEndpoint.isEmpty()
            || oauthConsumerKey.isEmpty()
            || oauthNonce_.isEmpty()
            || oauthSignatureMethod.isEmpty()
            || oauthTimestamp_.isEmpty()
            || oauthVersion.isEmpty())
        {
            return false;
        }
        return true;

    case KQOAuthRequest::AccessToken:
        if (oauthRequestEndpoint.isEmpty()
            || oauthVerifier.isEmpty()
            || oauthConsumerKey.isEmpty()
            || oauthNonce_.isEmpty()
            || oauthSignatureMethod.isEmpty()
            || oauthTimestamp_.isEmpty()
            || oauthToken.isEmpty()
            || oauthTokenSecret.isEmpty()
            || oauthVersion.isEmpty())
        {
            return false;
        }
        return true;

    case KQOAuthRequest::AuthorizedRequest:
        if (oauthRequestEndpoint.isEmpty()
            || oauthConsumerKey.isEmpty()
            || oauthNonce_.isEmpty()
            || oauthSignatureMethod.isEmpty()
            || oauthTimestamp_.isEmpty()
            || oauthToken.isEmpty()
            || oauthTokenSecret.isEmpty()
            || oauthVersion.isEmpty())
        {
            return false;
        }
        return true;

    default:
        return false;
    }

    // We should not come here.
    // Prevent "unreachable code" warning.
    // return false;
}

//////////// Public implementation ////////////////

KQOAuthRequest::KQOAuthRequest(QObject *parent) :
    QObject(parent),
    d_ptr(new KQOAuthRequestPrivate)
{
    Q_D(KQOAuthRequest);
    d_ptr->debugOutput = false;  // No debug output by default.
    connect(&(d->timer), SIGNAL(timeout()), this, SIGNAL(requestTimedout()));
}

KQOAuthRequest::~KQOAuthRequest()
{
    delete d_ptr;
}

void KQOAuthRequest::initRequest(KQOAuthRequest::RequestType type, const QUrl &requestEndpoint) {
    Q_D(KQOAuthRequest);

    if (!requestEndpoint.isValid()) {
        qWarning() << "Endpoint URL is not valid. Ignoring. This request might not work.";
        return;
    }

    if (type < 0 || type > KQOAuthRequest::AuthorizedRequest) {
        qWarning() << "Invalid request type. Ignoring. This request might not work.";
        return;
    }

    // Clear the request
    clearRequest();

    // Set smart defaults.
    d->requestType = type;
    d->oauthRequestEndpoint = requestEndpoint;
    d->oauthTimestamp_ = d->oauthTimestamp();
    d->oauthNonce_ = d->oauthNonce();
    this->setSignatureMethod(KQOAuthRequest::HMAC_SHA1);
    this->setHttpMethod(KQOAuthRequest::POST);
    d->oauthVersion = "1.0"; // Currently supports only version 1.0

    d->contentType = "application/x-www-form-urlencoded";
}

void KQOAuthRequest::setConsumerKey(const QString &consumerKey) {
    Q_D(KQOAuthRequest);
    d->oauthConsumerKey = consumerKey;
}

void KQOAuthRequest::setConsumerSecretKey(const QString &consumerSecretKey) {
    Q_D(KQOAuthRequest);
    d->oauthConsumerSecretKey = consumerSecretKey;
}

void KQOAuthRequest::setCallbackUrl(const QUrl &callbackUrl) {
    Q_D(KQOAuthRequest);

    d->oauthCallbackUrl = callbackUrl;
}

void KQOAuthRequest::setSignatureMethod(KQOAuthRequest::RequestSignatureMethod requestMethod) {
    Q_D(KQOAuthRequest);
    QString requestMethodString;

    switch (requestMethod) {
    case KQOAuthRequest::PLAINTEXT:
        requestMethodString = "PLAINTEXT";
        break;
    case KQOAuthRequest::HMAC_SHA1:
        requestMethodString = "HMAC-SHA1";
        break;
    default:
        // We should not come here
        qWarning() << "Invalid signature method set.";
        break;
    }

    d->oauthSignatureMethod = requestMethodString;
    d->requestSignatureMethod = requestMethod;
}

void KQOAuthRequest::setTokenSecret(const QString &tokenSecret) {
    Q_D(KQOAuthRequest);

    d->oauthTokenSecret = tokenSecret;
}

void KQOAuthRequest::setToken(const QString &token) {
    Q_D(KQOAuthRequest);

    d->oauthToken = token;
}

void KQOAuthRequest::setVerifier(const QString &verifier) {
    Q_D(KQOAuthRequest);

    d->oauthVerifier = verifier;
}


void KQOAuthRequest::setHttpMethod(KQOAuthRequest::RequestHttpMethod httpMethod) {
    Q_D(KQOAuthRequest);

    QString requestHttpMethodString;

    switch (httpMethod) {
    case KQOAuthRequest::GET:
        requestHttpMethodString = "GET";
        break;
    case KQOAuthRequest::POST:
        requestHttpMethodString = "POST";
        break;
    case KQOAuthRequest::HEAD:
        requestHttpMethodString = "HEAD";
        break;
    case KQOAuthRequest::DELETE:
        requestHttpMethodString = "DELETE";
        break;
    default:
        qWarning() << "Invalid HTTP method set.";
        break;
    }

    d->oauthHttpMethod = httpMethod;
    d->oauthHttpMethodString = requestHttpMethodString;
}

KQOAuthRequest::RequestHttpMethod KQOAuthRequest::httpMethod() const {
    Q_D(const KQOAuthRequest);

    return d->oauthHttpMethod;
}

void KQOAuthRequest::setAdditionalParameters(const KQOAuthParameters &additionalParams) {
    Q_D(KQOAuthRequest);

    QList<QString> additionalKeys = additionalParams.keys();
    QList<QString> additionalValues = additionalParams.values();

    int i=0;
    foreach(QString key, additionalKeys) {
        QString value = additionalValues.at(i);
        d->additionalParameters.append( qMakePair(key, value) );
        i++;
    }
}

KQOAuthParameters KQOAuthRequest::additionalParameters() const {
    Q_D(const KQOAuthRequest);

    QMultiMap<QString, QString> additionalParams;
    for(int i=0; i<d->additionalParameters.size(); i++) {
        additionalParams.insert(d->additionalParameters.at(i).first,
                                d->additionalParameters.at(i).second);
    }

    return additionalParams;
}

KQOAuthRequest::RequestType KQOAuthRequest::requestType() const {
    Q_D(const KQOAuthRequest);
    return d->requestType;
}

QUrl KQOAuthRequest::requestEndpoint() const {
    Q_D(const KQOAuthRequest);
    return d->oauthRequestEndpoint;
}

QList<QByteArray> KQOAuthRequest::requestParameters() {
    Q_D(KQOAuthRequest);

    QList<QByteArray> requestParamList;

    d->prepareRequest();
    if (!isValid() ) {
        qWarning() << "Request is not valid! I will still sign it, but it will probably not work.";
    }

    d->signRequest();

    QPair<QString, QString> requestParam;
    QString param;
    QString value;
    foreach (requestParam, d->requestParameters) {
        param = requestParam.first;
        value = requestParam.second;
        if (param != OAUTH_KEY_SIGNATURE) {
            value = QUrl::toPercentEncoding(value);
        }

        requestParamList.append(QString(param + "=\"" + value +"\"").toUtf8());
    }

    return requestParamList;
}

QString KQOAuthRequest::contentType()
{
    Q_D(const KQOAuthRequest);
    return d->contentType;
}

void KQOAuthRequest::setContentType(const QString &contentType)
{
    Q_D(KQOAuthRequest);
    d->contentType = contentType;
}

QByteArray KQOAuthRequest::rawData()
{
    Q_D(const KQOAuthRequest);
    return d->postRawData;
}

void KQOAuthRequest::setRawData(const QByteArray &rawData)
{
    Q_D(KQOAuthRequest);
    d->postRawData = rawData;
}

QHttpMultiPart* KQOAuthRequest::httpMultiPart()
{
    Q_D(const KQOAuthRequest);
    return d->httpMultiPart;
}

void KQOAuthRequest::setHttpMultiPart(QHttpMultiPart* httpMultiPart)
{
    Q_D(KQOAuthRequest);
    d->httpMultiPart = httpMultiPart;
}


QByteArray KQOAuthRequest::requestBody() const {
    Q_D(const KQOAuthRequest);

    QByteArray postBodyContent;
    bool first = true;
    for(int i=0; i < d->additionalParameters.size(); i++) {
        if(!first) {
            postBodyContent.append("&");
        } else {
            first = false;
        }

        QString key = d->additionalParameters.at(i).first;
        QString value = d->additionalParameters.at(i).second;

        postBodyContent.append(QUrl::toPercentEncoding(key) + QString("=").toUtf8() +
                               QUrl::toPercentEncoding(value));
    }
    return postBodyContent;
}

bool KQOAuthRequest::isValid() const {
    Q_D(const KQOAuthRequest);

    return d->validateRequest();
}

void KQOAuthRequest::setTimeout(int timeoutMilliseconds) {
    Q_D(KQOAuthRequest);
    d->timeout = timeoutMilliseconds;
}

void KQOAuthRequest::clearRequest() {
    Q_D(KQOAuthRequest);

    d->oauthRequestEndpoint = "";
    d->oauthHttpMethodString = "";
    d->oauthConsumerKey = "";
    d->oauthConsumerSecretKey = "";
    d->oauthToken = "";
    d->oauthTokenSecret = "";
    d->oauthSignatureMethod = "";
    d->oauthCallbackUrl = "";
    d->oauthVerifier = "";
    d->oauthTimestamp_ = "";
    d->oauthNonce_ = "";
    d->requestParameters.clear();
    d->additionalParameters.clear();
    d->timeout = 0;
}

void KQOAuthRequest::setEnableDebugOutput(bool enabled) {
    Q_D(KQOAuthRequest);
    d->debugOutput = enabled;
}

/**
 * Protected implementations for inherited classes
 */
bool KQOAuthRequest::validateXAuthRequest() const {
    Q_D(const KQOAuthRequest);

    if (d->oauthRequestEndpoint.isEmpty()
        || d->oauthConsumerKey.isEmpty()
        || d->oauthNonce_.isEmpty()
        || d->oauthSignatureMethod.isEmpty()
        || d->oauthTimestamp_.isEmpty()
        || d->oauthVersion.isEmpty())
    {
        return false;
    }
    return true;
}


/**
 * Private implementations for friend classes
 */
QString KQOAuthRequest::consumerKeyForManager() const {
    Q_D(const KQOAuthRequest);
    return d->oauthConsumerKey;
}

QString KQOAuthRequest::consumerKeySecretForManager() const {
    Q_D(const KQOAuthRequest);
    return d->oauthConsumerSecretKey;
}

KQOAuthRequest::RequestSignatureMethod KQOAuthRequest::requestSignatureMethodForManager() const {
    Q_D(const KQOAuthRequest);
    return d->requestSignatureMethod;
}

QUrl KQOAuthRequest::callbackUrlForManager() const {
    Q_D(const KQOAuthRequest);
    return d->oauthCallbackUrl;
}

void KQOAuthRequest::requestTimerStart()
{
    Q_D(KQOAuthRequest);
    if (d->timeout > 0) {
        d->timer.start(d->timeout);
    }
}

void KQOAuthRequest::requestTimerStop()
{
    Q_D(KQOAuthRequest);
    if( d->timer.isActive() )
        d->timer.stop();
}
