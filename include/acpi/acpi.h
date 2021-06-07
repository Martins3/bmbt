/* SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0 */
/******************************************************************************
 *
 * Name: acpi.h - Master public include file used to interface to ACPICA
 *
 * Copyright (C) 2000 - 2021, Intel Corp.
 *
 *****************************************************************************/

#ifndef __ACPI_H__
#define __ACPI_H__

/*
 * Public include files for use by code that will interface to ACPICA.
 *
 * Information includes the ACPICA data types, names, exceptions, and
 * external interface prototypes. Also included are the definitions for
 * all ACPI tables (FADT, MADT, etc.)
 *
 * Note: The order of these include files is important.
 */
// TODO change this back when header fixed
// change #include  "../../include/acpi/acpi.h" to #include <acpi/acpi.h>

// #include <acpi/platform/acenv.h>	[> Environment-specific items <]
// #include <acpi/actypes.h>		[> ACPICA data types and structures <]
// #include <acpi/platform/acenvex.h>	[> Extra environment-specific items <]
// #include <acpi/acnames.h>		[> Common ACPI names and strings <]
// #include <acpi/acexcep.h>		[> ACPICA exceptions <]
// #include <acpi/actbl.h>		[> ACPI table definitions <]
// #include <acpi/acoutput.h>		[> Error output and Debug macros <]
// #include <acpi/acrestyp.h>		[> Resource Descriptor structs <]
// #include <acpi/acpiosxf.h>		[> OSL interfaces (ACPICA-to-OS) <]
// #include <acpi/acpixf.h>		[> ACPI core subsystem external interfaces <]

#include "platform/acenv.h"	/* Environment-specific items */
#include "actypes.h"		/* ACPICA data types and structures */
#include "platform/acenvex.h"	/* Extra environment-specific items */
#include "acnames.h"		/* Common ACPI names and strings */
#include "acexcep.h"		/* ACPICA exceptions */
#include "actbl.h"		/* ACPI table definitions */
#include "acoutput.h"		/* Error output and Debug macros */
#include "acrestyp.h"		/* Resource Descriptor structs */
#include "acpiosxf.h"		/* OSL interfaces (ACPICA-to-OS) */
#include "acpixf.h"		/* ACPI core subsystem external interfaces */

#endif				/* __ACPI_H__ */
