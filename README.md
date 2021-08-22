pg_getenv
======

A set-returning function to return all the server's shell environment variables.

To build it, you'll need the usual things for developing PostgreSQL extensions.

Once pg_getenv is installed, you can add it to a database.

    CREATE EXTENSION pg_getenv;

This provides one function, `pg_getenv()`, which is executable only by
database superusers. It returns all the server's environment variables.

Dependencies
------------
The `pg_getenv` extension has no dependencies other than PostgreSQL.

Copyright and License
---------------------

Copyright (c) 2021 David Fetter.
