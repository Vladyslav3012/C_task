#include "process.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

static int file_exists(const char *path)
{
#ifdef _WIN32
    DWORD a = GetFileAttributesA(path);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
#else
    return access(path, F_OK) == 0;
#endif
}

int programs_resolve_path(const char *path, char *buf, size_t buflen)
{
    if (!path || !buf || buflen == 0)
        return -1;

    if (file_exists(path)) {
        strncpy(buf, path, buflen - 1);
        buf[buflen - 1] = '\0';
        return 0;
    }

#ifdef _WIN32
    {
        size_t len = strlen(path);
        if (len + 4 < buflen && (len < 4 || _stricmp(path + len - 4, ".exe") != 0)) {
            snprintf(buf, buflen, "%s.exe", path);
            if (file_exists(buf))
                return 0;
        }
    }
#endif

    return -1;
}

int programs_executable(const char *path)
{
    char tmp[PATH_LEN];
    return programs_resolve_path(path, tmp, sizeof tmp) == 0;
}

int programs_spawn_async(const Program *prog, char *errbuf, size_t errlen)
{
    char resolved[PATH_LEN];

    if (!prog) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen, "Програму не обрано.");
        return -1;
    }
    if (programs_resolve_path(prog->path, resolved, sizeof resolved) != 0) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen,
                     "Файл «%s» не знайдено.\nЗберіть: make (у MSYS2 UCRT64).", prog->path);
        return -1;
    }

    programs_chdir_home();

#ifdef _WIN32
    {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        char cmd[PATH_LEN + 32];

        ZeroMemory(&si, sizeof si);
        si.cb = sizeof si;
        ZeroMemory(&pi, sizeof pi);
        snprintf(cmd, sizeof cmd, "\"%s\"", resolved);

        {
            const char *home = getenv("LAUNCHER_HOME");

            if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
                                NULL, home && home[0] ? home : NULL, &si, &pi)) {
                if (errbuf && errlen)
                    snprintf(errbuf, errlen, "CreateProcess: код %lu", GetLastError());
                return -1;
            }
        }
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 0;
    }
#else
    {
        pid_t pid = fork();
        if (pid < 0) {
            if (errbuf && errlen)
                snprintf(errbuf, errlen, "fork: %s", strerror(errno));
            return -1;
        }
        if (pid == 0) {
            execl(resolved, resolved, (char *)NULL);
            fprintf(stderr, "exec %s: %s\n", resolved, strerror(errno));
            _exit(127);
        }
        return 0;
    }
#endif
}

int programs_run_blocking(const Program *prog, char *errbuf, size_t errlen, int *exit_code)
{
    char resolved[PATH_LEN];

    if (exit_code)
        *exit_code = -1;

    if (programs_resolve_path(prog->path, resolved, sizeof resolved) != 0) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen, "Файл «%s» недоступний.", prog->path);
        return -1;
    }

    programs_chdir_home();

#ifdef _WIN32
    {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        char cmd[PATH_LEN + 32];
        DWORD code;

        ZeroMemory(&si, sizeof si);
        si.cb = sizeof si;
        ZeroMemory(&pi, sizeof pi);
        snprintf(cmd, sizeof cmd, "\"%s\"", resolved);

        {
            const char *home = getenv("LAUNCHER_HOME");

            if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
                                NULL, home && home[0] ? home : NULL, &si, &pi)) {
                if (errbuf && errlen)
                    snprintf(errbuf, errlen, "CreateProcess: код %lu", GetLastError());
                return -1;
            }
        }
        WaitForSingleObject(pi.hProcess, INFINITE);
        GetExitCodeProcess(pi.hProcess, &code);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        if (exit_code)
            *exit_code = (int)code;
        return 0;
    }
#else
    {
        pid_t pid = fork();
        int status;

        if (pid < 0) {
            if (errbuf && errlen)
                snprintf(errbuf, errlen, "fork: %s", strerror(errno));
            return -1;
        }
        if (pid == 0) {
            execl(resolved, resolved, (char *)NULL);
            _exit(127);
        }
        if (waitpid(pid, &status, 0) < 0)
            return -1;
        if (exit_code && WIFEXITED(status))
            *exit_code = WEXITSTATUS(status);
        return 0;
    }
#endif
}
