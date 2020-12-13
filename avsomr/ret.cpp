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

#include "ret.h"

#include <QObject>

#include "avslog.h"

using namespace Ms::Avs;

Ret::Ret()
      {
      }

Ret::Ret(Code c)
      : _code(c)
      {
      }

//---------------------------------------------------------
//   valid
//---------------------------------------------------------

bool Ret::valid() const
      {
      return _code > Undefined;
      }

//---------------------------------------------------------
//   success
//---------------------------------------------------------

bool Ret::success() const
      {
      return _code == Ok;
      }

//---------------------------------------------------------
//   code
//---------------------------------------------------------

Ret::Code Ret::code() const
      {
      return _code;
      }

//---------------------------------------------------------
//   formatedText
//---------------------------------------------------------

QString Ret::formatedText() const
      {
      return formatedText(_code);
      }

//---------------------------------------------------------
//   supportHint
//---------------------------------------------------------

QString Ret::supportHint() const
      {
      return supportHint(_code);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Ret::text() const
      {
      return text(_code);
      }

//---------------------------------------------------------
//   formatedText /static/
//---------------------------------------------------------

QString Ret::formatedText(Code c)
      {
      QString str;
      str += "[" + QString::number(static_cast<int>(c)) + "] ";
      str += text(c);
      return str;
      }

//---------------------------------------------------------
//   text /static/
//---------------------------------------------------------

QString Ret::text(Code c)
      {
      switch (c) {
            case Undefined:         return "Undefined error";
            case Ok:                return QObject::tr("OK");
                  // common
            case UnknownError:      return QObject::tr("Unknown error");
            case FailedReadFile:    return QObject::tr("Failed reading file");
            case FailedClearDir:    return QObject::tr("Failed clearing directory");
            case FileNotSupported:  return QObject::tr("File not supported");
                  // network
            case NetworkError:      return QObject::tr("Network error");
            case ServerError:       return QObject::tr("Server error");
                  // local
            case LocalNotInstalled: return QObject::tr("Local OMR engine not installed");
            case LocalInstaling:    return QObject::tr("Local OMR engine installing…");
            case LocalFailedExec:   return QObject::tr("Failed executing local OMR engine");
            case LocalAlreadyBuilding: return QObject::tr("Local OMR engine already building…");

            case AvsOmrFirst:
            case AvsOmrLast: break;
            }
      return QString();
      }

//---------------------------------------------------------
//   supportHint /static/
//---------------------------------------------------------

QString Ret::supportHint(Code c)
      {
      //! TODO Make information better

      static QString PleaseTryAgain = QObject::tr("Please try again");

      switch (c) {
            case Undefined:         return QString();
            case Ok:                return QString();
                  // common
            case UnknownError:      return PleaseTryAgain;
            case FailedReadFile:    return PleaseTryAgain;
            case FailedClearDir:    return PleaseTryAgain;
            case FileNotSupported:  return QObject::tr("Choose another file");
                  // network
            case NetworkError:      return QObject::tr("Check your internet connection");
            case ServerError:       return PleaseTryAgain;
                  // local
            case LocalNotInstalled: return QObject::tr("Try to uncheck and check again to use the local OMR engine");
            case LocalInstaling:    return PleaseTryAgain;
            case LocalFailedExec:   return PleaseTryAgain;
            case LocalAlreadyBuilding: return PleaseTryAgain;

            case AvsOmrFirst:
            case AvsOmrLast: break;
            }
      return QString();
      }
