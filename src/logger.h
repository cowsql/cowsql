#ifndef LOGGER_H_
#define LOGGER_H_

#include <raft.h>

#include "../include/cowsql.h"

/* Log levels */
enum { COWSQL_DEBUG = 0, COWSQL_INFO, COWSQL_WARN, COWSQL_LOG_ERROR };

/* Function to emit log messages. */
typedef void (*cowsql_emit)(void *data,
			    int level,
			    const char *fmt,
			    va_list args);

struct logger
{
	void *data;
	cowsql_emit emit;
};

/* Default implementation of cowsql_emit, using stderr. */
void loggerDefaultEmit(void *data, int level, const char *fmt, va_list args);

/* Emit a log message with a certain level. */
/* #define debugf(L, FORMAT, ...) \ */
/* 	logger__emit(L, COWSQL_DEBUG, FORMAT, ##__VA_ARGS__) */
#define debugf(C, FORMAT, ...)                                             \
	C->gateway.raft->io->emit(C->gateway.raft->io, RAFT_DEBUG, FORMAT, \
				  ##__VA_ARGS__)

#endif /* LOGGER_H_ */
