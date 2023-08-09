#include "translate.h"

#include <raft.h>

#include "assert.h"
#include "leader.h"
#include "protocol.h"

/* Translate a raft error to a cowsql one. */
int translateRaftErrCode(int code)
{
	switch (code) {
		case RAFT_NOTLEADER:
			return SQLITE_IOERR_NOT_LEADER;
		case RAFT_LEADERSHIPLOST:
			return SQLITE_IOERR_LEADERSHIP_LOST;
		case RAFT_CANTCHANGE:
			return SQLITE_BUSY;
		default:
			return SQLITE_ERROR;
	}
}

/* Translate a cowsql role code to its raft equivalent. */
int translateCowsqlRole(int role)
{
	switch (role) {
		case COWSQL_VOTER:
			return RAFT_VOTER;
		case COWSQL_STANDBY:
			return RAFT_STANDBY;
		case COWSQL_SPARE:
			return RAFT_SPARE;
		default:
			/* For backward compat with clients that don't set a
			 * role. */
			return COWSQL_VOTER;
	}
}

/* Translate a raft role code to its cowsql equivalent. */
int translateRaftRole(int role)
{
	switch (role) {
		case RAFT_VOTER:
			return COWSQL_VOTER;
		case RAFT_STANDBY:
			return COWSQL_STANDBY;
		case RAFT_SPARE:
			return COWSQL_SPARE;
		default:
			assert(0);
			return -1;
	}
}
