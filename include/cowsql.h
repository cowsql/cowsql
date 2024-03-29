#ifndef COWSQL_H
#define COWSQL_H

#include <sqlite3.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef COWSQL_API
#define COWSQL_API __attribute__((visibility("default")))
#endif

/**
 * This "pseudo-attribute" marks declarations that are only a provisional part
 * of the cowsql public API. These declarations may change or be removed
 * entirely in minor or point releases of cowsql, without bumping the soversion
 * of libcowsql.so. Consumers of cowsql who use these declarations are
 * responsible for updating their code in response to such breaking changes.
 */
#define COWSQL_EXPERIMENTAL

/* XXX */
#define COWSQL_VISIBLE_TO_TESTS COWSQL_API

/**
 * Version.
 */
#define COWSQL_VERSION_MAJOR 1
#define COWSQL_VERSION_MINOR 15
#define COWSQL_VERSION_RELEASE 5
#define COWSQL_VERSION_NUMBER                                            \
	(COWSQL_VERSION_MAJOR * 100 * 100 + COWSQL_VERSION_MINOR * 100 + \
	 COWSQL_VERSION_RELEASE)

COWSQL_API int cowsql_version_number(void);

/**
 * Hold the value of a cowsql node ID. Guaranteed to be at least 64-bit long.
 */
typedef unsigned long long cowsql_node_id;

COWSQL_EXPERIMENTAL typedef struct cowsql_server cowsql_server;

/**
 * Signature of a custom callback used to establish network connections
 * to cowsql servers.
 *
 * @arg is a user data parameter, copied from the third argument of
 * cowsql_server_set_connect_func. @addr is a (borrowed) abstract address
 * string, as passed to cowsql_server_create or cowsql_server_set_auto_join. @fd
 * is an address where a socket representing the connection should be stored.
 * The callback should return zero if a connection was established successfully
 * or nonzero if the attempt failed.
 */
COWSQL_EXPERIMENTAL typedef int (*cowsql_connect_func)(void *arg,
						       const char *addr,
						       int *fd);

/* The following cowsql_server functions return zero on success or nonzero on
 * error. More specific error codes may be specified in the future. */

/**
 * Start configuring a cowsql server.
 *
 * The server will not start running until cowsql_server_start is called. @path
 * is the path to a directory where the server (and attached client) will store
 * its persistent state; the directory must exist. A pointer to the new server
 * object is stored in @server on success.
 *
 * Whether or not this function succeeds, you should call cowsql_server_destroy
 * to release resources owned by the server object.
 *
 * No reference to @path is kept after this function returns.
 */
COWSQL_API COWSQL_EXPERIMENTAL int cowsql_server_create(const char *path,
							cowsql_server **server);

/**
 * Set the abstract address of this server.
 *
 * This function must be called when the server starts for the first time, and
 * is a no-op when the server is restarting. The abstract address is recorded in
 * the Raft log and passed to the connect function on each server (see
 * cowsql_server_set_connect_func). The server will also bind to this address to
 * listen for incoming connections from clients and other servers, unless
 * cowsql_server_set_bind_address is used. For the address syntax accepted by
 * the default connect function (and for binding/listening), see
 * cowsql_server_set_bind_address.
 */
COWSQL_API COWSQL_EXPERIMENTAL int cowsql_server_set_address(
    cowsql_server *server,
    const char *address);

/**
 * Turn on or off automatic bootstrap for this server.
 *
 * The bootstrap server should be the first to start up. It automatically
 * becomes the leader in the first term, and is responsible for adding all other
 * servers to the cluster configuration. There must be exactly one bootstrap
 * server in each cluster. After the first startup, the bootstrap server is no
 * longer special and this function is a no-op.
 */
COWSQL_API COWSQL_EXPERIMENTAL int cowsql_server_set_auto_bootstrap(
    cowsql_server *server,
    bool on);

/**
 * Declare the addresses of existing servers in the cluster, which should
 * already be running.
 *
 * The server addresses declared with this function will not be used unless
 * @server is starting up for the first time; after the first startup, the list
 * of servers stored on disk will be used instead. (It is harmless to call this
 * function unconditionally.)
 */
COWSQL_API COWSQL_EXPERIMENTAL int cowsql_server_set_auto_join(
    cowsql_server *server,
    const char *const *addrs,
    unsigned n);

/**
 * Configure @server to listen on the address @addr for incoming connections
 * (from clients and other servers).
 *
 * If no bind address is configured with this function, the abstract address
 * passed to cowsql_server_create will be used. The point of this function is to
 * support decoupling the abstract address from the networking implementation
 * (for example, if a proxy is going to be used).
 *
 * @addr must use one of the following formats:
 *
 * 1. "<HOST>"
 * 2. "<HOST>:<PORT>"
 * 3. "@<PATH>"
 *
 * Where <HOST> is a numeric IPv4/IPv6 address, <PORT> is a port number, and
 * <PATH> is an abstract Unix socket path. The port number defaults to 8080 if
 * not specified. In the second form, if <HOST> is an IPv6 address, it must be
 * enclosed in square brackets "[]". In the third form, if <PATH> is empty, the
 * implementation will automatically select an available abstract Unix socket
 * path.
 *
 * If an abstract Unix socket is used, the server will accept only
 * connections originating from the same process.
 */
COWSQL_API COWSQL_EXPERIMENTAL int cowsql_server_set_bind_address(
    cowsql_server *server,
    const char *addr);

/**
 * Configure the function that this server will use to connect to other servers.
 *
 * The same function will be used by the server's attached client to establish
 * connections to all servers in the cluster. @arg is a user data parameter that
 * will be passed to all invocations of the connect function.
 */
COWSQL_API COWSQL_EXPERIMENTAL int cowsql_server_set_connect_func(
    cowsql_server *server,
    cowsql_connect_func f,
    void *arg);

/**
 * Start running the server.
 *
 * Once this function returns successfully, the server will be ready to accept
 * client requests using the functions below.
 */
COWSQL_API COWSQL_EXPERIMENTAL int cowsql_server_start(cowsql_server *server);

/**
 * Get the ID of the server.
 *
 * This will return 0 (an invalid ID) if the server has not been started.
 */
COWSQL_API COWSQL_EXPERIMENTAL cowsql_node_id
cowsql_server_get_id(cowsql_server *server);

/**
 * Hand over the server's privileges to other servers.
 *
 * This is intended to be called before cowsql_server_stop. The server will try
 * to surrender leadership and voting rights to other nodes in the cluster, if
 * applicable. This avoids some disruptions that can result when a privileged
 * server stops suddenly.
 */
COWSQL_API COWSQL_EXPERIMENTAL int cowsql_server_handover(
    cowsql_server *server);

/**
 * Stop the server.
 *
 * The server will stop processing requests from client or other servers. To
 * smooth over some possible disruptions to the cluster, call
 * cowsql_server_handover before this function. After this function returns
 * (successfully or not), you should call cowsql_server_destroy to free
 * resources owned by the server.
 */
COWSQL_API COWSQL_EXPERIMENTAL int cowsql_server_stop(cowsql_server *server);

/**
 * Free resources owned by the server.
 *
 * You should always call this function to finalize a server created with
 * cowsql_server_create, whether or not that function returned successfully.
 * If the server has been successfully started with cowsql_server_start,
 * then you must stop it with cowsql_server_stop before calling this function.
 */
COWSQL_API COWSQL_EXPERIMENTAL void cowsql_server_destroy(
    cowsql_server *server);

/**
 * Error codes.
 *
 * These are used only with the cowsql_node family of functions.
 */
enum {
	COWSQL_ERROR = 1, /* Generic error */
	COWSQL_MISUSE,    /* Library used incorrectly */
	COWSQL_NOMEM      /* A malloc() failed */
};

/**
 * Cowsql node handle.
 *
 * Opaque handle to a single cowsql node that can serve database requests from
 * connected clients and exchanges data replication messages with other cowsql
 * nodes.
 */
typedef struct cowsql_node cowsql_node;

/**
 * Create a new cowsql node object.
 *
 * The @id argument a is positive number that identifies this particular cowsql
 * node in the cluster. Each cowsql node part of the same cluster must be
 * created with a different ID. The very first node, used to bootstrap a new
 * cluster, must have ID #1. Every time a node is started again, it must be
 * passed the same ID.

 * The @address argument is the network address that clients or other nodes in
 * the cluster must use to connect to this cowsql node. If no custom connect
 * function is going to be set using cowsql_node_set_connect_func(), then the
 * format of the string must be "<HOST>" or "<HOST>:<PORT">, where <HOST> is a
 * numeric IPv4/IPv6 address and <PORT> is a port number. The port number
 * defaults to 8080 if not specified. If a port number is specified with an
 * IPv6 address, the address must be enclosed in square brackets "[]".
 *
 * If a custom connect function is used, then the format of the string must by
 * whatever the custom connect function accepts.
 *
 * The @data_dir argument the file system path where the node should store its
 * durable data, such as Raft log entries containing WAL frames of the SQLite
 * databases being replicated.
 *
 * No reference to the memory pointed to by @address and @data_dir is kept by
 * the cowsql library, so any memory associated with them can be released after
 * the function returns.
 *
 * Even if an error is returned, the caller should call cowsql_node_destroy()
 * on the cowsql_node* value pointed to by @n, and calling cowsql_node_errmsg()
 * with that value will return a valid error string. (In some cases *n will be
 * set to NULL, but cowsql_node_destroy() and cowsql_node_errmsg() will handle
 * this gracefully.)
 */
COWSQL_API int cowsql_node_create(cowsql_node_id id,
				  const char *address,
				  const char *data_dir,
				  cowsql_node **n);

/**
 * Destroy a cowsql node object.
 *
 * This will release all memory that was allocated by the node. If
 * cowsql_node_start() was successfully invoked, then cowsql_node_stop() must be
 * invoked before destroying the node.
 */
COWSQL_API void cowsql_node_destroy(cowsql_node *n);

/**
 * Instruct the cowsql node to bind a network address when starting, and
 * listening for incoming client connections.
 *
 * The given address might match the one passed to @cowsql_node_create or be a
 * different one (for example if the application wants to proxy it).
 *
 * The format of the @address argument must be one of
 *
 * 1. "<HOST>"
 * 2. "<HOST>:<PORT>"
 * 3. "@<PATH>"
 *
 * Where <HOST> is a numeric IPv4/IPv6 address, <PORT> is a port number, and
 * <PATH> is an abstract Unix socket path. The port number defaults to 8080 if
 * not specified. In the second form, if <HOST> is an IPv6 address, it must be
 * enclosed in square brackets "[]". In the third form, if <PATH> is empty, the
 * implementation will automatically select an available abstract Unix socket
 * path, which can then be retrieved with cowsql_node_get_bind_address().
 *
 * If an abstract Unix socket is used the cowsql node will accept only
 * connections originating from the same process.
 *
 * No reference to the memory pointed to by @address is kept, so any memory
 * associated with them can be released after the function returns.
 *
 * This function must be called before calling cowsql_node_start().
 */
COWSQL_API int cowsql_node_set_bind_address(cowsql_node *n,
					    const char *address);

/**
 * Get the network address that the cowsql node is using to accept incoming
 * connections.
 */
COWSQL_API const char *cowsql_node_get_bind_address(cowsql_node *n);

/**
 * Set a custom connect function.
 *
 * The function should block until a network connection with the cowsql node at
 * the given @address is established, or an error occurs.
 *
 * In case of success, the file descriptor of the connected socket must be saved
 * into the location pointed by the @fd argument. The socket must be either a
 * TCP or a Unix socket.
 *
 * This function must be called before calling cowsql_node_start().
 */
COWSQL_API int cowsql_node_set_connect_func(cowsql_node *n,
					    int (*f)(void *arg,
						     const char *address,
						     int *fd),
					    void *arg);

/**
 * DEPRECATED - USE `cowsql_node_set_network_latency_ms`
 * Set the average one-way network latency, expressed in nanoseconds.
 *
 * This value is used internally by cowsql to decide how frequently the leader
 * node should send heartbeats to other nodes in order to maintain its
 * leadership, and how long other nodes should wait before deciding that the
 * leader has died and initiate a failover.
 *
 * This function must be called before calling cowsql_node_start().
 */
COWSQL_API int cowsql_node_set_network_latency(cowsql_node *n,
					       unsigned long long nanoseconds);

/**
 * Set the average one-way network latency, expressed in milliseconds.
 *
 * This value is used internally by cowsql to decide how frequently the leader
 * node should send heartbeats to other nodes in order to maintain its
 * leadership, and how long other nodes should wait before deciding that the
 * leader has died and initiate a failover.
 *
 * This function must be called before calling cowsql_node_start().
 *
 * Latency should not be 0 or larger than 3600000 milliseconds.
 */
COWSQL_API int cowsql_node_set_network_latency_ms(cowsql_node *t,
						  unsigned milliseconds);

/**
 * Set the failure domain associated with this node.
 *
 * This is effectively a tag applied to the node and that can be inspected later
 * with the "Describe node" client request.
 */
COWSQL_API int cowsql_node_set_failure_domain(cowsql_node *n,
					      unsigned long long code);

/**
 * Set the snapshot parameters for this node.
 *
 * This function determines how frequently a node will snapshot the state
 * of the database and how many raft log entries will be kept around after
 * a snapshot has been taken.
 *
 * `snapshot_threshold` : Determines the frequency of taking a snapshot, the
 * lower the number, the higher the frequency.
 *
 * `snapshot_trailing` : Determines the amount of log entries kept around after
 * taking a snapshot. Lowering this number decreases disk and memory footprint
 * but increases the chance of having to send a full snapshot (instead of a
 * number of log entries to a node that has fallen behind.
 *
 * This function must be called before calling cowsql_node_start().
 */
COWSQL_API int cowsql_node_set_snapshot_params(cowsql_node *n,
					       unsigned snapshot_threshold,
					       unsigned snapshot_trailing);

/**
 * Set the block size used for performing disk IO when writing raft log segments
 * to disk. @size is limited to a list of preset values.
 *
 * This function must be called before calling cowsql_node_start().
 */
COWSQL_API int cowsql_node_set_block_size(cowsql_node *n, size_t size);

/**
 * Set the target number of voting nodes for the cluster.
 *
 * If automatic role management is enabled, the cluster leader will attempt to
 * promote nodes to reach the target. If automatic role management is disabled,
 * this has no effect.
 *
 * The default target is 3 voters.
 */
COWSQL_API int cowsql_node_set_target_voters(cowsql_node *n, int voters);

/**
 * Set the target number of standby nodes for the cluster.
 *
 * If automatic role management is enabled, the cluster leader will attempt to
 * promote nodes to reach the target. If automatic role management is disabled,
 * this has no effect.
 *
 * The default target is 0 standbys.
 */
COWSQL_API int cowsql_node_set_target_standbys(cowsql_node *n, int standbys);

/**
 * Enable or disable auto-recovery for corrupted disk files.
 *
 * When auto-recovery is enabled, files in the data directory that are
 * determined to be corrupt may be removed by cowsql at startup. This allows
 * the node to start up successfully in more situations, but comes at the cost
 * of possible data loss, and may mask bugs.
 *
 * This must be called before cowsql_node_start.
 *
 * Auto-recovery is enabled by default.
 */
COWSQL_API int cowsql_node_set_auto_recovery(cowsql_node *n, bool enabled);

/**
 * Enable automatic role management on the server side for this node.
 *
 * When automatic role management is enabled, servers in a cowsql cluster will
 * autonomously (without client intervention) promote and demote each other
 * to maintain a specified number of voters and standbys, taking into account
 * the health, failure domain, and weight of each server.
 *
 * By default, no automatic role management is performed.
 */
COWSQL_API int cowsql_node_enable_role_management(cowsql_node *n);

/**
 * Start a cowsql node.
 *
 * A background thread will be spawned which will run the node's main loop. If
 * this function returns successfully, the cowsql node is ready to accept new
 * connections.
 */
COWSQL_API int cowsql_node_start(cowsql_node *n);

/**
 * Attempt to hand over this node's privileges to other nodes in preparation
 * for a graceful shutdown.
 *
 * Specifically, if this node is the cluster leader, this will cause another
 * voting node (if one exists) to be elected leader; then, if this node is a
 * voter, another non-voting node (if one exists) will be promoted to voter, and
 * then this node will be demoted to spare.
 *
 * This function returns 0 if all privileges were handed over successfully,
 * and nonzero otherwise. Callers can continue to cowsql_node_stop immediately
 * after this function returns (whether or not it succeeded), or include their
 * own graceful shutdown logic before cowsql_node_stop.
 */
COWSQL_API int cowsql_node_handover(cowsql_node *n);

/**
 * Stop a cowsql node.
 *
 * The background thread running the main loop will be notified and the node
 * will not accept any new client connections. Once inflight requests are
 * completed, open client connections get closed and then the thread exits.
 */
COWSQL_API int cowsql_node_stop(cowsql_node *n);

struct cowsql_node_info
{
	cowsql_node_id id;
	const char *address;
};
typedef struct cowsql_node_info cowsql_node_info;

/* Defined to be an extensible struct, future additions to this struct should be
 * 64-bits wide and 0 should not be used as a valid value. */
struct cowsql_node_info_ext
{
	uint64_t size; /* The size of this struct */
	uint64_t id;   /* cowsql_node_id */
	uint64_t address;
	uint64_t cowsql_role;
};
typedef struct cowsql_node_info_ext cowsql_node_info_ext;
#define COWSQL_NODE_INFO_EXT_SZ_ORIG 32U /* (4 * 64) / 8 */

/**
 * !!! Deprecated, use `cowsql_node_recover_ext` instead which also includes
 * cowsql roles. !!!
 *
 * Force recovering a cowsql node which is part of a cluster whose majority of
 * nodes have died, and therefore has become unavailable.
 *
 * In order for this operation to be safe you must follow these steps:
 *
 * 1. Make sure no cowsql node in the cluster is running.
 *
 * 2. Identify all cowsql nodes that have survived and that you want to be part
 *    of the recovered cluster.
 *
 * 3. Among the survived cowsql nodes, find the one with the most up-to-date
 *    raft term and log.
 *
 * 4. Invoke @cowsql_node_recover exactly one time, on the node you found in
 *    step 3, and pass it an array of #cowsql_node_info filled with the IDs and
 *    addresses of the survived nodes, including the one being recovered.
 *
 * 5. Copy the data directory of the node you ran @cowsql_node_recover on to all
 *    other non-dead nodes in the cluster, replacing their current data
 *    directory.
 *
 * 6. Restart all nodes.
 */
COWSQL_API int cowsql_node_recover(cowsql_node *n,
				   cowsql_node_info infos[],
				   int n_info);

/**
 * Force recovering a cowsql node which is part of a cluster whose majority of
 * nodes have died, and therefore has become unavailable.
 *
 * In order for this operation to be safe you must follow these steps:
 *
 * 1. Make sure no cowsql node in the cluster is running.
 *
 * 2. Identify all cowsql nodes that have survived and that you want to be part
 *    of the recovered cluster.
 *
 * 3. Among the survived cowsql nodes, find the one with the most up-to-date
 *    raft term and log.
 *
 * 4. Invoke @cowsql_node_recover_ext exactly one time, on the node you found in
 *    step 3, and pass it an array of #cowsql_node_info filled with the IDs,
 *    addresses and roles of the survived nodes, including the one being
 *    recovered.
 *
 * 5. Copy the data directory of the node you ran @cowsql_node_recover_ext on to
 *    all other non-dead nodes in the cluster, replacing their current data
 *    directory.
 *
 * 6. Restart all nodes.
 */
COWSQL_API int cowsql_node_recover_ext(cowsql_node *n,
				       cowsql_node_info_ext infos[],
				       int n_info);

/**
 * Return a human-readable description of the last error occurred.
 */
COWSQL_API const char *cowsql_node_errmsg(cowsql_node *n);

/**
 * Generate a unique ID for the given address.
 */
COWSQL_API cowsql_node_id cowsql_generate_node_id(const char *address);

/**
 * This function is DEPRECATED and will be removed in a future major release.
 *
 * Initialize the given SQLite VFS interface object with cowsql's custom
 * implementation, which can be used for replication.
 */
COWSQL_API int cowsql_vfs_init(sqlite3_vfs *vfs, const char *name);

/**
 * This function is DEPRECATED and will be removed in a future major release.
 *
 * Release all memory used internally by a SQLite VFS object that was
 * initialized using @qlite_vfs_init.
 */
COWSQL_API void cowsql_vfs_close(sqlite3_vfs *vfs);

/**
 * This type is DEPRECATED and will be removed in a future major release.
 *
 * A single WAL frame to be replicated.
 */
struct cowsql_vfs_frame
{
	unsigned long page_number; /* Database page number. */
	void *data;                /* Content of the database page. */
};
typedef struct cowsql_vfs_frame cowsql_vfs_frame;

/**
 * This function is DEPRECATED and will be removed in a future major release.
 *
 * Check if the last call to sqlite3_step() has triggered a write transaction on
 * the database with the given filename. In that case acquire a WAL write lock
 * to prevent further write transactions, and return all new WAL frames
 * generated by the transaction. These frames are meant to be replicated across
 * nodes and then actually added to the WAL with cowsql_vfs_apply() once a
 * quorum is reached. If a quorum is not reached within a given time, then
 * cowsql_vfs_abort() can be used to abort and release the WAL write lock.
 */
COWSQL_API int cowsql_vfs_poll(sqlite3_vfs *vfs,
			       const char *filename,
			       cowsql_vfs_frame **frames,
			       unsigned *n);

/**
 * This function is DEPRECATED and will be removed in a future major release.
 *
 * Add to the WAL all frames that were generated by a write transaction
 * triggered by sqlite3_step() and that were obtained via cowsql_vfs_poll().
 *
 * This interface is designed to match the typical use case of a node receiving
 * the frames by sequentially reading a byte stream from a network socket and
 * passing the data to this routine directly without any copy or futher
 * allocation, possibly except for integer encoding/decoding.
 */
COWSQL_API int cowsql_vfs_apply(sqlite3_vfs *vfs,
				const char *filename,
				unsigned n,
				unsigned long *page_numbers,
				void *frames);

/**
 * This function is DEPRECATED and will be removed in a future major release.
 *
 * Abort a pending write transaction that was triggered by sqlite3_step() and
 * whose frames were obtained via cowsql_vfs_poll().
 *
 * This should be called if the transaction could not be safely replicated. In
 * particular it will release the write lock acquired by cowsql_vfs_poll().
 */
COWSQL_API int cowsql_vfs_abort(sqlite3_vfs *vfs, const char *filename);

/**
 * This function is DEPRECATED and will be removed in a future major release.
 *
 * Return a snapshot of the main database file and of the WAL file.
 */
COWSQL_API int cowsql_vfs_snapshot(sqlite3_vfs *vfs,
				   const char *filename,
				   void **data,
				   size_t *n);

/**
 * This type is DEPRECATED and will be removed in a future major release.
 *
 * A data buffer.
 */
struct cowsql_buffer
{
	void *base; /* Pointer to the buffer data. */
	size_t len; /* Length of the buffer. */
};

/**
 * This function is DEPRECATED and will be removed in a future major release.
 *
 * Return a shallow snapshot of the main database file and of the WAL file.
 * Expects a bufs array of size x + 1, where x is obtained from
 * `cowsql_vfs_num_pages`.
 */
COWSQL_API int cowsql_vfs_shallow_snapshot(sqlite3_vfs *vfs,
					   const char *filename,
					   struct cowsql_buffer bufs[],
					   unsigned n);

/**
 * This function is DEPRECATED and will be removed in a future major release.
 *
 * Return the number of database pages (excluding WAL).
 */
COWSQL_API int cowsql_vfs_num_pages(sqlite3_vfs *vfs,
				    const char *filename,
				    unsigned *n);

/**
 * This function is DEPRECATED and will be removed in a future major release.
 *
 * Restore a snapshot of the main database file and of the WAL file.
 */
COWSQL_API int cowsql_vfs_restore(sqlite3_vfs *vfs,
				  const char *filename,
				  const void *data,
				  size_t n);

#endif /* COWSQL_H */
