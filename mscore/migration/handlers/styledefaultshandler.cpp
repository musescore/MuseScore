#include "styledefaultshandler.h"

#include <QFile>

bool StyleDefaultsHandler::handle(Ms::Score* score)
      {
      if (!score)
            return false;

      if (!score->styleB(Ms::Sid::usePre_3_6_defaults) && score->mscVersion() < Ms::MSCVERSION)
            score->style().set(Ms::Sid::usePre_3_6_defaults, true);

      Ms::MStyle baseStyle;

      if (score->styleB(Ms::Sid::usePre_3_6_defaults)) {
            QFile oldDefaultsFile(":/styles/Pre-3.6-defaults.mss");

            if (!oldDefaultsFile.open(QIODevice::ReadOnly))
                return false;

            baseStyle.load(&oldDefaultsFile);

            score->style().applyNewDefaults(baseStyle);
            }

      QVariant lul = baseStyle.value(Ms::Sid::pageTwosided);

      Ms::MScore::setBaseStyle(baseStyle);

      return true;
      }
