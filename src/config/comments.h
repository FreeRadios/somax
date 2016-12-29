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

#define COMMENT_USER \
"# User - set the user of somad. Somad changes its permission as this user.\n"

#define COMMENT_GROUP \
"# Group - set the group of somad. Somad changes its permission as this group.\n"

#define COMMENT_BACKGROUND \
"# Background - set somad in background. Old versions of somaplayer have\n" \
"# some problems in background. If your somaplayer has this problem, update\n"\
"# it.\n"

#define COMMENT_UNIXSOCKET \
"# UnixSocket - if this flag is true, somad actives a unix socket. Else a\n" \
"# tcp socket.\n"

#define COMMENT_UNIXPATH \
"# UnixPath - Default /tmp/somad.sock\n"

#define COMMENT_SERVERNAME \
"# ServerName - set the IP to binding. Default 0.0.0.0 so any interfaces.\n"

#define COMMENT_PORT \
"# Port - Default: 12521\n"

#define COMMENT_LISTEN \
"# Listen - how many clients can admin somad in the same time? Default: 5\n"

#define COMMENT_PASSWORD \
"# Password - password for admin. If you don't set this variable, the\n" \
"# password is empty.\n"

#define COMMENT_SSL \
"# Ssl - if this flag is actived, somad runs in SSL mode.\n" \
"# If you want create a privatekey and certificate files, use this commands:\n"\
"# openssl genrsa -out private.pem 1024\n" \
"# openssl req -new -x509 -key private.pem -out certificate.pem\n"

#define COMMENT_CERTIFICATE \
"# Certificate - The SSL certificate (PEM file)\n"

#define COMMENT_PRIVATEKEY \
"# PrivateKey - The SSL private key (PEM file)\n"

#define COMMENT_DEBUG \
"# Debug - change the verbouse in the log. The value is 0 to 3.\n" \
"# 0 -> Disactive\n" \
"# 1 -> Only Error\n" \
"# 2 -> Error and Warning\n" \
"# 3 -> Info, error and warning\n" \
"# 4 -> Debug (only for developers)\n"

#define COMMENT_LOGFILE \
"# LogFile - Set the LogFile\n"

#define COMMENT_PIDFILE \
"# PidFile - Set the PidFile\n"

#define COMMENT_PATHITEM \
"# PathItem - Like in palinsesto.cfg item, this is the list is absolute, so\n"\
"# if you insert some values in the list, any other list in the trasmission\n"\
"# contains those elements.\n"

#define COMMENT_PATHSPOT \
"# PathSpot - like PathItem.\n"

#define COMMENT_PROGRAMITEM \
"# This program is the default software for the \"Files\". If your software\n" \
"# are in the correct PATH enviroment, you can don't set the full path:\n"

#define COMMENT_OPTIONSITEM \
"# Some parameters...\n"

#define COMMENT_PROGRAMSTREAM \
"# If you want use another software per streaming, this is the variable.\n" \
"# If you don't set this variable, the ProgramItem software plays any file.\n"

#define COMMENT_OPTIONSSTREAM \
"# Like OptionsItem.\n"

#define COMMENT_PALINSESTO \
"# The palinsesto file:\n"

#define COMMENT_SPOT \
"# The spot file:\n"

#define COMMENT_XMLSYNTAX \
"# The true or false xml read... Somax need this flag to true.\n"

#define COMMENT_SYMLINKS \
"# If this flag is true, somad follows the symbolic links founded in,\n" \
"# PathItem, PathSpot and Path list of directories and in the.\n" \
"# subdirectories. Default is: false.\n"

#define COMMENT_PATHMODULES \
"# PathModules - is the path of modules. Default: /usr/share/soma/\n"

#define COMMENT_DISTRIBUITEDPATH \
"# DistribuitedPath - List of path for the filesystem distribuited of somad\n"

#define COMMENT_HOSTALLOW \
"# \n"

#define COMMENT_HOSTDENY \
"# \n"

/* EOF */
