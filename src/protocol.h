#ifndef COWSQL_PROTOCOL_H_
#define COWSQL_PROTOCOL_H_

/* Special datatypes */
#define COWSQL_UNIXTIME 9
#define COWSQL_ISO8601 10
#define COWSQL_BOOLEAN 11

#define COWSQL_PROTO 1001 /* Protocol error */

/* Role codes */
enum { COWSQL_VOTER, COWSQL_STANDBY, COWSQL_SPARE };

/* Current protocol version */
#define COWSQL_PROTOCOL_VERSION 1

/* Legacly pre-1.0 version. */
#define COWSQL_PROTOCOL_VERSION_LEGACY 0x86104dd760433fe5

/* Special value indicating that a batch of rows is over, but there are more. */
#define COWSQL_RESPONSE_ROWS_PART 0xeeeeeeeeeeeeeeee

/* Special value indicating that the result set is complete. */
#define COWSQL_RESPONSE_ROWS_DONE 0xffffffffffffffff

/* Request types */
enum {
	COWSQL_REQUEST_LEADER,
	COWSQL_REQUEST_CLIENT,
	COWSQL_REQUEST_HEARTBEAT,
	COWSQL_REQUEST_OPEN,
	COWSQL_REQUEST_PREPARE,
	COWSQL_REQUEST_EXEC,
	COWSQL_REQUEST_QUERY,
	COWSQL_REQUEST_FINALIZE,
	COWSQL_REQUEST_EXEC_SQL,
	COWSQL_REQUEST_QUERY_SQL,
	COWSQL_REQUEST_INTERRUPT,
	COWSQL_REQUEST_CONNECT,
	COWSQL_REQUEST_ADD,
	/* The PROMOTE and ASSIGN requests share a type tag. We expose it under
	 * two names here to facilitate the macro shenanigans in request.h. */
	COWSQL_REQUEST_PROMOTE_OR_ASSIGN,
	COWSQL_REQUEST_ASSIGN = COWSQL_REQUEST_PROMOTE_OR_ASSIGN,
	COWSQL_REQUEST_REMOVE,
	COWSQL_REQUEST_DUMP,
	COWSQL_REQUEST_CLUSTER,
	COWSQL_REQUEST_TRANSFER,
	COWSQL_REQUEST_DESCRIBE,
	COWSQL_REQUEST_WEIGHT
};

#define COWSQL_REQUEST_CLUSTER_FORMAT_V0 0 /* ID and address */
#define COWSQL_REQUEST_CLUSTER_FORMAT_V1 1 /* ID, address and role */

#define COWSQL_REQUEST_DESCRIBE_FORMAT_V0 0 /* Failure domain and weight */

/* These apply to REQUEST_EXEC, REQUEST_EXEC_SQL, REQUEST_QUERY, and
 * REQUEST_QUERY_SQL. */
#define COWSQL_REQUEST_PARAMS_SCHEMA_V0 0 /* One-byte params count */
#define COWSQL_REQUEST_PARAMS_SCHEMA_V1 1 /* Four-byte params count */

/* These apply to REQUEST_PREPARE and RESPONSE_STMT. */

/* At most one statement in request, no tail offset in response */
#define COWSQL_PREPARE_STMT_SCHEMA_V0 0
/* Any number of statements in request, tail offset in response */
#define COWSQL_PREPARE_STMT_SCHEMA_V1 1

/* Response types */
enum {
	COWSQL_RESPONSE_FAILURE,
	COWSQL_RESPONSE_SERVER,
	COWSQL_RESPONSE_SERVER_LEGACY = COWSQL_RESPONSE_SERVER,
	COWSQL_RESPONSE_WELCOME,
	COWSQL_RESPONSE_SERVERS,
	COWSQL_RESPONSE_DB,
	COWSQL_RESPONSE_STMT,
	COWSQL_RESPONSE_STMT_WITH_OFFSET = COWSQL_RESPONSE_STMT,
	COWSQL_RESPONSE_RESULT,
	COWSQL_RESPONSE_ROWS,
	COWSQL_RESPONSE_EMPTY,
	COWSQL_RESPONSE_FILES,
	COWSQL_RESPONSE_METADATA
};

#endif /* COWSQL_PROTOCOL_H_ */
