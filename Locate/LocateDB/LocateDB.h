/* Copyright (c) 1997-2005 Janne Huttunen
   database updater v2.99.5.7030                  */

#ifndef LOCATEDB_H
#define LOCATEDB_H

#define UDBATTRIB_HIDDEN	0x1
#define UDBATTRIB_READONLY	0x2
#define UDBATTRIB_ARCHIVE	0x4
#define UDBATTRIB_SYSTEM	0x8

#define UDBATTRIB_FILE		0x10
#define UDBATTRIB_DIRECTORY	0x80
#define UDBATTRIB_TYPEFLAG	0xF0

#include "Definitions.h"
#include "Database.h"
#include "DatabaseUpdater.h"
#include "DatabaseInfo.h"


#endif