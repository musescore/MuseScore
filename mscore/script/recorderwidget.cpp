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

#include "recorderwidget.h"

#include "ui_script_recorder.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   ScriptRecorderWidget
//---------------------------------------------------------

ScriptRecorderWidget::ScriptRecorderWidget(MuseScore* mscore, QWidget* parent)
   : QDockWidget(parent), _ui(new Ui::ScriptRecorderWidget), _mscore(mscore),
   _recorder(mscore), _scriptDir(QDir::homePath())
      {
      _ui->setupUi(this);
      updateDirLabel();
      updateActions();
      }

//---------------------------------------------------------
//   ~ScriptRecorderWidget
//---------------------------------------------------------

ScriptRecorderWidget::~ScriptRecorderWidget()
      {
      delete _ui;
      }

//---------------------------------------------------------
//   ScriptRecorderWidget::changeEvent
//---------------------------------------------------------

void ScriptRecorderWidget::changeEvent(QEvent* e)
      {
      QDockWidget::changeEvent(e);
      switch(e->type()) {
            case QEvent::LanguageChange:
                  _ui->retranslateUi(this);
                  break;
            default:
                  break;
            }
      }

//---------------------------------------------------------
//   ScriptRecorderWidget::scriptFileName
//---------------------------------------------------------

QString ScriptRecorderWidget::scriptFileName() const
      {
      QDir dir(scriptDir());
      return dir.filePath(_ui->scriptNameEdit->text() + ".script");
      }

//---------------------------------------------------------
//   ScriptRecorderWidget::updateDirLabel
//---------------------------------------------------------

void ScriptRecorderWidget::updateDirLabel()
      {
      QFontMetrics metrics(_ui->folderLabel->font());
      _ui->folderLabel->setText(metrics.elidedText(_scriptDir, Qt::ElideLeft, _ui->folderLabel->width()));
      _ui->folderLabel->setToolTip(_scriptDir);
      }
//---------------------------------------------------------
//   ScriptRecorderWidget::updateActions
//---------------------------------------------------------

void ScriptRecorderWidget::updateActions()
      {
      QDir dir(scriptDir());
      QFileInfo script(scriptFileName());
      _ui->recordButton->setEnabled(dir.exists() && !script.baseName().isEmpty() && !script.exists());
      _ui->replayButton->setEnabled(script.isFile());
      }

//---------------------------------------------------------
//   ScriptRecorderWidget::on_changeFolderButton_clicked
//---------------------------------------------------------

void ScriptRecorderWidget::on_changeFolderButton_clicked()
      {
      QString newDir = QFileDialog::getExistingDirectory(this, "Open scripts directory");
      if (!newDir.isEmpty())
            _scriptDir = newDir;
      updateDirLabel();
      updateActions();
      }

//---------------------------------------------------------
//   ScriptRecorderWidget::on_scriptNameEdit_textEdited
//---------------------------------------------------------

void ScriptRecorderWidget::on_scriptNameEdit_textEdited()
      {
      updateActions();
      }

//---------------------------------------------------------
//   ScriptRecorderWidget::on_recordButton_clicked
//---------------------------------------------------------

void ScriptRecorderWidget::on_recordButton_clicked()
      {
      const bool startRecord = !_recorder.isRecording();
      _ui->changeFolderButton->setEnabled(!startRecord);
      _ui->scriptNameEdit->setEnabled(!startRecord);
      _ui->replayButton->setEnabled(false);
      _ui->recordButton->setText(startRecord ? "Stop recording" : "Start recording");

      if (startRecord) {
            _recorder.setFile(scriptFileName());
            _recorder.context().setCwd(scriptDir());
            _recorder.setRecording(true);
            _recorder.recordInitState();
            }
      else {
            QFileInfo scriptInfo(scriptFileName());
            QString scoreName(scriptInfo.baseName() + ".mscx");
            _recorder.recordScoreTest(scoreName);
            _recorder.close();
            }

      if (!_recorder.isRecording())
            updateActions();
      }

//---------------------------------------------------------
//   ScriptRecorderWidget::on_replayButton_clicked
//---------------------------------------------------------

void ScriptRecorderWidget::on_replayButton_clicked()
      {
      auto script = Script::fromFile(scriptFileName());
      if (!script)
            return;
      ScriptContext ctx(_mscore);
      ctx.setCwd(scriptDir());
      ctx.setStopOnError(false);
      script->execute(ctx);
      }
}
