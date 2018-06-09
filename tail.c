#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

#define CHAR_SIZE sizeof(char)
#define MAX_DIR_NAME_SIZE 8192

void get_stat(char *file_name, char *directory, struct stat *stat_var) {
  char file_dir[MAX_DIR_NAME_SIZE] = "";
  strcat(file_dir, directory);
  strcat(file_dir, file_name);
  stat(file_dir, stat_var);
}

void find(char *dir_name, char *file_name, char *user) {
  DIR *dir = opendir(dir_name);
  if (dir == NULL) {
    printf("Can not open directory %s.\n", dir_name);
    closedir(dir);
    return;
  }
  size_t files_in_directory = 0;
  struct dirent *file;
  while ((file = readdir(dir)) != NULL)
    ++files_in_directory;
  rewinddir(dir);
  char files_list[files_in_directory][MAX_DIR_NAME_SIZE];
  for (size_t i = 0; i < files_in_directory; ++i) {
    strcpy(files_list[i], readdir(dir)->d_name);
  }

  char directory_name[MAX_DIR_NAME_SIZE] = "";
  strcat(directory_name, dir_name);
  if (directory_name[strlen(directory_name) - 1] != '/')
    strcat(directory_name, "/");
  for (size_t i = 0; i < files_in_directory; ++i) {
    if (strcmp(files_list[i], file_name) == 0) {
      struct stat file_stat;
      get_stat(files_list[i], directory_name, &file_stat);
      if (strcmp(getpwuid(file_stat.st_uid)->pw_name, user) == 0) {
        printf("%s%s\n", directory_name, file_name);
      }
    }
  }

  for (size_t i = 0; i < files_in_directory; ++i)
    if (strcmp(files_list[i], ".") != 0 && strcmp(files_list[i], "..") != 0) {
      struct stat file_stat;
      get_stat(files_list[i], directory_name, &file_stat);
      if (S_ISDIR(file_stat.st_mode)) {
        char file_dir[MAX_DIR_NAME_SIZE] = "";
        strcat(file_dir, directory_name);
        strcat(file_dir, files_list[i]);
        find(file_dir, file_name, user);
      }
    }

  closedir(dir);
}

int main(int argc, char **argv) {
  if (argc < 4) {
    printf("Not enough arguments. Use find \"directory\" \"file name\" \"user\".\n");
    return 0;
  }
  find(argv[1], argv[2], argv[3]);
}
