/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
*
* DoxyPress is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
*************************************************************************/

#include <QDateTime>
#include <QDir>
#include <QFSFileEngine>
#include <qglobal.h>

#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_WINDOWS_H

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

#include <portable.h>
#include <message.h>

static double g_sysElapsedTime;
static QTime  g_time;

int portable_system(const QString &command, const QString &args, bool commandHasConsole)
{
   QString fullCmd = command.trimmed();

   if (fullCmd.isEmpty()) {
      return 1;
   }

#if defined(Q_OS_WIN)
   fullCmd = fullCmd.replace('/', '\\' );
#endif

   if (! fullCmd.startsWith('"') && fullCmd.contains(' ')) {
      // add quotes around command as it contains spaces and is not quoted already
      fullCmd = "\"" + fullCmd + "\"";
   }

   fullCmd += " " + args;

#ifdef HAVE_FORK

   int pid;
   int status = 0;

   pid = fork();

   if (pid == -1) {
      perror("fork error");
      return -1;
   }

   if (pid == 0) {
      const char *argv[4];

      argv[0] = "sh";
      argv[1] = "-c";
      argv[2] = fullCmd.constData();
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
      return system(fullCmd.constData());

   } else {
      CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

      auto commandW = command.toStdWString();
      auto argsW = args.toStdWString();

      SHELLEXECUTEINFOW sInfo = {
         sizeof(SHELLEXECUTEINFOW),

         // wait till the process is done, do not display msg box if there is an error
         SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI,

         NULL,               /* window handle */
         NULL,               /* action to perform: open */
         commandW.c_str(),   /* file to execute */
         argsW.c_str(),      /* argument list */
         NULL,               /* use current working dir */
         SW_HIDE,            /* minimize on start-up */
         0,                  /* application instance handle */
         NULL,               /* ignored: id list */
         NULL,               /* ignored: class name */
         NULL,               /* ignored: key class */
         0,                  /* ignored: hot key */
         NULL,               /* ignored: icon */
         NULL                /* resulting application handle */
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

#ifdef HAVE_GETPID
   pid = (uint)getpid();

#else
   pid = (uint)GetCurrentProcessId();

#endif

   return pid;
}

#if defined(_WIN32) && ! defined(__CYGWIN__)
   // nothing
#else
   static char **last_environ;
#endif

void portable_setenv(const QString &name_T, const QString &value_T)
{
#ifdef HAVE_WINDOWS_H
   // windows only

   std::wstring name  = name_T.toStdWString();
   std::wstring value = value_T.toStdWString();

   SetEnvironmentVariableW(&name[0], &value[0]);

#else
   char **ep = 0;
   size_t size;

   const size_t namelen = name_T.size_storage();
   const size_t vallen  = value_T.size_storage() + 1;

   size = 0;
   if (environ != 0) {

      for (ep = environ; *ep; ++ep) {
         if (! qstrncmp (*ep, name_T.constData(), namelen) && (*ep)[namelen] == '=') {
            break;
         } else {
            ++size;
         }
      }
   }

   if (environ == 0 || *ep == 0) {
      // add new string
      char **new_environ;

      if (environ == last_environ && environ != 0) {
         // allocated this space, can extend it
         new_environ = (char **) realloc (last_environ, (size + 2) * sizeof (char *));

      } else {
         new_environ = (char **) malloc ((size + 2) * sizeof (char *));

      }

      if (new_environ == 0) {
         // no more memory
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

      memcpy(new_environ[size], name_T.constData(), namelen);
      new_environ[size][namelen] = '=';

      memcpy(&new_environ[size][namelen + 1], value_T.constData(), vallen);
      new_environ[size + 1] = 0;
      last_environ = environ = new_environ;

   } else {
      // replace existing string
      size_t len = qstrlen (*ep);

      if (len + 1 < namelen + 1 + vallen) {
         /* The existing string is too short; malloc a new one.  */
         char *newString = (char *)malloc(namelen + 1 + vallen);
         if (newString == 0) {
            return;
         }

         *ep = newString;
      }

      memcpy(*ep, name_T.constData(), namelen);
      (*ep)[namelen] = '=';

      memcpy(&(*ep)[namelen + 1], value_T.constData(), vallen);
   }

#endif
}

void portable_unsetenv(const QString &key)
{

#ifdef HAVE_WINDOWS_H
   // windows only

   std::wstring tmp = key.toStdWString();
   SetEnvironmentVariableW(&tmp[0], 0);

#else

  if (key.isEmpty() || key.contains('=')) {
      // not properly formatted
      return;
   }

   // Some systems do not have unsetenv(), so do it ourselves
   size_t len = key.size_storage();

   char **ep;
   ep  = environ;

   while (*ep != nullptr) {
      if (! qstrncmp(*ep, key.constData(), len) && (*ep)[len] == '=') {
         // found it, remove this pointer by moving later ones back
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

QString portable_getenv(const QString &key)
{
   return QString::fromUtf8(getenv(key.constData()));
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

char portable_pathListSeparator()
{
#ifdef Q_OS_WIN
   return ';';
#else
   return ':';
#endif
}

QString portable_commandExtension()
{
#ifdef Q_OS_WIN
   return QString(".exe");
#else
   return QString("");
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
#ifdef HAVE_WINDOWS_H
   Sleep(ms);
#else
   usleep(1000 * ms);
#endif
}

