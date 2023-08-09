#include <stdlib.h>

#include "./lib/assert.h"

#include "metrics.h"

void cowsql__metrics_init(struct cowsql__metrics *m)
{
	assert(m != NULL);

	m->requests = 0;
	m->duration = 0;
}
