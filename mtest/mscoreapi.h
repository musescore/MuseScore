     //api for openscore
    #ifndef _MSCOREAPI_H_
    #define _MSCOREAPI_H_

    #include "musescore.h"
    #include "libmscore/score.h"
    #include "omr/omr.h"
    #include "omr/omrpage.h"
    #include "libmscore/part.h"
    #include "libmscore/staff.h"
    #include "libmscore/page.h"
    #include "libmscore/measure.h"
    #include "libmscore/durationtype.h"
    #include "libmscore/segment.h"
    #include "libmscore/clef.h"
    #include "libmscore/rest.h"
    #include "libmscore/system.h"
    #include "libmscore/excerpt.h"
    #include "libmscore/stafftype.h"
    #include "scoreview.h"

    class Openmscore: public QObject {
                  Q_OBJECT
          public:
                  Openmscore(QWidget* parent = 0);
                  ~Openmscore();
                  Q_INVOKABLE Score* currentScore() const         { return mscore->currentScore(); }
                  Q_INVOKABLE void newFile()                                              { mscore->newFile(); }
                  Q_INVOKABLE QString getLocaleISOCode()                  { return mscore->getLocaleISOCode();}
                  Q_INVOKABLE bool openPDF(QString path);
                  //Q_INVOKABLE void resetScore(); ==> needs fixing

          };

    class Openscore: public QObject {
                  Q_OBJECT
          public:
                  Openscore();
                  ~Openscore();
                  Q_INVOKABLE QString getName();
                  Q_INVOKABLE void setName(QString name);
                  Q_INVOKABLE int selectMeasure(int id, int colorOn);
                  Q_INVOKABLE int numberOfTU();
          };

    #endif

