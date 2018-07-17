#ifndef TOURHANDLER_H
#define TOURHANDLER_H

#include "libmscore/xml.h"

namespace Ms {

//---------------------------------------------------------
//   TourMessage
//---------------------------------------------------------

struct TourMessage {
      QString message;
      QList<QString> widgetNames;
      TourMessage(QString m, QList<QString> w) : message(m), widgetNames(w) {}
      };

//---------------------------------------------------------
//   Tour
//---------------------------------------------------------

class Tour
      {
      QList<TourMessage> _messages;
      QMultiMap<QString, QWidget*> nameToWidget;
      QString _tourName;
      bool _completed = false;

   public:
      Tour(QString name) { _tourName = name; }

      void addMessage(QString m, QList<QString> w) { TourMessage message(m, w);
                                                     _messages.append(message); }
      QList<TourMessage> messages() { return _messages; }

      QList<QWidget*> getWidgetsByName(QString n)  { return nameToWidget.values(n);   }
      void addNameAndWidget(QString n, QWidget* w) { nameToWidget.insert(n, w);       }
      void clearWidgets()                          { nameToWidget.clear();            }
      bool hasNameForWidget(QString n)             { return nameToWidget.contains(n); }

      void setTourName(QString n) { _tourName = n;     }
      QString tourName()          { return _tourName;  }

      void setCompleted(bool c)   { _completed = c;    }
      bool completed()            { return _completed; }
      };

//---------------------------------------------------------
//   OverlayWidget
//---------------------------------------------------------

class OverlayWidget : public QWidget
      {
      void newParent();
      bool eventFilter(QObject * obj, QEvent * ev);
      virtual bool event(QEvent* ev);
      virtual void paintEvent(QPaintEvent *);

      QList<QWidget*> widgets;

   public:
      OverlayWidget(QList<QWidget*> widgetList, QWidget* parent = 0);
      };

//---------------------------------------------------------
//   TourHandler
//---------------------------------------------------------

class TourHandler : public QObject
      {
      Q_OBJECT

      QMap<QObject*, QMap<QEvent::Type, QString>*> eventHandler;

      void loadTour(XmlReader& tourXml);

      static void displayTour(Tour* tour);
      static void positionMessage(QList<QWidget*> widgets, QMessageBox* mbox);
      static QHash<QString, Tour*> allTours;
      static QHash<QString, Tour*> shortcutToTour;
      static QMap<QString, QMap<QString, QString>*> eventNameLookup;
      static QList<QWidget*> getWidgetsByNames(Tour* tour, QList<QString> names);

   public:
      TourHandler(QObject* parent) : QObject(parent) {}
      void loadTours();
      void readCompletedTours();
      void writeCompletedTours();

      bool eventFilter(QObject *obj, QEvent* event);
      static void startTour(QString tourName);
      void attachTour(QObject* obj, QEvent::Type eventType, QString tourName);

      static void addWidgetToTour(QString tourName, QWidget* widget, QString widgetName);
      static void clearWidgetsFromTour(QString tourName);

      static QList<QString> allTourShortcuts() { return shortcutToTour.keys(); }
      };

}

#endif // TOURHANDLER_H
