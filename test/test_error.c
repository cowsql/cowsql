#include <uv.h>

#include "../include/cowsql.h"
#include "../src/error.h"

#include "./lib/heap.h"
#include "./lib/runner.h"
#include "./lib/sqlite.h"

TEST_MODULE(error);

/******************************************************************************
 *
 * Setup and tear down
 *
 ******************************************************************************/

static void *setup(const MunitParameter params[], void *user_data)
{
	cowsql__error *error;

	test_heap_setup(params, user_data);
	test_sqlite_setup(params);

	error = (cowsql__error *)munit_malloc(sizeof(*error));

	cowsql__error_init(error);

	return error;
}

static void tear_down(void *data)
{
	cowsql__error *error = data;

	cowsql__error_close(error);

	test_sqlite_tear_down();
	test_heap_tear_down(data);

	free(error);
}

/******************************************************************************
 *
 * cowsql__error_printf
 *
 ******************************************************************************/

TEST_SUITE(printf);
TEST_SETUP(printf, setup);
TEST_TEAR_DOWN(printf, tear_down);

TEST_CASE(printf, success, NULL)
{
	cowsql__error *error = data;

	(void)params;

	munit_assert_true(cowsql__error_is_null(error));

	cowsql__error_printf(error, "hello %s", "world");

	munit_assert_string_equal(*error, "hello world");

	return MUNIT_OK;
}

TEST_CASE(printf, override, NULL)
{
	cowsql__error *error = data;

	(void)params;

	cowsql__error_printf(error, "hello %s", "world");
	cowsql__error_printf(error, "I'm %s!", "here");

	munit_assert_string_equal(*error, "I'm here!");

	return MUNIT_OK;
}

TEST_CASE(printf, oom, NULL)
{
	cowsql__error *error = data;

	(void)params;

	test_heap_fault_config(0, 1);
	test_heap_fault_enable();

	cowsql__error_printf(error, "hello %s", "world");

	munit_assert_string_equal(*error,
				  "error message unavailable (out of memory)");

	return MUNIT_OK;
}

/******************************************************************************
 *
 * cowsql__error_wrapf
 *
 ******************************************************************************/

TEST_SUITE(wrapf);
TEST_SETUP(wrapf, setup);
TEST_TEAR_DOWN(wrapf, tear_down);

TEST_CASE(wrapf, success, NULL)
{
	cowsql__error *error = data;
	cowsql__error cause;

	(void)params;

	cowsql__error_init(&cause);

	cowsql__error_printf(&cause, "hello %s", "world");

	cowsql__error_wrapf(error, &cause, "boom");

	cowsql__error_close(&cause);

	munit_assert_string_equal(*error, "boom: hello world");

	return MUNIT_OK;
}

TEST_CASE(wrapf, null_cause, NULL)
{
	cowsql__error *error = data;
	cowsql__error cause;

	(void)params;

	cowsql__error_init(&cause);

	cowsql__error_wrapf(error, &cause, "boom");

	cowsql__error_close(&cause);

	munit_assert_string_equal(*error, "boom: (null)");

	return MUNIT_OK;
}

TEST_CASE(wrapf, itself, NULL)
{
	cowsql__error *error = data;

	(void)params;

	cowsql__error_printf(error, "I'm %s!", "here");

	cowsql__error_wrapf(error, error, "boom");

	munit_assert_string_equal(*error, "boom: I'm here!");

	return MUNIT_OK;
}

/******************************************************************************
 *
 * cowsql__error_oom
 *
 ******************************************************************************/

TEST_SUITE(oom);
TEST_SETUP(oom, setup);
TEST_TEAR_DOWN(oom, tear_down);

TEST_CASE(oom, success, NULL)
{
	cowsql__error *error = data;

	(void)params;

	cowsql__error_oom(error, "boom");

	munit_assert_string_equal(*error, "boom: out of memory");

	return MUNIT_OK;
}

TEST_CASE(oom, vargs, NULL)
{
	cowsql__error *error = data;

	(void)params;

	cowsql__error_oom(error, "boom %d", 123);

	munit_assert_string_equal(*error, "boom 123: out of memory");

	return MUNIT_OK;
}

/******************************************************************************
 *
 * cowsql__error_sys
 *
 ******************************************************************************/

TEST_SUITE(sys);
TEST_SETUP(sys, setup);
TEST_TEAR_DOWN(sys, tear_down);

TEST_CASE(sys, success, NULL)
{
	cowsql__error *error = data;

	(void)params;

	open("/foo/bar/egg/baz", 0);
	cowsql__error_sys(error, "boom");

	munit_assert_string_equal(*error, "boom: No such file or directory");

	return MUNIT_OK;
}

/******************************************************************************
 *
 * cowsql__error_uv
 *
 ******************************************************************************/

TEST_SUITE(uv);
TEST_SETUP(uv, setup);
TEST_TEAR_DOWN(uv, tear_down);

TEST_CASE(uv, success, NULL)
{
	cowsql__error *error = data;

	(void)params;

	cowsql__error_uv(error, UV_EBUSY, "boom");

	munit_assert_string_equal(*error,
				  "boom: resource busy or locked (EBUSY)");

	return MUNIT_OK;
}

/******************************************************************************
 *
 * cowsql__error_copy
 *
 ******************************************************************************/

TEST_SUITE(copy);
TEST_SETUP(copy, setup);
TEST_TEAR_DOWN(copy, tear_down);

TEST_CASE(copy, success, NULL)
{
	cowsql__error *error = data;
	int err;
	char *msg;

	(void)params;

	cowsql__error_printf(error, "hello %s", "world");
	err = cowsql__error_copy(error, &msg);

	munit_assert_int(err, ==, 0);
	munit_assert_string_equal(msg, "hello world");

	sqlite3_free(msg);

	return MUNIT_OK;
}

TEST_CASE(copy, null, NULL)
{
	cowsql__error *error = data;
	int err;
	char *msg;

	(void)params;

	err = cowsql__error_copy(error, &msg);

	munit_assert_int(err, ==, COWSQL_ERROR);
	munit_assert_ptr_equal(msg, NULL);

	return MUNIT_OK;
}

TEST_CASE(copy, oom, NULL)
{
	cowsql__error *error = data;
	int err;
	char *msg;

	(void)params;
	return MUNIT_SKIP;

	test_heap_fault_config(2, 1);
	test_heap_fault_enable();

	cowsql__error_printf(error, "hello");

	err = cowsql__error_copy(error, &msg);

	munit_assert_int(err, ==, COWSQL_NOMEM);
	munit_assert_ptr_equal(msg, NULL);

	return MUNIT_OK;
}

/******************************************************************************
 *
 * cowsql__error_is_disconnect
 *
 ******************************************************************************/

TEST_SUITE(is_disconnect);
TEST_SETUP(is_disconnect, setup);
TEST_TEAR_DOWN(is_disconnect, tear_down);

TEST_CASE(is_disconnect, eof, NULL)
{
	cowsql__error *error = data;

	(void)params;

	cowsql__error_uv(error, UV_EOF, "boom");

	munit_assert_true(cowsql__error_is_disconnect(error));

	return MUNIT_OK;
}

TEST_CASE(is_disconnect, econnreset, NULL)
{
	cowsql__error *error = data;

	(void)params;

	cowsql__error_uv(error, UV_ECONNRESET, "boom");

	munit_assert_true(cowsql__error_is_disconnect(error));

	return MUNIT_OK;
}

TEST_CASE(is_disconnect, other, NULL)
{
	cowsql__error *error = data;

	(void)params;

	cowsql__error_printf(error, "boom");

	munit_assert_true(!cowsql__error_is_disconnect(error));

	return MUNIT_OK;
}

TEST_CASE(is_disconnect, null, NULL)
{
	cowsql__error *error = data;

	(void)params;

	munit_assert_true(!cowsql__error_is_disconnect(error));

	return MUNIT_OK;
}
