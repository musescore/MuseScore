//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __UPLOADSCOREDIALOG_H__
#define __UPLOADSCOREDIALOG_H__

#include "ui_uploadscoredialog.h"

namespace Ms {
class CloudManager;

//---------------------------------------------------------
//   LoginDialog
//---------------------------------------------------------

class UploadScoreDialog : public QDialog, public Ui::UploadScoreDialog
{
    Q_OBJECT

    CloudManager * _loginManager;
    int _nid;
    bool _newScore = true;
    QString _url;

    void showEvent(QShowEvent*) override;

private slots:
    void buttonBoxClicked(QAbstractButton* button);
    void uploadSuccess(const QString& url, const QString& nid, const QString& vid);
    void uploadError(const QString& error);
    void onGetScoreSuccess(const QString& title, const QString& description, bool priv, const QString& license,const QString& tags,
                           const QString& url);
    void onGetScoreError(const QString& error);
    void logout();
    void display();
    void updateScoreData(const QString& nid, bool newScore);
    void updateScoreData();
    void displaySuccess();

private:
    void upload(int nid);
    void clear();
    void showOrHideUploadAudio();

public:
    UploadScoreDialog(CloudManager*);
    void setTitle(const QString& t) { title->setText(t); }
};
}

#endif
