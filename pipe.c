#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Not enough arguments. Use pipe pr_1 pr_2 ... pr_n.\n");
    return 0;
  }

  int *fd_prev = NULL;
  int *fd_this = NULL;

  for (int i = 1; i < argc; ++i) {
    printf("%s:\n", argv[i]);
    if (i != 1) {
      if (fd_prev != NULL) {
        close(fd_prev[0]);
        close(fd_prev[1]);
        free(fd_prev);
      }
      fd_prev = fd_this;
    }
    if (i != argc - 1) {
      fd_this = malloc(2 * sizeof(int));
      pipe(fd_this);
    } else {
      fd_this = NULL;
    }
    if (fork() == 0) {
      if (fd_prev != NULL) {
        printf("set in %s\n", argv[i]);
        dup2(fd_prev[0], 0);
        close(fd_prev[0]);
        close(fd_prev[1]);
      }
      if (fd_this != NULL) {
        printf("set out %s\n", argv[i]);
        dup2(fd_this[1], 1);
        close(fd_this[0]);
        close(fd_this[1]);
      }
      execl(argv[i], argv[i], NULL);
      printf("Ошибка запуска %s.\n", argv[i]);
      exit(EXIT_FAILURE);
    }
  }

  close(fd_prev[0]);
  close(fd_prev[1]);
  free(fd_prev);

  for (int i = 0; i < argc; ++i)
    wait(NULL);
}
