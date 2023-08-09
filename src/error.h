#ifndef COWSQL_ERROR_H
#define COWSQL_ERROR_H

#include <string.h>

#include <sqlite3.h>

/* A message describing the last error occurred on an object */
typedef char *cowsql__error;

/* Initialize the error with an empty message */
void cowsql__error_init(cowsql__error *e);

/* Release the memory of the error message, if any is set */
void cowsql__error_close(cowsql__error *e);

/* Set the error message */
void cowsql__error_printf(cowsql__error *e, const char *fmt, ...);

/* Wrap an error with an additional message */
void cowsql__error_wrapf(cowsql__error *e,
			 const cowsql__error *cause,
			 const char *fmt,
			 ...);

/* Out of memory error */
void cowsql__error_oom(cowsql__error *e, const char *msg, ...);

/* Wrap a system error */
void cowsql__error_sys(cowsql__error *e, const char *msg);

/* Wrap an error from libuv */
void cowsql__error_uv(cowsql__error *e, int err, const char *msg);

/* Copy the underlying error message.
 *
 * Client code is responsible of invoking sqlite3_free to deallocate the
 * returned string.
 */
int cowsql__error_copy(cowsql__error *e, char **msg);

/* Whether the error is not set */
int cowsql__error_is_null(cowsql__error *e);

/* Whether the error is due to client disconnection */
int cowsql__error_is_disconnect(cowsql__error *e);

#endif /* COWSQL_ERROR_H */
