/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2001 Hiroyuki Yamamoto
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

#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

#include <glib.h>
#include <gtk/gtkwidget.h>

typedef struct _TextView	TextView;

#include "messageview.h"
#include "procmime.h"

struct _TextView
{
	GtkWidget *vbox;
	GtkWidget *scrolledwin;
	GtkWidget *scrolledwin_sb;
	GtkWidget *scrolledwin_mb;
	GtkWidget *text;
	GtkWidget *text_sb;
	GtkWidget *text_mb;

	gboolean text_is_mb;

	GSList *uri_list;

	GdkFont *msgfont;
	GdkFont *boldfont;
	gint prev_ascent;
	gint prev_descent;

	MessageView *messageview;
};

TextView *textview_create	(void);
void textview_init		(TextView	*textview);
void textview_show_message	(TextView	*textview,
				 MimeInfo	*mimeinfo,
				 const gchar	*file);
void textview_show_part		(TextView	*textview,
				 MimeInfo	*mimeinfo,
				 FILE		*fp);
void textview_show_mime_part	(TextView	*textview,
				 MimeInfo	*partinfo);
#if USE_GPGME
void textview_show_signature_part(TextView	*textview,
				  MimeInfo 	*partinfo);
#endif
void textview_clear		(TextView	*textview);
void textview_destroy		(TextView	*textview);
void textview_set_font		(TextView	*textview,
				 const gchar	*codeset);
void textview_scroll_one_line	(TextView	*textview,
				 gboolean	 up);
gboolean textview_scroll_page	(TextView	*textview,
				 gboolean	 up);
void textview_update_message_colors(void);

#endif /* __TEXTVIEW_H__ */
