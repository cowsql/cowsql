#include "tracing.h"

#include <stdlib.h>

#define LIBCOWSQL_TRACE "LIBCOWSQL_TRACE"

bool _dqliteTracingEnabled = false;

void dqliteTracingMaybeEnable(bool enable)
{
	if (getenv(LIBCOWSQL_TRACE) != NULL) {
		_dqliteTracingEnabled = enable;
	}
}
