#include "tracing.h"

#include <stdlib.h>

#define LIBCOWSQL_TRACE "LIBCOWSQL_TRACE"

bool _cowsqlTracingEnabled = false;

void cowsqlTracingMaybeEnable(bool enable)
{
	if (getenv(LIBCOWSQL_TRACE) != NULL) {
		_cowsqlTracingEnabled = enable;
	}
}
