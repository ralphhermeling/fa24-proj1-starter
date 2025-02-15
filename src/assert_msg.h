#ifndef ASSERT_MSG_H
#define ASSERT_MSG_H

#include <stdio.h>
#include <stdlib.h>

#define assert_msg_exit(condition, exit_code, fmt, ...)                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "Assertion failed: %s\nFile: %s, Line: %d\nMessage: ",   \
              #condition, __FILE__, __LINE__);                                 \
      fprintf(stderr, fmt, ##__VA_ARGS__);                                     \
      fprintf(stderr, "\n");                                                   \
      exit(exit_code);                                                         \
    }                                                                          \
  } while (0)

#define assert_msg(condition, fmt, ...)                                        \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "Assertion failed: %s\nFile: %s, Line: %d\nMessage: ",   \
              #condition, __FILE__, __LINE__);                                 \
      fprintf(stderr, fmt, ##__VA_ARGS__);                                     \
      fprintf(stderr, "\n");                                                   \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

#endif // ASSERT_MSG_H
