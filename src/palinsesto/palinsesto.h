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

#ifndef PALINSESTO_H
#define PALINSESTO_H

/* spot.c */
void spot_free (struct somad_spot_data *);
struct somad_spot_data *spot_dump (struct somad_spot_data *);

int spot_check_file (char *);
int spot_parser(char *, struct somad_spot_data **);
int spot_parser_file(char *, struct somad_spot_data **);
int spot_save_file(char *, struct somad_spot_data *);

struct somad_spot_data *spot_now(struct somad_spot_data *);
void spot_insert (struct somad_spot_data *, struct somad_spot_data **);

/* palinsesto.c */
void palinsesto_free (struct somad_data *);
struct somad_data *palinsesto_dump (struct somad_data *);
GList *palinsesto_now (struct somad_data *);

int palinsesto_check_file (char *);
int palinsesto_parser(char *, struct somad_data **);
int palinsesto_parser_file(char *, struct somad_data **);
int palinsesto_save_file(char *, struct somad_data *);

void palinsesto_insert (struct somad_data *, struct somad_data **);

char *palinsesto_trim (char *);

/* timer.c */
int timer_check_item(int, int, int ,int);
int timer_check_day(struct somad_time *, int, int, int, int);
int timer_check(struct somad_time *, struct tm *);
void timer_time_to_string (struct somad_time *, char **, char **);
struct somad_time *timer_time_parser (char *, char *, int);
char * p_old_markup (char *text);
char * p_xml_markup (char *text);
char *p_utf8(char *text);

#endif

/* EOF */
