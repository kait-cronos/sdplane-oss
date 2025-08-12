/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2023-2025 Yasuhiro Ohara
 *
 * This software is released under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#ifndef __FILE_H__
#define __FILE_H__

void path_disassemble (char *pathname, char **dirname, char **filename);

FILE *fopen_create (char *file, char *mode);
int file_truncate (char *file);

int redirect_stdio (FILE *std, FILE *fp);
int restore_stdio ();

int dirent_cmp (const void *va, const void *vb);
struct dirent *dirent_copy (struct dirent *dirent);

#endif /*__FILE_H__*/
