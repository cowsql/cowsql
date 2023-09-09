#include "../include/cowsql.h"

#include "vfs.h"

int cowsql_version_number(void)
{
	return COWSQL_VERSION_NUMBER;
}

int cowsql_vfs_init(sqlite3_vfs *vfs, const char *name)
{
	return VfsInit(vfs, name);
}

void cowsql_vfs_close(sqlite3_vfs *vfs)
{
	VfsClose(vfs);
}

int cowsql_vfs_poll(sqlite3_vfs *vfs,
		    const char *filename,
		    cowsql_vfs_frame **frames,
		    unsigned *n)
{
	return VfsPoll(vfs, filename, frames, n);
}

int cowsql_vfs_apply(sqlite3_vfs *vfs,
		     const char *filename,
		     unsigned n,
		     unsigned long *page_numbers,
		     void *frames)
{
	return VfsApply(vfs, filename, n, page_numbers, frames);
}

int cowsql_vfs_abort(sqlite3_vfs *vfs, const char *filename)
{
	return VfsAbort(vfs, filename);
}

int cowsql_vfs_snapshot(sqlite3_vfs *vfs,
			const char *filename,
			void **data,
			size_t *n)
{
	return VfsSnapshot(vfs, filename, data, n);
}

int cowsql_vfs_num_pages(sqlite3_vfs *vfs, const char *filename, unsigned *n)
{
	return VfsDatabaseNumPages(vfs, filename, n);
}

int cowsql_vfs_shallow_snapshot(sqlite3_vfs *vfs,
				const char *filename,
				struct cowsql_buffer bufs[],
				unsigned n)
{
	return VfsShallowSnapshot(vfs, filename, bufs, n);
}

int cowsql_vfs_restore(sqlite3_vfs *vfs,
		       const char *filename,
		       const void *data,
		       size_t n)
{
	return VfsRestore(vfs, filename, data, n);
}
