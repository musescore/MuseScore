//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "openfilelocation.h"

namespace Ms {
//---------------------------------------------------------
//   platformText
///   Returns the platform specific text for opening the file.
///   There is no official standard, but there seems to be a general agreement
///
///   Windows: "Show in Explorer"
///   MacOs:   "Reveal in Finder"
///   Else:    "Open Containing Folder"
//---------------------------------------------------------

const QString OpenFileLocation::platformText()
{
#if defined(Q_OS_WIN)
    return tr("Show in Explorer");
#elif defined(Q_OS_MAC)
    return tr("Reveal in Finder");
#else
    return tr("Open Containing Folder");
#endif
}

//---------------------------------------------------------
//   openFileLocation
///   reveals the file in explorer (Windows) / finder (MacOs) or the default file manager in other cases.
///   Note that opening the default file manager doesn't show the file, but simply opens the containing folder.
///   Note also that all platforms will fallback to opening the containing folder if they can't show the file.
///
///   Returns true on success, false on failure.
///   See MetaEditDialog::openFileLocation example usage
//---------------------------------------------------------

bool OpenFileLocation::openFileLocation(const QString& path)
{
    const QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return false;
    }
    QStringList args;
#if defined(Q_OS_WIN)
    if (!fileInfo.isDir()) {
        args += QLatin1String("/select,");
    }
    args += QDir::toNativeSeparators(fileInfo.canonicalFilePath());
    if (QProcess::startDetached("explorer.exe", args)) {
        return true;
    }
#elif defined(Q_OS_MAC)
    args << QLatin1String("-e")
         << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
        .arg(fileInfo.canonicalFilePath());
    QProcess::execute(QLatin1String("/usr/bin/osascript"), args);
    args.clear();
    args << QLatin1String("-e")
         << QLatin1String("tell application \"Finder\" to activate");
    if (!QProcess::execute("/usr/bin/osascript", args)) {
        return true;
    }
#endif // not #else so that the following can be used as a fallback.
    return QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.isDir() ? path : fileInfo.path()));
}
} // namespace Ms
