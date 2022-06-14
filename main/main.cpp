//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA
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

#include "mscore/musescore.h"

#include "modulessetup.h"

#if (defined (_MSCVER) || defined (_MSC_VER))
#include <vector>
#include <algorithm>
#include <windows.h>
#endif

static void initResources()
      {
#ifdef Q_OS_MAC
      Q_INIT_RESOURCE(musescore);
      Q_INIT_RESOURCE(qml);
      Q_INIT_RESOURCE(musescorefonts_Mac);
      Q_INIT_RESOURCE(shortcut_Mac);
#else
      Q_INIT_RESOURCE(musescore);
      Q_INIT_RESOURCE(qml);
      Q_INIT_RESOURCE(musescorefonts_MScore);
      Q_INIT_RESOURCE(musescorefonts_Gootville);
      Q_INIT_RESOURCE(musescorefonts_Leland);
      Q_INIT_RESOURCE(musescorefonts_Bravura);
      Q_INIT_RESOURCE(musescorefonts_MuseJazz);
      Q_INIT_RESOURCE(musescorefonts_Campania);
      Q_INIT_RESOURCE(musescorefonts_Edwin);
      Q_INIT_RESOURCE(musescorefonts_FreeSerif);
      Q_INIT_RESOURCE(musescorefonts_Free);
      Q_INIT_RESOURCE(musescorefonts_Petaluma);
      Q_INIT_RESOURCE(musescorefonts_FinaleMaestro);
      Q_INIT_RESOURCE(musescorefonts_FinaleBroadway);
      Q_INIT_RESOURCE(shortcut);
#endif
      }

int main(int argc, char** argv)
      {
      // Force the 8-bit text encoding to UTF-8. This is the default encoding on all supported platforms except for MSVC under Windows, which
      // would otherwise default to the local ANSI code page and cause corruption of any non-ANSI Unicode characters in command-line arguments.
      QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

      initResources();

      ModulesSetup::instance()->setup();

#if (defined (_MSCVER) || defined (_MSC_VER))
      // On MSVC under Windows, we need to manually retrieve the command-line arguments and convert them from UTF-16 to UTF-8.
      // This prevents data loss if there are any characters that wouldn't fit in the local ANSI code page.
      int argcUTF16 = 0;
      LPWSTR* argvUTF16 = CommandLineToArgvW(GetCommandLineW(), &argcUTF16);

      std::vector<QByteArray> argvUTF8Q;

      std::for_each(argvUTF16, argvUTF16 + argcUTF16, [&argvUTF8Q](const auto& arg) {
            argvUTF8Q.emplace_back(QString::fromUtf16(reinterpret_cast<const char16_t*>(arg), -1).toUtf8());
            });

      LocalFree(argvUTF16);

      // Ms::runApplication() wants an argv-style array of raw pointers to the arguments, so let's create a vector of them.
      std::vector<char*> argvUTF8;

      for (auto& arg : argvUTF8Q)
            argvUTF8.push_back(arg.data());

      // Don't use the arguments passed to main(), because they're in the local ANSI code page.
      Q_UNUSED(argc);
      Q_UNUSED(argv);

      int argcFinal = argcUTF16;
      char** argvFinal = argvUTF8.data();
#else
      int argcFinal = argc;
      char** argvFinal = argv;
#endif

      return Ms::runApplication(argcFinal, argvFinal);
      }
