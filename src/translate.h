/* Translate to/from cowsql types */

#ifndef COWSQL_TRANSLATE_H_
#define COWSQL_TRANSLATE_H_

/* Translate a raft error to a cowsql one. */
int translateRaftErrCode(int code);

/* Translate a cowsql role code to its raft equivalent. */
int translateCowsqlRole(int role);

/* Translate a raft role code to its cowsql equivalent. */
int translateRaftRole(int role);

#endif /* COWSQL_TRANSLATE_H_ */
