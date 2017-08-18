//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Nikolaos Hatzopoulos (nickhatz@csu.fullerton.edu)
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

#include "crash_handler.h"

namespace Breakpad {

      /************************************************************************/
      /* CrashHandlerPrivate                                                  */
      /************************************************************************/
      class CrashHandlerPrivate {
      public:
            CrashHandlerPrivate()
                  {
                  pHandler = NULL;
                  }

            ~CrashHandlerPrivate()
                  {
                  delete pHandler;
                  }

            void InitCrashHandler(wstring dumpPath);
            static google_breakpad::ExceptionHandler* pHandler;
            static bool bReportCrashesToSystem;
      };

      google_breakpad::ExceptionHandler* CrashHandlerPrivate::pHandler = NULL;
      bool CrashHandlerPrivate::bReportCrashesToSystem = false;

      std::map<string,string> crashTable;
      HANDLE ghMutex;
      wstring crash_reporter_path;

      /************************************************************************/
      /* AnnotateCrashReport :                                                */
      /* Annotate a key and a parameter to the crash report                   */
      /************************************************************************/

      int AnnotateCrashReport(string aKey, string aData)
            {
            ghMutex = CreateMutex(
                              NULL,              // default security attributes
                              FALSE,             // initially not owned
                              NULL);             // unnamed mutex
            //crashTable.insert(aKey,aData);
            crashTable[aKey] = aData;
            CloseHandle(ghMutex);
            return 0;
            }

      /************************************************************************/
      /* PrintMyCrahsReport :                                                 */
      /* Prints the Crash Report from the annotated keys and parameters       */
      /************************************************************************/

      int PrintMyCrashReport()
            {
            for (std::map<string,string>::iterator it=crashTable.begin(); it!=crashTable.end(); ++it)
                  std::cout << it->first << " " << it->second << std::endl;

            return 0;
            }

      /************************************************************************/
      /* writeMyCrashReport :                                                 */
      /* exports crash report to a text file with as: key,parameter           */
      /************************************************************************/

      void writeMyCrashReport(wstring mypath)
            {
            string my_strpath = std::string(mypath.begin(), mypath.end());
            std::ofstream myfile;
            myfile.open(my_strpath);
            for (std::map<string,string>::iterator it=crashTable.begin(); it!=crashTable.end(); ++it)
                  myfile << it->first << "," << it->second << std::endl;

            myfile.close();
            }

      /************************************************************************/
      /* str2wstr : converts a string to wstring                              */
      /************************************************************************/

      wstring str2wstr(string mystr)
            {
            wstring res(mystr.begin(), mystr.end());
            return res;
            }

      /************************************************************************/
      /* wstr2str : Converts a string to wstring                              */
      /************************************************************************/

      string wstr2str(wstring mystr)
            {
            string res(mystr.begin(), mystr.end());
            return res;
            }

      /************************************************************************/
      /* get_musescore_path : Locates the directory path of MuseScore.exe     */
      /************************************************************************/

      string get_musescore_path()
            {
            wchar_t buffer[MAX_PATH];
            GetModuleFileName(NULL, buffer, MAX_PATH);
            string mscore_path = wstr2str(wstring(buffer));

            int i;
            int path_n;

            for(i=mscore_path.size(); i>=0; i--) {
                  if(mscore_path[i]=='\\') {
                        path_n = i;
                        break;
                  }
            }

            string res;
            for(i=0; i<path_n; i++)
                  res += mscore_path[i];

            return res;
            }

      /************************************************************************/
      /* file_exists : Returns True if a file exists                          */
      /************************************************************************/

      bool file_exists(string name)
            {
            ifstream f(name.c_str());
            return f.good();
            }

      /************************************************************************/
      /* get_crash_reporter_path :                                            */
      /* Locates the directory path of the musescore_crashreproter.exe        */
      /************************************************************************/

      string get_crash_reporter_path()
            {
            string musescore_path = get_musescore_path();
            string res = musescore_path+"\\musescore_crashreporter.exe";
            string res_empty = "";
            if (file_exists(res))
                  return res;
            res = musescore_path + "\\..\\tools\\musescore_crashreporter\\musescore_crashreporter.exe";
            if (file_exists(res))
                  return res;

            return res_empty;

            }

      /************************************************************************/
      /* replaceChar :                                                        */
      /* from an input string str replaces character ch1 with character ch2   */
      /* and retrurn the result into a new string                             */
      /************************************************************************/

      string replaceChar(string str, char ch1, char ch2)
            {
            for (unsigned int i=0; i<str.length(); ++i) {
                  if (str[i]==ch1)
                        str[i] = ch2;
            }

            return str;
            }

      /************************************************************************/
      /* DumpCallback : This is the function that is called after a crash     */
      /************************************************************************/

#if defined(Q_OS_WIN32)
      bool DumpCallback(const wchar_t* _dump_dir, const wchar_t* _minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool success)
#endif
            {
            Q_UNUSED(context);
      #if defined(Q_OS_WIN32)
            Q_UNUSED(_dump_dir);
            Q_UNUSED(_minidump_id);
            Q_UNUSED(assertion);
            Q_UNUSED(exinfo);
      #endif

            qDebug("BreakpadQt crash");

      #if defined(Q_OS_WIN32)
            wstring minidump_path;
            wstring metadata_path;
            wstring program_path;

            // Minidump Path
            minidump_path =  wstring(_dump_dir) + L"/" + wstring(_minidump_id) + L".dmp";
            metadata_path =  wstring(_dump_dir) + L"/" + wstring(_minidump_id) + L".txt";

            qDebug("minidump path: %s\n", std::string(minidump_path.begin(),minidump_path.end()).c_str());

            writeMyCrashReport(metadata_path);

            //program_path = L"C:/Users/nickhatz/MuseScore/build.release/thirdparty/breakpad/musescore_crashreporter.exe";
            launcher(crash_reporter_path, minidump_path, metadata_path);

      #endif

            /*
            NO STACK USE, NO HEAP USE THERE !!!
            Creating QString's, using qDebug, etc. - everything is crash-unfriendly.
            */
            return CrashHandlerPrivate::bReportCrashesToSystem ? success : true;
            }

      /************************************************************************/
      /* InitCrashHandler : Here we Initialize Breakpad ExceptionHandler      */
      /************************************************************************/

      void CrashHandlerPrivate::InitCrashHandler(wstring dumpPath)
            {
            if (pHandler!=NULL)
                  return;
      #if defined(Q_OS_WIN32)
            //std::wstring pathAsStr = string2wstring(dumpPath);
            pHandler = new google_breakpad::ExceptionHandler(
                  dumpPath,
                  /*FilterCallback*/ 0,
                  DumpCallback,
                  /*context*/
                  0,
                  true
            );
      #endif

            }

      /************************************************************************/
      /* CrashHandler                                                         */
      /************************************************************************/

      CrashHandler* CrashHandler::instance()
            {
            static CrashHandler globalHandler;
            return &globalHandler;
            }

      CrashHandler::CrashHandler()
            {
            d = new CrashHandlerPrivate();
            }

      CrashHandler::~CrashHandler()
            {
            delete d;
            }

      void CrashHandler::setReportCrashesToSystem(bool report)
            {
            d->bReportCrashesToSystem = report;
            }

      bool CrashHandler::writeMinidump()
            {
            bool res = d->pHandler->WriteMinidump();
            if (res) {
                  qDebug("BreakpadQt: writeMinidump() success.");
            }
            else {
                  qWarning("BreakpadQt: writeMinidump() failed.");
            }

            return res;
            }

      /************************************************************************/
      /* CrashHandler : Initialize Breakpad and Crashreproter                 */
      /************************************************************************/

      void CrashHandler::Init(wstring reportPath)
            {
            d->InitCrashHandler(reportPath);
            crash_reporter_path = str2wstr(replaceChar(get_crash_reporter_path(), '\\', '/'));
            qDebug("Crash Reporter Path: %s", crash_reporter_path.c_str());
            }

      /************************************************************************/
      /* launcher : launches musescore_crashreporter.exe with arguments       */
      /* the minidump location and the metadata txt file                      */
      /************************************************************************/

      bool launcher(wstring program, wstring minidump_path, wstring metadata_path)
            {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            wstring mycmd;

            //example command: c:\...\crashReporter.exe minidump_path
            mycmd = program+L" "+minidump_path+L" "+metadata_path;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));


            std::wcout << "CrashReporter: " << program << "Dmppath: " << minidump_path;

            // Open a Windows Process equivelant to fork() for Linux
            if(!CreateProcess(NULL,   // No module name (use command line)
                              (WCHAR *)mycmd.c_str(),          // Command line
                              NULL,           // Process handle not inheritable
                              NULL,           // Thread handle not inheritable
                              FALSE,          // Set handle inheritance to FALSE
                              0,              // No creation flags
                              NULL,           // Use parent's environment block
                              NULL,           // Use parent's starting directory
                              &si,            // Pointer to STARTUPINFO structure
                              &pi)            // Pointer to PROCESS_INFORMATION structure
            ) {
                printf("CreateProcess failed (%lu).\n", GetLastError());
                return true;
            }

            // Wait until child process exits.
            //WaitForSingleObject( pi.hProcess, INFINITE );

            // Close process and thread handles.
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            Q_UNUSED(program);
            return false;

            }

}
