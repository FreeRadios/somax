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

#ifndef _X_H_
#define _X_H_

/* x.c */

GtkWidget *	somax_win		(void);

soma_controller *somax_open_tcp		(char *, int, char *, int);
soma_controller *somax_open_unix	(char *, char *, int);

int	somax_quit			(soma_controller *);
void	somax_free			(soma_controller *);

int	somax_stop			(soma_controller *, int);
int	somax_start			(soma_controller *);

int	somax_skip			(soma_controller *);
int	somax_skip_next			(soma_controller *);

int	somax_read_directory		(soma_controller *);
int	somax_read_palinsesto		(soma_controller *);
int	somax_read_spot			(soma_controller *);

char *	somax_get_palinsesto		(soma_controller *);
int	somax_old_palinsesto		(soma_controller *);
char *	somax_get_old_palinsesto	(soma_controller *);
int	somax_set_default_palinsesto	(soma_controller *);
int	somax_new_palinsesto_file	(soma_controller *, char *);

char *	somax_get_spot			(soma_controller *);
int	somax_old_spot			(soma_controller *);
char *	somax_get_old_spot		(soma_controller *);
int	somax_set_default_spot		(soma_controller *);
int	somax_new_spot_file		(soma_controller *, char *);

char **	somax_get_path			(soma_controller *, char *);
void	somax_get_path_free		(char **);

int	somax_nextitem_set		(GtkWidget *, char *,
					 soma_controller *c, char *);
int	somax_nextitem_path_set		(GtkWidget *, char *,
					 soma_controller *c, char *);
int	somax_nextitem_set_spot		(soma_controller *c, int);
int	somax_nextitem_remove		(GtkWidget *, char *,
					 soma_controller *c, int);

soma_stat * somax_get_stat		(GtkWidget *, char *,
					 soma_controller *, char *);
soma_stat_dir * somax_get_stat_dir	(GtkWidget *, char *,
					 soma_controller *, char *);

soma_stat * somax_get_stat_path		(GtkWidget *, char *,
					 soma_controller *, char *);
soma_stat_dir * somax_get_stat_dir_path	(GtkWidget *, char *,
					 soma_controller *, char *);

#define somax_stat_free			soma_stat_free
#define somax_stat_dir_free		soma_stat_dir_free

soma_stat * somax_local_stat		(GtkWidget *, char *, char *);
soma_stat_dir * somax_local_stat_dir	(GtkWidget *, char *, char *);

#define somax_local_stat_free		soma_local_stat_free
#define somax_local_stat_dir_free	soma_local_stat_dir_free

int	somax_remove_item		(GtkWidget *, char *,
					 soma_controller *c, int);

int	somax_remove_spot		(GtkWidget *, char *,
					 soma_controller *c, int);

int	somax_set_pause			(soma_controller *c);
int	somax_set_unpause		(soma_controller *c);

#endif

/* EOF */
