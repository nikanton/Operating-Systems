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
#define ANSI_COLOR_BLUE   "\e[1;36m"
#define ANSI_COLOR_RESET  "\x1b[0m"
#define MAX_DIR_NAME_SIZE 8192

void get_stat(char *file_name, char *directory, struct stat *stat_var) {
  char file_dir[MAX_DIR_NAME_SIZE] = "";
  strcat(file_dir, directory);
  strcat(file_dir, file_name);
  stat(file_dir, stat_var);
}

int cmp(char *first, char *second, char *directory_name) {
  struct stat first_stat;
  struct stat second_stat;
  get_stat(first, directory_name, &first_stat);
  get_stat(second, directory_name, &second_stat);
  return second_stat.st_size - first_stat.st_size;
}

void print_info(char *file, char *directory) {
  struct stat file_stat;
  get_stat(file, directory, &file_stat);

  if (S_ISREG(file_stat.st_mode))
    printf("-");
  else if (S_ISDIR(file_stat.st_mode))
    printf("d");
  else if (S_ISLNK(file_stat.st_mode))
    printf("l");
  else
    printf(" ");

  char mode[10] = "---------";
  if (file_stat.st_mode & S_IRUSR) mode[0] = 'r';
  if (file_stat.st_mode & S_IWUSR) mode[1] = 'w';
  if (file_stat.st_mode & S_IXUSR) mode[2] = 'x';
  if (file_stat.st_mode & S_IRGRP) mode[3] = 'r';
  if (file_stat.st_mode & S_IWGRP) mode[4] = 'w';
  if (file_stat.st_mode & S_IXGRP) mode[5] = 'x';
  if (file_stat.st_mode & S_IROTH) mode[6] = 'r';
  if (file_stat.st_mode & S_IWOTH) mode[7] = 'w';
  if (file_stat.st_mode & S_IXOTH) mode[8] = 'x';
  printf("%s ", mode);

  printf("%d ", file_stat.st_nlink);

  printf("%s ", getgrgid(file_stat.st_gid)->gr_name);

  printf("%s ", getpwuid(file_stat.st_uid)->pw_name);

  printf("%.6s ", 4 + ctime(&file_stat.st_mtime));

  printf("%d\t", file_stat.st_size);
  if (S_ISDIR(file_stat.st_mode))
    printf(ANSI_COLOR_BLUE "%s\n" ANSI_COLOR_RESET, file);
  else
    printf("%s\n", file);
}

void ls(char *dir_name) {
  DIR *dir = opendir(dir_name);
  if(dir == NULL) {
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
  if(directory_name[strlen(directory_name) - 1] != '/')
    strcat(directory_name, "/");
  qsort_r(files_list,
          files_in_directory,
          sizeof(files_list[0]),
          (int (*)(const void *, const void *)) cmp,
          directory_name);
  printf("%s:\n", directory_name);
  for (size_t i = 0; i < files_in_directory; ++i)
    print_info(files_list[i], directory_name);
  printf("\n");
  for (size_t i = 0; i < files_in_directory; ++i)
    if (strcmp(files_list[i], ".") != 0 && strcmp(files_list[i], "..") != 0) {
      struct stat file_stat;
      get_stat(files_list[i], directory_name, &file_stat);
      if (S_ISDIR(file_stat.st_mode)) {
        char file_dir[MAX_DIR_NAME_SIZE] = "";
        strcat(file_dir, directory_name);
        strcat(file_dir, files_list[i]);
        ls(file_dir);
      }
    }
  closedir(dir);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    ls(".");
  }
  for (int i = 1; i < argc; ++i) {
    ls(argv[i]);
  }
}
