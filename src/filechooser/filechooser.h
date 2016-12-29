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

#ifndef FILE_CHOOSER_H
#define FILE_CHOOSER_H

/* filechooser.c */
void *file_chooser (char *, GtkSelectionMode, GtkFileChooserAction);
void file_chooser_cb (char *, GtkSelectionMode, GtkFileChooserAction, void (*func)(void *, void *), void *);

/* r_filechooser.c */
void *remote_file_chooser (char *, GtkSelectionMode, GtkFileChooserAction, soma_controller *);
void remote_file_chooser_cb (char *, GtkSelectionMode, GtkFileChooserAction, soma_controller *,void (*func)(void *, void *), void *);

#endif

/* EOF */
