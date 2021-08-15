#include "postgres.h"

#include "fmgr.h"
#include "funcapi.h"
#include "access/tupdesc.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_getenv);
Datum pg_getenv(PG_FUNCTION_ARGS);

extern char **environ;

Datum
pg_getenv(PG_FUNCTION_ARGS)
{
	FuncCallContext	*funcctx;
	int				 call_cntr,
					 max_calls;
	TupleDesc		 tupdesc;
	AttInMetadata	*attinmeta;

	char		   **env = environ;

	if (SRF_IS_FIRSTCALL())
	{
		MemoryContext	oldcontext;
		funcctx = SRF_FIRSTCALL_INIT();

		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		funcctx->max_calls = PG_GETARG_UINT32(0);

		if (get_call_result_type(fcinfo, NULL, &tupdesc) != TYPEFUNC_COMPOSITE)
			ereport(ERROR,
					(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
					 errmsg("function returning record called in context "
							" that cannot accept type record")));

		attinmeta = TupleDescGetAttInMetadata(tupdesc);
		funcctx->attinmeta = attinmeta;

		MemoryContextSwitchTo(oldcontext);
	}

	funcctx = SRF_PERCALL_SETUP();

	call_cntr = funcctx->call_cntr;
	max_calls = funcctx->max_calls;
	attinmeta = funcctx->attinmeta;

	if (call_cntr < max_calls && *env)
	{
		char		**values;
		HeapTuple	  tuple;
		Datum		  result;
		char		 *name, *value;
		char		 *e = *env++;
		char		 *eq;

		/* Get what's on either side of the first equals sign into n and v. */
		eq = strchr(e, '=');
		*eq = '\0';
		name = e;
		value = eq + 1;

		values = (char **) palloc(2 * sizeof(char *));
		values[0] = (char *) palloc(256 * sizeof(char));
		values[1] = (char *) palloc(256 * sizeof(char));

		snprintf(values[0], 256, "%s", name);
		snprintf(values[1], 256, "%s", value);

		/* Build tuple */
		tuple = BuildTupleFromCStrings(attinmeta, values);

		/* Turn it into a datum */
		result = HeapTupleGetDatum(tuple);

		pfree(values[0]);
		pfree(values[1]);
		pfree(values);

		SRF_RETURN_NEXT(funcctx, result);
	}
	else
	{
		SRF_RETURN_DONE(funcctx);
	}
}
