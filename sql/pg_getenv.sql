/*
 * Author: David Fetter
 * Created at: 2021-08-14 22:07:48 -0700
 *
 */

CREATE OR REPLACE FUNCTION pg_getenv(OUT name TEXT, OUT val TEXT)
RETURNS SETOF RECORD
AS 'pg_getenv'
LANGUAGE C IMMUTABLE;
