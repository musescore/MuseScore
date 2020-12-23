#include "styledefaultshandler.h"

#include <QFile>
#include "libmscore/excerpt.h"
#include "libmscore/score.h"

static const int LEGACY_MSC_VERSION_V3 = 301;
static const int LEGACY_MSC_VERSION_V2 = 206;
static const int LEGACY_MSC_VERSION_V1 = 114;

bool StyleDefaultsHandler::handle(Ms::Score* score)
      {
      if (!score)
            return false;

      if (!score->styleB(Ms::Sid::usePre_3_6_defaults) && score->mscVersion() < Ms::MSCVERSION) {
            score->style().set(Ms::Sid::usePre_3_6_defaults, true);
            score->style().set(Ms::Sid::defaultsVersion, resolveDefaultsVersion(score->mscVersion()));
            }

      if (score->styleB(Ms::Sid::usePre_3_6_defaults))
            applyStyleDefaults(score);

      return true;
      }

int StyleDefaultsHandler::resolveDefaultsVersion(const int mscVersion) const
      {
      if (mscVersion > LEGACY_MSC_VERSION_V2 && mscVersion < Ms::MSCVERSION)
            return LEGACY_MSC_VERSION_V3;

      if (mscVersion > LEGACY_MSC_VERSION_V1 && mscVersion <= LEGACY_MSC_VERSION_V2)
            return LEGACY_MSC_VERSION_V2;

      if (mscVersion <= LEGACY_MSC_VERSION_V1)
            return LEGACY_MSC_VERSION_V1;

      return Ms::MSCVERSION;
      }

void StyleDefaultsHandler::applyStyleDefaults(Ms::Score* score) const
      {
      int defaultsVersion = score->styleI(Ms::Sid::defaultsVersion);

      Ms::MStyle baseStyle = *Ms::MStyle::resolveStyleDefaults(defaultsVersion);

      score->style().applyNewDefaults(baseStyle, defaultsVersion);

      for (Ms::Excerpt* excerpt : score->excerpts()) {
            excerpt->partScore()->style().applyNewDefaults(baseStyle, defaultsVersion);
            excerpt->partScore()->styleChanged();
            }
      }
