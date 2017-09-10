//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "helpBrowser.h"
#include "icons.h"
#include "help.h"
#include "musescore.h"
#include "preferences.h"

namespace Ms {

//    manual css style sheets:

static const char* css = "body {"
"	font-family:	Arial, Helvetica, FreeSans, \"DejaVu Sans\", sans-serif;"
"	font-size:		11pt;"
"	margin:		15px;"
"      }"
"h2, h3 {"
"	font-size:		24px;"
"	padding:		6px 0 6px 48px;"
"	/*background:		#dcdcdc left center no-repeat url('mscore.png');*/"
"	background-size:32px 32px;"
"      }"
"h4 {"
"	margin:  10px 0 8px 16px;"
"      }"
"table {"
"	border-collapse:collapse;"
"      }"
"td {"
"	padding:		2px 12px 2px 0;"
"	vertical-align:	text-top;"
"      }"
".class-description {"
"	margin:			0 0 16px 0;"
"      }"
".class-inherit {"
"	margin:			0 0 16px 0;"
"	font-size:		0.8em;"
"      }"
".method {"
"	font-family:	\"Lucida Console\", Monaco, \"DejaVu Sans Mono\", monospace;"
"	font-size:		0.8em;"
"     }"
".prop-odd {"
"	background:		#dcdcdc;"
"      }"
".prop-name {"
"	font-weight:	bold;"
"      }"
".prop-type {"
"	font-style:		italic"
"      }"
".prop-desc {"
"      }"
".footer {"
"	margin-top:		24px;"
"	background:		#dcdcdc;"
"	padding:		16px;"
"	text-align:		center;"
"	font-size:		0.8em;"
"     }";

static const char* cssDark = "body {"
"	font-family:	Arial, Helvetica, FreeSans, \"DejaVu Sans\", sans-serif;"
"	font-size:		11pt;"
"	margin-left:	15px;"
"     }"
"h1, h2, h3 {"
"      }"
"h4 {"
"	margin-left:      0px;"
"      }"
"table {"
"	border-collapse:collapse;"
"      }"
"td {"
"	padding:		2px 12px 2px 0;"
"	vertical-align:	text-top;"
"      }"
".class-description {"
"	margin-top:		16px;"
"      }"
".class-inherit {"
"	margin-top:		16px;"
"	font-size:		0.8em;"
"      }"
".method {"
"	font-family:	\"Lucida Console\", Monaco, \"DejaVu Sans Mono\", monospace;"
"	font-size:		0.7em;"
"      }"
".prop-odd {"
"	background:		#989898;"
"      }"
".prop-name {"
"	font-weight:	bold;"
"      }"
".prop-type {"
"	font-style:		italic;"
"      }"
".prop-desc {"
"      }";


//---------------------------------------------------------
//   HelpBrowser
//---------------------------------------------------------

HelpBrowser::HelpBrowser(QWidget* parent)
   : QWidget(parent)
      {
      view    = new HelpView(mscore->helpEngine());
      toolbar = new QWidget;
      toolbar->setSizePolicy(QSizePolicy::Expanding,
      QSizePolicy::Fixed);
      QVBoxLayout* l = new QVBoxLayout;
      l->addWidget(toolbar);
      l->addWidget(view);
      setLayout(l);
      QHBoxLayout* bl = new QHBoxLayout;

      QToolButton* home = new QToolButton;
      home->setIcon(QIcon(*icons[int(Icons::goHome_ICON)]));
      bl->addWidget(home);
      connect(home, SIGNAL(clicked()), SLOT(homeClicked()));

      bl->addStretch(2);

      QToolButton* previous = new QToolButton;
      previous->setIcon(QIcon(*icons[int(Icons::goPrevious_ICON)]));
      bl->addWidget(previous);
      connect(previous, SIGNAL(clicked()), view, SLOT(backward()));
      connect(view, SIGNAL(backwardAvailable(bool)), previous, SLOT(setEnabled(bool)));

      QToolButton* next = new QToolButton;
      next->setIcon(QIcon(*icons[int(Icons::goNext_ICON)]));
      bl->addWidget(next);
      connect(next, SIGNAL(clicked()), view, SLOT(forward()));
      connect(view, SIGNAL(forwardAvailable(bool)), next, SLOT(setEnabled(bool)));

      bl->addStretch(10);

      view->document()->setDefaultStyleSheet(
            preferences.globalStyle == MuseScoreStyleType::DARK ? cssDark : css);
      view->setOpenExternalLinks(true);

      toolbar->setLayout(bl);
      }

//---------------------------------------------------------
//   setContent
//---------------------------------------------------------

void HelpBrowser::setContent(const QString& path)
      {
      homePath = QUrl::fromLocalFile(path);
      view->setSource(homePath);
      }

void HelpBrowser::setContent(const QUrl& url)
      {
      homePath = url;
      view->setSource(url);
      }

//---------------------------------------------------------
//   homeClicked
//---------------------------------------------------------

void HelpBrowser::homeClicked()
      {
      view->setSource(homePath);
      }

//---------------------------------------------------------
//   loadResource
//---------------------------------------------------------

QVariant HelpView::loadResource(int type, const QUrl& name)
      {
      if (name.scheme() == "qthelp")
            return QVariant(helpEngine->fileData(name));
#if 0
      if (preferences.globalStyle == MuseScoreStyleType::DARK) {
            QFileInfo fi(name.path());
            if (fi.fileName() == "manual.css") {
                  QUrl url(QString("file://%1/manual-dark.css").arg(fi.absolutePath()));
//                  qDebug("exchange css <%s>\n", qPrintable(url.toString()));
                  return QTextBrowser::loadResource(type, url);
                  }
            }
#endif
      return QTextBrowser::loadResource(type, name);
      }
}

