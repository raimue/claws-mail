/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 1999-2005 Hiroyuki Yamamoto & The Sylpheed-Claws Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "defs.h"

#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if (HAVE_WCTYPE_H && HAVE_WCHAR_H)
#  include <wchar.h>
#  include <wctype.h>
#endif
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#if HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif
#include <dirent.h>
#include <time.h>
#include <regex.h>
#include <sys/utsname.h>

#ifdef G_OS_WIN32
#  include <direct.h>
#  include <io.h>
#endif

#include "utils.h"
#include "socket.h"
#include "../codeconv.h"

#define BUFFSIZE	8192

static gboolean debug_mode = FALSE;


#if !GLIB_CHECK_VERSION(2, 7, 0) && !defined(G_OS_UNIX)
gint g_chdir(const gchar *path)
{
#ifdef G_OS_WIN32
	if (G_WIN32_HAVE_WIDECHAR_API()) {
		wchar_t *wpath;
		gint retval;
		gint save_errno;

		wpath = g_utf8_to_utf16(path, -1, NULL, NULL, NULL);
		if (wpath == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = _wchdir(wpath);
		save_errno = errno;

		g_free(wpath);

		errno = save_errno;
		return retval;
	} else {
		gchar *cp_path;
		gint retval;
		gint save_errno;

		cp_path = g_locale_from_utf8(path, -1, NULL, NULL, NULL);
		if (cp_path == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = chdir(cp_path);
		save_errno = errno;

		g_free(cp_path);

		errno = save_errno;
		return retval;
	}
#else
	return chdir(path);
#endif
}

gint g_chmod(const gchar *path, gint mode)
{
#ifdef G_OS_WIN32
	if (G_WIN32_HAVE_WIDECHAR_API()) {
		wchar_t *wpath;
		gint retval;
		gint save_errno;

		wpath = g_utf8_to_utf16(path, -1, NULL, NULL, NULL);
		if (wpath == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = _wchmod(wpath, mode);
		save_errno = errno;

		g_free(wpath);

		errno = save_errno;
		return retval;
	} else {
		gchar *cp_path;
		gint retval;
		gint save_errno;

		cp_path = g_locale_from_utf8(path, -1, NULL, NULL, NULL);
		if (cp_path == NULL) {
			errno = EINVAL;
			return -1;
		}

		retval = chmod(cp_path, mode);
		save_errno = errno;

		g_free(cp_path);

		errno = save_errno;
		return retval;
	}
#else
	return chmod(path, mode);
#endif
}
#endif /* GLIB_CHECK_VERSION && G_OS_UNIX */

void list_free_strings(GList *list)
{
	list = g_list_first(list);

	while (list != NULL) {
		g_free(list->data);
		list = list->next;
	}
}

void slist_free_strings(GSList *list)
{
	while (list != NULL) {
		g_free(list->data);
		list = list->next;
	}
}

GSList *slist_concat_unique (GSList *first, GSList *second)
{
	GSList *tmp, *ret;
	if (first == NULL) {
		if (second == NULL)
			return NULL;
		else 
			return second;
	} else if (second == NULL)
		return first;
	ret = first;
	for (tmp = second; tmp != NULL; tmp = g_slist_next(tmp)) {
		if (g_slist_find(ret, tmp->data) == NULL)
			ret = g_slist_prepend(ret, tmp->data);
	}
	return ret;
}
 
static void hash_free_strings_func(gpointer key, gpointer value, gpointer data)
{
	g_free(key);
}

void hash_free_strings(GHashTable *table)
{
	g_hash_table_foreach(table, hash_free_strings_func, NULL);
}

static void hash_free_value_mem_func(gpointer key, gpointer value,
				     gpointer data)
{
	g_free(value);
}

void hash_free_value_mem(GHashTable *table)
{
	g_hash_table_foreach(table, hash_free_value_mem_func, NULL);
}

gint str_case_equal(gconstpointer v, gconstpointer v2)
{
	return g_ascii_strcasecmp((const gchar *)v, (const gchar *)v2) == 0;
}

guint str_case_hash(gconstpointer key)
{
	const gchar *p = key;
	guint h = *p;

	if (h) {
		h = g_ascii_tolower(h);
		for (p += 1; *p != '\0'; p++)
			h = (h << 5) - h + g_ascii_tolower(*p);
	}

	return h;
}

void ptr_array_free_strings(GPtrArray *array)
{
	gint i;
	gchar *str;

	g_return_if_fail(array != NULL);

	for (i = 0; i < array->len; i++) {
		str = g_ptr_array_index(array, i);
		g_free(str);
	}
}

gboolean str_find(const gchar *haystack, const gchar *needle)
{
	return strstr(haystack, needle) != NULL ? TRUE : FALSE;
}

gboolean str_case_find(const gchar *haystack, const gchar *needle)
{
	return strcasestr(haystack, needle) != NULL ? TRUE : FALSE;
}

gboolean str_find_equal(const gchar *haystack, const gchar *needle)
{
	return strcmp(haystack, needle) == 0;
}

gboolean str_case_find_equal(const gchar *haystack, const gchar *needle)
{
	return g_ascii_strcasecmp(haystack, needle) == 0;
}

gint to_number(const gchar *nstr)
{
	register const gchar *p;

	if (*nstr == '\0') return -1;

	for (p = nstr; *p != '\0'; p++)
		if (!g_ascii_isdigit(*p)) return -1;

	return atoi(nstr);
}

/* convert integer into string,
   nstr must be not lower than 11 characters length */
gchar *itos_buf(gchar *nstr, gint n)
{
	g_snprintf(nstr, 11, "%d", n);
	return nstr;
}

/* convert integer into string */
gchar *itos(gint n)
{
	static gchar nstr[11];

	return itos_buf(nstr, n);
}

gchar *to_human_readable(off_t size)
{
	static gchar str[10];

	if (size < 1024)
		g_snprintf(str, sizeof(str), _("%dB"), (gint)size);
	else if (size >> 10 < 1024)
		g_snprintf(str, sizeof(str), _("%.1fKB"), (gfloat)size / (1 << 10));
	else if (size >> 20 < 1024)
		g_snprintf(str, sizeof(str), _("%.2fMB"), (gfloat)size / (1 << 20));
	else
		g_snprintf(str, sizeof(str), _("%.2fGB"), (gfloat)size / (1 << 30));

	return str;
}

/* strcmp with NULL-checking */
gint strcmp2(const gchar *s1, const gchar *s2)
{
	if (s1 == NULL || s2 == NULL)
		return -1;
	else
		return strcmp(s1, s2);
}
/* strstr with NULL-checking */
gchar *strstr2(const gchar *s1, const gchar *s2)
{
	if (s1 == NULL || s2 == NULL)
		return NULL;
	else
		return strstr(s1, s2);
}
/* compare paths */
gint path_cmp(const gchar *s1, const gchar *s2)
{
	gint len1, len2;

	if (s1 == NULL || s2 == NULL) return -1;
	if (*s1 == '\0' || *s2 == '\0') return -1;

	len1 = strlen(s1);
	len2 = strlen(s2);

	if (s1[len1 - 1] == G_DIR_SEPARATOR) len1--;
	if (s2[len2 - 1] == G_DIR_SEPARATOR) len2--;

	return strncmp(s1, s2, MAX(len1, len2));
}

/* remove trailing return code */
gchar *strretchomp(gchar *str)
{
	register gchar *s;

	if (!*str) return str;

	for (s = str + strlen(str) - 1;
	     s >= str && (*s == '\n' || *s == '\r');
	     s--)
		*s = '\0';

	return str;
}

/* remove trailing character */
gchar *strtailchomp(gchar *str, gchar tail_char)
{
	register gchar *s;

	if (!*str) return str;
	if (tail_char == '\0') return str;

	for (s = str + strlen(str) - 1; s >= str && *s == tail_char; s--)
		*s = '\0';

	return str;
}

/* remove CR (carriage return) */
gchar *strcrchomp(gchar *str)
{
	register gchar *s;

	if (!*str) return str;

	s = str + strlen(str) - 1;
	if (*s == '\n' && s > str && *(s - 1) == '\r') {
		*(s - 1) = '\n';
		*s = '\0';
	}

	return str;
}

void file_strip_crs(const gchar *file) 
{
	FILE *fp = NULL, *outfp = NULL;
	gchar buf[4096];
	gchar *out = get_tmp_file();
	if (file == NULL)
		goto freeout;
	
	fp = fopen(file, "rb");
	if (!fp)
		goto freeout;

	outfp = fopen(out, "wb");
	if (!outfp) {
		fclose(fp);
		goto freeout;
	}

	while (fgets(buf, sizeof (buf), fp) != NULL) {
		strcrchomp(buf);
		fputs(buf, outfp);
	}
	
	fclose(fp);
	fclose(outfp);
	rename_force(out, file);
freeout:
	g_free(out);
}

/* Similar to `strstr' but this function ignores the case of both strings.  */
gchar *strcasestr(const gchar *haystack, const gchar *needle)
{
	register size_t haystack_len, needle_len;

	haystack_len = strlen(haystack);
	needle_len   = strlen(needle);

	if (haystack_len < needle_len || needle_len == 0)
		return NULL;

	while (haystack_len >= needle_len) {
		if (!g_ascii_strncasecmp(haystack, needle, needle_len))
			return (gchar *)haystack;
		else {
			haystack++;
			haystack_len--;
		}
	}

	return NULL;
}

gpointer my_memmem(gconstpointer haystack, size_t haystacklen,
		   gconstpointer needle, size_t needlelen)
{
	const gchar *haystack_ = (const gchar *)haystack;
	const gchar *needle_ = (const gchar *)needle;
	const gchar *haystack_cur = (const gchar *)haystack;

	if (needlelen == 1)
		return memchr(haystack_, *needle_, haystacklen);

	while ((haystack_cur = memchr(haystack_cur, *needle_, haystacklen))
	       != NULL) {
		if (haystacklen - (haystack_cur - haystack_) < needlelen)
			break;
		if (memcmp(haystack_cur + 1, needle_ + 1, needlelen - 1) == 0)
			return (gpointer)haystack_cur;
		else
			haystack_cur++;
	}

	return NULL;
}

/* Copy no more than N characters of SRC to DEST, with NULL terminating.  */
gchar *strncpy2(gchar *dest, const gchar *src, size_t n)
{
	register const gchar *s = src;
	register gchar *d = dest;

	while (--n && *s)
		*d++ = *s++;
	*d = '\0';

	return dest;
}

#if !HAVE_ISWALNUM
int iswalnum(wint_t wc)
{
	return g_ascii_isalnum((int)wc);
}
#endif

#if !HAVE_ISWSPACE
int iswspace(wint_t wc)
{
	return g_ascii_isspace((int)wc);
}
#endif

#if !HAVE_TOWLOWER
wint_t towlower(wint_t wc)
{
	if (wc >= L'A' && wc <= L'Z')
		return wc + L'a' - L'A';

	return wc;
}
#endif

#if !HAVE_WCSLEN
size_t wcslen(const wchar_t *s)
{
	size_t len = 0;

	while (*s != L'\0')
		++len, ++s;

	return len;
}
#endif

#if !HAVE_WCSCPY
/* Copy SRC to DEST.  */
wchar_t *wcscpy(wchar_t *dest, const wchar_t *src)
{
	wint_t c;
	wchar_t *s = dest;

	do {
		c = *src++;
		*dest++ = c;
	} while (c != L'\0');

	return s;
}
#endif

#if !HAVE_WCSNCPY
/* Copy no more than N wide-characters of SRC to DEST.  */
wchar_t *wcsncpy (wchar_t *dest, const wchar_t *src, size_t n)
{
	wint_t c;
	wchar_t *s = dest;

	do {
		c = *src++;
		*dest++ = c;
		if (--n == 0)
			return s;
	} while (c != L'\0');

	/* zero fill */
	do
		*dest++ = L'\0';
	while (--n > 0);

	return s;
}
#endif

/* Duplicate S, returning an identical malloc'd string. */
wchar_t *wcsdup(const wchar_t *s)
{
	wchar_t *new_str;

	if (s) {
		new_str = g_new(wchar_t, wcslen(s) + 1);
		wcscpy(new_str, s);
	} else
		new_str = NULL;

	return new_str;
}

/* Duplicate no more than N wide-characters of S,
   returning an identical malloc'd string. */
wchar_t *wcsndup(const wchar_t *s, size_t n)
{
	wchar_t *new_str;

	if (s) {
		new_str = g_new(wchar_t, n + 1);
		wcsncpy(new_str, s, n);
		new_str[n] = (wchar_t)0;
	} else
		new_str = NULL;

	return new_str;
}

wchar_t *strdup_mbstowcs(const gchar *s)
{
	wchar_t *new_str;

	if (s) {
		new_str = g_new(wchar_t, strlen(s) + 1);
		if (mbstowcs(new_str, s, strlen(s) + 1) < 0) {
			g_free(new_str);
			new_str = NULL;
		} else
			new_str = g_realloc(new_str,
					    sizeof(wchar_t) * (wcslen(new_str) + 1));
	} else
		new_str = NULL;

	return new_str;
}

gchar *strdup_wcstombs(const wchar_t *s)
{
	gchar *new_str;
	size_t len;

	if (s) {
		len = wcslen(s) * MB_CUR_MAX + 1;
		new_str = g_new(gchar, len);
		if (wcstombs(new_str, s, len) < 0) {
			g_free(new_str);
			new_str = NULL;
		} else
			new_str = g_realloc(new_str, strlen(new_str) + 1);
	} else
		new_str = NULL;

	return new_str;
}

/* Compare S1 and S2, ignoring case.  */
gint wcsncasecmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	wint_t c1;
	wint_t c2;

	while (n--) {
		c1 = towlower(*s1++);
		c2 = towlower(*s2++);
		if (c1 != c2)
			return c1 - c2;
		else if (c1 == 0 && c2 == 0)
			break;
	}

	return 0;
}

/* Find the first occurrence of NEEDLE in HAYSTACK, ignoring case.  */
wchar_t *wcscasestr(const wchar_t *haystack, const wchar_t *needle)
{
	register size_t haystack_len, needle_len;

	haystack_len = wcslen(haystack);
	needle_len   = wcslen(needle);

	if (haystack_len < needle_len || needle_len == 0)
		return NULL;

	while (haystack_len >= needle_len) {
		if (!wcsncasecmp(haystack, needle, needle_len))
			return (wchar_t *)haystack;
		else {
			haystack++;
			haystack_len--;
		}
	}

	return NULL;
}

gint get_mbs_len(const gchar *s)
{
	const gchar *p = s;
	gint mb_len;
	gint len = 0;

	if (!p)
		return -1;

	while (*p != '\0') {
		mb_len = g_utf8_skip[*(guchar *)p];
		if (mb_len == 0)
			break;
		else
			len++;

		p += mb_len;
	}

	return len;
}

/* Examine if next block is non-ASCII string */
gboolean is_next_nonascii(const gchar *s)
{
	const gchar *p;

	/* skip head space */
	for (p = s; *p != '\0' && g_ascii_isspace(*p); p++)
		;
	for (; *p != '\0' && !g_ascii_isspace(*p); p++) {
		if (*(guchar *)p > 127 || *(guchar *)p < 32)
			return TRUE;
	}

	return FALSE;
}

gint get_next_word_len(const gchar *s)
{
	gint len = 0;

	for (; *s != '\0' && !g_ascii_isspace(*s); s++, len++)
		;

	return len;
}

/* compare subjects */
gint subject_compare(const gchar *s1, const gchar *s2)
{
	gchar *str1, *str2;

	if (!s1 || !s2) return -1;
	if (!*s1 || !*s2) return -1;

	Xstrdup_a(str1, s1, return -1);
	Xstrdup_a(str2, s2, return -1);

	trim_subject_for_compare(str1);
	trim_subject_for_compare(str2);

	if (!*str1 || !*str2) return -1;

	return strcmp(str1, str2);
}

gint subject_compare_for_sort(const gchar *s1, const gchar *s2)
{
	gchar *str1, *str2;

	if (!s1 || !s2) return -1;

	Xstrdup_a(str1, s1, return -1);
	Xstrdup_a(str2, s2, return -1);

	trim_subject_for_sort(str1);
	trim_subject_for_sort(str2);

	return g_utf8_collate(str1, str2);
}

void trim_subject_for_compare(gchar *str)
{
	gchar *srcp;

	eliminate_parenthesis(str, '[', ']');
	eliminate_parenthesis(str, '(', ')');
	g_strstrip(str);

	srcp = str + subject_get_prefix_length(str);
	if (srcp != str)
		memmove(str, srcp, strlen(srcp) + 1);
}

void trim_subject_for_sort(gchar *str)
{
	gchar *srcp;

	g_strstrip(str);

	srcp = str + subject_get_prefix_length(str);
	if (srcp != str)	
		memmove(str, srcp, strlen(srcp) + 1);
}

void trim_subject(gchar *str)
{
	register gchar *srcp;
	gchar op, cl;
	gint in_brace;
	
	g_strstrip(str);

	srcp = str + subject_get_prefix_length(str);

	if (*srcp == '[') {
		op = '[';
		cl = ']';
	} else if (*srcp == '(') {
		op = '(';
		cl = ')';
	} else
		op = 0;

	if (op) {
		++srcp;
		in_brace = 1;
		while (*srcp) {
			if (*srcp == op)
				in_brace++;
			else if (*srcp == cl)
				in_brace--;
			srcp++;
			if (in_brace == 0)
				break;
		}
	}
	while (g_ascii_isspace(*srcp)) srcp++;
	memmove(str, srcp, strlen(srcp) + 1);
}

void eliminate_parenthesis(gchar *str, gchar op, gchar cl)
{
	register gchar *srcp, *destp;
	gint in_brace;

	srcp = destp = str;

	while ((destp = strchr(destp, op))) {
		in_brace = 1;
		srcp = destp + 1;
		while (*srcp) {
			if (*srcp == op)
				in_brace++;
			else if (*srcp == cl)
				in_brace--;
			srcp++;
			if (in_brace == 0)
				break;
		}
		while (g_ascii_isspace(*srcp)) srcp++;
		memmove(destp, srcp, strlen(srcp) + 1);
	}
}

void extract_parenthesis(gchar *str, gchar op, gchar cl)
{
	register gchar *srcp, *destp;
	gint in_brace;

	srcp = destp = str;

	while ((srcp = strchr(destp, op))) {
		if (destp > str)
			*destp++ = ' ';
		memmove(destp, srcp + 1, strlen(srcp));
		in_brace = 1;
		while(*destp) {
			if (*destp == op)
				in_brace++;
			else if (*destp == cl)
				in_brace--;

			if (in_brace == 0)
				break;

			destp++;
		}
	}
	*destp = '\0';
}

void extract_parenthesis_with_skip_quote(gchar *str, gchar quote_chr,
					 gchar op, gchar cl)
{
	register gchar *srcp, *destp;
	gint in_brace;
	gboolean in_quote = FALSE;

	srcp = destp = str;

	while ((srcp = strchr_with_skip_quote(destp, quote_chr, op))) {
		if (destp > str)
			*destp++ = ' ';
		memmove(destp, srcp + 1, strlen(srcp));
		in_brace = 1;
		while(*destp) {
			if (*destp == op && !in_quote)
				in_brace++;
			else if (*destp == cl && !in_quote)
				in_brace--;
			else if (*destp == quote_chr)
				in_quote ^= TRUE;

			if (in_brace == 0)
				break;

			destp++;
		}
	}
	*destp = '\0';
}

void eliminate_quote(gchar *str, gchar quote_chr)
{
	register gchar *srcp, *destp;

	srcp = destp = str;

	while ((destp = strchr(destp, quote_chr))) {
		if ((srcp = strchr(destp + 1, quote_chr))) {
			srcp++;
			while (g_ascii_isspace(*srcp)) srcp++;
			memmove(destp, srcp, strlen(srcp) + 1);
		} else {
			*destp = '\0';
			break;
		}
	}
}

void extract_quote(gchar *str, gchar quote_chr)
{
	register gchar *p;

	if ((str = strchr(str, quote_chr))) {
		p = str;
		while ((p = strchr(p + 1, quote_chr)) && (p[-1] == '\\')) {
			memmove(p - 1, p, strlen(p) + 1);
			p--;
		}
		if(p) {
			*p = '\0';
			memmove(str, str + 1, p - str);
		}
	}
}

void eliminate_address_comment(gchar *str)
{
	register gchar *srcp, *destp;
	gint in_brace;

	srcp = destp = str;

	while ((destp = strchr(destp, '"'))) {
		if ((srcp = strchr(destp + 1, '"'))) {
			srcp++;
			if (*srcp == '@') {
				destp = srcp + 1;
			} else {
				while (g_ascii_isspace(*srcp)) srcp++;
				memmove(destp, srcp, strlen(srcp) + 1);
			}
		} else {
			*destp = '\0';
			break;
		}
	}

	srcp = destp = str;

	while ((destp = strchr_with_skip_quote(destp, '"', '('))) {
		in_brace = 1;
		srcp = destp + 1;
		while (*srcp) {
			if (*srcp == '(')
				in_brace++;
			else if (*srcp == ')')
				in_brace--;
			srcp++;
			if (in_brace == 0)
				break;
		}
		while (g_ascii_isspace(*srcp)) srcp++;
		memmove(destp, srcp, strlen(srcp) + 1);
	}
}

gchar *strchr_with_skip_quote(const gchar *str, gint quote_chr, gint c)
{
	gboolean in_quote = FALSE;

	while (*str) {
		if (*str == c && !in_quote)
			return (gchar *)str;
		if (*str == quote_chr)
			in_quote ^= TRUE;
		str++;
	}

	return NULL;
}

gchar *strrchr_with_skip_quote(const gchar *str, gint quote_chr, gint c)
{
	gboolean in_quote = FALSE;
	const gchar *p;

	p = str + strlen(str) - 1;
	while (p >= str) {
		if (*p == c && !in_quote)
			return (gchar *)p;
		if (*p == quote_chr)
			in_quote ^= TRUE;
		p--;
	}

	return NULL;
}

void extract_address(gchar *str)
{
	eliminate_address_comment(str);
	if (strchr_with_skip_quote(str, '"', '<'))
		extract_parenthesis_with_skip_quote(str, '"', '<', '>');
	g_strstrip(str);
}

void extract_list_id_str(gchar *str)
{
	if (strchr_with_skip_quote(str, '"', '<'))
		extract_parenthesis_with_skip_quote(str, '"', '<', '>');
	g_strstrip(str);
}

static GSList *address_list_append_real(GSList *addr_list, const gchar *str, gboolean removecomments)
{
	gchar *work;
	gchar *workp;

	if (!str) return addr_list;

	Xstrdup_a(work, str, return addr_list);

	if (removecomments)
		eliminate_address_comment(work);
	workp = work;

	while (workp && *workp) {
		gchar *p, *next;

		if ((p = strchr_with_skip_quote(workp, '"', ','))) {
			*p = '\0';
			next = p + 1;
		} else
			next = NULL;

		if (removecomments && strchr_with_skip_quote(workp, '"', '<'))
			extract_parenthesis_with_skip_quote
				(workp, '"', '<', '>');

		g_strstrip(workp);
		if (*workp)
			addr_list = g_slist_append(addr_list, g_strdup(workp));

		workp = next;
	}

	return addr_list;
}

GSList *address_list_append(GSList *addr_list, const gchar *str)
{
	return address_list_append_real(addr_list, str, TRUE);
}

GSList *address_list_append_with_comments(GSList *addr_list, const gchar *str)
{
	return address_list_append_real(addr_list, str, FALSE);
}

GSList *references_list_prepend(GSList *msgid_list, const gchar *str)
{
	const gchar *strp;

	if (!str) return msgid_list;
	strp = str;

	while (strp && *strp) {
		const gchar *start, *end;
		gchar *msgid;

		if ((start = strchr(strp, '<')) != NULL) {
			end = strchr(start + 1, '>');
			if (!end) break;
		} else
			break;

		msgid = g_strndup(start + 1, end - start - 1);
		g_strstrip(msgid);
		if (*msgid)
			msgid_list = g_slist_prepend(msgid_list, msgid);
		else
			g_free(msgid);

		strp = end + 1;
	}

	return msgid_list;
}

GSList *references_list_append(GSList *msgid_list, const gchar *str)
{
	GSList *list;

	list = references_list_prepend(NULL, str);
	list = g_slist_reverse(list);
	msgid_list = g_slist_concat(msgid_list, list);

	return msgid_list;
}

GSList *newsgroup_list_append(GSList *group_list, const gchar *str)
{
	gchar *work;
	gchar *workp;

	if (!str) return group_list;

	Xstrdup_a(work, str, return group_list);

	workp = work;

	while (workp && *workp) {
		gchar *p, *next;

		if ((p = strchr_with_skip_quote(workp, '"', ','))) {
			*p = '\0';
			next = p + 1;
		} else
			next = NULL;

		g_strstrip(workp);
		if (*workp)
			group_list = g_slist_append(group_list,
						    g_strdup(workp));

		workp = next;
	}

	return group_list;
}

GList *add_history(GList *list, const gchar *str)
{
	GList *old;

	g_return_val_if_fail(str != NULL, list);

	old = g_list_find_custom(list, (gpointer)str, (GCompareFunc)strcmp2);
	if (old) {
		g_free(old->data);
		list = g_list_remove(list, old->data);
	} else if (g_list_length(list) >= MAX_HISTORY_SIZE) {
		GList *last;

		last = g_list_last(list);
		if (last) {
			g_free(last->data);
			g_list_remove(list, last->data);
		}
	}

	list = g_list_prepend(list, g_strdup(str));

	return list;
}

void remove_return(gchar *str)
{
	register gchar *p = str;

	while (*p) {
		if (*p == '\n' || *p == '\r')
			memmove(p, p + 1, strlen(p));
		else
			p++;
	}
}

void remove_space(gchar *str)
{
	register gchar *p = str;
	register gint spc;

	while (*p) {
		spc = 0;
		while (g_ascii_isspace(*(p + spc)))
			spc++;
		if (spc)
			memmove(p, p + spc, strlen(p + spc) + 1);
		else
			p++;
	}
}

void unfold_line(gchar *str)
{
	register gchar *p = str;
	register gint spc;

	while (*p) {
		if (*p == '\n' || *p == '\r') {
			*p++ = ' ';
			spc = 0;
			while (g_ascii_isspace(*(p + spc)))
				spc++;
			if (spc)
				memmove(p, p + spc, strlen(p + spc) + 1);
		} else
			p++;
	}
}

void subst_char(gchar *str, gchar orig, gchar subst)
{
	register gchar *p = str;

	while (*p) {
		if (*p == orig)
			*p = subst;
		p++;
	}
}

void subst_chars(gchar *str, gchar *orig, gchar subst)
{
	register gchar *p = str;

	while (*p) {
		if (strchr(orig, *p) != NULL)
			*p = subst;
		p++;
	}
}

void subst_for_filename(gchar *str)
{
	if (!str)
		return;
	subst_chars(str, "\t\r\n\\/*", '_');
}

void subst_for_shellsafe_filename(gchar *str)
{
	if (!str)
		return;
	subst_for_filename(str);
	subst_chars(str, " \"'|&;()<>'!{}[]",'_');
}

gboolean is_header_line(const gchar *str)
{
	if (str[0] == ':') return FALSE;

	while (*str != '\0' && *str != ' ') {
		if (*str == ':')
			return TRUE;
		str++;
	}

	return FALSE;
}

gboolean is_ascii_str(const gchar *str)
{
	const guchar *p = (const guchar *)str;

	while (*p != '\0') {
		if (*p != '\t' && *p != ' ' &&
		    *p != '\r' && *p != '\n' &&
		    (*p < 32 || *p >= 127))
			return FALSE;
		p++;
	}

	return TRUE;
}

gint get_quote_level(const gchar *str, const gchar *quote_chars)
{
	const gchar *first_pos;
	const gchar *last_pos;
	const gchar *p = str;
	gint quote_level = -1;

	/* speed up line processing by only searching to the last '>' */
	if ((first_pos = line_has_quote_char(str, quote_chars)) != NULL) {
		/* skip a line if it contains a '<' before the initial '>' */
		if (memchr(str, '<', first_pos - str) != NULL)
			return -1;
		last_pos = line_has_quote_char_last(first_pos, quote_chars);
	} else
		return -1;

	while (p <= last_pos) {
		while (p < last_pos) {
			if (g_ascii_isspace(*p))
				p++;
			else
				break;
		}

		if (strchr(quote_chars, *p))
			quote_level++;
		else if (*p != '-' && !g_ascii_isspace(*p) && p <= last_pos) {
			/* any characters are allowed except '-' and space */
			while (*p != '-' 
			       && !strchr(quote_chars, *p) 
			       && !g_ascii_isspace(*p) 
			       && p < last_pos)
				p++;
			if (strchr(quote_chars, *p))
				quote_level++;
			else
				break;
		}

		p++;
	}

	return quote_level;
}

gint check_line_length(const gchar *str, gint max_chars, gint *line)
{
	const gchar *p = str, *q;
	gint cur_line = 0, len;

	while ((q = strchr(p, '\n')) != NULL) {
		len = q - p + 1;
		if (len > max_chars) {
			if (line)
				*line = cur_line;
			return -1;
		}
		p = q + 1;
		++cur_line;
	}

	len = strlen(p);
	if (len > max_chars) {
		if (line)
			*line = cur_line;
		return -1;
	}

	return 0;
}

const gchar * line_has_quote_char(const gchar * str, const gchar *quote_chars) 
{
	gchar * position = NULL;
	gchar * tmp_pos = NULL;
	int i;

	if (quote_chars == NULL)
		return FALSE;
	
	for (i = 0; i < strlen(quote_chars); i++) {
		tmp_pos = strchr (str,	quote_chars[i]);
		if(position == NULL 
		   || (tmp_pos != NULL && position >= tmp_pos) )
			position = tmp_pos;
	}
	return position; 
}

const gchar * line_has_quote_char_last(const gchar * str, const gchar *quote_chars) 
{
	gchar * position = NULL;
	gchar * tmp_pos = NULL;
	int i;

	if (quote_chars == NULL)
		return FALSE;
	
	for (i = 0; i < strlen(quote_chars); i++) {
		tmp_pos = strrchr (str,	quote_chars[i]);
		if(position == NULL 
		   || (tmp_pos != NULL && position <= tmp_pos) )
			position = tmp_pos;
	}
	return position; 
}

gchar *strstr_with_skip_quote(const gchar *haystack, const gchar *needle)
{
	register guint haystack_len, needle_len;
	gboolean in_squote = FALSE, in_dquote = FALSE;

	haystack_len = strlen(haystack);
	needle_len   = strlen(needle);

	if (haystack_len < needle_len || needle_len == 0)
		return NULL;

	while (haystack_len >= needle_len) {
		if (!in_squote && !in_dquote &&
		    !strncmp(haystack, needle, needle_len))
			return (gchar *)haystack;

		/* 'foo"bar"' -> foo"bar"
		   "foo'bar'" -> foo'bar' */
		if (*haystack == '\'') {
			if (in_squote)
				in_squote = FALSE;
			else if (!in_dquote)
				in_squote = TRUE;
		} else if (*haystack == '\"') {
			if (in_dquote)
				in_dquote = FALSE;
			else if (!in_squote)
				in_dquote = TRUE;
		}

		haystack++;
		haystack_len--;
	}

	return NULL;
}

gchar *strchr_parenthesis_close(const gchar *str, gchar op, gchar cl)
{
	const gchar *p;
	gchar quote_chr = '"';
	gint in_brace;
	gboolean in_quote = FALSE;

	p = str;

	if ((p = strchr_with_skip_quote(p, quote_chr, op))) {
		p++;
		in_brace = 1;
		while (*p) {
			if (*p == op && !in_quote)
				in_brace++;
			else if (*p == cl && !in_quote)
				in_brace--;
			else if (*p == quote_chr)
				in_quote ^= TRUE;

			if (in_brace == 0)
				return (gchar *)p;

			p++;
		}
	}

	return NULL;
}

gchar **strsplit_parenthesis(const gchar *str, gchar op, gchar cl,
			     gint max_tokens)
{
	GSList *string_list = NULL, *slist;
	gchar **str_array;
	const gchar *s_op, *s_cl;
	guint i, n = 1;

	g_return_val_if_fail(str != NULL, NULL);

	if (max_tokens < 1)
		max_tokens = G_MAXINT;

	s_op = strchr_with_skip_quote(str, '"', op);
	if (!s_op) return NULL;
	str = s_op;
	s_cl = strchr_parenthesis_close(str, op, cl);
	if (s_cl) {
		do {
			guint len;
			gchar *new_string;

			str++;
			len = s_cl - str;
			new_string = g_new(gchar, len + 1);
			strncpy(new_string, str, len);
			new_string[len] = 0;
			string_list = g_slist_prepend(string_list, new_string);
			n++;
			str = s_cl + 1;

			while (*str && g_ascii_isspace(*str)) str++;
			if (*str != op) {
				string_list = g_slist_prepend(string_list,
							      g_strdup(""));
				n++;
				s_op = strchr_with_skip_quote(str, '"', op);
				if (!--max_tokens || !s_op) break;
				str = s_op;
			} else
				s_op = str;
			s_cl = strchr_parenthesis_close(str, op, cl);
		} while (--max_tokens && s_cl);
	}

	str_array = g_new(gchar*, n);

	i = n - 1;

	str_array[i--] = NULL;
	for (slist = string_list; slist; slist = slist->next)
		str_array[i--] = slist->data;

	g_slist_free(string_list);

	return str_array;
}

gchar **strsplit_with_quote(const gchar *str, const gchar *delim,
			    gint max_tokens)
{
	GSList *string_list = NULL, *slist;
	gchar **str_array, *s, *new_str;
	guint i, n = 1, len;

	g_return_val_if_fail(str != NULL, NULL);
	g_return_val_if_fail(delim != NULL, NULL);

	if (max_tokens < 1)
		max_tokens = G_MAXINT;

	s = strstr_with_skip_quote(str, delim);
	if (s) {
		guint delimiter_len = strlen(delim);

		do {
			len = s - str;
			new_str = g_strndup(str, len);

			if (new_str[0] == '\'' || new_str[0] == '\"') {
				if (new_str[len - 1] == new_str[0]) {
					new_str[len - 1] = '\0';
					memmove(new_str, new_str + 1, len - 1);
				}
			}
			string_list = g_slist_prepend(string_list, new_str);
			n++;
			str = s + delimiter_len;
			s = strstr_with_skip_quote(str, delim);
		} while (--max_tokens && s);
	}

	if (*str) {
		new_str = g_strdup(str);
		if (new_str[0] == '\'' || new_str[0] == '\"') {
			len = strlen(str);
			if (new_str[len - 1] == new_str[0]) {
				new_str[len - 1] = '\0';
				memmove(new_str, new_str + 1, len - 1);
			}
		}
		string_list = g_slist_prepend(string_list, new_str);
		n++;
	}

	str_array = g_new(gchar*, n);

	i = n - 1;

	str_array[i--] = NULL;
	for (slist = string_list; slist; slist = slist->next)
		str_array[i--] = slist->data;

	g_slist_free(string_list);

	return str_array;
}

gchar *get_abbrev_newsgroup_name(const gchar *group, gint len)
{
	gchar *abbrev_group;
	gchar *ap;
	const gchar *p = group;
	const gchar *last;

	g_return_val_if_fail(group != NULL, NULL);

	last = group + strlen(group);
	abbrev_group = ap = g_malloc(strlen(group) + 1);

	while (*p) {
		while (*p == '.')
			*ap++ = *p++;
		if ((ap - abbrev_group) + (last - p) > len && strchr(p, '.')) {
			*ap++ = *p++;
			while (*p != '.') p++;
		} else {
			strcpy(ap, p);
			return abbrev_group;
		}
	}

	*ap = '\0';
	return abbrev_group;
}

gchar *trim_string(const gchar *str, gint len)
{
	const gchar *p = str;
	gint mb_len;
	gchar *new_str;
	gint new_len = 0;

	if (!str) return NULL;
	if (strlen(str) <= len)
		return g_strdup(str);
	if (g_utf8_validate(str, -1, NULL) == FALSE)
		return g_strdup(str);

	while (*p != '\0') {
		mb_len = g_utf8_skip[*(guchar *)p];
		if (mb_len == 0)
			break;
		else if (new_len + mb_len > len)
			break;

		new_len += mb_len;
		p += mb_len;
	}

	Xstrndup_a(new_str, str, new_len, return g_strdup(str));
	return g_strconcat(new_str, "...", NULL);
}

GList *uri_list_extract_filenames(const gchar *uri_list)
{
	GList *result = NULL;
	const gchar *p, *q;
	gchar *escaped_utf8uri;

	p = uri_list;

	while (p) {
		if (*p != '#') {
			while (g_ascii_isspace(*p)) p++;
			if (!strncmp(p, "file:", 5)) {
				q = p;
				q += 5;
				while (*q && *q != '\n' && *q != '\r') q++;

				if (q > p) {
					gchar *file, *locale_file = NULL;
					q--;
					while (q > p && g_ascii_isspace(*q))
						q--;
					Xalloca(escaped_utf8uri, q - p + 2,
						return result);
					Xalloca(file, q - p + 2,
						return result);
					*file = '\0';
					strncpy(escaped_utf8uri, p, q - p + 1);
					escaped_utf8uri[q - p + 1] = '\0';
					decode_uri(file, escaped_utf8uri);
                    /*
		     * g_filename_from_uri() rejects escaped/locale encoded uri
		     * string which come from Nautilus.
		     */
					if (g_utf8_validate(file, -1, NULL))
						locale_file
							= conv_codeset_strdup(
								file + 5,
								CS_UTF_8,
								conv_get_locale_charset_str());
					if (!locale_file)
						locale_file = g_strdup(file + 5);
					result = g_list_append(result, locale_file);
				}
			}
		}
		p = strchr(p, '\n');
		if (p) p++;
	}

	return result;
}

/* Converts two-digit hexadecimal to decimal.  Used for unescaping escaped 
 * characters
 */
static gint axtoi(const gchar *hexstr)
{
	gint hi, lo, result;
       
	hi = hexstr[0];
	if ('0' <= hi && hi <= '9') {
		hi -= '0';
	} else
		if ('a' <= hi && hi <= 'f') {
			hi -= ('a' - 10);
		} else
			if ('A' <= hi && hi <= 'F') {
				hi -= ('A' - 10);
			}

	lo = hexstr[1];
	if ('0' <= lo && lo <= '9') {
		lo -= '0';
	} else
		if ('a' <= lo && lo <= 'f') {
			lo -= ('a'-10);
		} else
			if ('A' <= lo && lo <= 'F') {
				lo -= ('A' - 10);
			}
	result = lo + (16 * hi);
	return result;
}

gboolean is_uri_string(const gchar *str)
{
	return (g_ascii_strncasecmp(str, "http://", 7) == 0 ||
		g_ascii_strncasecmp(str, "https://", 8) == 0 ||
		g_ascii_strncasecmp(str, "ftp://", 6) == 0 ||
		g_ascii_strncasecmp(str, "www.", 4) == 0);
}

gchar *get_uri_path(const gchar *uri)
{
	if (g_ascii_strncasecmp(uri, "http://", 7) == 0)
		return (gchar *)(uri + 7);
	else if (g_ascii_strncasecmp(uri, "https://", 8) == 0)
		return (gchar *)(uri + 8);
	else if (g_ascii_strncasecmp(uri, "ftp://", 6) == 0)
		return (gchar *)(uri + 6);
	else
		return (gchar *)uri;
}

gint get_uri_len(const gchar *str)
{
	const gchar *p;

	if (is_uri_string(str)) {
		for (p = str; *p != '\0'; p++) {
			if (!g_ascii_isgraph(*p) || strchr("()<>\"", *p))
				break;
		}
		return p - str;
	}

	return 0;
}

/* Decodes URL-Encoded strings (i.e. strings in which spaces are replaced by
 * plusses, and escape characters are used)
 */
void decode_uri(gchar *decoded_uri, const gchar *encoded_uri)
{
	gchar *dec = decoded_uri;
	const gchar *enc = encoded_uri;

	while (*enc) {
		if (*enc == '%') {
			enc++;
			if (isxdigit((guchar)enc[0]) &&
			    isxdigit((guchar)enc[1])) {
				*dec = axtoi(enc);
				dec++;
				enc += 2;
			}
		} else {
			if (*enc == '+')
				*dec = ' ';
			else
				*dec = *enc;
			dec++;
			enc++;
		}
	}

	*dec = '\0';
}

gint scan_mailto_url(const gchar *mailto, gchar **to, gchar **cc, gchar **bcc,
		     gchar **subject, gchar **body)
{
	gchar *tmp_mailto;
	gchar *p;

	Xstrdup_a(tmp_mailto, mailto, return -1);

	if (!strncmp(tmp_mailto, "mailto:", 7))
		tmp_mailto += 7;

	p = strchr(tmp_mailto, '?');
	if (p) {
		*p = '\0';
		p++;
	}

	if (to && !*to)
		*to = g_strdup(tmp_mailto);

	while (p) {
		gchar *field, *value;

		field = p;

		p = strchr(p, '=');
		if (!p) break;
		*p = '\0';
		p++;

		value = p;

		p = strchr(p, '&');
		if (p) {
			*p = '\0';
			p++;
		}

		if (*value == '\0') continue;

		if (cc && !*cc && !g_ascii_strcasecmp(field, "cc")) {
			*cc = g_strdup(value);
		} else if (bcc && !*bcc && !g_ascii_strcasecmp(field, "bcc")) {
			*bcc = g_strdup(value);
		} else if (subject && !*subject &&
			   !g_ascii_strcasecmp(field, "subject")) {
			*subject = g_malloc(strlen(value) + 1);
			decode_uri(*subject, value);
		} else if (body && !*body && !g_ascii_strcasecmp(field, "body")) {
			*body = g_malloc(strlen(value) + 1);
			decode_uri(*body, value);
		}
	}

	return 0;
}

const gchar *get_home_dir(void)
{
#ifdef G_OS_WIN32
	static const gchar *home_dir = NULL;

	if (!home_dir) {
		home_dir = g_get_home_dir();
		if (!home_dir)
			home_dir = "C:\\Sylpheed";
	}

	return home_dir;
#else
	return g_get_home_dir();
#endif
}

const gchar *get_rc_dir(void)
{
	static gchar *rc_dir = NULL;

	if (!rc_dir)
#ifdef G_OS_WIN32
		rc_dir = g_strconcat(get_home_dir(), G_DIR_SEPARATOR_S,
				     "Application Data", G_DIR_SEPARATOR_S,
				     RC_DIR, NULL);
#else
		rc_dir = g_strconcat(get_home_dir(), G_DIR_SEPARATOR_S,
				     RC_DIR, NULL);
#endif

	return rc_dir;
}

const gchar *get_mail_base_dir(void)
{
#ifdef G_OS_WIN32
	static gchar *mail_base_dir = NULL;

	if (!mail_base_dir)
		mail_base_dir = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
					    "Mailboxes", NULL);

	return mail_base_dir;
#else
	return get_home_dir();
#endif
}

const gchar *get_news_cache_dir(void)
{
	static gchar *news_cache_dir = NULL;

	if (!news_cache_dir)
		news_cache_dir = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
					     NEWS_CACHE_DIR, NULL);

	return news_cache_dir;
}

const gchar *get_imap_cache_dir(void)
{
	static gchar *imap_cache_dir = NULL;

	if (!imap_cache_dir)
		imap_cache_dir = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
					     IMAP_CACHE_DIR, NULL);

	return imap_cache_dir;
}

const gchar *get_mbox_cache_dir(void)
{
	static gchar *mbox_cache_dir = NULL;

	if (!mbox_cache_dir)
		mbox_cache_dir = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
					     MBOX_CACHE_DIR, NULL);

	return mbox_cache_dir;
}

const gchar *get_mime_tmp_dir(void)
{
	static gchar *mime_tmp_dir = NULL;

	if (!mime_tmp_dir)
		mime_tmp_dir = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
					   MIME_TMP_DIR, NULL);

	return mime_tmp_dir;
}

const gchar *get_template_dir(void)
{
	static gchar *template_dir = NULL;

	if (!template_dir)
		template_dir = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
					   TEMPLATE_DIR, NULL);

	return template_dir;
}

const gchar *get_header_cache_dir(void)
{
	static gchar *header_dir = NULL;

	if (!header_dir)
		header_dir = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
					 HEADER_CACHE_DIR, NULL);

	return header_dir;
}

const gchar *get_tmp_dir(void)
{
	static gchar *tmp_dir = NULL;

	if (!tmp_dir)
		tmp_dir = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S,
				      TMP_DIR, NULL);

	return tmp_dir;
}

gchar *get_tmp_file(void)
{
	gchar *tmp_file;
	static guint32 id = 0;

	tmp_file = g_strdup_printf("%s%ctmpfile.%08x",
				   get_tmp_dir(), G_DIR_SEPARATOR, id++);

	return tmp_file;
}

const gchar *get_domain_name(void)
{
#ifdef G_OS_UNIX
	static gchar *domain_name = NULL;

	if (!domain_name) {
		struct hostent *hp;
		struct utsname uts;

		if (uname(&uts) < 0) {
			perror("uname");
			domain_name = "unknown";
		} else {
			if ((hp = my_gethostbyname(uts.nodename)) == NULL) {
				perror("gethostbyname");
				domain_name = g_strdup(uts.nodename);
			} else {
				domain_name = g_strdup(hp->h_name);
			}
		}

		debug_print("domain name = %s\n", domain_name);
	}

	return domain_name;
#else
	return "unknown";
#endif
}

off_t get_file_size(const gchar *file)
{
	struct stat s;

	if (g_stat(file, &s) < 0) {
		FILE_OP_ERROR(file, "stat");
		return -1;
	}

	return s.st_size;
}

off_t get_file_size_as_crlf(const gchar *file)
{
	FILE *fp;
	off_t size = 0;
	gchar buf[BUFFSIZE];

	if ((fp = g_fopen(file, "rb")) == NULL) {
		FILE_OP_ERROR(file, "fopen");
		return -1;
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		strretchomp(buf);
		size += strlen(buf) + 2;
	}

	if (ferror(fp)) {
		FILE_OP_ERROR(file, "fgets");
		size = -1;
	}

	fclose(fp);

	return size;
}

off_t get_left_file_size(FILE *fp)
{
	glong pos;
	glong end;
	off_t size;

	if ((pos = ftell(fp)) < 0) {
		perror("ftell");
		return -1;
	}
	if (fseek(fp, 0L, SEEK_END) < 0) {
		perror("fseek");
		return -1;
	}
	if ((end = ftell(fp)) < 0) {
		perror("fseek");
		return -1;
	}
	size = end - pos;
	if (fseek(fp, pos, SEEK_SET) < 0) {
		perror("fseek");
		return -1;
	}

	return size;
}

gboolean file_exist(const gchar *file, gboolean allow_fifo)
{
	struct stat s;

	if (file == NULL)
		return FALSE;

	if (g_stat(file, &s) < 0) {
		if (ENOENT != errno) FILE_OP_ERROR(file, "stat");
		return FALSE;
	}

	if (S_ISREG(s.st_mode) || (allow_fifo && S_ISFIFO(s.st_mode)))
		return TRUE;

	return FALSE;
}

gboolean is_dir_exist(const gchar *dir)
{
	if (dir == NULL)
		return FALSE;

	return g_file_test(dir, G_FILE_TEST_IS_DIR);
}

gboolean is_file_entry_exist(const gchar *file)
{
	if (file == NULL)
		return FALSE;

	return g_file_test(file, G_FILE_TEST_EXISTS);
}

gboolean dirent_is_regular_file(struct dirent *d)
{
#ifdef HAVE_DIRENT_D_TYPE
	if (d->d_type == DT_REG)
		return TRUE;
	else if (d->d_type != DT_UNKNOWN)
		return FALSE;
#endif

	return g_file_test(d->d_name, G_FILE_TEST_IS_REGULAR);
}

gboolean dirent_is_directory(struct dirent *d)
{
#ifdef HAVE_DIRENT_D_TYPE
	if (d->d_type == DT_DIR)
		return TRUE;
	else if (d->d_type != DT_UNKNOWN)
		return FALSE;
#endif

	return g_file_test(d->d_name, G_FILE_TEST_IS_DIR);
}

gint change_dir(const gchar *dir)
{
	gchar *prevdir = NULL;

	if (debug_mode)
		prevdir = g_get_current_dir();

	if (g_chdir(dir) < 0) {
		FILE_OP_ERROR(dir, "chdir");
		if (debug_mode) g_free(prevdir);
		return -1;
	} else if (debug_mode) {
		gchar *cwd;

		cwd = g_get_current_dir();
		if (strcmp(prevdir, cwd) != 0)
			g_print("current dir: %s\n", cwd);
		g_free(cwd);
		g_free(prevdir);
	}

	return 0;
}

gint make_dir(const gchar *dir)
{
	if (g_mkdir(dir, S_IRWXU) < 0) {
		FILE_OP_ERROR(dir, "mkdir");
		return -1;
	}
	if (g_chmod(dir, S_IRWXU) < 0)
		FILE_OP_ERROR(dir, "chmod");

	return 0;
}

gint make_dir_hier(const gchar *dir)
{
	gchar *parent_dir;
	const gchar *p;

	for (p = dir; (p = strchr(p, G_DIR_SEPARATOR)) != NULL; p++) {
		parent_dir = g_strndup(dir, p - dir);
		if (*parent_dir != '\0') {
			if (!is_dir_exist(parent_dir)) {
				if (make_dir(parent_dir) < 0) {
					g_free(parent_dir);
					return -1;
				}
			}
		}
		g_free(parent_dir);
	}

	if (!is_dir_exist(dir)) {
		if (make_dir(dir) < 0)
			return -1;
	}

	return 0;
}

gint remove_all_files(const gchar *dir)
{
	GDir *dp;
	const gchar *dir_name;
	gchar *prev_dir;

	prev_dir = g_get_current_dir();

	if (g_chdir(dir) < 0) {
		FILE_OP_ERROR(dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	if ((dp = g_dir_open(".", 0, NULL)) == NULL) {
		g_warning("failed to open directory: %s\n", dir);
		g_free(prev_dir);
		return -1;
	}

	while ((dir_name = g_dir_read_name(dp)) != NULL) {
		if (g_unlink(dir_name) < 0)
			FILE_OP_ERROR(dir_name, "unlink");
	}

	g_dir_close(dp);

	if (g_chdir(prev_dir) < 0) {
		FILE_OP_ERROR(prev_dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	g_free(prev_dir);

	return 0;
}

gint remove_numbered_files(const gchar *dir, guint first, guint last)
{
	GDir *dp;
	const gchar *dir_name;
	gchar *prev_dir;
	gint file_no;

	prev_dir = g_get_current_dir();

	if (g_chdir(dir) < 0) {
		FILE_OP_ERROR(dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	if ((dp = g_dir_open(".", 0, NULL)) == NULL) {
		g_warning("failed to open directory: %s\n", dir);
		g_free(prev_dir);
		return -1;
	}

	while ((dir_name = g_dir_read_name(dp)) != NULL) {
		file_no = to_number(dir_name);
		if (file_no > 0 && first <= file_no && file_no <= last) {
			if (is_dir_exist(dir_name))
				continue;
			if (g_unlink(dir_name) < 0)
				FILE_OP_ERROR(dir_name, "unlink");
		}
	}

	g_dir_close(dp);

	if (g_chdir(prev_dir) < 0) {
		FILE_OP_ERROR(prev_dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	g_free(prev_dir);

	return 0;
}

gint remove_numbered_files_not_in_list(const gchar *dir, GSList *numberlist)
{
	GDir *dp;
	const gchar *dir_name;
	gchar *prev_dir;
	gint file_no;

	prev_dir = g_get_current_dir();

	if (g_chdir(dir) < 0) {
		FILE_OP_ERROR(dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	if ((dp = g_dir_open(".", 0, NULL)) == NULL) {
		FILE_OP_ERROR(dir, "opendir");
		g_free(prev_dir);
		return -1;
	}

	while ((dir_name = g_dir_read_name(dp)) != NULL) {
		file_no = to_number(dir_name);
		if (file_no > 0 && (g_slist_find(numberlist, GINT_TO_POINTER(file_no)) == NULL)) {
			debug_print("removing unwanted file %d from %s\n", file_no, dir);
			if (is_dir_exist(dir_name))
				continue;
			if (g_unlink(dir_name) < 0)
				FILE_OP_ERROR(dir_name, "unlink");
		}
	}

	g_dir_close(dp);

	if (g_chdir(prev_dir) < 0) {
		FILE_OP_ERROR(prev_dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	g_free(prev_dir);

	return 0;
}

gint remove_all_numbered_files(const gchar *dir)
{
	return remove_numbered_files(dir, 0, UINT_MAX);
}

gint remove_expired_files(const gchar *dir, guint hours)
{
	GDir *dp;
	const gchar *dir_name;
	struct stat s;
	gchar *prev_dir;
	gint file_no;
	time_t mtime, now, expire_time;

	prev_dir = g_get_current_dir();

	if (g_chdir(dir) < 0) {
		FILE_OP_ERROR(dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	if ((dp = g_dir_open(".", 0, NULL)) == NULL) {
		g_warning("failed to open directory: %s\n", dir);
		g_free(prev_dir);
		return -1;
	}

	now = time(NULL);
	expire_time = hours * 60 * 60;

	while ((dir_name = g_dir_read_name(dp)) != NULL) {
		file_no = to_number(dir_name);
		if (file_no > 0) {
			if (g_stat(dir_name, &s) < 0) {
				FILE_OP_ERROR(dir_name, "stat");
				continue;
			}
			if (S_ISDIR(s.st_mode))
				continue;
			mtime = MAX(s.st_mtime, s.st_atime);
			if (now - mtime > expire_time) {
				if (g_unlink(dir_name) < 0)
					FILE_OP_ERROR(dir_name, "unlink");
			}
		}
	}

	g_dir_close(dp);

	if (g_chdir(prev_dir) < 0) {
		FILE_OP_ERROR(prev_dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	g_free(prev_dir);

	return 0;
}

gint remove_dir_recursive(const gchar *dir)
{
	struct stat s;
	GDir *dp;
	const gchar *dir_name;
	gchar *prev_dir;

	if (g_stat(dir, &s) < 0) {
		FILE_OP_ERROR(dir, "stat");
		if (ENOENT == errno) return 0;
		return -1;
	}

	if (!S_ISDIR(s.st_mode)) {
		if (g_unlink(dir) < 0) {
			FILE_OP_ERROR(dir, "unlink");
			return -1;
		}

		return 0;
	}

	prev_dir = g_get_current_dir();
	/* g_print("prev_dir = %s\n", prev_dir); */

	if (!path_cmp(prev_dir, dir)) {
		g_free(prev_dir);
		if (g_chdir("..") < 0) {
			FILE_OP_ERROR(dir, "chdir");
			return -1;
		}
		prev_dir = g_get_current_dir();
	}

	if (g_chdir(dir) < 0) {
		FILE_OP_ERROR(dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	if ((dp = g_dir_open(".", 0, NULL)) == NULL) {
		g_warning("failed to open directory: %s\n", dir);
		g_chdir(prev_dir);
		g_free(prev_dir);
		return -1;
	}

	/* remove all files in the directory */
	while ((dir_name = g_dir_read_name(dp)) != NULL) {
		/* g_print("removing %s\n", dir_name); */

		if (is_dir_exist(dir_name)) {
			if (remove_dir_recursive(dir_name) < 0) {
				g_warning("can't remove directory\n");
				return -1;
			}
		} else {
			if (g_unlink(dir_name) < 0)
				FILE_OP_ERROR(dir_name, "unlink");
		}
	}

	g_dir_close(dp);

	if (g_chdir(prev_dir) < 0) {
		FILE_OP_ERROR(prev_dir, "chdir");
		g_free(prev_dir);
		return -1;
	}

	g_free(prev_dir);

	if (g_rmdir(dir) < 0) {
		FILE_OP_ERROR(dir, "rmdir");
		return -1;
	}

	return 0;
}

gint rename_force(const gchar *oldpath, const gchar *newpath)
{
#ifndef G_OS_UNIX
	if (!is_file_entry_exist(oldpath)) {
		errno = ENOENT;
		return -1;
	}
	if (is_file_exist(newpath)) {
		if (g_unlink(newpath) < 0)
			FILE_OP_ERROR(newpath, "unlink");
	}
#endif
	return g_rename(oldpath, newpath);
}

#if 0
/* this seems to be slower than the stdio version... */
gint copy_file(const gchar *src, const gchar *dest)
{
	gint src_fd, dest_fd;
	gint n_read;
	gint n_write;
	gchar buf[BUFSIZ];
	gchar *dest_bak = NULL;

	if ((src_fd = open(src, O_RDONLY)) < 0) {
		FILE_OP_ERROR(src, "open");
		return -1;
	}

	if (is_file_exist(dest)) {
		dest_bak = g_strconcat(dest, ".bak", NULL);
		if (rename_force(dest, dest_bak) < 0) {
			FILE_OP_ERROR(dest, "rename");
			close(src_fd);
			g_free(dest_bak);
			return -1;
		}
	}

	if ((dest_fd = open(dest, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR)) < 0) {
		FILE_OP_ERROR(dest, "open");
		close(src_fd);
		if (dest_bak) {
			if (rename(dest_bak, dest) < 0)
				FILE_OP_ERROR(dest_bak, "rename");
			g_free(dest_bak);
		}
		return -1;
	}

	while ((n_read = read(src_fd, buf, sizeof(buf))) > 0) {
		gint len = n_read;
		gchar *bufp = buf;

		while (len > 0) {
			n_write = write(dest_fd, bufp, len);
			if (n_write <= 0) {
				g_warning("writing to %s failed.\n", dest);
				close(dest_fd);
				close(src_fd);
				g_unlink(dest);
				if (dest_bak) {
					if (rename(dest_bak, dest) < 0)
						FILE_OP_ERROR(dest_bak, "rename");
					g_free(dest_bak);
				}
				return -1;
			}
			len -= n_write;
			bufp += n_write;
		}
	}

	close(src_fd);
	close(dest_fd);

	if (n_read < 0 || get_file_size(src) != get_file_size(dest)) {
		g_warning("File copy from %s to %s failed.\n", src, dest);
		g_unlink(dest);
		if (dest_bak) {
			if (rename(dest_bak, dest) < 0)
				FILE_OP_ERROR(dest_bak, "rename");
			g_free(dest_bak);
		}
		return -1;
	}
	g_free(dest_bak);

	return 0;
}
#endif


/*
 * Append src file body to the tail of dest file.
 * Now keep_backup has no effects.
 */
gint append_file(const gchar *src, const gchar *dest, gboolean keep_backup)
{
	FILE *src_fp, *dest_fp;
	gint n_read;
	gchar buf[BUFSIZ];

	gboolean err = FALSE;

	if ((src_fp = g_fopen(src, "rb")) == NULL) {
		FILE_OP_ERROR(src, "fopen");
		return -1;
	}
	
	if ((dest_fp = g_fopen(dest, "ab")) == NULL) {
		FILE_OP_ERROR(dest, "fopen");
		fclose(src_fp);
		return -1;
	}

	if (change_file_mode_rw(dest_fp, dest) < 0) {
		FILE_OP_ERROR(dest, "chmod");
		g_warning("can't change file mode\n");
	}

	while ((n_read = fread(buf, sizeof(gchar), sizeof(buf), src_fp)) > 0) {
		if (n_read < sizeof(buf) && ferror(src_fp))
			break;
		if (fwrite(buf, 1, n_read, dest_fp) < n_read) {
			g_warning("writing to %s failed.\n", dest);
			fclose(dest_fp);
			fclose(src_fp);
			g_unlink(dest);
			return -1;
		}
	}

	if (ferror(src_fp)) {
		FILE_OP_ERROR(src, "fread");
		err = TRUE;
	}
	fclose(src_fp);
	if (fclose(dest_fp) == EOF) {
		FILE_OP_ERROR(dest, "fclose");
		err = TRUE;
	}

	if (err) {
		g_unlink(dest);
		return -1;
	}

	return 0;
}

gint copy_file(const gchar *src, const gchar *dest, gboolean keep_backup)
{
	FILE *src_fp, *dest_fp;
	gint n_read;
	gchar buf[BUFSIZ];
	gchar *dest_bak = NULL;
	gboolean err = FALSE;

	if ((src_fp = g_fopen(src, "rb")) == NULL) {
		FILE_OP_ERROR(src, "fopen");
		return -1;
	}
	if (is_file_exist(dest)) {
		dest_bak = g_strconcat(dest, ".bak", NULL);
		if (rename_force(dest, dest_bak) < 0) {
			FILE_OP_ERROR(dest, "rename");
			fclose(src_fp);
			g_free(dest_bak);
			return -1;
		}
	}

	if ((dest_fp = g_fopen(dest, "wb")) == NULL) {
		FILE_OP_ERROR(dest, "fopen");
		fclose(src_fp);
		if (dest_bak) {
			if (rename_force(dest_bak, dest) < 0)
				FILE_OP_ERROR(dest_bak, "rename");
			g_free(dest_bak);
		}
		return -1;
	}

	if (change_file_mode_rw(dest_fp, dest) < 0) {
		FILE_OP_ERROR(dest, "chmod");
		g_warning("can't change file mode\n");
	}

	while ((n_read = fread(buf, sizeof(gchar), sizeof(buf), src_fp)) > 0) {
		if (n_read < sizeof(buf) && ferror(src_fp))
			break;
		if (fwrite(buf, 1, n_read, dest_fp) < n_read) {
			g_warning("writing to %s failed.\n", dest);
			fclose(dest_fp);
			fclose(src_fp);
			g_unlink(dest);
			if (dest_bak) {
				if (rename_force(dest_bak, dest) < 0)
					FILE_OP_ERROR(dest_bak, "rename");
				g_free(dest_bak);
			}
			return -1;
		}
	}

	if (ferror(src_fp)) {
		FILE_OP_ERROR(src, "fread");
		err = TRUE;
	}
	fclose(src_fp);
	if (fclose(dest_fp) == EOF) {
		FILE_OP_ERROR(dest, "fclose");
		err = TRUE;
	}

	if (err) {
		g_unlink(dest);
		if (dest_bak) {
			if (rename_force(dest_bak, dest) < 0)
				FILE_OP_ERROR(dest_bak, "rename");
			g_free(dest_bak);
		}
		return -1;
	}

	if (keep_backup == FALSE && dest_bak)
		g_unlink(dest_bak);

	g_free(dest_bak);

	return 0;
}

gint move_file(const gchar *src, const gchar *dest, gboolean overwrite)
{
	if (overwrite == FALSE && is_file_exist(dest)) {
		g_warning("move_file(): file %s already exists.", dest);
		return -1;
	}

	if (rename_force(src, dest) == 0) return 0;

	if (EXDEV != errno) {
		FILE_OP_ERROR(src, "rename");
		return -1;
	}

	if (copy_file(src, dest, FALSE) < 0) return -1;

	g_unlink(src);

	return 0;
}

gint copy_file_part_to_fp(FILE *fp, off_t offset, size_t length, FILE *dest_fp)
{
	gint n_read;
	gint bytes_left, to_read;
	gchar buf[BUFSIZ];

	if (fseek(fp, offset, SEEK_SET) < 0) {
		perror("fseek");
		return -1;
	}

	bytes_left = length;
	to_read = MIN(bytes_left, sizeof(buf));

	while ((n_read = fread(buf, sizeof(gchar), to_read, fp)) > 0) {
		if (n_read < to_read && ferror(fp))
			break;
		if (fwrite(buf, 1, n_read, dest_fp) < n_read) {
			return -1;
		}
		bytes_left -= n_read;
		if (bytes_left == 0)
			break;
		to_read = MIN(bytes_left, sizeof(buf));
	}

	if (ferror(fp)) {
		perror("fread");
		return -1;
	}

	return 0;
}

gint copy_file_part(FILE *fp, off_t offset, size_t length, const gchar *dest)
{
	FILE *dest_fp;
	gboolean err = FALSE;

	if ((dest_fp = g_fopen(dest, "wb")) == NULL) {
		FILE_OP_ERROR(dest, "fopen");
		return -1;
	}

	if (change_file_mode_rw(dest_fp, dest) < 0) {
		FILE_OP_ERROR(dest, "chmod");
		g_warning("can't change file mode\n");
	}

	if (copy_file_part_to_fp(fp, offset, length, dest_fp) < 0)
		err = TRUE;

	if (!err && fclose(dest_fp) == EOF) {
		FILE_OP_ERROR(dest, "fclose");
		err = TRUE;
	}

	if (err) {
		g_warning("writing to %s failed.\n", dest);
		g_unlink(dest);
		return -1;
	}

	return 0;
}

/* convert line endings into CRLF. If the last line doesn't end with
 * linebreak, add it.
 */
gchar *canonicalize_str(const gchar *str)
{
	const gchar *p;
	guint new_len = 0;
	gchar *out, *outp;

	for (p = str; *p != '\0'; ++p) {
		if (*p != '\r') {
			++new_len;
			if (*p == '\n')
				++new_len;
		}
	}
	if (p == str || *(p - 1) != '\n')
		new_len += 2;

	out = outp = g_malloc(new_len + 1);
	for (p = str; *p != '\0'; ++p) {
		if (*p != '\r') {
			if (*p == '\n')
				*outp++ = '\r';
			*outp++ = *p;
		}
	}
	if (p == str || *(p - 1) != '\n') {
		*outp++ = '\r';
		*outp++ = '\n';
	}
	*outp = '\0';

	return out;
}

gint canonicalize_file(const gchar *src, const gchar *dest)
{
	FILE *src_fp, *dest_fp;
	gchar buf[BUFFSIZE];
	gint len;
	gboolean err = FALSE;
	gboolean last_linebreak = FALSE;

	if ((src_fp = g_fopen(src, "rb")) == NULL) {
		FILE_OP_ERROR(src, "fopen");
		return -1;
	}

	if ((dest_fp = g_fopen(dest, "wb")) == NULL) {
		FILE_OP_ERROR(dest, "fopen");
		fclose(src_fp);
		return -1;
	}

	if (change_file_mode_rw(dest_fp, dest) < 0) {
		FILE_OP_ERROR(dest, "chmod");
		g_warning("can't change file mode\n");
	}

	while (fgets(buf, sizeof(buf), src_fp) != NULL) {
		gint r = 0;

		len = strlen(buf);
		if (len == 0) break;
		last_linebreak = FALSE;

		if (buf[len - 1] != '\n') {
			last_linebreak = TRUE;
			r = fputs(buf, dest_fp);
		} else if (len > 1 && buf[len - 1] == '\n' && buf[len - 2] == '\r') {
			r = fputs(buf, dest_fp);
		} else {
			if (len > 1) {
				r = fwrite(buf, 1, len - 1, dest_fp);
				if (r != (len -1))
					r = EOF;
			}
			if (r != EOF)
				r = fputs("\r\n", dest_fp);
		}

		if (r == EOF) {
			g_warning("writing to %s failed.\n", dest);
			fclose(dest_fp);
			fclose(src_fp);
			g_unlink(dest);
			return -1;
		}
	}

	if (last_linebreak == TRUE) {
		if (fputs("\r\n", dest_fp) == EOF)
			err = TRUE;
	}

	if (ferror(src_fp)) {
		FILE_OP_ERROR(src, "fgets");
		err = TRUE;
	}
	fclose(src_fp);
	if (fclose(dest_fp) == EOF) {
		FILE_OP_ERROR(dest, "fclose");
		err = TRUE;
	}

	if (err) {
		g_unlink(dest);
		return -1;
	}

	return 0;
}

gint canonicalize_file_replace(const gchar *file)
{
	gchar *tmp_file;

	tmp_file = get_tmp_file();

	if (canonicalize_file(file, tmp_file) < 0) {
		g_free(tmp_file);
		return -1;
	}

	if (move_file(tmp_file, file, TRUE) < 0) {
		g_warning("can't replace %s .\n", file);
		g_unlink(tmp_file);
		g_free(tmp_file);
		return -1;
	}

	g_free(tmp_file);
	return 0;
}

gint uncanonicalize_file(const gchar *src, const gchar *dest)
{
	FILE *src_fp, *dest_fp;
	gchar buf[BUFFSIZE];
	gboolean err = FALSE;

	if ((src_fp = g_fopen(src, "rb")) == NULL) {
		FILE_OP_ERROR(src, "fopen");
		return -1;
	}

	if ((dest_fp = g_fopen(dest, "wb")) == NULL) {
		FILE_OP_ERROR(dest, "fopen");
		fclose(src_fp);
		return -1;
	}

	if (change_file_mode_rw(dest_fp, dest) < 0) {
		FILE_OP_ERROR(dest, "chmod");
		g_warning("can't change file mode\n");
	}

	while (fgets(buf, sizeof(buf), src_fp) != NULL) {
		strcrchomp(buf);
		if (fputs(buf, dest_fp) == EOF) {
			g_warning("writing to %s failed.\n", dest);
			fclose(dest_fp);
			fclose(src_fp);
			g_unlink(dest);
			return -1;
		}
	}

	if (ferror(src_fp)) {
		FILE_OP_ERROR(src, "fgets");
		err = TRUE;
	}
	fclose(src_fp);
	if (fclose(dest_fp) == EOF) {
		FILE_OP_ERROR(dest, "fclose");
		err = TRUE;
	}

	if (err) {
		g_unlink(dest);
		return -1;
	}

	return 0;
}

gint uncanonicalize_file_replace(const gchar *file)
{
	gchar *tmp_file;

	tmp_file = get_tmp_file();

	if (uncanonicalize_file(file, tmp_file) < 0) {
		g_free(tmp_file);
		return -1;
	}

	if (move_file(tmp_file, file, TRUE) < 0) {
		g_warning("can't replace %s .\n", file);
		g_unlink(tmp_file);
		g_free(tmp_file);
		return -1;
	}

	g_free(tmp_file);
	return 0;
}

gchar *normalize_newlines(const gchar *str)
{
	const gchar *p = str;
	gchar *out, *outp;

	out = outp = g_malloc(strlen(str) + 1);
	for (p = str; *p != '\0'; ++p) {
		if (*p == '\r') {
			if (*(p + 1) != '\n')
				*outp++ = '\n';
		} else
			*outp++ = *p;
	}

	*outp = '\0';

	return out;
}

gchar *get_outgoing_rfc2822_str(FILE *fp)
{
	gchar buf[BUFFSIZE];
	GString *str;
	gchar *ret;

	str = g_string_new(NULL);

	/* output header part */
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		strretchomp(buf);
		if (!g_ascii_strncasecmp(buf, "Bcc:", 4)) {
			gint next;

			for (;;) {
				next = fgetc(fp);
				if (next == EOF)
					break;
				else if (next != ' ' && next != '\t') {
					ungetc(next, fp);
					break;
				}
				if (fgets(buf, sizeof(buf), fp) == NULL)
					break;
			}
		} else {
			g_string_append(str, buf);
			g_string_append(str, "\r\n");
			if (buf[0] == '\0')
				break;
		}
	}

	/* output body part */
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		strretchomp(buf);
		if (buf[0] == '.')
			g_string_append_c(str, '.');
		g_string_append(str, buf);
		g_string_append(str, "\r\n");
	}

	ret = str->str;
	g_string_free(str, FALSE);

	return ret;
}

/*
 * Create a new boundary in a way that it is very unlikely that this
 * will occur in the following text.  It would be easy to ensure
 * uniqueness if everything is either quoted-printable or base64
 * encoded (note that conversion is allowed), but because MIME bodies
 * may be nested, it may happen that the same boundary has already
 * been used. 
 *
 *   boundary := 0*69<bchars> bcharsnospace
 *   bchars := bcharsnospace / " "
 *   bcharsnospace := DIGIT / ALPHA / "'" / "(" / ")" /
 *                    "+" / "_" / "," / "-" / "." /
 *                    "/" / ":" / "=" / "?"
 *
 * some special characters removed because of buggy MTAs
 */

gchar *generate_mime_boundary(const gchar *prefix)
{
	static gchar tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			     "abcdefghijklmnopqrstuvwxyz"
			     "1234567890+_./=";
	gchar buf_uniq[24];
	gint i;

	for (i = 0; i < sizeof(buf_uniq) - 1; i++)
		buf_uniq[i] = tbl[g_random_int_range(0, sizeof(tbl) - 1)];
	buf_uniq[i] = '\0';

	return g_strdup_printf("%s_%s", prefix ? prefix : "MP",
			       buf_uniq);
}

gint change_file_mode_rw(FILE *fp, const gchar *file)
{
#if HAVE_FCHMOD
	return fchmod(fileno(fp), S_IRUSR|S_IWUSR);
#else
	return g_chmod(file, S_IRUSR|S_IWUSR);
#endif
}

FILE *my_tmpfile(void)
{
#if HAVE_MKSTEMP
	const gchar suffix[] = ".XXXXXX";
	const gchar *tmpdir;
	guint tmplen;
	const gchar *progname;
	guint proglen;
	gchar *fname;
	gint fd;
	FILE *fp;

	tmpdir = get_tmp_dir();
	tmplen = strlen(tmpdir);
	progname = g_get_prgname();
	if (progname == NULL)
		progname = "sylpheed-claws";
	proglen = strlen(progname);
	Xalloca(fname, tmplen + 1 + proglen + sizeof(suffix),
		return tmpfile());

	memcpy(fname, tmpdir, tmplen);
	fname[tmplen] = G_DIR_SEPARATOR;
	memcpy(fname + tmplen + 1, progname, proglen);
	memcpy(fname + tmplen + 1 + proglen, suffix, sizeof(suffix));

	fd = mkstemp(fname);
	if (fd < 0)
		return tmpfile();

	g_unlink(fname);

	fp = fdopen(fd, "w+b");
	if (!fp)
		close(fd);
	else
		return fp;
#endif /* HAVE_MKSTEMP */

	return tmpfile();
}

FILE *get_tmpfile_in_dir(const gchar *dir, gchar **filename)
{
	int fd;
	
	*filename = g_strdup_printf("%s%csylpheed.XXXXXX", dir, G_DIR_SEPARATOR);
	fd = mkstemp(*filename);

	return fdopen(fd, "w+");
}

FILE *str_open_as_stream(const gchar *str)
{
	FILE *fp;
	size_t len;

	g_return_val_if_fail(str != NULL, NULL);

	fp = my_tmpfile();
	if (!fp) {
		FILE_OP_ERROR("str_open_as_stream", "my_tmpfile");
		return NULL;
	}

	len = strlen(str);
	if (len == 0) return fp;

	if (fwrite(str, 1, len, fp) != len) {
		FILE_OP_ERROR("str_open_as_stream", "fwrite");
		fclose(fp);
		return NULL;
	}

	rewind(fp);
	return fp;
}

gint str_write_to_file(const gchar *str, const gchar *file)
{
	FILE *fp;
	size_t len;

	g_return_val_if_fail(str != NULL, -1);
	g_return_val_if_fail(file != NULL, -1);

	if ((fp = g_fopen(file, "wb")) == NULL) {
		FILE_OP_ERROR(file, "fopen");
		return -1;
	}

	len = strlen(str);
	if (len == 0) {
		fclose(fp);
		return 0;
	}

	if (fwrite(str, 1, len, fp) != len) {
		FILE_OP_ERROR(file, "fwrite");
		fclose(fp);
		g_unlink(file);
		return -1;
	}

	if (fclose(fp) == EOF) {
		FILE_OP_ERROR(file, "fclose");
		g_unlink(file);
		return -1;
	}

	return 0;
}

gchar *file_read_to_str(const gchar *file)
{
	FILE *fp;
	gchar *str;

	g_return_val_if_fail(file != NULL, NULL);

	if ((fp = g_fopen(file, "rb")) == NULL) {
		FILE_OP_ERROR(file, "fopen");
		return NULL;
	}

	str = file_read_stream_to_str(fp);

	fclose(fp);

	return str;
}

gchar *file_read_stream_to_str(FILE *fp)
{
	GByteArray *array;
	guchar buf[BUFSIZ];
	gint n_read;
	gchar *str;

	g_return_val_if_fail(fp != NULL, NULL);

	array = g_byte_array_new();

	while ((n_read = fread(buf, sizeof(gchar), sizeof(buf), fp)) > 0) {
		if (n_read < sizeof(buf) && ferror(fp))
			break;
		g_byte_array_append(array, buf, n_read);
	}

	if (ferror(fp)) {
		FILE_OP_ERROR("file stream", "fread");
		g_byte_array_free(array, TRUE);
		return NULL;
	}

	buf[0] = '\0';
	g_byte_array_append(array, buf, 1);
	str = (gchar *)array->data;
	g_byte_array_free(array, FALSE);

	if (!g_utf8_validate(str, -1, NULL)) {
		const gchar *src_codeset, *dest_codeset;
		gchar *tmp = NULL;
		src_codeset = conv_get_locale_charset_str();
		dest_codeset = CS_UTF_8;
		tmp = conv_codeset_strdup(str, src_codeset, dest_codeset);
		g_free(str);
		str = tmp;
	}

	return str;
}

gint execute_async(gchar *const argv[])
{
	g_return_val_if_fail(argv != NULL && argv[0] != NULL, -1);

	if (g_spawn_async(NULL, (gchar **)argv, NULL, G_SPAWN_SEARCH_PATH,
			  NULL, NULL, NULL, FALSE) == FALSE) {
		g_warning("Can't execute command: %s\n", argv[0]);
		return -1;
	}

	return 0;
}

gint execute_sync(gchar *const argv[])
{
	gint status;

	g_return_val_if_fail(argv != NULL && argv[0] != NULL, -1);

	if (g_spawn_sync(NULL, (gchar **)argv, NULL, G_SPAWN_SEARCH_PATH,
			 NULL, NULL, NULL, NULL, &status, NULL) == FALSE) {
		g_warning("Can't execute command: %s\n", argv[0]);
		return -1;
	}

#ifdef G_OS_UNIX
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	else
		return -1;
#else
	return status;
#endif
}

gint execute_command_line(const gchar *cmdline, gboolean async)
{
	gchar **argv;
	gint ret;

	debug_print("execute_command_line(): executing: %s\n", cmdline);

	argv = strsplit_with_quote(cmdline, " ", 0);

	if (async)
		ret = execute_async(argv);
	else
		ret = execute_sync(argv);

	g_strfreev(argv);

	return ret;
}

gchar *get_command_output(const gchar *cmdline)
{
	gchar *child_stdout;
	gint status;

	g_return_val_if_fail(cmdline != NULL, NULL);

	debug_print("get_command_output(): executing: %s\n", cmdline);

	if (g_spawn_command_line_sync(cmdline, &child_stdout, NULL, &status,
				      NULL) == FALSE) {
		g_warning("Can't execute command: %s\n", cmdline);
		return NULL;
	}

	return child_stdout;
}

static gint is_unchanged_uri_char(char c)
{
	switch (c) {
		case '(':
		case ')':
		case ',':
			return 0;
		default:
			return 1;
	}
}

void encode_uri(gchar *encoded_uri, gint bufsize, const gchar *uri)
{
	int i;
	int k;

	k = 0;
	for(i = 0; i < strlen(uri) ; i++) {
		if (is_unchanged_uri_char(uri[i])) {
			if (k + 2 >= bufsize)
				break;
			encoded_uri[k++] = uri[i];
		}
		else {
			char * hexa = "0123456789ABCDEF";
			
			if (k + 4 >= bufsize)
				break;
			encoded_uri[k++] = '%';
			encoded_uri[k++] = hexa[uri[i] / 16];
			encoded_uri[k++] = hexa[uri[i] % 16];
		}
	}
	encoded_uri[k] = 0;
}

gint open_uri(const gchar *uri, const gchar *cmdline)
{
	gchar buf[BUFFSIZE];
	gchar *p;
	gchar encoded_uri[BUFFSIZE];
	
	g_return_val_if_fail(uri != NULL, -1);

	/* an option to choose whether to use encode_uri or not ? */
	encode_uri(encoded_uri, BUFFSIZE, uri);
	
	if (cmdline &&
	    (p = strchr(cmdline, '%')) && *(p + 1) == 's' &&
	    !strchr(p + 2, '%'))
		g_snprintf(buf, sizeof(buf), cmdline, encoded_uri);
	else {
		if (cmdline)
			g_warning("Open URI command line is invalid "
				  "(there must be only one '%%s'): %s",
				  cmdline);
		g_snprintf(buf, sizeof(buf), DEFAULT_BROWSER_CMD, encoded_uri);
	}

	execute_command_line(buf, TRUE);

	return 0;
}

time_t remote_tzoffset_sec(const gchar *zone)
{
	static gchar ustzstr[] = "PSTPDTMSTMDTCSTCDTESTEDT";
	gchar zone3[4];
	gchar *p;
	gchar c;
	gint iustz;
	gint offset;
	time_t remoteoffset;

	strncpy(zone3, zone, 3);
	zone3[3] = '\0';
	remoteoffset = 0;

	if (sscanf(zone, "%c%d", &c, &offset) == 2 &&
	    (c == '+' || c == '-')) {
		remoteoffset = ((offset / 100) * 60 + (offset % 100)) * 60;
		if (c == '-')
			remoteoffset = -remoteoffset;
	} else if (!strncmp(zone, "UT" , 2) ||
		   !strncmp(zone, "GMT", 2)) {
		remoteoffset = 0;
	} else if (strlen(zone3) == 3) {
		for (p = ustzstr; *p != '\0'; p += 3) {
			if (!g_ascii_strncasecmp(p, zone3, 3)) {
				iustz = ((gint)(p - ustzstr) / 3 + 1) / 2 - 8;
				remoteoffset = iustz * 3600;
				break;
			}
		}
		if (*p == '\0')
			return -1;
	} else if (strlen(zone3) == 1) {
		switch (zone[0]) {
		case 'Z': remoteoffset =   0; break;
		case 'A': remoteoffset =  -1; break;
		case 'B': remoteoffset =  -2; break;
		case 'C': remoteoffset =  -3; break;
		case 'D': remoteoffset =  -4; break;
		case 'E': remoteoffset =  -5; break;
		case 'F': remoteoffset =  -6; break;
		case 'G': remoteoffset =  -7; break;
		case 'H': remoteoffset =  -8; break;
		case 'I': remoteoffset =  -9; break;
		case 'K': remoteoffset = -10; break; /* J is not used */
		case 'L': remoteoffset = -11; break;
		case 'M': remoteoffset = -12; break;
		case 'N': remoteoffset =   1; break;
		case 'O': remoteoffset =   2; break;
		case 'P': remoteoffset =   3; break;
		case 'Q': remoteoffset =   4; break;
		case 'R': remoteoffset =   5; break;
		case 'S': remoteoffset =   6; break;
		case 'T': remoteoffset =   7; break;
		case 'U': remoteoffset =   8; break;
		case 'V': remoteoffset =   9; break;
		case 'W': remoteoffset =  10; break;
		case 'X': remoteoffset =  11; break;
		case 'Y': remoteoffset =  12; break;
		default:  remoteoffset =   0; break;
		}
		remoteoffset = remoteoffset * 3600;
	} else
		return -1;

	return remoteoffset;
}

time_t tzoffset_sec(time_t *now)
{
	struct tm gmt, *lt;
	gint off;

	gmt = *gmtime(now);
	lt = localtime(now);

	off = (lt->tm_hour - gmt.tm_hour) * 60 + lt->tm_min - gmt.tm_min;

	if (lt->tm_year < gmt.tm_year)
		off -= 24 * 60;
	else if (lt->tm_year > gmt.tm_year)
		off += 24 * 60;
	else if (lt->tm_yday < gmt.tm_yday)
		off -= 24 * 60;
	else if (lt->tm_yday > gmt.tm_yday)
		off += 24 * 60;

	if (off >= 24 * 60)		/* should be impossible */
		off = 23 * 60 + 59;	/* if not, insert silly value */
	if (off <= -24 * 60)
		off = -(23 * 60 + 59);

	return off * 60;
}

/* calculate timezone offset */
gchar *tzoffset(time_t *now)
{
	static gchar offset_string[6];
	struct tm gmt, *lt;
	gint off;
	gchar sign = '+';

	gmt = *gmtime(now);
	lt = localtime(now);

	off = (lt->tm_hour - gmt.tm_hour) * 60 + lt->tm_min - gmt.tm_min;

	if (lt->tm_year < gmt.tm_year)
		off -= 24 * 60;
	else if (lt->tm_year > gmt.tm_year)
		off += 24 * 60;
	else if (lt->tm_yday < gmt.tm_yday)
		off -= 24 * 60;
	else if (lt->tm_yday > gmt.tm_yday)
		off += 24 * 60;

	if (off < 0) {
		sign = '-';
		off = -off;
	}

	if (off >= 24 * 60)		/* should be impossible */
		off = 23 * 60 + 59;	/* if not, insert silly value */

	sprintf(offset_string, "%c%02d%02d", sign, off / 60, off % 60);

	return offset_string;
}

void get_rfc822_date(gchar *buf, gint len)
{
	struct tm *lt;
	time_t t;
	gchar day[4], mon[4];
	gint dd, hh, mm, ss, yyyy;

	t = time(NULL);
	lt = localtime(&t);

	sscanf(asctime(lt), "%3s %3s %d %d:%d:%d %d\n",
	       day, mon, &dd, &hh, &mm, &ss, &yyyy);
	g_snprintf(buf, len, "%s, %d %s %d %02d:%02d:%02d %s",
		   day, dd, mon, yyyy, hh, mm, ss, tzoffset(&t));
}

/* just a wrapper to suppress the warning of gcc about %c */
size_t my_strftime(gchar *s, size_t max, const gchar *format,
		   const struct tm *tm)
{
	return strftime(s, max, format, tm);
}

void debug_set_mode(gboolean mode)
{
	debug_mode = mode;
}

gboolean debug_get_mode(void)
{
	return debug_mode;
}

void debug_print_real(const gchar *format, ...)
{
	va_list args;
	gchar buf[BUFFSIZE];

	if (!debug_mode) return;

	va_start(args, format);
	g_vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	g_print("%s", buf);
}

void * subject_table_lookup(GHashTable *subject_table, gchar * subject)
{
	if (subject == NULL)
		subject = "";
	else
		subject += subject_get_prefix_length(subject);

	return g_hash_table_lookup(subject_table, subject);
}

void subject_table_insert(GHashTable *subject_table, gchar * subject,
			  void * data)
{
	if (subject == NULL || *subject == 0)
		return;
	subject += subject_get_prefix_length(subject);
	g_hash_table_insert(subject_table, subject, data);
}

void subject_table_remove(GHashTable *subject_table, gchar * subject)
{
	if (subject == NULL)
		return;

	subject += subject_get_prefix_length(subject);	
	g_hash_table_remove(subject_table, subject);
}

/*!
 *\brief	Check if a string is prefixed with known (combinations) 
 *		of prefixes. The function assumes that each prefix 
 *		is terminated by zero or exactly _one_ space.
 *
 *\param	str String to check for a prefixes
 *
 *\return	int Number of chars in the prefix that should be skipped 
 *		for a "clean" subject line. If no prefix was found, 0
 *		is returned.
 */		
int subject_get_prefix_length(const gchar *subject)
{
	/*!< Array with allowable reply prefixes regexps. */
	static const gchar * const prefixes[] = {
		"Re\\:",			/* "Re:" */
		"Re\\[[1-9][0-9]*\\]\\:",	/* "Re[XXX]:" (non-conforming news mail clients) */
		"Antw\\:",			/* "Antw:" (Dutch / German Outlook) */
		"Aw\\:",			/* "Aw:"   (German) */
		"Antwort\\:",			/* "Antwort:" (German Lotus Notes) */
		"Res\\:",			/* "Res:" (Brazilian Outlook) */
		"Fw\\:",			/* "Fw:" Forward */
		"Enc\\:",			/* "Enc:" Forward (Brazilian Outlook) */
		"Odp\\:",			/* "Odp:" Re (Polish Outlook) */
		"Rif\\:"			/* "Rif:" (Italian Outlook) */
		/* add more */
	};
	const int PREFIXES = sizeof prefixes / sizeof prefixes[0];
	int n;
	regmatch_t pos;
	static regex_t regex;
	static gboolean init_;

	if (!subject) return 0;
	if (!*subject) return 0;

	if (!init_) {
		GString *s = g_string_new("");
		
		for (n = 0; n < PREFIXES; n++)
			/* Terminate each prefix regexpression by a
			 * "\ ?" (zero or ONE space), and OR them */
			g_string_append_printf(s, "(%s\\ ?)%s",
					  prefixes[n],
					  n < PREFIXES - 1 ? 
					  "|" : "");
		
		g_string_prepend(s, "(");
		g_string_append(s, ")+");	/* match at least once */
		g_string_prepend(s, "^\\ *");	/* from beginning of line */
		

		/* We now have something like "^\ *((PREFIX1\ ?)|(PREFIX2\ ?))+" 
		 * TODO: Should this be       "^\ *(((PREFIX1)|(PREFIX2))\ ?)+" ??? */
		if (regcomp(&regex, s->str, REG_EXTENDED | REG_ICASE)) { 
			debug_print("Error compiling regexp %s\n", s->str);
			g_string_free(s, TRUE);
			return 0;
		} else {
			init_ = TRUE;
			g_string_free(s, TRUE);
		}
	}
	
	if (!regexec(&regex, subject, 1, &pos, 0) && pos.rm_so != -1)
		return pos.rm_eo;
	else
		return 0;
}

guint g_stricase_hash(gconstpointer gptr)
{
	guint hash_result = 0;
	const char *str;

	for (str = gptr; str && *str; str++) {
		if (isupper((guchar)*str)) hash_result += (*str + ' ');
		else hash_result += *str;
	}

	return hash_result;
}

gint g_stricase_equal(gconstpointer gptr1, gconstpointer gptr2)
{
	const char *str1 = gptr1;
	const char *str2 = gptr2;

	return !g_utf8_collate(str1, str2);
}

gint g_int_compare(gconstpointer a, gconstpointer b)
{
	return GPOINTER_TO_INT(a) - GPOINTER_TO_INT(b);
}

gchar *generate_msgid(gchar *buf, gint len)
{
	struct tm *lt;
	time_t t;
	gchar *addr;

	t = time(NULL);
	lt = localtime(&t);

	addr = g_strconcat("@", get_domain_name(), NULL);

	g_snprintf(buf, len, "%04d%02d%02d%02d%02d%02d.%08x%s",
		   lt->tm_year + 1900, lt->tm_mon + 1,
		   lt->tm_mday, lt->tm_hour,
		   lt->tm_min, lt->tm_sec,
		   (guint) rand(), addr);

	g_free(addr);
	return buf;
}

/*
   quote_cmd_argument()
   
   return a quoted string safely usable in argument of a command.
   
   code is extracted and adapted from etPan! project -- DINH V. Ho�.
*/

gint quote_cmd_argument(gchar * result, guint size,
			const gchar * path)
{
	const gchar * p;
	gchar * result_p;
	guint remaining;

	result_p = result;
	remaining = size;

	for(p = path ; * p != '\0' ; p ++) {

		if (isalnum((guchar)*p) || (* p == '/')) {
			if (remaining > 0) {
				* result_p = * p;
				result_p ++; 
				remaining --;
			}
			else {
				result[size - 1] = '\0';
				return -1;
			}
		}
		else { 
			if (remaining >= 2) {
				* result_p = '\\';
				result_p ++; 
				* result_p = * p;
				result_p ++; 
				remaining -= 2;
			}
			else {
				result[size - 1] = '\0';
				return -1;
			}
		}
	}
	if (remaining > 0) {
		* result_p = '\0';
	}
	else {
		result[size - 1] = '\0';
		return -1;
	}
  
	return 0;
}

typedef struct 
{
	GNode 		*parent;
	GNodeMapFunc	 func;
	gpointer	 data;
} GNodeMapData;

static void g_node_map_recursive(GNode *node, gpointer data)
{
	GNodeMapData *mapdata = (GNodeMapData *) data;
	GNode *newnode;
	GNodeMapData newmapdata;
	gpointer newdata;

	newdata = mapdata->func(node->data, mapdata->data);
	if (newdata != NULL) {
		newnode = g_node_new(newdata);
		g_node_append(mapdata->parent, newnode);

		newmapdata.parent = newnode;
		newmapdata.func = mapdata->func;
		newmapdata.data = mapdata->data;

		g_node_children_foreach(node, G_TRAVERSE_ALL, g_node_map_recursive, &newmapdata);
	}
}

GNode *g_node_map(GNode *node, GNodeMapFunc func, gpointer data)
{
	GNode *root;
	GNodeMapData mapdata;

	g_return_val_if_fail(node != NULL, NULL);
	g_return_val_if_fail(func != NULL, NULL);

	root = g_node_new(func(node->data, data));

	mapdata.parent = root;
	mapdata.func = func;
	mapdata.data = data;

	g_node_children_foreach(node, G_TRAVERSE_ALL, g_node_map_recursive, &mapdata);

	return root;
}

#define HEX_TO_INT(val, hex)			\
{						\
	gchar c = hex;				\
						\
	if ('0' <= c && c <= '9') {		\
		val = c - '0';			\
	} else if ('a' <= c && c <= 'f') {	\
		val = c - 'a' + 10;		\
	} else if ('A' <= c && c <= 'F') {	\
		val = c - 'A' + 10;		\
	} else {				\
		val = -1;			\
	}					\
}

gboolean get_hex_value(guchar *out, gchar c1, gchar c2)
{
	gint hi, lo;

	HEX_TO_INT(hi, c1);
	HEX_TO_INT(lo, c2);

	if (hi == -1 || lo == -1)
		return FALSE;

	*out = (hi << 4) + lo;
	return TRUE;
}

#define INT_TO_HEX(hex, val)		\
{					\
	if ((val) < 10)			\
		hex = '0' + (val);	\
	else				\
		hex = 'A' + (val) - 10;	\
}

void get_hex_str(gchar *out, guchar ch)
{
	gchar hex;

	INT_TO_HEX(hex, ch >> 4);
	*out++ = hex;
	INT_TO_HEX(hex, ch & 0x0f);
	*out++ = hex;
}

#undef REF_DEBUG
#ifndef REF_DEBUG
#define G_PRINT_REF 1 == 1 ? (void) 0 : (void)
#else
#define G_PRINT_REF g_print
#endif

/*!
 *\brief	Register ref counted pointer. It is based on GBoxed, so should
 *		work with anything that uses the GType system. The semantics
 *		are similar to a C++ auto pointer, with the exception that
 *		C doesn't have automatic closure (calling destructors) when 
 *		exiting a block scope.
 *		Use the \ref G_TYPE_AUTO_POINTER macro instead of calling this
 *		function directly.
 *
 *\return	GType A GType type.
 */
GType g_auto_pointer_register(void)
{
	static GType auto_pointer_type;
	if (!auto_pointer_type)
		auto_pointer_type =
			g_boxed_type_register_static
				("G_TYPE_AUTO_POINTER",
				 (GBoxedCopyFunc) g_auto_pointer_copy,
				 (GBoxedFreeFunc) g_auto_pointer_free);
	return auto_pointer_type;						     
}

/*!
 *\brief	Structure with g_new() allocated pointer guarded by the
 *		auto pointer
 */
typedef struct AutoPointerRef {
	void	      (*free) (gpointer);
	gpointer	pointer;
	glong		cnt;
} AutoPointerRef;

/*!
 *\brief	The auto pointer opaque structure that references the
 *		pointer guard block.
 */
typedef struct AutoPointer {
	AutoPointerRef *ref;
	gpointer	ptr; /*!< access to protected pointer */
} AutoPointer;

/*!
 *\brief	Creates an auto pointer for a g_new()ed pointer. Example:
 *
 *\code	
 *
 *		... tell gtk_list_store it should use a G_TYPE_AUTO_POINTER
 *		... when assigning, copying and freeing storage elements
 *
 *		gtk_list_store_new(N_S_COLUMNS, 
 *				   G_TYPE_AUTO_POINTER,
 *				   -1);
 *
 *
 *		Template *precious_data = g_new0(Template, 1);
 *		g_pointer protect = g_auto_pointer_new(precious_data);
 *
 *		gtk_list_store_set(container, &iter,
 *				   S_DATA, protect,
 *				   -1);
 *
 *		... the gtk_list_store has copied the pointer and 
 *		... incremented its reference count, we should free
 *		... the auto pointer (in C++ a destructor would do
 *		... this for us when leaving block scope)
 * 
 *		g_auto_pointer_free(protect);
 *
 *		... gtk_list_store_set() now manages the data. When
 *		... *explicitly* requesting a pointer from the list 
 *		... store, don't forget you get a copy that should be 
 *		... freed with g_auto_pointer_free() eventually.
 *
 *\endcode
 *
 *\param	pointer Pointer to be guarded.
 *
 *\return	GAuto * Pointer that should be used in containers with
 *		GType support.
 */
GAuto *g_auto_pointer_new(gpointer p)
{
	AutoPointerRef *ref;
	AutoPointer    *ptr;
	
	if (p == NULL) 
		return NULL;

	ref = g_new0(AutoPointerRef, 1);
	ptr = g_new0(AutoPointer, 1);

	ref->pointer = p;
	ref->free = g_free;
	ref->cnt = 1;

	ptr->ref = ref;
	ptr->ptr = p;

#ifdef REF_DEBUG
	G_PRINT_REF ("XXXX ALLOC(%lx)\n", p);
#endif
	return ptr;
}

/*!
 *\brief	Allocate an autopointer using the passed \a free function to
 *		free the guarded pointer
 */
GAuto *g_auto_pointer_new_with_free(gpointer p, GFreeFunc free_)
{
	AutoPointer *aptr;
	
	if (p == NULL)
		return NULL;

	aptr = g_auto_pointer_new(p);
	aptr->ref->free = free_;
	return aptr; 
}

gpointer g_auto_pointer_get_ptr(GAuto *auto_ptr)
{
	if (auto_ptr == NULL) 
		return NULL;
	return ((AutoPointer *) auto_ptr)->ptr; 
}

/*!
 *\brief	Copies an auto pointer by. It's mostly not necessary
 *		to call this function directly, unless you copy/assign
 *		the guarded pointer.
 *
 *\param	auto_ptr Auto pointer returned by previous call to 
 *		g_auto_pointer_new_XXX()
 *
 *\return	gpointer An auto pointer
 */
GAuto *g_auto_pointer_copy(GAuto *auto_ptr)
{
	AutoPointer	*ptr;
	AutoPointerRef	*ref;
	AutoPointer	*newp;

	if (auto_ptr == NULL) 
		return NULL;

	ptr = auto_ptr;
	ref = ptr->ref;
	newp = g_new0(AutoPointer, 1);

	newp->ref = ref;
	newp->ptr = ref->pointer;
	++(ref->cnt);
	
#ifdef REF_DEBUG
	G_PRINT_REF ("XXXX COPY(%lx) -- REF (%d)\n", ref->pointer, ref->cnt);
#endif
	return newp;
}

/*!
 *\brief	Free an auto pointer
 */
void g_auto_pointer_free(GAuto *auto_ptr)
{
	AutoPointer	*ptr;
	AutoPointerRef	*ref;
	
	if (auto_ptr == NULL)
		return;

	ptr = auto_ptr;
	ref = ptr->ref;

	if (--(ref->cnt) == 0) {
#ifdef REF_DEBUG
		G_PRINT_REF ("XXXX FREE(%lx) -- REF (%d)\n", ref->pointer, ref->cnt);
#endif
		ref->free(ref->pointer);
		g_free(ref);
	} 
#ifdef REF_DEBUG
	else
		G_PRINT_REF ("XXXX DEREF(%lx) -- REF (%d)\n", ref->pointer, ref->cnt);
#endif
	g_free(ptr);		
}

void replace_returns(gchar *str)
{
	if (!str)
		return;

	while (strstr(str, "\n")) {
		*strstr(str, "\n") = ' ';
	}
	while (strstr(str, "\r")) {
		*strstr(str, "\r") = ' ';
	}
}

/* get_uri_part() - retrieves a URI starting from scanpos.
		    Returns TRUE if succesful */
gboolean get_uri_part(const gchar *start, const gchar *scanpos,
			     const gchar **bp, const gchar **ep)
{
	const gchar *ep_;

	g_return_val_if_fail(start != NULL, FALSE);
	g_return_val_if_fail(scanpos != NULL, FALSE);
	g_return_val_if_fail(bp != NULL, FALSE);
	g_return_val_if_fail(ep != NULL, FALSE);

	*bp = scanpos;

	/* find end point of URI */
	for (ep_ = scanpos; *ep_ != '\0'; ep_++) {
		if (!isgraph(*(const guchar *)ep_) ||
		    !IS_ASCII(*(const guchar *)ep_) ||
		    strchr("[]{}()<>\"", *ep_))
			break;
	}

	/* no punctuation at end of string */

	/* FIXME: this stripping of trailing punctuations may bite with other URIs.
	 * should pass some URI type to this function and decide on that whether
	 * to perform punctuation stripping */

#define IS_REAL_PUNCT(ch)	(ispunct(ch) && ((ch) != '/')) 

	for (; ep_ - 1 > scanpos + 1 &&
	       IS_REAL_PUNCT(*(const guchar *)(ep_ - 1));
	     ep_--)
		;

#undef IS_REAL_PUNCT

	*ep = ep_;

	return TRUE;		
}

gchar *make_uri_string(const gchar *bp, const gchar *ep)
{
	return g_strndup(bp, ep - bp);
}

/* valid mail address characters */
#define IS_RFC822_CHAR(ch) \
	(IS_ASCII(ch) && \
	 (ch) > 32   && \
	 (ch) != 127 && \
	 !g_ascii_isspace(ch) && \
	 !strchr("(),;<>\"", (ch)))

/* alphabet and number within 7bit ASCII */
#define IS_ASCII_ALNUM(ch)	(IS_ASCII(ch) && g_ascii_isalnum(ch))
#define IS_QUOTE(ch) ((ch) == '\'' || (ch) == '"')

static GHashTable *create_domain_tab(void)
{
	static const gchar *toplvl_domains [] = {
	    "museum", "aero",
	    "arpa", "coop", "info", "name", "biz", "com", "edu", "gov",
	    "int", "mil", "net", "org", "ac", "ad", "ae", "af", "ag",
	    "ai", "al", "am", "an", "ao", "aq", "ar", "as", "at", "au",
	    "aw", "az", "ba", "bb", "bd", "be", "bf", "bg", "bh", "bi",
	    "bj", "bm", "bn", "bo", "br", "bs", "bt", "bv", "bw", "by",
	    "bz", "ca", "cc", "cd", "cf", "cg", "ch", "ci", "ck", "cl",
	    "cm", "cn", "co", "cr", "cu", "cv", "cx", "cy", "cz", "de",
	    "dj", "dk", "dm", "do", "dz", "ec", "ee", "eg", "eh", "er",
	    "es", "et", "fi", "fj", "fk", "fm", "fo", "fr", "ga", "gd",
	    "ge", "gf", "gg", "gh", "gi", "gl", "gm", "gn", "gp", "gq",
	    "gr", "gs", "gt", "gu", "gw", "gy", "hk", "hm", "hn", "hr",
	    "ht", "hu", "id", "ie", "il", "im", "in", "io", "iq", "ir",
	    "is", "it", "je", "jm", "jo", "jp", "ke", "kg", "kh", "ki",
	    "km", "kn", "kp", "kr", "kw", "ky", "kz", "la", "lb", "lc",
	    "li", "lk", "lr", "ls", "lt", "lu", "lv", "ly", "ma", "mc",
	    "md", "mg", "mh", "mk", "ml", "mm", "mn", "mo", "mp", "mq",
	    "mr", "ms", "mt", "mu", "mv", "mw", "mx", "my", "mz", "na",
	    "nc", "ne", "nf", "ng", "ni", "nl", "no", "np", "nr", "nu",
	    "nz", "om", "pa", "pe", "pf", "pg", "ph", "pk", "pl", "pm",
	    "pn", "pr", "ps", "pt", "pw", "py", "qa", "re", "ro", "ru",
	    "rw", "sa", "sb", "sc", "sd", "se", "sg", "sh", "si", "sj",
	    "sk", "sl", "sm", "sn", "so", "sr", "st", "sv", "sy", "sz",
	    "tc", "td", "tf", "tg", "th", "tj", "tk", "tm", "tn", "to",
	    "tp", "tr", "tt", "tv", "tw", "tz", "ua", "ug", "uk", "um",
	    "us", "uy", "uz", "va", "vc", "ve", "vg", "vi", "vn", "vu",
            "wf", "ws", "ye", "yt", "yu", "za", "zm", "zw" 
	};
	gint n;
	GHashTable *htab = g_hash_table_new(g_stricase_hash, g_stricase_equal);
	
	g_return_val_if_fail(htab, NULL);
	for (n = 0; n < sizeof toplvl_domains / sizeof toplvl_domains[0]; n++) 
		g_hash_table_insert(htab, (gpointer) toplvl_domains[n], (gpointer) toplvl_domains[n]);
	return htab;
}

static gboolean is_toplvl_domain(GHashTable *tab, const gchar *first, const gchar *last)
{
	const gint MAX_LVL_DOM_NAME_LEN = 6;
	gchar buf[MAX_LVL_DOM_NAME_LEN + 1];
	const gchar *m = buf + MAX_LVL_DOM_NAME_LEN + 1;
	register gchar *p;
	
	if (last - first > MAX_LVL_DOM_NAME_LEN || first > last)
		return FALSE;

	for (p = buf; p < m &&  first < last; *p++ = *first++)
		;
	*p = 0;

	return g_hash_table_lookup(tab, buf) != NULL;
}

/* get_email_part() - retrieves an email address. Returns TRUE if succesful */
gboolean get_email_part(const gchar *start, const gchar *scanpos,
			       const gchar **bp, const gchar **ep)
{
	/* more complex than the uri part because we need to scan back and forward starting from
	 * the scan position. */
	gboolean result = FALSE;
	const gchar *bp_ = NULL;
	const gchar *ep_ = NULL;
	static GHashTable *dom_tab;
	const gchar *last_dot = NULL;
	const gchar *prelast_dot = NULL;
	const gchar *last_tld_char = NULL;
	
	/* the informative part of the email address (describing the name
	 * of the email address owner) may contain quoted parts. the
	 * closure stack stores the last encountered quotes. */
	gchar closure_stack[128];
	gchar *ptr = closure_stack;

	g_return_val_if_fail(start != NULL, FALSE);
	g_return_val_if_fail(scanpos != NULL, FALSE);
	g_return_val_if_fail(bp != NULL, FALSE);
	g_return_val_if_fail(ep != NULL, FALSE);

	if (!dom_tab)
		dom_tab = create_domain_tab();
	g_return_val_if_fail(dom_tab, FALSE);	

	/* scan start of address */
	for (bp_ = scanpos - 1;
	     bp_ >= start && IS_RFC822_CHAR(*(const guchar *)bp_); bp_--)
		;

	/* TODO: should start with an alnum? */
	bp_++;
	for (; bp_ < scanpos && !IS_ASCII_ALNUM(*(const guchar *)bp_); bp_++)
		;

	if (bp_ != scanpos) {
		/* scan end of address */
		for (ep_ = scanpos + 1;
		     *ep_ && IS_RFC822_CHAR(*(const guchar *)ep_); ep_++)
			if (*ep_ == '.') {
				prelast_dot = last_dot;
				last_dot = ep_;
		 		if (*(last_dot + 1) == '.') {
					if (prelast_dot == NULL)
	        				return FALSE;
					last_dot = prelast_dot;
					break;
				}
			}

		/* TODO: really should terminate with an alnum? */
		for (; ep_ > scanpos && !IS_ASCII_ALNUM(*(const guchar *)ep_);
		     --ep_)
			;
		ep_++;

		if (last_dot == NULL)
			return FALSE;
		if (last_dot >= ep_)
			last_dot = prelast_dot;
		if (last_dot == NULL || (scanpos + 1 >= last_dot))
			return FALSE;
		last_dot++;

		for (last_tld_char = last_dot; last_tld_char < ep_; last_tld_char++)
			if (*last_tld_char == '?')
				break;

		if (is_toplvl_domain(dom_tab, last_dot, last_tld_char))
			result = TRUE;

		*ep = ep_;
		*bp = bp_;
	}

	if (!result) return FALSE;

	if (*ep_ && *(bp_ - 1) == '"' && *(ep_) == '"' 
	&& *(ep_ + 1) == ' ' && *(ep_ + 2) == '<'
	&& IS_RFC822_CHAR(*(ep_ + 3))) {
		/* this informative part with an @ in it is 
		 * followed by the email address */
		ep_ += 3;
		
		/* go to matching '>' (or next non-rfc822 char, like \n) */
		for (; *ep_ != '>' && *ep != '\0' && IS_RFC822_CHAR(*ep_); ep_++)
			;
			
		/* include the bracket */
		if (*ep_ == '>') ep_++;
		
		/* include the leading quote */		
		bp_--;

		*ep = ep_;
		*bp = bp_;
		return TRUE;
	}

	/* skip if it's between quotes "'alfons@proteus.demon.nl'" <alfons@proteus.demon.nl> */
	if (bp_ - 1 > start && IS_QUOTE(*(bp_ - 1)) && IS_QUOTE(*ep_))
		return FALSE;

	/* see if this is <bracketed>; in this case we also scan for the informative part. */
	if (bp_ - 1 <= start || *(bp_ - 1) != '<' || *ep_ != '>')
		return TRUE;

#define FULL_STACK()	((size_t) (ptr - closure_stack) >= sizeof closure_stack)
#define IN_STACK()	(ptr > closure_stack)
/* has underrun check */
#define POP_STACK()	if(IN_STACK()) --ptr
/* has overrun check */
#define PUSH_STACK(c)	if(!FULL_STACK()) *ptr++ = (c); else return TRUE
/* has underrun check */
#define PEEK_STACK()	(IN_STACK() ? *(ptr - 1) : 0)

	ep_++;

	/* scan for the informative part. */
	for (bp_ -= 2; bp_ >= start; bp_--) {
		/* if closure on the stack keep scanning */
		if (PEEK_STACK() == *bp_) {
			POP_STACK();
			continue;
		}
		if (*bp_ == '\'' || *bp_ == '"') {
			PUSH_STACK(*bp_);
			continue;
		}

		/* if nothing in the closure stack, do the special conditions
		 * the following if..else expression simply checks whether 
		 * a token is acceptable. if not acceptable, the clause
		 * should terminate the loop with a 'break' */
		if (!PEEK_STACK()) {
			if (*bp_ == '-'
			&& (((bp_ - 1) >= start) && isalnum(*(bp_ - 1)))
			&& (((bp_ + 1) < ep_)    && isalnum(*(bp_ + 1)))) {
				/* hyphens are allowed, but only in
				   between alnums */
			} else if (!strchr(",;:=?./+<>!&\r\n\t", *bp_)) {
				/* but anything not being a punctiation
				   is ok */
			} else {
				break; /* anything else is rejected */
			}
		}
	}

	bp_++;

#undef PEEK_STACK
#undef PUSH_STACK
#undef POP_STACK
#undef IN_STACK
#undef FULL_STACK

	/* scan forward (should start with an alnum) */
	for (; *bp_ != '<' && isspace(*bp_) && *bp_ != '"'; bp_++)
		;

	*ep = ep_;
	*bp = bp_;
	
	return result;
}

#undef IS_QUOTE
#undef IS_ASCII_ALNUM
#undef IS_RFC822_CHAR

gchar *make_email_string(const gchar *bp, const gchar *ep)
{
	/* returns a mailto: URI; mailto: is also used to detect the
	 * uri type later on in the button_pressed signal handler */
	gchar *tmp;
	gchar *result;

	tmp = g_strndup(bp, ep - bp);
	result = g_strconcat("mailto:", tmp, NULL);
	g_free(tmp);

	return result;
}

gchar *make_http_string(const gchar *bp, const gchar *ep)
{
	/* returns an http: URI; */
	gchar *tmp;
	gchar *result;

	tmp = g_strndup(bp, ep - bp);
	result = g_strconcat("http://", tmp, NULL);
	g_free(tmp);

	return result;
}

static gchar *mailcap_get_command_in_file(const gchar *path, const gchar *type)
{
	FILE *fp = fopen(path, "rb");
	gchar buf[BUFFSIZE];
	gchar *result = NULL;
	if (!fp)
		return NULL;
	while (fgets(buf, sizeof (buf), fp) != NULL) {
		gchar **parts = g_strsplit(buf, ";", -1);
		gchar *trimmed = parts[0];
		while (trimmed[0] == ' ')
			trimmed++;
		while (trimmed[strlen(trimmed)-1] == ' ') 
			trimmed[strlen(trimmed)-1] = '\0';
			
		if (!strcmp(trimmed, type)) {
			trimmed = parts[1];
			while (trimmed[0] == ' ')
				trimmed++;
			while (trimmed[strlen(trimmed)-1] == ' ') 
				trimmed[strlen(trimmed)-1] = '\0';
			while (trimmed[strlen(trimmed)-1] == '\n') 
				trimmed[strlen(trimmed)-1] = '\0';
			while (trimmed[strlen(trimmed)-1] == '\r') 
				trimmed[strlen(trimmed)-1] = '\0';
			result = g_strdup(trimmed);
			g_strfreev(parts);
			fclose(fp);
			if (strstr(result, "%s") && !strstr(result, "'%s'")) {
				gchar *start = g_strdup(result);
				gchar *end = g_strdup(strstr(result, "%s")+2);
				gchar *tmp;
				*strstr(start, "%s") = '\0';
				tmp = g_strconcat(start,"'%s'",end, NULL);
				g_free(start);
				g_free(end);
				g_free(result);
				result = tmp;
			}
			return result;
		}
		g_strfreev(parts);
	}
	fclose(fp);
	return NULL;
}
gchar *mailcap_get_command_for_type(const gchar *type) 
{
	gchar *result = NULL;
	gchar *path = NULL;
	path = g_strconcat(get_home_dir(), G_DIR_SEPARATOR_S, ".mailcap", NULL);
	result = mailcap_get_command_in_file(path, type);
	g_free(path);
	if (result)
		return result;
	result = mailcap_get_command_in_file("/etc/mailcap", type);
	return result;
}
