/* Translate to/from dqlite types */

#ifndef COWSQL_TRANSLATE_H_
#define COWSQL_TRANSLATE_H_

/* Translate a raft error to a dqlite one. */
int translateRaftErrCode(int code);

/* Translate a dqlite role code to its raft equivalent. */
int translateDqliteRole(int role);

/* Translate a raft role code to its dqlite equivalent. */
int translateRaftRole(int role);

#endif /* COWSQL_TRANSLATE_H_ */
