#include "postgres.h"

#include "access/tupdesc.h"
#include "fmgr.h"
#include "funcapi.h"
#include "utils/builtins.h"
#include "utils/guc.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_getenv);
Datum pg_getenv(PG_FUNCTION_ARGS);

extern char **environ;

Datum
pg_getenv(PG_FUNCTION_ARGS)
{
	/* Input parameters (PG_GETARG_* go here, but not yet */

	ReturnSetInfo	   *rsinfo  =(ReturnSetInfo *) fcinfo->resultinfo;
	bool				randomAccess;
	int					maxKBytes = 1024;
	TupleDesc			tupdesc;
	Tuplestorestate	   *tupstore;
	MemoryContext		oldcontext;

	/* Stash environ's head here */
	char			  **env = environ;

	/* Must be superuser because duh */
	if (!session_auth_is_superuser)
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 errmsg("must be superuser to see server environment variables")));

	/* It's a SRF, so kinda need to be that */
	if (rsinfo == NULL || !IsA(rsinfo, ReturnSetInfo))
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("set-valued function called in context that cannot accept a set")));

	/* Can we actually return a tuplestore? */
	if (!(rsinfo->allowedModes & SFRM_Materialize))
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("materialize mode required, but not allowed in this context")));

	oldcontext = MemoryContextSwitchTo(rsinfo->econtext->ecxt_per_query_memory);

	if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
		elog(ERROR, "return type must be a row type");

	randomAccess = (rsinfo->allowedModes & SFRM_Materialize_Random) != 0;
	tupstore = tuplestore_begin_heap(randomAccess, false, maxKBytes);
	rsinfo->returnMode = SFRM_Materialize;
	rsinfo->setResult = tupstore;
	rsinfo->setDesc = tupdesc;

	MemoryContextSwitchTo(oldcontext);

	/* Get everything out of the environment and into tuples */
	while (*environ)
	{
		Datum		values[2];
		bool		nulls[2] = {false};
		char	   *name_val = palloc(256 * sizeof(char));
		char	   *ptr;
		char	   *name;

		snprintf(name_val, 256, "%s", *environ);
		environ++;

		name = strtok_r(name_val, "=", &ptr);

		values[0] = CStringGetTextDatum(name);
		values[1] = CStringGetTextDatum(ptr);

		tuplestore_putvalues(tupstore, tupdesc, values, nulls);
	}

	environ = env;

	return (Datum) 0;
}
