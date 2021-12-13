/*-------------------------------------------------------------------------
 *
 * toasterapi.c
 *	  Support routines for API for Postgres PG_TOASTER methods.
 *
 * Copyright (c) 2015-2021, PostgreSQL Global Development Group
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/index/toasterapi.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/toasterapi.h"
#include "access/htup_details.h"
#include "catalog/pg_toaster.h"
#include "utils/builtins.h"
#include "utils/syscache.h"


/*
 * GetIndexTsrRoutine - call the specified toaster handler routine to get
 * its IndexTsrRoutine struct, which will be palloc'd in the caller's context.
 *
 */
IndexTsrRoutine *
GetIndexTsrRoutine(Oid tsrhandler)
{
	Datum		datum;
	IndexTsrRoutine *routine;

	datum = OidFunctionCall0(tsrhandler);
	routine = (IndexTsrRoutine *) DatumGetPointer(datum);

	if (routine == NULL || !IsA(routine, IndexTsrRoutine))
		elog(ERROR, "toaster access method handler function %u did not return an IndexTsrRoutine struct",
			 tsrhandler);

	return routine;
}

/*
 * GetIndexAmRoutineByAmId - look up the handler of the index access method
 * with the given OID, and get its IndexAmRoutine struct.
 *
 * If the given OID isn't a valid index access method, returns NULL if
 * noerror is true, else throws error.
 */
IndexTsrRoutine *
GetIndexTsrRoutineByAmId(Oid tsroid, bool noerror)
{
	HeapTuple	tuple;
	Form_pg_toaster	tsrform;
	regproc		tsrhandler;

	/* Get handler function OID for the access method */
	tuple = SearchSysCache1(TOASTEROID, ObjectIdGetDatum(tsroid));
	if (!HeapTupleIsValid(tuple))
	{
		if (noerror)
			return NULL;
		elog(ERROR, "cache lookup failed for toaster %u",
			 tsroid);
	}
	tsrform = (Form_pg_toaster) GETSTRUCT(tuple);

	/* TODO: type check for toaster */
	if (tsrform->oid != tsroid)
	{
		if (noerror)
		{
			ReleaseSysCache(tuple);
			return NULL;
		}
		ereport(ERROR,
				(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
				 errmsg("toaster \"%s\" is not of type %s",
						NameStr(tsrform->tsrtype), "TYPE")));
	}

	tsrhandler = tsrform->tsrhandler;

	/* Complain if handler OID is invalid */
	if (!RegProcedureIsValid(tsrhandler))
	{
		if (noerror)
		{
			ReleaseSysCache(tuple);
			return NULL;
		}
		ereport(ERROR,
				(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
				 errmsg("toaster \"%s\" does not have a handler",
						NameStr(tsrform->tsrname))));
	}

	ReleaseSysCache(tuple);

	/* And finally, call the handler function to get the API struct. */
	return GetIndexTsrRoutine(tsrhandler);
}

/*
 * Ask appropriate access method to validate the specified opclass.
 */
Datum
tsrvalidate(PG_FUNCTION_ARGS)
{
	Oid			toasteroid = PG_GETARG_OID(0);
	bool		result;
	HeapTuple	toastertup;
	Form_pg_toaster toasterform;
	Oid			tsroid;
	IndexTsrRoutine *tsrroutine;

	toastertup = SearchSysCache1(TOASTEROID, ObjectIdGetDatum(opclassoid));
	if (!HeapTupleIsValid(toastertup))
		elog(ERROR, "cache lookup failed for toaster %u", toasteroid);
	toasterform = (Form_pg_toaster) GETSTRUCT(toastertup);

	tsroid = toasterform->oid;

	ReleaseSysCache(toastertup);

	tsrroutine = GetIndexTsrRoutineByAmId(tsroid, false);

	if (tsrroutine->tsrvalidate == NULL)
		elog(ERROR, "function tsrvalidate is not defined for toaster %u",
			 tsroid);

	result = tsrroutine->tsrvalidate(toasteroid);

	pfree(tsrroutine);

	PG_RETURN_BOOL(result);
}
