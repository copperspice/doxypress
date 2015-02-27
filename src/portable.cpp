/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch. 
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <qglobal.h>
#include <QDateTime>
#include <QDir>
#include <QFSFileEngine>

#include <stdlib.h>
#include <ctype.h>

#if HAVE_WINDOWS_H
#undef UNICODE
#define _WIN32_DCOM
#include <windows.h>

#else
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

extern char **environ;

#endif

#include <message.h>
#include <portable.h>

static double g_sysElapsedTime;
static QTime  g_time;

int portable_system(const QString &command, const QString &args, bool commandHasConsole)
{
   QString fullCmd = command.trimmed();

   if (fullCmd.isEmpty()) {
      return 1;
   }

   if (fullCmd.at(0) != '"' && fullCmd.contains(' ')) {
      // add quotes around command as it contains spaces and is not quoted already
      fullCmd = "\"" + fullCmd + "\"";
   }

   fullCmd += " " + args;


#if HAVE_EXECVE
  
   int pid;
   int status = 0;

   pid = fork();

   if (pid == -1) {
      perror("fork error");
      return -1;
   }

   if (pid == 0) {
      QByteArray tempCmd = fullCmd.toUtf8();

      const char *argv[4];

      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = tempCmd.constData();
      argv[3] = 0;

      execve("/bin/sh", (char *const *)argv, environ);
      exit(127);
   }

   while (true) {
      if (waitpid(pid, &status, 0) == -1) {
         if (errno != EINTR) {
            return -1;
         }

      } else {
         if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
         } else {
            return status;
         }
      }
   }


#else 
   // Windows

   if (commandHasConsole) {
      return system(fullCmd);

   } else {               
      CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

      QString commandw = QString::fromUtf8( command );
      QString argsw    = QString::fromUtf8( args );

      // gswin32 is a GUI api which will pop up a window and run
      // asynchronously. To prevent both, we use ShellExecuteEx and
      // WaitForSingleObject (thanks to Robert Golias for the code)

      SHELLEXECUTEINFOW sInfo = {
         sizeof(SHELLEXECUTEINFOW),   
         SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI,     // wait till the process is  done, do not display msg box if there is an error                                                       
         NULL,                       /* window handle */
         NULL,                       /* action to perform: open */
         (LPCWSTR)commandw.utf16(),  /* file to execute */
         (LPCWSTR)argsw.utf16(),     /* argument list */
         NULL,                       /* use current working dir */
         SW_HIDE,                    /* minimize on start-up */
         0,                          /* application instance handle */
         NULL,                       /* ignored: id list */
         NULL,                       /* ignored: class name */
         NULL,                       /* ignored: key class */
         0,                          /* ignored: hot key */
         NULL,                       /* ignored: icon */
         NULL                        /* resulting application handle */
      };

      if (! ShellExecuteExW(&sInfo)) {
         return -1;

      } else if (sInfo.hProcess) {  
         // executable was launched, wait for it to finish 
         WaitForSingleObject(sInfo.hProcess, INFINITE);

         // get process exit code 
         DWORD exitCode;
         if (! GetExitCodeProcess(sInfo.hProcess, &exitCode)) {
            exitCode = -1;
         }

         CloseHandle(sInfo.hProcess);
         return exitCode;
      }
   }
#endif

}

uint portable_pid()
{
   uint pid;

#if HAVE_GETPID
   pid = (uint)getpid();

#else
   pid = (uint)GetCurrentProcessId();

#endif

   return pid;
}

static char **last_environ;

void portable_setenv(const char *name, const QString &valueX)
{  
   QByteArray value = valueX.toUtf8();

#if HAVE_WINDOWS_H
   SetEnvironmentVariable(name, value.constData());

#else
   char **ep = 0;
   size_t size;

   const size_t namelen = qstrlen(name);
   const size_t vallen  = value.size() + 1;

   size = 0;
   if (environ != 0) {
      for (ep = environ; *ep; ++ep) {
         if (!qstrncmp (*ep, name, (uint)namelen) && (*ep)[namelen] == '=') {
            break;
         } else {
            ++size;
         }
      }
   }

   if (environ == 0 || *ep == 0) { /* add new string */
      char **new_environ;

      if (environ == last_environ && environ != 0) {
         // We allocated this space; we can extend it.
         new_environ = (char **) realloc (last_environ, (size + 2) * sizeof (char *));

      } else {
         new_environ = (char **) malloc ((size + 2) * sizeof (char *));

      }

      if (new_environ == 0) { // no more memory
         return;
      }

      new_environ[size] = (char *)malloc (namelen + 1 + vallen);
      if (new_environ[size] == 0) {
         free (new_environ);
         return;
      }

      if (environ != last_environ) {
         memcpy ((char *) new_environ, environ, size * sizeof (char *));
      }

      memcpy(new_environ[size], name, namelen);
      new_environ[size][namelen] = '=';

      memcpy(&new_environ[size][namelen + 1], value.constData(), vallen);
      new_environ[size + 1] = 0;
      last_environ = environ = new_environ;

   } else { 
      /* replace existing string */

      size_t len = qstrlen (*ep);

      if (len + 1 < namelen + 1 + vallen) {
         /* The existing string is too short; malloc a new one.  */
         char *newString = (char *)malloc(namelen + 1 + vallen);
         if (newString == 0) {
            return;
         }
         *ep = newString;
      }

      memcpy(*ep, name, namelen);
      (*ep)[namelen] = '=';

      memcpy(&(*ep)[namelen + 1], value.constData(), vallen);
   }

#endif
}

void portable_unsetenv(const char *variable)
{

#if HAVE_WINDOWS_H
   SetEnvironmentVariable(variable, 0);

#else
   /* Some systems do not have unsetenv(), so we do it ourselves */
   size_t len;
   char **ep;

   if (variable == NULL || *variable == '\0' || strchr (variable, '=') != NULL) {
      return; // not properly formatted
   }

   len = qstrlen(variable);
   ep = environ;

   while (*ep != NULL) {
      if (!qstrncmp(*ep, variable, (uint)len) && (*ep)[len] == '=') {
         /* Found it.  Remove this pointer by moving later ones back.  */
         char **dp = ep;
         do {
            dp[0] = dp[1];
         } while (*dp++);
         /* Continue the loop in case NAME appears again.  */
      } else {
         ++ep;
      }
   }
#endif
}

const char *portable_getenv(const char *variable)
{
   return getenv(variable);
}

portable_off_t portable_fseek(FILE *f, portable_off_t offset, int whence)
{
#if defined(__MINGW32__)
   return fseeko64(f, offset, whence);

#elif defined(_WIN32) && ! defined(__CYGWIN__)
   return _fseeki64(f, offset, whence);

#else
   return fseeko(f, offset, whence);

#endif
}

portable_off_t portable_ftell(FILE *f)
{
#if defined(__MINGW32__)
   return ftello64(f);

#elif defined(_WIN32) && !defined(__CYGWIN__)
   return _ftelli64(f);

#else
   return ftello(f);

#endif
}

char portable_pathSeparator()
{
   return QDir::separator().toUtf8();   
}

char portable_pathListSeparator()
{
#if Q_OS_WIN
   return ';';
#else
   return ':';
#endif
}

const char *portable_commandExtension()
{
#if Q_OS_WIN
   return ".exe";
#else
   return "";
#endif
}

Qt::CaseSensitivity portable_fileSystemIsCaseSensitive()
{
   QFSFileEngine engine;
   
   if (engine.caseSensitive()){ 
      return Qt::CaseSensitive;
   } else {
      return Qt::CaseInsensitive;
   }
}

void portable_sysTimerStart()
{
   g_time.start();
}

void portable_sysTimerStop()
{
   g_sysElapsedTime += ((double)g_time.elapsed()) / 1000.0;
}

double portable_getSysElapsedTime()
{
   return g_sysElapsedTime;
}

void portable_sleep(int ms)
{
#if HAVE_WINDOWS_H
   Sleep(ms);
#else
   usleep(1000 * ms);
#endif
}

