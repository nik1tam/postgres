/*-------------------------------------------------------------------------
 *
 * pg_toaster.c
 *		PG_Toaster functions
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *		src/backend/catalog/pg_toaster.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "access/genam.h"
#include "access/heapam.h"
#include "access/htup_details.h"
#include "access/tableam.h"
#include "access/xact.h"
#include "catalog/indexing.h"
#include "catalog/pg_toaster.h"
#include "catalog/pg_type.h"
#include "miscadmin.h"
#include "nodes/makefuncs.h"
#include "storage/lmgr.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/pg_lsn.h"
#include "utils/rel.h"
#include "utils/syscache.h"

static List *textarray_to_stringlist(ArrayType *textarray);

/*
 * Fetch the toaster from the syscache.
 */
Toaster *
GetToaster(Oid tsrid, bool missing_ok)
{
	HeapTuple	tup;
	Toaster *tsr;
	Form_pg_toaster tsrform;
	Datum		datum;
	bool		isnull;

	tup = SearchSysCache1(TOASTEROID, ObjectIdGetDatum(tsrid));

	if (!HeapTupleIsValid(tup))
	{
		if (missing_ok)
			return NULL;

		elog(ERROR, "cache lookup failed for toaster %u", tsrid);
	}

	tsrform = (Form_pg_toaster) GETSTRUCT(tup);

	tsr = (Toaster *) palloc(sizeof(Toaster));
	tsr->oid = tsrid;
	tsr->tsrname = pstrdup(NameStr(tsrform->tsrname));
	tsr->tsrtype = tsrform->tsrtype;
	tsr->tsrhandler = tsrform->tsrhandler;

	/* Complain if handler OID is invalid */
	if (!RegProcedureIsValid(tsr->tsrhandler))
	{
		if (missing_ok)
		{
			ReleaseSysCache(tup);
			return NULL;
		}
		ereport(ERROR,
				(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
				 errmsg("toaster \"%s\" does not have a handler",
						NameStr(tsrform->tsrname))));
	}

	ReleaseSysCache(tup);

	return tsr;
}

/*
 * get_toaster_oid - given a toaster name, look up the OID
 *
 * If missing_ok is false, throw an error if name not found.  If true, just
 * return InvalidOid.
 */
Oid
get_toaster_oid(const char *tsrname, bool missing_ok)
{
	Oid			oid;

	oid = GetSysCacheOid2(TOASTERNAME, Anum_pg_toaster_oid,
						  MyDatabaseId, CStringGetDatum(tsrname));
	if (!OidIsValid(oid) && !missing_ok)
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_OBJECT),
				 errmsg("toaster \"%s\" does not exist", tsrname)));
	return oid;
}
