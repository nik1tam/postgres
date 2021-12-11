/*-------------------------------------------------------------------------
 *
 * pg_toaster.h
 *	  definition of the "generalized toaster" system catalog (pg_toaster)
 *
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_toaster.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_TOASTER_H
#define PG_TOASTER_H

#include "catalog/genbki.h"
#include "catalog/pg_toaster_d.h"

/* ----------------
 *		pg_toaster definition.  cpp turns this into
 *		typedef struct FormData_pg_toaster
 * ----------------
 */
CATALOG(pg_toaster,9861,AccessMethodRelationId)
{
	Oid			oid;			/* oid */

	/* access method name */
	NameData	tsrname;

	/* handler function */
	regproc		tsrhandler BKI_LOOKUP(pg_proc);

	/* see TSTTYPE_xxx constants below */
	char		tsrtype;
} FormData_pg_toaster;

/* ----------------
 *		Form_pg_toaster corresponds to a pointer to a tuple with
 *		the format of pg_toaster relation.
 * ----------------
 */
typedef FormData_pg_toaster *Form_pg_toaster;

DECLARE_UNIQUE_INDEX(pg_toaster_name_index, 9862, TsrNameIndexId, on pg_toaster using btree(tsrname name_ops));
DECLARE_UNIQUE_INDEX_PKEY(pg_toaster_oid_index, 9863, TsrOidIndexId, on pg_toaster using btree(oid oid_ops));

#ifdef EXPOSE_TO_CLIENT_CODE

/*
 * Allowed values for tsrtype
 */
#define TSRTYPE_DUMMY					'd' /* dummy toaster for testing purposes */

#endif							/* EXPOSE_TO_CLIENT_CODE */

#endif							/* PG_TOASTER_H */
