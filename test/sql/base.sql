\set ECHO none
BEGIN;
\i sql/pg_getenv.sql
\set ECHO all

-- You should write your tests

SELECT pg_getenv();

ROLLBACK;
