/******************************************************************************
 *
 * Collect various performance metrics.
 *
 *****************************************************************************/

#ifndef COWSQL_METRICS_H
#define COWSQL_METRICS_H

#include <stdint.h>

struct cowsql__metrics
{
	uint64_t requests; /* Total number of requests served. */
	uint64_t duration; /* Total time spent to server requests. */
};

void cowsql__metrics_init(struct cowsql__metrics *m);

#endif /* COWSQL_METRICS_H */
