// Copyright (c) 2018-2026 Jean-Philippe Bruyère <jp_bruyere@hotmail.com>
//
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
#include "cross_os.h"
#include <sys/types.h>
#include <sys/stat.h>

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
