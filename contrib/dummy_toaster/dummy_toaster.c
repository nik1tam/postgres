/*-------------------------------------------------------------------------
 *
 * dummy_toaster.c
 *		Dummy toaster utilities.
 *
 * Portions Copyright (c) 2016-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1990-1993, Regents of the University of California
 *
 * IDENTIFICATION
 *	  contrib/dummy_toaster/dummy_toaster.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"
#include "fmgr.h"
#include "access/toasterapi.h"
#include "access/detoast.h"
#include "access/htup_details.h"
#include "catalog/pg_toaster.h"
#include "utils/builtins.h"
#include "utils/syscache.h"
#include "access/toast_compression.h"
#include "access/xact.h"
#include "catalog/binary_upgrade.h"
#include "catalog/catalog.h"
#include "catalog/dependency.h"
#include "catalog/heap.h"
#include "catalog/index.h"
#include "catalog/namespace.h"
#include "catalog/pg_am.h"
#include "catalog/pg_namespace.h"
#include "catalog/pg_opclass.h"
#include "catalog/pg_type.h"
#include "catalog/toasting.h"
#include "miscadmin.h"
#include "nodes/makefuncs.h"
#include "storage/lock.h"
#include "utils/rel.h"

PG_MODULE_MAGIC;
PG_FUNCTION_INFO_V1(dummy_toaster_handler);

Datum dummyToast (Relation toast_rel,
								Datum value,
								int max_inline_size);
Datum dummyDetoast (Relation toast_rel,
								Datum toast_ptr,
								int offset, int length);
void * dummyGetVtable (Datum toast_ptr);
bool dummyToasterValidate (Oid toasteroid);


Datum
dummy_toaster_handler(PG_FUNCTION_ARGS)
{
	TsrRoutine *tsrroutine = makeNode(TsrRoutine);
	tsrroutine->toast = dummyToast;
	tsrroutine->detoast = dummyDetoast;
	tsrroutine->get_vtable = dummyGetVtable;
	tsrroutine->toastervalidate = dummyToasterValidate;
	elog(ERROR, "dummy_toaster_handler is uninimplemented yet");
	PG_RETURN_POINTER(tsrroutine);
}

static struct varlena *dummy_detoast_custom(struct varlena *attr);
/*static struct varlena *dummy_toast_custom_slice(struct varlena *attr,
											   int32 sliceoffset,
											   int32 slicelength);
*/
/*static struct varlena *dummy_toast_decompress_datum(struct varlena *attr);
static struct varlena *dummy_toast_decompress_datum_slice(struct varlena *attr, int32 slicelength);
*/
/*
 * Validate the generic options given to a FOREIGN DATA WRAPPER, SERVER,
 * USER MAPPING or FOREIGN TABLE that uses file_fdw.
 *
 * Raise an ERROR if the option or its value is considered invalid.
 */
Datum
dummyDetoast(Relation toast_rel,
								Datum toast_ptr,
								int offset, int length)
{
	Datum		tsrDatum = toast_ptr;
	struct varlena *attr = (struct varlena *) DatumGetPointer(tsrDatum);
	/*Oid			tsrOid;*/
	struct varlena *result = 0;

	if (VARATT_IS_CUSTOM(attr))
	{
		/*
		 * Custom Toast pointer
		 */
		struct varatt_custom customPtr;

		VARATT_CUSTOM_GET_POINTER(customPtr, attr);
		attr = (struct varlena *) (&customPtr);
		/*tsrOid = customPtr.va_toasterid;*/

		/* nested indirect Datums aren't allowed */
		Assert(!VARATT_IS_EXTERNAL_INDIRECT(attr));

		/* recurse if value is still custom in some other way */
		if (VARATT_IS_CUSTOM(attr))
			return PointerGetDatum(dummy_detoast_custom(attr));

		/*
		 * Copy into the caller's memory context, in case caller tries to
		 * pfree the result.
		 */
		result = (struct varlena *) palloc(VARSIZE_ANY(attr));
		memcpy(result, attr, VARSIZE_ANY(attr));
	}
	else 
	{
		result = detoast_attr(attr);		
	}

	return PointerGetDatum(result);
}

Datum
dummyToast(Relation toast_rel,
								Datum value,
								int max_inline_size)
{
	Datum		tsrDatum = value;
	struct varlena *attr = (struct varlena *) DatumGetPointer(tsrDatum);

	struct varlena *result = 0;

	if (VARATT_IS_CUSTOM(attr))
	{
		/*
		 * Custom Toast pointer
		 */
		/*struct (varatt_custom *) customPtr;*/

		result = (struct varlena *) palloc(VARSIZE_ANY(attr));
		memcpy(result, attr, VARSIZE_ANY(attr));
	}

	return PointerGetDatum(result);
}

void *
dummyGetVtable(Datum toast_ptr)
{
/*
typedef void * (*get_vtable_function) (Datum toast_ptr);
*/
	Datum		tsrDatum = toast_ptr;
	struct varlena *attr = (struct varlena *) DatumGetPointer(tsrDatum);

	struct varlena *result = 0;

	if (VARATT_IS_CUSTOM(attr))
	{
		/*
		 * Custom Toast pointer
		 */
		/* struct (varatt_custom *) customPtr; */

		result = (struct varlena *) palloc(VARSIZE_ANY(attr));
		memcpy(result, attr, VARSIZE_ANY(attr));
	}

	return result;

}

bool
dummyToasterValidate(Oid toasteroid)
{
	bool result = true;

	return result;
}

static struct varlena *dummy_detoast_custom(struct varlena *attr)
{
	//Datum		tsrDatum;
	//struct varlena *tAttr = (struct varlena *) attr;
	//bool result = true;

	PG_RETURN_VOID();
}

/*
static struct varlena *dummy_toast_custom_slice(struct varlena *attr,
											   int32 sliceoffset,
											   int32 slicelength)
{
	PG_RETURN_VOID();
}

static struct varlena *dummy_toast_decompress_datum(struct varlena *attr)
{
	PG_RETURN_VOID();
}

static struct varlena *dummy_toast_decompress_datum_slice(struct varlena *attr, int32 slicelength)
{
	PG_RETURN_VOID();
}
*/