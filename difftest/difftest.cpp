#include "libmscore/mscore.h"
#include "libmscore/score.h"

Q_LOGGING_CATEGORY(undoRedo, "undoRedo", QtCriticalMsg)
static Ms::MScore* mscore;

namespace Ms {
      QString revision;
      QString dataPath;
}

static void initResources() {
      Q_INIT_RESOURCE(mtest);
      Q_INIT_RESOURCE(musescorefonts_MScore);
      Q_INIT_RESOURCE(musescorefonts_Gootville);
      Q_INIT_RESOURCE(musescorefonts_Bravura);
      Q_INIT_RESOURCE(musescorefonts_MuseJazz);
      Q_INIT_RESOURCE(musescorefonts_FreeSerif);
      Q_INIT_RESOURCE(musescorefonts_Free);
      }

using namespace Ms;

static bool loadSaveScore(const QString& src, const QString& dest)
      {
      MasterScore score(mscore->baseStyle());
      Score::FileError err = score.loadMsc(src, false);
      if (err != Score::FileError::FILE_NO_ERROR) {
            qWarning() << "Error while opening " << src;
            return false;
            }

      for (Score* s : score.scoreList())
            s->doLayout();
      score.doLayout();

      QFileInfo fi(dest);
      if (!score.Score::saveFile(fi, false)) {
            qWarning() << "Error while saving " << dest;
            return false;
            }
      return true;
      }


int main(int argc, char** argv)
      {
      if (argc != 3) {
            qCritical() << "Usage: " << argv[0] << " <score1> <score2>";
            return 1;
            }

      QApplication app(argc, argv);

      initResources();
      Ms::MScore::noGui = true;
      mscore = new MScore;
      mscore->init();

      QString src1(argv[1]);
      QString dest1(src1 + "-test1.mscx");
      QString src2(argv[2]);
      QString dest2(src2 + "-test2.mscx");

      if (!loadSaveScore(src1, dest1))
            return 1;
      if (!loadSaveScore(src2, dest2))
            return 1;

      QString cmd("diff");
      QStringList args;
      args.append(dest1);
      args.append(dest2);

      QProcess proc;
      proc.start(cmd, args);
      proc.waitForFinished();

      QTextStream(stdout) << proc.readAll();

      return 0;
      }
