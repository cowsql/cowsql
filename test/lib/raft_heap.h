/**
 * Helpers for injecting failures into raft's allocator.
 */

#ifndef COWSQL_TEST_RAFT_HEAP_H
#define COWSQL_TEST_RAFT_HEAP_H

#include "munit.h"

void test_raft_heap_setup(const MunitParameter params[], void *user_data);
void test_raft_heap_tear_down(void *data);
void test_raft_heap_fault_config(int delay, int repeat);
void test_raft_heap_fault_enable(void);

#endif /* COWSQL_TEST_RAFT_HEAP_H */
