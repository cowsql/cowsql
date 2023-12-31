#ifndef CONFIG_H_
#define CONFIG_H_

#include "logger.h"

/**
 * Value object holding cowsql configuration.
 */
struct config
{
	cowsql_node_id id;             /* Unique instance ID */
	char *address;                 /* Instance address */
	unsigned heartbeat_timeout;    /* In milliseconds */
	unsigned page_size;            /* Database page size */
	unsigned checkpoint_threshold; /* In outstanding WAL frames */
	struct logger logger;          /* Custom logger */
	char name[256];                /* VFS/replication registriatio name */
	unsigned long long failure_domain; /* User-provided failure domain */
	unsigned long long int weight;     /* User-provided node weight */
	int voters;                        /* Target number of voters */
	int standbys;                      /* Target number of standbys */
};

/**
 * Initialize the config object with required values and set the rest to sane
 * defaults. A copy will be made of the given @address.
 */
int config__init(struct config *c, cowsql_node_id id, const char *address);

/**
 * Release any memory held by the config object.
 */
void config__close(struct config *c);

#endif /* COWSQL_OPTIONS_H */
