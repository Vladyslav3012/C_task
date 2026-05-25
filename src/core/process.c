#include "process.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#else
#define _POSIX_C_SOURCE 200809L
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
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

static void child_clear(ChildProc *child)
{
    if (!child)
        return;
#ifdef _WIN32
    if (child->active && child->process) {
        CloseHandle((HANDLE)child->process);
        child->process = NULL;
    }
#else
    (void)child;
#endif
    child->active = 0;
    child->pid = 0;
}

#ifdef _WIN32
static int build_cmdline(const char *exe, char *const argv[], char *buf, size_t buflen)
{
    size_t pos = 0;
    int i;

    pos += (size_t)snprintf(buf + pos, buflen - pos, "\"%s\"", exe);
    for (i = 0; argv && argv[i]; i++)
        pos += (size_t)snprintf(buf + pos, buflen - pos, " \"%s\"", argv[i]);
    return pos < buflen ? 0 : -1;
}
#endif

int programs_spawn_path_argv(const char *path, char *const argv[],
                             ChildProc *child, char *errbuf, size_t errlen)
{
    char resolved[PATH_LEN];

    if (!path) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen, "Шлях програми не задано.");
        return -1;
    }
    if (programs_resolve_path(path, resolved, sizeof resolved) != 0) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen, "Файл «%s» не знайдено.\nЗберіть: make", path);
        return -1;
    }

    programs_chdir_home();
    if (child)
        child_clear(child);

#ifdef _WIN32
    {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        char cmd[4096];

        if (build_cmdline(resolved, argv, cmd, sizeof cmd) != 0) {
            if (errbuf && errlen)
                snprintf(errbuf, errlen, "Занадто довга командний рядок.");
            return -1;
        }

        ZeroMemory(&si, sizeof si);
        si.cb = sizeof si;
        ZeroMemory(&pi, sizeof pi);

        {
            const char *home = getenv("LAUNCHER_HOME");
            if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW,
                                NULL, home && home[0] ? home : NULL, &si, &pi)) {
                if (errbuf && errlen)
                    snprintf(errbuf, errlen, "CreateProcess: код %lu", GetLastError());
                return -1;
            }
        }
        if (child) {
            child->process = pi.hProcess;
            child->pid = pi.dwProcessId;
            child->active = 1;
            CloseHandle(pi.hThread);
        } else {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
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
            char *args[64];
            int i, n = 1;
            args[0] = resolved;
            if (argv) {
                for (i = 0; argv[i] && n < 62; i++)
                    args[n++] = argv[i];
            }
            args[n] = NULL;
            execv(resolved, args);
            fprintf(stderr, "exec %s: %s\n", resolved, strerror(errno));
            _exit(127);
        }
        if (child) {
            child->pid = (int)pid;
            child->active = 1;
        }
        return 0;
    }
#endif
}

void programs_child_kill(ChildProc *child)
{
    if (!child || !child->active)
        return;
#ifdef _WIN32
    TerminateProcess((HANDLE)child->process, 1);
    child_clear(child);
#else
    kill(child->pid, SIGTERM);
    waitpid(child->pid, NULL, 0);
    child_clear(child);
#endif
}

int programs_child_poll(ChildProc *child, int *exit_code)
{
    if (!child || !child->active)
        return 0;

#ifdef _WIN32
    {
        DWORD code;
        if (WaitForSingleObject((HANDLE)child->process, 0) == WAIT_OBJECT_0) {
            GetExitCodeProcess((HANDLE)child->process, &code);
            if (exit_code)
                *exit_code = (int)code;
            child_clear(child);
            return 1;
        }
        return 0;
    }
#else
    {
        int status;
        pid_t r = waitpid(child->pid, &status, WNOHANG);
        if (r == 0)
            return 0;
        if (r > 0) {
            if (exit_code && WIFEXITED(status))
                *exit_code = WEXITSTATUS(status);
            else if (exit_code)
                *exit_code = -1;
            child->active = 0;
            child->pid = 0;
            return 1;
        }
        return 0;
    }
#endif
}

int programs_spawn_async(const Program *prog, char *errbuf, size_t errlen)
{
    if (!prog) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen, "Програму не обрано.");
        return -1;
    }
    return programs_spawn_path_argv(prog->path, NULL, NULL, errbuf, errlen);
}

int programs_run_blocking(const Program *prog, char *errbuf, size_t errlen, int *exit_code)
{
    ChildProc child;
    int code = -1;

    if (exit_code)
        *exit_code = -1;
    if (!prog) {
        if (errbuf && errlen)
            snprintf(errbuf, errlen, "Програму не обрано.");
        return -1;
    }
    if (programs_spawn_path_argv(prog->path, NULL, &child, errbuf, errlen) < 0)
        return -1;
    while (!programs_child_poll(&child, &code))
#ifdef _WIN32
        Sleep(50);
#else
        usleep(50000);
#endif
    if (exit_code)
        *exit_code = code;
    return 0;
}
