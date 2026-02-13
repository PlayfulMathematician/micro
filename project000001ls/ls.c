/*
ls.c - A single file rewrite of the ls command
Copyright (C) 2026 Playful Mathematician <me@playfulmathematician>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

 */
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <grp.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
/*
stat(const char *restrict file, struct stat *restrict buf)
stat.st_gid
*getgrgid(group id)

*/

enum LS_FLAGS {
  LS_a = 0x00000001,
  LS_l = 0x00000002,
  LS_A = 0x00000004,
  LS_A_or_a = 0x00000005
};
uint32_t flag_list[1000];

void handle_flag(uint32_t *flags, uint32_t flag) {
  if (flag == LS_a) {
    *flags &= ~LS_A;
    *flags |= LS_a;
  }
  if (flag == LS_l) {
    *flags |= LS_l;
  }
  if (flag == LS_A) {
    *flags &= ~LS_a;
    *flags |= LS_A;
  }
}

int main(int argc, char *argv[]) {
  char *path = NULL;
  uint32_t flags = 0;
  uint32_t flag_idx = 0;
  for (int i = 0; i < argc; i++) {
    if (i == 0) {
      continue;
    }
    const char *param = argv[i];
    if (param[0] == '-') {
      if (param[1] == '-') {
        uint32_t cur_flag = 0;
        if (!strcmp(param, "--all")) {
          cur_flag = LS_a;
        }

        if (!strcmp(param, "--almost-all")) {
          cur_flag = LS_A;
        }

        flag_list[flag_idx] = cur_flag;
        flag_idx++;
      } else {
        unsigned long length = strlen(param);

        for (unsigned long j = 0; j < length; j++) {
          if (j == 0) {
            continue;
          }
          char c = param[j];
          uint32_t cur_flag = 0;
          switch (c) {
          case 'a':
            cur_flag = LS_a;
            break;
          case 'l':
            cur_flag = LS_l;
            break;
          case 'A':
            cur_flag = LS_A;
            break;
          }
          flag_list[flag_idx] = cur_flag;
          flag_idx++;
        }
      }
    } else {
      path = param;
    }
  }
  for (uint32_t fli = 0; fli < flag_idx; fli++) {
    handle_flag(&flags, flag_list[fli]);
  }

  if (path == NULL) {
    path = ".";
  }

  DIR *dir = opendir(path);

  if (dir == NULL) {
    return 0;
  }
  while (true) {
    struct dirent *dirent_ptr = readdir(dir);
    if (dirent_ptr == NULL) {
      break;
    }
    const char *filename = dirent_ptr->d_name;
    if (filename[0] == '.' && !(flags & LS_A_or_a)) {
      continue;
    }
    if ((!strcmp(filename, ".") || !strcmp(filename, "..")) && (flags & LS_A)) {
      continue;
    }
    printf("%s", filename);
    if (flags & LS_l) {
      printf("\n");
    } else {
      printf(" ");
    }
  }
  // make sure to close
  closedir(dir);
  return 0;
}
