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
#include <limits.h>
#include <pwd.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

enum LS_FLAGS {
  LS_a = 0x00000001,
  LS_l = 0x00000002,
  LS_A = 0x00000004,
  LS_A_or_a = 0x00000005
};

typedef struct {
  char *filename;
  char *permissions;
  char* link_count;
  char *owner;
  char *owner_group;
  char* file_size; // because -h
} FileInfo;

void print_file(const char *filename, char *path, uint32_t flags) {
  if (filename[0] == '.' && !(flags & LS_A_or_a)) {
    return;
  }
  if ((!strcmp(filename, ".") || !strcmp(filename, "..")) && (flags & LS_A)) {
    return;
  }

  if (!(flags & LS_l)) {
    printf("%s\n", filename);
    return;
  }
  char a_path[4000] = "";
  strcpy(a_path, path);
  strcat(a_path, "/");
  strcat(a_path, filename);
  /// char *real_path = realpath(a_path, NULL);
  struct stat file_stat = {0};
  lstat(a_path, &file_stat);
  mode_t mod = file_stat.st_mode;
  char permissions_string[12] = "rwxrwxrwx\0\0\0";
  for (int permission = 0; permission < 9; permission++) {
    if (!(mod & (1 << (8 - permission)))) {
          permissions_string[permission] = '-';
	}
  }
  /*
     S_IFSOCK   0140000   socket
           S_IFLNK    0120000   symbolic link
           S_IFREG    0100000   regular file
           S_IFBLK    0060000   block device
           S_IFDIR    0040000   directory
           S_IFCHR    0020000   character device
           S_IFIFO    0010000   FIFO
   */
  char thing_type = ' ';
  if (S_ISDIR(mod)) {
    thing_type = 'd';
  } else if (S_ISREG(mod)) {
    thing_type = '-';
  } else if (S_ISLNK(mod)) {
    thing_type = 'l';
  }
  // off_t      st_size;     /* Total size, in bytes */
  /*
    Plans for tommorow: Set up permissions, get permissions to work

           S_IRWXU     00700   owner has read, write, and execute permission
           S_IRUSR     00400   owner has read permission
           S_IWUSR     00200   owner has write permission
           S_IXUSR     00100   owner has execute permission

           S_IRWXG     00070   group has read, write, and execute permission
           S_IRGRP     00040   group has read permission
           S_IWGRP     00020   group has write permission
           S_IXGRP     00010   group has execute permission

           S_IRWXO     00007   others (not in group) have read, write, and exe‐
                               cute permission
           S_IROTH     00004   others have read permission
           S_IWOTH     00002   others have write permission
           S_IXOTH     00001   others have execute permission
*/

  struct group *group_struct = getgrgid(file_stat.st_gid);
  struct passwd *pass_struct = getpwuid(file_stat.st_uid);
  struct tm *filetime = localtime(&file_stat.st_mtim.tv_sec);

  char datetime_string[14];
  strftime(datetime_string, sizeof(datetime_string), "%b %d %H:%M", filetime);
  FileInfo fi = {};
  fi.permissions = permissions_string;
  // fi.link_count = file_stat.st_nlink; // resolve later
  fi.owner = pass_struct->pw_name;
  fi.owner_group = group_struct->gr_name;
  // fi.file_size = file_stat.st_size;
  // fi.data_time = datetime_string
  fi.filename = (char*)filename;

  // plans for tommorow:
  // Make this export FileType thingy
  // seperate filteration and this step
  // iterate through files to get length
  // store maybe in length data struct
  // then we will have perfect alignment
  // we can add more flags, easily
  // goodbye :)
  printf("%c%s % *li %s %s % *li %s %s\n",thing_type,permissions_string, 2, file_stat.st_nlink,
         pass_struct->pw_name, group_struct->gr_name, 6, file_stat.st_size,
         datetime_string, filename);
}

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
  for (int i = 0; i < argc; i++) {
    if (i == 0) {
      continue;
    }
    char *param = argv[i];
    if (param[0] == '-') {
      if (param[1] == '-') {
        uint32_t cur_flag = 0;
        if (!strcmp(param, "--all")) {
          cur_flag = LS_a;
        }

        if (!strcmp(param, "--almost-all")) {
          cur_flag = LS_A;
        }
        handle_flag(&flags, cur_flag);

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
          handle_flag(&flags, cur_flag);
        }
      }
    } else {
      path = param;
    }
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

    print_file(filename, path, flags);
  }
  // make sure to close
  closedir(dir);
  return 0;
}
