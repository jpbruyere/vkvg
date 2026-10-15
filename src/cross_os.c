// Copyright (c) 2018-2026 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#include "cross_os.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#define _CRT_SECURE_NO_WARNINGS

int directoryExists(const char* path) {
#if defined(_WIN32) || defined(_WIN64)
#elif __APPLE__
#elif __unix__
    struct stat st = {0};
    return stat(path, &st) + 1;
#else
    return -1;
#endif
}
int create_dir(const char* path) {
#if defined(_WIN32)
    return (CreateDirectoryA(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS);
#else
    return (mkdir(path, 0755) == 0 || errno == EEXIST);
#endif
}
// Creates the entire directory tree recursively/iteratively
int create_dir_tree(const char* path) {
    char tmp[512];
    size_t len = strlen(path);

    if (len >= sizeof(tmp)) {
        return false; // Path too long
    }
    strncpy(tmp, path, len);

    for (size_t i = 0; i < len; i++) {
        if (tmp[i] == '\\') {
            tmp[i] = '/';
        }
    }

    for (size_t i = 0; i < len; i++) {
        if (tmp[i] == '/') {
            if (i == 0) {
                continue;
            }

#if defined(_WIN32)
            if (i == 3 && tmp[1] == ':') {
                continue;
            }
#endif

            tmp[i] = '\0';
            if (!create_dir(tmp)) {
                return false;
            }
            tmp[i] = '/';
        }
    }
    return create_dir(tmp);
}


const char* getUserDir() {
#if defined(_WIN32) || defined(_WIN64)
    return getenv("HOME");
#elif __APPLE__
#elif __unix__
    struct passwd* pw = getpwuid(getuid());
    return pw->pw_dir;
#endif
}

#if defined(__linux__) && defined(__GLIBC__)
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig) {
    void*  array[100];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 100);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

void _linux_register_error_handler() {
    signal(SIGSEGV, handler); // install our handler
    signal(SIGABRT, handler); // install our handler
}
#endif
