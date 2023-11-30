/*
 * Copyright (C) 2007-2023 Yasuhiro Ohara. All rights reserved.
 */

#ifndef _FILE_H_
#define _FILE_H_

void path_disassemble (char *pathname, char **dirname, char **filename);

FILE *fopen_create (char *file, char *mode);
int file_truncate (char *file);

int redirect_stdio (FILE *std, FILE *fp);
int restore_stdio ();

#endif /*_FILE_H_*/


