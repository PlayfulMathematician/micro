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
#define max(a, b) ((a) > (b) ? (a) : (b))

enum LS_FLAGS {
  LS_a = 0x00000001,
  LS_l = 0x00000002,
  LS_A = 0x00000004,
  LS_A_or_a = 0x00000005
};

typedef struct {
  char filename[40];
  char permissions[40];
  char link_count[40];
  char owner[40];
  char owner_group[40];
  char file_size[40];
  char date_time[40];
  bool valid;
} FileInfo;

typedef struct {
  unsigned long file_len; //
  unsigned long perm_len; //
  unsigned long ln_len; //
  unsigned long owner_len; //
  unsigned long group_len; //
  unsigned long size_len;
  unsigned long time_len;
} FileSizeInfo;

void handle_fi(FileInfo fi, FileSizeInfo *fsi) {
  if (!fi.valid) {
    return;
  }

  fsi->file_len = max(strlen(fi.filename), fsi->file_len);
  fsi->group_len = max(strlen(fi.owner_group), fsi->group_len);
  fsi->owner_len = max(strlen(fi.owner), fsi->owner_len);
  fsi->ln_len = max(strlen(fi.link_count), fsi->ln_len);
  fsi->perm_len = max(strlen(fi.permissions), fsi->perm_len);
  fsi->size_len = max(strlen(fi.file_size), fsi->size_len);
  fsi->time_len = max(strlen(fi.date_time), fsi->time_len);
}




FileInfo generate_file_info(const char *filename, char *path, uint32_t flags) {
  if (filename[0] == '.' && !(flags & LS_A_or_a)) {
    return (FileInfo){0};
  }
  if ((!strcmp(filename, ".") || !strcmp(filename, "..")) && (flags & LS_A)) {
    return (FileInfo){0};
  }

  if (!(flags & LS_l)) {
    printf("%s\n", filename);
    return (FileInfo){0};
  }
  char a_path[4000] = "";
  strcpy(a_path, path);
  strcat(a_path, "/");
  strcat(a_path, filename);
  /// char *real_path = realpath(a_path, NULL);
  struct stat file_stat = {0};
  lstat(a_path, &file_stat);
  mode_t mod = file_stat.st_mode;
  char permissions_string[13] = "-rwxrwxrwx\0\0\0";
  for (int permission = 0; permission < 9; permission++) {
    if (!(mod & (1 << (8 - permission)))) {
          permissions_string[permission + 1] = '-';
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
  permissions_string[0] = thing_type;
  
  struct group *group_struct = getgrgid(file_stat.st_gid);
  struct passwd *pass_struct = getpwuid(file_stat.st_uid);
  struct tm *filetime = localtime(&file_stat.st_mtim.tv_sec);

  char datetime_string[14];
  strftime(datetime_string, sizeof(datetime_string), "%b %d %H:%M", filetime);
  FileInfo fi = {};
  char *link_string;
  asprintf(&link_string, "%li", file_stat.st_nlink);
  
  char *size_string;
  asprintf(&size_string, "%li", file_stat.st_size);

  fi.valid = true;
  strcpy(fi.permissions, permissions_string);
  strcpy(fi.link_count, link_string);
  strcpy(fi.date_time, datetime_string);
  strcpy(fi.owner, pass_struct->pw_name);
  strcpy(fi.owner_group, group_struct->gr_name);
  strcpy(fi.file_size, size_string);
  strcpy(fi.filename, filename);
  return fi;
}

void print_everything(FileInfo *fil, int file_count, FileSizeInfo fsi) {
  for (int i = 0; i < file_count; i++) {
    FileInfo fi = fil[i];
    if (!fi.valid) {
      return;
};          
    printf("%*s %*s %*s %*s %*s %*s %*s\n", (int)fsi.perm_len, fi.permissions,
           (int)fsi.ln_len, fi.link_count,
           (int)fsi.owner_len, fi.owner, (int)fsi.group_len,
           fi.owner_group, (int)fsi.size_len, fi.file_size, (int)fsi.time_len, fi.date_time, (int)fsi.file_len, fi.filename);
           
  }
  
};


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
  FileInfo *fis = malloc(10000 * sizeof(FileInfo));
  FileSizeInfo fsi = (FileSizeInfo){0};
  int capacity = 10000;
  int fisidx = 0; // rename better lateer
  while (true) {
    struct dirent *dirent_ptr = readdir(dir);
    if (dirent_ptr == NULL) {
      break;
    }
    const char *filename = dirent_ptr->d_name;

    FileInfo fi = generate_file_info(filename, path, flags);
    
    memcpy(fis + fisidx, &fi, sizeof(FileInfo));
    fisidx++;
    if (fisidx >= capacity) {
      fis = realloc(fis, capacity * 2);
      capacity *= 2;
    }
    handle_fi(fi, &fsi);
  }
  print_everything(fis, fisidx, fsi);
  // make sure to close
  closedir(dir);
  return 0;
}
