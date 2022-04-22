#include "../../src/client.h"
#include "../../src/server.h"
#include "../lib/client.h"
#include "../lib/heap.h"
#include "../lib/runner.h"
#include "../lib/server.h"
#include "../lib/sqlite.h"

/******************************************************************************
 *
 * Fixture
 *
 ******************************************************************************/

#define N_SERVERS 1
#define FIXTURE                                \
	struct test_server servers[N_SERVERS]; \
	struct client *client

#define SETUP                                                 \
	unsigned i_;                                          \
	test_heap_setup(params, user_data);                   \
	test_sqlite_setup(params);                            \
	for (i_ = 0; i_ < N_SERVERS; i_++) {                  \
		struct test_server *server = &f->servers[i_]; \
		test_server_setup(server, i_ + 1, params);    \
	}                                                     \
	test_server_network(f->servers, N_SERVERS);           \
	for (i_ = 0; i_ < N_SERVERS; i_++) {                  \
		struct test_server *server = &f->servers[i_]; \
		test_server_start(server);                    \
	}                                                     \
	SELECT(1)

#define TEAR_DOWN                                       \
	unsigned i_;                                    \
	for (i_ = 0; i_ < N_SERVERS; i_++) {            \
		test_server_tear_down(&f->servers[i_]); \
	}                                               \
	test_sqlite_tear_down();                        \
	test_heap_tear_down(data)

/******************************************************************************
 *
 * Helper macros.
 *
 ******************************************************************************/

/* Use the client connected to the server with the given ID. */
#define SELECT(ID) f->client = test_server_client(&f->servers[ID - 1])

/******************************************************************************
 *
 * snapshot
 *
 ******************************************************************************/

SUITE(fsm)

struct fixture
{
	FIXTURE;
};

static void *setUp(const MunitParameter params[], void *user_data)
{
	struct fixture *f = munit_malloc(sizeof *f);
	SETUP;
	return f;
}

static void tearDown(void *data)
{
	struct fixture *f = data;
	TEAR_DOWN;
	free(f);
}

TEST(fsm, snapshotFreshDb, setUp, tearDown, 0, NULL)
{
	struct fixture *f = data;
	struct raft_fsm *fsm = &f->servers[0].dqlite->raft_fsm;
	struct raft_buffer *bufs;
	unsigned n_bufs = 0;
	int rv;

	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	munit_assert_uint(n_bufs, ==, 1); /* Snapshot header */

	rv = fsm->snapshot_finalize(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	munit_assert_ptr_null(bufs);
	munit_assert_uint(n_bufs, ==, 0);
	return MUNIT_OK;
}

TEST(fsm, snapshotWrittenDb, setUp, tearDown, 0, NULL)
{
	struct fixture *f = data;
	struct raft_fsm *fsm = &f->servers[0].dqlite->raft_fsm;
	struct raft_buffer *bufs;
	unsigned n_bufs = 0;
	int rv;

	unsigned stmt_id;
	unsigned last_insert_id;
	unsigned rows_affected;

	/* Add some data to database */
	HANDSHAKE;
	OPEN;
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test(n) VALUES(1)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	munit_assert_uint(n_bufs, >, 1);

	rv = fsm->snapshot_finalize(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	munit_assert_ptr_null(bufs);
	munit_assert_uint(n_bufs, ==, 0);
	return MUNIT_OK;
}

TEST(fsm, snapshotHeapFaultSingleDB, setUp, tearDown, 0, NULL)
{
	struct fixture *f = data;
	struct raft_fsm *fsm = &f->servers[0].dqlite->raft_fsm;
	struct raft_buffer *bufs;
	unsigned n_bufs = 0;
	int rv;

	unsigned stmt_id;
	unsigned last_insert_id;
	unsigned rows_affected;

	/* Add some data to database */
	HANDSHAKE;
	OPEN;
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test(n) VALUES(1)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	/* Inject heap faults at different stages of fsm__snapshot */
	test_heap_fault_config(0, 1);
	test_heap_fault_enable();
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	test_heap_fault_config(1, 1);
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	test_heap_fault_config(2, 1);
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	test_heap_fault_config(3, 1);
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	return MUNIT_OK;
}

TEST(fsm, snapshotHeapFaultTwoDB, setUp, tearDown, 0, NULL)
{
	struct fixture *f = data;
	struct raft_fsm *fsm = &f->servers[0].dqlite->raft_fsm;
	struct raft_buffer *bufs;
	unsigned n_bufs = 0;
	int rv;

	unsigned stmt_id;
	unsigned last_insert_id;
	unsigned rows_affected;

	/* Open 2 databases and add data to them */
	HANDSHAKE;
	OPEN_NAME("test");
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test(n) VALUES(1)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	/* Close and reopen the client and open a second database */
	rv = test_server_client_reconnect(&f->servers[0]);
	munit_assert_int(rv, ==, 0);

	HANDSHAKE;
	OPEN_NAME("test2");
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test(n) VALUES(1)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	/* Inject heap faults at different stages of fsm__snapshot */
	test_heap_fault_config(0, 1);
	test_heap_fault_enable();
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	test_heap_fault_config(1, 1);
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	test_heap_fault_config(2, 1);
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	test_heap_fault_config(3, 1);
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	test_heap_fault_config(4, 1);
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	test_heap_fault_config(5, 1);
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, !=, 0);

	return MUNIT_OK;
}

TEST(fsm, snapshotNewDbAddedBeforeFinalize, setUp, tearDown, 0, NULL)
{
	struct fixture *f = data;
	struct raft_fsm *fsm = &f->servers[0].dqlite->raft_fsm;
	struct raft_buffer *bufs;
	unsigned n_bufs = 0;
	int rv;

	unsigned stmt_id;
	unsigned last_insert_id;
	unsigned rows_affected;

	/* Add some data to database */
	HANDSHAKE;
	OPEN_NAME("test");
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test(n) VALUES(1)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	munit_assert_uint(n_bufs, >, 1);

	/* Close and reopen the client and open a second database,
	 * and ensure finalize succeeds. */
	rv = test_server_client_reconnect(&f->servers[0]);
	munit_assert_int(rv, ==, 0);

	HANDSHAKE;
	OPEN_NAME("test2");
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test(n) VALUES(1)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	rv = fsm->snapshot_finalize(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	munit_assert_ptr_null(bufs);
	munit_assert_uint(n_bufs, ==, 0);
	return MUNIT_OK;
}

TEST(fsm, snapshotWritesBeforeFinalize, setUp, tearDown, 0, NULL)
{
	struct fixture *f = data;
	struct raft_fsm *fsm = &f->servers[0].dqlite->raft_fsm;
	struct raft_buffer *bufs;
	unsigned n_bufs = 0;
	unsigned stmt_id;
	unsigned last_insert_id;
	unsigned rows_affected;
	char sql[128];
	int rv;

	/* Add some data to database */
	HANDSHAKE;
	OPEN;
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test(n) VALUES(0)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	munit_assert_uint(n_bufs, >, 1);

	/* Add (a lot) more data to the database */
	for (unsigned i = 0; i < 1000; ++i) {
	    sprintf(sql, "INSERT INTO test(n) VALUES(%d)", i + 1);
	    PREPARE(sql, &stmt_id);
	    EXEC(stmt_id, &last_insert_id, &rows_affected);
	}

	/* Finalize succeeds */
	rv = fsm->snapshot_finalize(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	munit_assert_ptr_null(bufs);
	munit_assert_uint(n_bufs, ==, 0);

	/* Triggers a checkpoint */
	PREPARE("INSERT INTO test(n) VALUES(1001)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	return MUNIT_OK;
}

/* Copies n raft buffers to a single raft buffer */
static struct raft_buffer n_bufs_to_buf(struct raft_buffer bufs[], unsigned n)
{
	uint8_t *cursor;
	struct raft_buffer buf = {0};

	/* Allocate a suitable buffer */
	for (unsigned i = 0; i < n; ++i) {
		buf.len += bufs[i].len;
	}
	buf.base = raft_malloc(buf.len);
	munit_assert_ptr_not_null(buf.base);

	/* Copy all data */
	cursor = buf.base;
	for (unsigned i = 0; i < n; ++i) {
		memcpy(cursor, bufs[i].base, bufs[i].len);
		cursor += bufs[i].len;
	}
	munit_assert_ullong((uintptr_t)(cursor - (uint8_t*)buf.base), ==, buf.len);

	return buf;
}

static char* num_records[] = {
    "0", "1", "256",
    /* WAL will just have been checkpointed after 993 writes. */
    "993",
    /* Non-empty WAL, checkpointed twice */
    "2200", NULL
};

static MunitParameterEnum num_writes_params[] = {
    { "num_records", num_records },
    { NULL, NULL },
};

TEST(fsm, snapshotRestore, setUp, tearDown, 0, num_writes_params)
{
	struct fixture *f = data;
	struct raft_fsm *fsm = &f->servers[0].dqlite->raft_fsm;
	struct raft_buffer *bufs;
	struct raft_buffer snapshot;
	long n_records = strtol(munit_parameters_get(params, "num_records"), NULL, 0);
	unsigned n_bufs = 0;
	unsigned stmt_id;
	unsigned last_insert_id;
	unsigned rows_affected;
	struct rows rows;
	int rv;
	char sql[128];

	/* Add some data to database */
	HANDSHAKE;
	OPEN;
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	for (int i = 0; i < n_records; ++i) {
	    sprintf(sql, "INSERT INTO test(n) VALUES(%d)", i + 1);
	    PREPARE(sql, &stmt_id);
	    EXEC(stmt_id, &last_insert_id, &rows_affected);
	}

	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);

	/* Deep copy snapshot */
	snapshot = n_bufs_to_buf(bufs, n_bufs);

	rv = fsm->snapshot_finalize(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);

	/* Additionally frees snapshot.base */
	rv = fsm->restore(fsm, &snapshot);
	munit_assert_int(rv, ==, 0);

	/* Table is there on fresh connection. */
	rv = test_server_client_reconnect(&f->servers[0]);
	munit_assert_int(rv, ==, 0);
	HANDSHAKE;
	OPEN;
	PREPARE("SELECT COUNT(*) from test", &stmt_id);
	QUERY(stmt_id, &rows);
	munit_assert_long(rows.next->values->integer, ==, n_records);
	clientCloseRows(&rows);

	/* Still possible to insert entries */
	for (int i = 0; i < n_records; ++i) {
	    sprintf(sql, "INSERT INTO test(n) VALUES(%ld)", n_records + i + 1);
	    PREPARE(sql, &stmt_id);
	    EXEC(stmt_id, &last_insert_id, &rows_affected);
	}

	return MUNIT_OK;
}

TEST(fsm, concurrentSnapshots, setUp, tearDown, 0, NULL)
{
	struct fixture *f = data;
	struct raft_fsm *fsm = &f->servers[0].dqlite->raft_fsm;
	struct raft_buffer *bufs;
	struct raft_buffer *bufs2;
	unsigned n_bufs = 0;
	unsigned n_bufs2 = 0;
	unsigned stmt_id;
	unsigned last_insert_id;
	unsigned rows_affected;
	int rv;

	/* Add some data to database */
	HANDSHAKE;
	OPEN;
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	/* Second snapshot fails when first isn't finalized */
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	rv = fsm->snapshot(fsm, &bufs2, &n_bufs2);
	munit_assert_int(rv, ==, RAFT_BUSY);
	rv = fsm->snapshot_finalize(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);

	/* Second snapshot succeeds after first is finalized */
	rv = fsm->snapshot(fsm, &bufs2, &n_bufs2);
	munit_assert_int(rv, ==, 0);
	rv = fsm->snapshot_finalize(fsm, &bufs2, &n_bufs2);
	munit_assert_int(rv, ==, 0);

	return MUNIT_OK;
}

TEST(fsm, snapshotRestoreMultipleDBs, setUp, tearDown, 0, NULL)
{
	struct fixture *f = data;
	struct raft_fsm *fsm = &f->servers[0].dqlite->raft_fsm;
	struct raft_buffer *bufs;
	struct raft_buffer snapshot;
	unsigned n_bufs = 0;
	unsigned stmt_id;
	unsigned last_insert_id;
	unsigned rows_affected;
	struct rows rows;
	uint64_t code;
	const char *msg;
	int rv;

	/* Create 2 databases and add data to them. */
	HANDSHAKE;
	OPEN_NAME("test");
	PREPARE("CREATE TABLE test (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test(n) VALUES(1)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	rv = test_server_client_reconnect(&f->servers[0]);
	munit_assert_int(rv, ==, 0);
	HANDSHAKE;
	OPEN_NAME("test2");
	PREPARE("CREATE TABLE test2a (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test2a(n) VALUES(1)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	/* Snapshot both databases and restore the data. */
	rv = fsm->snapshot(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);
	/* Copy the snapshot to restore it */
	snapshot = n_bufs_to_buf(bufs, n_bufs);
	rv = fsm->snapshot_finalize(fsm, &bufs, &n_bufs);
	munit_assert_int(rv, ==, 0);

	/* Create a new table in test2 that shouldn't be visible after
	 * restoring the snapshot. */
	PREPARE("CREATE TABLE test2b (n INT)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);
	PREPARE("INSERT INTO test2b(n) VALUES(1)", &stmt_id);
	EXEC(stmt_id, &last_insert_id, &rows_affected);

	/* Restore snapshot */
	rv = fsm->restore(fsm, &snapshot);
	munit_assert_int(rv, ==, 0);

	/* Reopen connection */
	rv = test_server_client_reconnect(&f->servers[0]);
	munit_assert_int(rv, ==, 0);
	HANDSHAKE;
	OPEN_NAME("test2");

	/* Table before snapshot is there on second DB */
	PREPARE("SELECT * from test2a", &stmt_id);
	QUERY(stmt_id, &rows);
	clientCloseRows(&rows);

	/* Table after snapshot is not there on second DB */
	PREPARE_FAIL("SELECT * from test2b", &stmt_id, &code, &msg);
	munit_assert_uint64(code, ==, DQLITE_ERROR);
	munit_assert_string_equal(msg, "no such table: test2b");

	/* Table is there on first DB */
	rv = test_server_client_reconnect(&f->servers[0]);
	munit_assert_int(rv, ==, 0);
	HANDSHAKE;
	OPEN_NAME("test");
	PREPARE("SELECT * from test", &stmt_id);
	QUERY(stmt_id, &rows);
	clientCloseRows(&rows);

	return MUNIT_OK;
}