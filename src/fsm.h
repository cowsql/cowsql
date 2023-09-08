/**
 * Cowsql Raft FSM
 */

#ifndef COWSQL_FSM_H_
#define COWSQL_FSM_H_

#include <raft.h>

#include "config.h"
#include "registry.h"

/**
 * Initialize the given SQLite replication interface with cowsql's raft based
 * implementation.
 */
int fsm__init(struct raft_fsm *fsm,
	      struct config *config,
	      struct registry *registry);

void fsm__close(struct raft_fsm *fsm);

#endif /* COWSQL_REPLICATION_METHODS_H_ */
