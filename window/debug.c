/*
 * debug.c - Debug logging
 * Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef DEBUG

#include <stdio.h>
#include <stdarg.h>

static FILE *log = NULL;

void Window_LogFile(const char *filename)
{
  if(log) fclose(log);
  log = fopen(filename,"wt");
}

void Window_LogWrite(const char *fmt, ...)
{
  char logbuffer[256];
  va_list argptr;

  va_start(argptr, fmt);
  vsprintf(logbuffer, fmt, argptr);
  va_end(argptr);

  if(log) {
    fprintf(log,logbuffer);
    fflush(log);
  } else
    fprintf(stderr,logbuffer);
}

#else

void Window_LogFile(char *filename) { }
void Window_LogWrite(char *fmt, ...) { }

#endif
