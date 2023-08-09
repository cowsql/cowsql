#ifndef COWSQL_TEST_FS_H
#define COWSQL_TEST_FS_H

#define TEST__DIR_TEMPLATE "/tmp/cowsql-test-XXXXXX"

/* Setup a temporary directory. */
char *test_dir_setup(void);

/* Remove the temporary directory. */
void test_dir_tear_down(char *dir);

#endif /* COWSQL_TEST_FS_H */
