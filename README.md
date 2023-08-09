CowSQL [![CI Tests](https://github.com/cowsql/cowsql/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/cowsql/cowsql/actions/workflows/build-and-test.yml) [![codecov](https://codecov.io/gh/cowsql/cowsql/branch/master/graph/badge.svg)](https://codecov.io/gh/cowsql/cowsql)
======

[English](./README.md)|[简体中文](./README_CH.md)

[CowSQL](https://cowsql.dev) is a C library that implements an embeddable and
replicated SQL database engine with high availability and automatic failover.

CowSQL extends [SQLite](https://sqlite.org/) with a network protocol that can
connect together various instances of your application and have them act as a
highly-available cluster, with no dependency on external databases.

The name "CowSQL" loosely refers to the ["Pets
vs. Cattle"](https://iamondemand.com/blog/devops-concepts-pets-vs-cattle/)
concept, since it's generaly fine to delete or rebuild a particular node of an
application that uses CowSQL for data storage.

Fork of Canonical dqlite
------------------------

CowSQL is a fork of Canonical's [dqlite](https://github.com/canonical/dqlite)
project, which was originally written by CowSQL's author
[himself](https://github.com/canonical/dqlite/commits?author=freeekanayaka)
while working at Canonical.

Design highlights
----------------

* Asynchronous single-threaded implementation using [libuv](https://libuv.org/)
  as event loop.
* Custom wire protocol optimized for SQLite primitives and data types.
* Data replication based on the [Raft](https://raft.github.io/) algorithm and its
  efficient [C-raft](https://github.com/cowsql/raft) implementation.

License
-------

The CowSQL library is released under a slightly modified version of LGPLv3, that
includes a copyright exception allowing users to statically link the library
code in their project and release the final work under their own terms. See the
full [license](https://github.com/cowsql/cowsql/blob/master/LICENSE) text.

Compatibility
-------------

CowSQL runs on Linux and requires a kernel with support for [native async
I/O](https://man7.org/linux/man-pages/man2/io_setup.2.html) (not to be confused
with [POSIX AIO](https://man7.org/linux/man-pages/man7/aio.7.html)), which is
used by the libuv backend of C-raft.

Try it
-------

The simplest way to see CowSQL in action is to use the demo program that comes
with the Go CowSQL bindings. Please see the [relevant
documentation](https://github.com/cowsql/go-cowsql#demo) in that project.

Media
-----

There's no video about CowSQL itself yet, but a talk about dqlite was given at
FOSDEM 2020, you can watch it
[here](https://fosdem.org/2020/schedule/event/dqlite/).

[Here](https://gcore.com/blog/comparing-litestream-rqlite-dqlite/) is a blog post from 2022 comparing dqlite with rqlite and Litestream, other replication software for SQLite. It largly applies to CowSQL too.

Wire protocol
-------------

If you wish to write a client, please refer to the [wire protocol](https://dqlite.io/docs/protocol)
documentation.

Install
-------

If you are on a Debian-based system, you can get the latest development release from
CowSQL's [dev PPA](https://launchpad.net/~cowsql/+archive/ubuntu/master):

```
sudo add-apt-repository ppa:cowsql/master
sudo apt-get update
sudo apt-get install libcowsql-dev
```

Build
-----

To build libcowsql from source you'll need:

* A reasonably recent version of [libuv](http://libuv.org/) (v1.8.0 or beyond).
* A reasonably recent version of sqlite3-dev
* A build of the [C-raft](https://github.com/cowsql/raft) Raft library.

Your distribution should already provide you with a pre-built libuv shared
library and libsqlite3-dev.

To build the raft library:

```
git clone https://github.com/cowsql/raft.git
cd raft
autoreconf -i
./configure
make
sudo make install
cd ..
```

Once all the required libraries are installed, in order to build the CowSQL
shared library itself, you can run:

```
autoreconf -i
./configure
make
sudo make install
```
