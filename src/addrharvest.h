/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 2002 Match Grun
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Definitions for an E-Mail address harvester.
 */

#ifndef __ADDRHARVEST_H__
#define __ADDRHARVEST_H__

#include <stdio.h>
#include <glib.h>
#include "addrbook.h"

/* Headers that will be recognized */
#define HEADER_FROM      "From"
#define HEADER_REPLY_TO  "Reply-to"
#define HEADER_SENDER    "Sender"
#define HEADER_ERRORS_TO "Errors-to"
#define HEADER_CC        "Cc"
#define HEADER_TO        "To"

#define MESSAGEBUFSIZE   2048

/* Harvester file object */
typedef struct _AddressHarvester AddressHarvester;
struct _AddressHarvester {
	GList      *headerTable;
	gchar      *path;
	gchar      *bufptr;
	GHashTable *dupTable;
	gint       folderSize;
	gchar      buffer[ MESSAGEBUFSIZE ];
	gint       retVal;
};

/* Function prototypes */
AddressHarvester *addrharvest_create	( void );
void addrharvest_free			( AddressHarvester *harvester );
void addrharvest_set_path		( AddressHarvester *harvester,
					  const gchar *value );
void addrharvest_set_folder_size	( AddressHarvester* harvester,
					  const gint value );
void addrharvest_set_header		( AddressHarvester* harvester,
					  const gchar *name,
					  const gboolean value );
gint addrharvest_get_count		( AddressHarvester* harvester,
					  const gchar *name );
void addrharvest_print			( AddressHarvester *harvester,
					  FILE *stream );
gint addrharvest_harvest		( AddressHarvester *harvester,
					  AddressCache *cache );
gboolean addrharvest_check_header	( AddressHarvester *harvester );

#endif /* __ADDRHARVEST_H__ */

