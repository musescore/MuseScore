//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "recognitionproccessdialog.h"

#include <QPushButton>

#include "taskbarprogress.h"

using namespace Ms::Avs;

RecognitionProccessDialog::RecognitionProccessDialog()
    : QProgressDialog()
{
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint& ~Qt::WindowContextHelpButtonHint);

    setFixedSize(300, 140);
    setAutoReset(false);

    _closeBtn = new QPushButton(QObject::tr("Close"));
    setCancelButton(_closeBtn);

    _taskbarProgress = new TaskbarProgress(this);

    setRange(0, 100);
    _taskbarProgress->setRange(0, 100);

    setValue(0);
    _taskbarProgress->setValue(0);

    _updater.setInterval(1000);
    _updater.setSingleShot(false);

    QObject::connect(&_updater, &QTimer::timeout, [this]() {
        update();
    });
}

//---------------------------------------------------------
//   formatStep
//---------------------------------------------------------

QString RecognitionProccessDialog::formatStep(const IAvsOmrRecognizer::Step& step) const
{
    switch (step.type) {
    case IAvsOmrRecognizer::Step::Undefined:
        return QString();
    case IAvsOmrRecognizer::Step::PrepareStart:
        return QObject::tr("Preparing…");
    case IAvsOmrRecognizer::Step::PrepareFinish:
        if (step.success()) {
            return QObject::tr("Successfully prepared");
        }

        return QObject::tr("Failed to prepare")
               + "\n" + step.error.formatedText()
               + "\n" + step.error.supportHint();
    case IAvsOmrRecognizer::Step::ProcessingStart:
        return QObject::tr("Processing…");
    case IAvsOmrRecognizer::Step::ProcessingFinish:
        if (step.success()) {
            return QObject::tr("Successfully processed");
        }

        return QObject::tr("Failed to process")
               + "\n" + step.error.formatedText()
               + "\n" + step.error.supportHint();
    case IAvsOmrRecognizer::Step::LoadStart:
        return QObject::tr("Loading…");
    case IAvsOmrRecognizer::Step::LoadFinish:
        if (step.success()) {
            return QObject::tr("Successfully loaded");
        }

        return QObject::tr("Failed to load")
               + "\n" + step.error.formatedText()
               + "\n" + step.error.supportHint();
    }
    return QString();
}

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void RecognitionProccessDialog::setType(const QString& type)
{
    _type = type;
}

//---------------------------------------------------------
//   show
//---------------------------------------------------------

void RecognitionProccessDialog::show()
{
    setWindowTitle(QString("Optical Music Recognition (%1)").arg(_type));
    _closeBtn->setEnabled(false);

    _updater.start();
    _time.start();

    QProgressDialog::show();
    _taskbarProgress->show();
}

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void RecognitionProccessDialog::update()
{
    //! NOTE Imitation of continuous progress
    if (_lastStep.percent < _lastStep.percentMax) {
        _lastStep.percent++;
    }

    setValue(_lastStep.percent);
    _taskbarProgress->setValue(_lastStep.percent);

    setLabelText(formatStep(_lastStep));
}

//---------------------------------------------------------
//   onStep
//---------------------------------------------------------

void RecognitionProccessDialog::onStep(const IAvsOmrRecognizer::Step& step)
{
    _lastStep = step;
    update();
}

//---------------------------------------------------------
//   onFinished
//---------------------------------------------------------

void RecognitionProccessDialog::onFinished(bool success)
{
    _updater.stop();

    if (success) {
        QProgressDialog::hide();
        _taskbarProgress->hide();
    } else {
        _closeBtn->setEnabled(true);
        _taskbarProgress->stop();
        QProgressDialog::exec();
        _taskbarProgress->hide();
    }
}
