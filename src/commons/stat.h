/* SomaX - Copyright (C) 2005-2007 bakunin - Andrea Marchesini 
 *                                 <bakunin@autistici.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 * 02110-1301, USA.
 */

#ifndef __STAT_H__
#define __STAT_H__

typedef struct {
  soma_stat *stat;
  int ref;
} somax_stat_t;

/* stat.c */
void		stat_init		(void);

void		stat_check		(soma_controller *, char **);

void		stat_new		(soma_controller * controller,
					 char *file);

somax_stat_t *	stat_get		(char *file);

char *		stat_duration		(int64_t duration);

void		stat_ref		(somax_stat_t *stat);
void		stat_unref		(somax_stat_t *stat);

void		stat_struct_popup	(soma_stat *stat);
void		stat_struct_dir_popup	(soma_stat_dir *stat);

void		stat_popup		(char *file);
char *		stat_make_name		(char *file);

#endif

/* EOF */
