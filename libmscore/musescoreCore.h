//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MUSESCORECORE_H__
#define __MUSESCORECORE_H__

namespace Ms {

class MasterScore;
class Score;
enum class SaveReplacePolicy;

//---------------------------------------------------------
//   class MuseScoreCore
//---------------------------------------------------------

class MuseScoreCore
      {
   protected:
      Score* cs  { 0 };              // current score
      QList<MasterScore*> scoreList;

   public:
      static MuseScoreCore* mscoreCore;
      MuseScoreCore()                    { mscoreCore = this; }
      Score* currentScore() const        { return cs;     }
      void setCurrentScore(Score* score) { cs = score;    }

      virtual bool saveAs(Score*, bool /*saveCopy*/, const QString& /*path*/, const QString& /*ext*/, SaveReplacePolicy* /*replacePolicy*/ = nullptr) { return false; }
      virtual void closeScore(Score*) {}
      virtual void cmd(QAction* /*a*/) {}
      virtual void setCurrentView(int /*tabIdx*/, int /*idx*/) {}

      virtual int appendScore(MasterScore* s)               { scoreList.append(s); return 0;  }
      virtual void endCmd() {}
      virtual Score* openScore(const QString& /*fn*/, bool /*switchTab*/, bool considerInCurrentSession = true) { Q_UNUSED(considerInCurrentSession); return 0;}
      QList<MasterScore*>& scores()                         { return scoreList; }
      virtual void updateInspector() {}
      };

} // namespace Ms
#endif
