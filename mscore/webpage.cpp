//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  The webview is shown on startup with a local file inviting user
//  to start connecting with the community. They can press start and
//  MuseScore will go online. If no connection, display a can't connect message
//  On next startup, if no connection, the panel is closed. If connection, the
//  MuseScore goes online directly. If the autoclose panel is reopen, the user
//  can retry; retry should not close the panel.
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "webpage.h"
#include "musescore.h"
#include "preferences.h"
#include "libmscore/score.h"


namespace Ms {

static const char* staticUrl = "http://connect.musescore.com";

//---------------------------------------------------------
//   MyNetworkAccessManager
//---------------------------------------------------------

QNetworkReply * MyNetworkAccessManager::createRequest(Operation op,
                                          const QNetworkRequest & req,
                                          QIODevice * outgoingData)
      {
      QNetworkRequest new_req(req);
      new_req.setRawHeader("User-Agent",  QString("MuseScore %1").arg(VERSION).toAscii());
      new_req.setRawHeader("Accept-Language",  QString("%1;q=0.8,en-US;q=0.6,en;q=0.4").arg(mscore->getLocaleISOCode()).toAscii());
      return QNetworkAccessManager::createRequest(op, new_req, outgoingData);
      }

//---------------------------------------------------------
//   MyWebPage
//---------------------------------------------------------

MyWebPage::MyWebPage(QObject *parent)
   : QWebPage(parent)
      {
      // Enable plugin support
      settings()->setAttribute(QWebSettings::PluginsEnabled, true);
      }

//---------------------------------------------------------
//   createPlugin
//---------------------------------------------------------

QObject* MyWebPage::createPlugin(
   const QString &/*classid*/,
   const QUrl &/*url*/,
   const QStringList &/*paramNames*/,
   const QStringList &/*paramValues*/)
      {
      // Create the widget using QUiLoader.
      // This means that the widgets don't need to be registered
      // with the meta object system.
      // On the other hand, non-gui objects can't be created this
      // way. When we'd like to create non-visual objects in
      // Html to use them via JavaScript, we'd use a different
      // mechanism than this.
#if 0
      if (classid == "WebScoreView") {
            WebScoreView* sv = new WebScoreView(view());
            int idx = paramNames.indexOf("score");
            if (idx != -1) {
                  QString score = paramValues[idx];
                  sv->setScore(paramValues[idx]);
                  }
            else {
                  qDebug("create WebScoreView: property score not found(%d)",
                     paramNames.size());
                  }
            return sv;
            }
#endif
      return 0;

      /*QUiLoader loader;
      return loader.createWidget(classid, view());*/
      }

//---------------------------------------------------------
//   MyWebView
//---------------------------------------------------------

MyWebView::MyWebView(QWidget *parent):
   QWebView(parent),
   m_page(this)
      {
      // Set the page of our own PageView class, MyPageView,
      // because only objects of this class will handle
      // object-tags correctly.

      m_page.setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
      QNetworkAccessManager *networkManager = new MyNetworkAccessManager(this);
#ifndef QT_NO_OPENSSL
      connect(networkManager,SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),this, SLOT(ignoreSSLErrors(QNetworkReply*,QList<QSslError>)));
#endif
      m_page.setNetworkAccessManager(networkManager);
      setPage(&m_page);

      //set cookie jar for persistent cookies
      CookieJar* jar = new CookieJar(QString(dataPath + "/cookies.txt"));
      page()->networkAccessManager()->setCookieJar(jar);

      progressBar = 0;
      connect(this, SIGNAL(linkClicked(const QUrl&)), SLOT(link(const QUrl&)));
      }

//---------------------------------------------------------
//   ~MyWebView
//---------------------------------------------------------

MyWebView::~MyWebView()
      {
      disconnect(this, SIGNAL(loadFinished(bool)), this, SLOT(stopBusy(bool)));
      }

#ifndef QT_NO_OPENSSL
/**
Slot connected to the sslErrors signal of QNetworkAccessManager
When this slot is called, call ignoreSslErrors method of QNetworkReply
*/
void MyWebView::ignoreSSLErrors(QNetworkReply *reply, QList<QSslError> sslErrors)
      {      
      foreach (const QSslError &error, sslErrors)
            qDebug("Ignore SSL error: %d %s", error.error(), qPrintable(error.errorString()));
      reply->ignoreSslErrors(sslErrors);
      }
#endif

//---------------------------------------------------------
//   stopBusy
//---------------------------------------------------------

void MyWebView::stopBusy(bool val)
      {
      if (!val) {
            setHtml(QString("<html><head>"
                 "<script type=\"text/javascript\">"
                  "function closePermanently() { mscore.closeWebPanelPermanently(); return false;}"
                  "</script>"
                  "<link rel=\"stylesheet\" href=\"data/webview.css\" type=\"text/css\" /></head>"
            "<body>"
            "<div id=\"content\">"
            "<div id=\"middle\">"
            "  <div class=\"title\" align=\"center\"><h2>%1</h2></div>"
            "  <ul><li>%2</li></ul>"
            "  <div align=\"center\"><a class=\"button\" href=\"#\" onclick=\"return panel.load();\">%3</a></div>"
            "  <div align=\"center\"><a class=\"close\" href=\"#\" onclick=\"return closePermanently();\">%4</div>"
            "</div></div>"
            "</body></html>")
            .arg(tr("Could not<br /> connect"))
            .arg(tr("To connect with the community, <br /> you need to have internet <br /> connection enabled"))
            .arg(tr("Retry"))
            .arg(tr("Close this permanently")),
            QUrl("qrc:/"));
            }
      mscore->hideProgressBar();
      setCursor(Qt::ArrowCursor);
      }

//---------------------------------------------------------
//   setBusy
//---------------------------------------------------------

void MyWebView::setBusy()
      {
      setCursor(Qt::WaitCursor);
      }

//---------------------------------------------------------
//   link
//---------------------------------------------------------

void MyWebView::link(const QUrl& url)
      {
      QString path(url.path());
      QFileInfo fi(path);
      if (fi.suffix() == "mscz" || fi.suffix() == "xml" || fi.suffix() == "mxl")
            mscore->loadFile(url);
      else if(url.host().startsWith("connect."))
            load(QNetworkRequest(url));
      else
            QDesktopServices::openUrl(url);
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize MyWebView::sizeHint() const
      {
      return QSize(300 , 300);
      }

//---------------------------------------------------------
//   WebPageDockWidget
//---------------------------------------------------------

WebPageDockWidget::WebPageDockWidget(MuseScore* /*mscore*/, QWidget* parent)
   : QDockWidget(parent)
      {
      setWindowTitle("MuseScore Connect");
      setFloating(false);
      setFeatures(QDockWidget::DockWidgetClosable);

      setObjectName("webpage");
      setAllowedAreas(Qt::LeftDockWidgetArea);

      web = new MyWebView;
      web->setContextMenuPolicy(Qt::PreventContextMenu);
      QWebFrame* frame = web->webPage()->mainFrame();
      connect(frame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addToJavascript()));

      connect(web, SIGNAL(loadFinished(bool)), web, SLOT(stopBusy(bool)));
      web->setBusy();
      web->load(QNetworkRequest(webUrl()));
      
      setWidget(web);

      //removing every widget from the tabbing order until suport for
      //accessibility is provided
      QList<QWidget*> widgets = this->findChildren<QWidget*>();
      for(int i = 0; i < widgets.size(); i++){
            QWidget* currentWidget = widgets.at(i);
            switch (currentWidget->focusPolicy()){
                 case Qt::TabFocus:
                       currentWidget->setFocusPolicy(Qt::NoFocus);
                       break;
                 case Qt::WheelFocus:
                 case Qt::StrongFocus:
                       currentWidget->setFocusPolicy(Qt::ClickFocus);
                       break;
                 case Qt::ClickFocus:
                 case Qt::NoFocus:
                       break;
                 }
           }
      }

//---------------------------------------------------------
//   addToJavascript
//---------------------------------------------------------

void WebPageDockWidget::addToJavascript()
      {
      QWebFrame* frame = web->webPage()->mainFrame();
      frame->addToJavaScriptWindowObject("panel", this);
      frame->addToJavaScriptWindowObject("mscore", mscore);
      }

QObject* WebPageDockWidget::currentScore() {
  QObject* score = mscore->currentScore();
  return score;
}

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void WebPageDockWidget::load()
      {
      web->setBusy();
      web->load(QNetworkRequest(webUrl()));
      }

bool WebPageDockWidget::saveCurrentScoreOnline(QString action, QVariantMap parameters, QString fileFieldName)
      {
      qDebug("saveCurrentOnline");
      QWebPage * page = web->webPage();
      QNetworkAccessManager* manager = page->networkAccessManager();

      QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

      QMap<QString, QVariant>::const_iterator i = parameters.constBegin();
      while (i != parameters.constEnd()) {
            QHttpPart part;
            part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"%1\"").arg(i.key())));
            part.setBody(i.value().toString().toLatin1());
            multiPart->append(part);
            //qDebug("%s ", qPrintable(i.key()));
            //qDebug("%s ", qPrintable(i.value().toString()));
            ++i;
            }

      if(!fileFieldName.isEmpty()) {
            QDir dir;
            QFile *file = new QFile(dir.tempPath() + "/temp.mscz");
            Score* score = mscore->currentScore();
            if(score) {
                  mscore->saveAs(score, true, file->fileName(), "mscz");
                  }
            else {
                  delete multiPart;
                  return false;
                  }
            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
            filePart.setRawHeader("Content-Transfer-Encoding", "binary");
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"%1\"; filename=\"temp.mscz\"").arg(fileFieldName)));
            file->open(QIODevice::ReadOnly);
            filePart.setBodyDevice(file);
            file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
            multiPart->append(filePart);
            }

      QUrl url(action);
      QNetworkRequest request(url);

      //QNetworkAccessManager manager;
      QNetworkReply *reply = manager->post(request, multiPart);
      multiPart->setParent(reply); // delete the multiPart with the reply
      // here connect signals etc.
      connect(reply, SIGNAL(finished()),
         this, SLOT(saveOnlineFinished()));

      return true;
      }

void WebPageDockWidget::saveOnlineFinished() {
      qDebug("Save online finished");
      // delete file
      QDir dir;
      QFile file(dir.tempPath() + "/temp.mscz");
      file.remove();

      QNetworkReply *reply = (QNetworkReply *)sender();
      // Reading attributes of the reply
      // e.g. the HTTP status code
      int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      QString message = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();

      // no error received?
      if (reply->error() == QNetworkReply::NoError) {
            //deal with redirect
            if (300 <= httpStatus && httpStatus < 400) {
                  qDebug("Redirecting to: %s", qPrintable(reply->url().toString()));
                  web->load(QNetworkRequest(reply->url()));
                  }
            else if (httpStatus == 200) {
                  //Reading bytes form the reply
                  QByteArray bytes = reply->readAll();
                  QString string(bytes);
                  web->setHtml(string);
                  }
            else {
                  qDebug("Unknown HTTP status: %d - %s", httpStatus, qPrintable(message));
                  }
            }
      else { //error received
            qDebug("Save online error %d, HTTP status: %d - %s", reply->error(), httpStatus, qPrintable(message));
            }
      reply->deleteLater();
      }

bool WebPageDockWidget::setCurrentScoreSource(QString /*source*/)
      {
      Score* score = mscore->currentScore();
      if(score) {
            score->metaTags().insert("source", "");
            return true;
            }
      else {
            return false;
            }
      }

//---------------------------------------------------------
//   webUrl
//---------------------------------------------------------
QUrl WebPageDockWidget::webUrl()
    {
    return QUrl(staticUrl);
    }

//---------------------------------------------------------
//   CookieJar
//
//   Once the QNetworkCookieJar object is deleted, all cookies it held will be
//   discarded as well. If you want to save the cookies, you should derive from
//   this class and implement the saving to disk to your own storage format.
//   (From QNetworkCookieJar documentation.)
//---------------------------------------------------------

CookieJar::CookieJar(QString path, QObject *parent)
    : QNetworkCookieJar(parent)
      {
      file = path;
      QFile cookieFile(this->file);

      if (cookieFile.exists() && cookieFile.open(QIODevice::ReadOnly)) {
            QList<QNetworkCookie> list;
            QByteArray line;

            while(!(line = cookieFile.readLine()).isNull()) {
                  list.append(QNetworkCookie::parseCookies(line));
                  }
            setAllCookies(list);
            }
      else {
            if (MScore::debugMode)
                  qDebug() << "Can't open "<< this->file << " to read cookies";
            }
      }

//---------------------------------------------------------
//   ~CookieJar
//---------------------------------------------------------

CookieJar::~CookieJar()
      {
      QList <QNetworkCookie> cookieList;
      cookieList = allCookies();

      QFile file(this->file);

      if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            if (MScore::debugMode)
                  qDebug() << "Can't open "<< this->file << " to save cookies";
            return;
            }

      QTextStream out(&file);
      for(int i = 0 ; i < cookieList.size() ; i++) {
                //get cookie data
                QNetworkCookie cookie = cookieList.at(i);
                if (!cookie.isSessionCookie()) {
                      QByteArray line =  cookieList.at(i).toRawForm(QNetworkCookie::Full);
                      out << line << "\n";
                      }
            }
      file.close();
      }

#if 0
//---------------------------------------------------------
//   WebScoreView
//---------------------------------------------------------

WebScoreView::WebScoreView(QWidget* parent)
   : ScoreView(parent)
      {
      networkManager = 0;
      }

WebScoreView::WebScoreView(const WebScoreView& wsv)
   : ScoreView((QWidget*)(wsv.parent()))
      {
      networkManager = 0;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void WebScoreView::setScore(const QString& url)
      {
      if (!networkManager) {
            networkManager = new QNetworkAccessManager(this);
            connect(networkManager, SIGNAL(finished(QNetworkReply*)),
               SLOT(networkFinished(QNetworkReply*)));
            }
      networkManager->get(QNetworkRequest(QUrl(url)));
      }

//---------------------------------------------------------
//   networkFinished
//---------------------------------------------------------

void WebScoreView::networkFinished(QNetworkReply* reply)
      {
      if (reply->error() != QNetworkReply::NoError) {
            if (MScore::debugMode)
                  qDebug("Error while checking update [%s]", qPrintable(reply->errorString()));
            return;
            }
      QByteArray ha = reply->rawHeader("Content-Disposition");
      QString s(ha);
      QString name;
      QRegExp re(".*filename=\"(.*)\"");
      if (s.isEmpty() || re.indexIn(s) == -1)
            name = "unknown.mscz";
      else
            name = re.cap(1);

      QByteArray data = reply->readAll();
      QString tmpName = QDir::tempPath () + "/"+ name;
      QFile f(tmpName);
      f.open(QIODevice::WriteOnly);
      f.write(data);
      f.close();

      Score* score = mscore->readScore(tmpName);
      if (!score) {
            qDebug("readScore failed");
            return;
            }
      ScoreView::setScore(score);
      update();
      }

#endif
}

