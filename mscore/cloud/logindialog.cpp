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

#include "musescore.h"
#include "logindialog.h"
#include "mu4/cloud/internal/cloudmanager.h"

namespace Ms {
//---------------------------------------------------------
//   LoginDialog
//---------------------------------------------------------

LoginDialog::LoginDialog(CloudManager* loginManager)
    : QDialog(0)
{
    setObjectName("LoginDialog");
    setupUi(this);
    setStyleSheet("QLineEdit { "
                  "padding: 8px 8px;"
                  "}");

    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(buttonBox,   SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
    _loginManager = loginManager;
    createAccountLabel->setText("<a href=\"https://musescore.com/user/register\">" + tr("Create an account") + "</a>");
    forgotPasswordLabel->setText("<a href=\"https://musescore.com/user/password\">" + tr("Forgot password?") + "</a>");
    connect(_loginManager, SIGNAL(loginSuccess()), this, SLOT(onLoginSuccess()));
    connect(_loginManager, SIGNAL(loginError(const QString&)), this, SLOT(onLoginError(const QString&)));

    MuseScore::restoreGeometry(this);
}

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void LoginDialog::buttonBoxClicked(QAbstractButton* button)
{
    QDialogButtonBox::StandardButton sb = buttonBox->standardButton(button);
    if (sb == QDialogButtonBox::Ok) {
        if (usernameEdit->text().trimmed().isEmpty() || passwordEdit->text().trimmed().isEmpty()) {
            QMessageBox::critical(this, tr("Login error"), tr("Please fill in your username and password"));
        }
        login();
    } else {
        setVisible(false);
    }
}

//---------------------------------------------------------
//   login
//---------------------------------------------------------

void LoginDialog::login()
{
    _loginManager->login(usernameEdit->text(), passwordEdit->text());
}

//---------------------------------------------------------
//   onLoginError
//---------------------------------------------------------

void LoginDialog::onLoginError(const QString& error)
{
    QMessageBox::critical(this, tr("Login error"), error);
}

//---------------------------------------------------------
//   onLoginSuccess
//---------------------------------------------------------

void LoginDialog::onLoginSuccess()
{
    emit loginSuccessful();
    setVisible(false);
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void LoginDialog::hideEvent(QHideEvent* event)
{
    MuseScore::saveGeometry(this);
    QDialog::hideEvent(event);
}
}
