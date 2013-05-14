#ifndef __MICONENGINE_H__
#define __MICONENGINE_H__

#include <QtGui/qiconengine.h>
#include <QtCore/qshareddata.h>

class MIconEnginePrivate;

//---------------------------------------------------------
//   MIconEngine
//---------------------------------------------------------

class MIconEngine : public QIconEngine
      {
   public:
      MIconEngine();
      MIconEngine(const MIconEngine &other);
      ~MIconEngine();
      void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state);
      QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state);
      QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);

      void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state);
      void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state);

      QString key() const;
      QIconEngine *clone() const;
      bool read(QDataStream &in) { return false; }
      bool write(QDataStream &out) const { return false; }

   private:
      QSharedDataPointer<MIconEnginePrivate> d;
      };

#endif
