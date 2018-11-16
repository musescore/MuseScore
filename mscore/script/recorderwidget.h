//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SCRIPT_RECORDER_WIDGET_H__
#define __SCRIPT_RECORDER_WIDGET_H__

#include "script.h"

namespace Ui {
      class ScriptRecorderWidget;
      }

namespace Ms {

class MuseScore;

//---------------------------------------------------------
//   ScriptRecorderWidget
//---------------------------------------------------------

class ScriptRecorderWidget : public QDockWidget {
      Q_OBJECT

   private:
      Ui::ScriptRecorderWidget* _ui;
      MuseScore* _mscore;
      ScriptRecorder _recorder;
      QString _scriptDir;

      const QString& scriptDir() const { return _scriptDir; }
      QString scriptFileName() const;
      void updateDirLabel();
      void updateActions();

   private slots:
      void on_changeFolderButton_clicked();
      void on_scriptNameEdit_textEdited();
      void on_recordButton_clicked();
      void on_replayButton_clicked();

   protected:
      void changeEvent(QEvent* e) override;

   public:
      explicit ScriptRecorderWidget(MuseScore* mscore, QWidget* parent = nullptr);
      ScriptRecorderWidget(const ScriptRecorder&) = delete;
      ScriptRecorderWidget& operator=(const ScriptRecorderWidget&) = delete;
      ~ScriptRecorderWidget();

      void recordCommand(const QString& cmd) { _recorder.recordCommand(cmd); }
      void recordPaletteElement(Element* e) { _recorder.recordPaletteElement(e); }
      };

}     // namespace Ms
#endif
