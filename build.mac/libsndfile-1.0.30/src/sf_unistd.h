/*
** Copyright (C) 2002-2017 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* Microsoft declares some 'unistd.h' functions in 'io.h'. */

#include <sys/stat.h>
#ifdef HAVE_IO_H
#include <io.h>
#endif

/* Some defines that microsoft 'forgot' to implement. */

#ifndef R_OK
#define R_OK	4		/* Test for read permission.  */
#endif

#ifndef W_OK
#define W_OK	2		/* Test for write permission.  */
#endif

#ifndef X_OK
#ifdef _WIN32
#define	X_OK	0
#else
#define	X_OK	1		/* execute permission - unsupported in windows*/
#endif
#endif

#ifndef F_OK
#define	F_OK	0		/* Test for existence.  */
#endif

#ifndef S_IRWXU
#define	S_IRWXU 	0000700	/* rwx, owner */
#endif

#ifndef		S_IRUSR
#define		S_IRUSR	0000400	/* read permission, owner */
#endif

#ifndef		S_IWUSR
#define		S_IWUSR	0000200	/* write permission, owner */
#endif

#ifndef		S_IXUSR
#define		S_IXUSR	0000100	/* execute/search permission, owner */
#endif

/* Windows (except MinGW) doesn't have group permissions so set all these to zero. */
#ifndef S_IRWXG
#define S_IRWXG		0	/* rwx, group */
#endif

#ifndef S_IRGRP
#define S_IRGRP		0	/* read permission, group */
#endif

#ifndef S_IWGRP
#define S_IWGRP		0	/* write permission, grougroup */
#endif

#ifndef S_IXGRP
#define S_IXGRP		0	/* execute/search permission, group */
#endif

/* Windows (except MinGW) doesn't have others permissions so set all these to zero. */
#ifndef S_IRWXO
#define S_IRWXO		0	/* rwx, other */
#endif

#ifndef S_IROTH
#define S_IROTH		0	/* read permission, other */
#endif

#ifndef S_IWOTH
#define S_IWOTH		0	/* write permission, other */
#endif

#ifndef S_IXOTH
#define S_IXOTH		0	/* execute/search permission, other */
#endif

#ifndef S_ISFIFO
#define S_ISFIFO(mode)	(((mode) & _S_IFMT) == _S_IFIFO)
#endif

#ifndef S_ISREG
#define	S_ISREG(mode)	(((mode) & _S_IFREG) == _S_IFREG)
#endif

/*
**	Don't know if these are still needed.
**
**	#define	_IFMT		_S_IFMT
**	#define _IFREG		_S_IFREG
*/

