/*####################################################
# File Name: dbg.h
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-16 19:56:32
# Last Modified: 2017-05-16 20:07:15
####################################################*/
#ifndef DGB_H
#define DGB_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef NO_DEBUG
#define debug(M, ...)
#else
#define debug(M, ...) fprintf(stderr, "[DEBUG] (%s:%d:)" M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s)" M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warning(M, ...) fprintf(stderr, "[WARNING] (%s:%d: errno: %s)" M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d)" M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) {log_error(M, "\n", ##__VA_ARGS__);}

#define check_exit(A, M, ...) if(!(A)) {log_error(M, "\n", ##__VA_ARGS__); exit(1);}

#define check_debug(A, M, ...) if(!(A)) {debug(M, "\n", ##__VA_ARGS__);}

#endif
