#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define MAX_VALUE 2e6
#define MIN_GRAIN 5 //it optimises performance

struct checkPartInfo {
  int from;
  int to;
  int value;
  int *numbers;
};

void checkPart(struct checkPartInfo *info) {
  for (int i = info->from; i < info->to; i += info->value) {
    info->numbers[i] = 1;
  }
  pthread_exit(NULL);
}

void sieve(int threads_number, int max_value) {
  pthread_t threads_[threads_number];
  struct checkPartInfo info_[threads_number];
  int *numbers_ = malloc(sizeof(int) * (max_value + 1));
  if (numbers_ == NULL) {
    printf("no memory\n");
    return;
  }
  memset(numbers_, 0, sizeof(numbers_));
  numbers_[0] = 1;
  numbers_[1] = 1;
  for (int i = 2; i <= max_value; ++i) {
    if (numbers_[i] == 0) {
      printf("%d\n", i);
      int number_of_i_ = (max_value) / i;
      int i_per_thread_ = (number_of_i_ + threads_number - 1) / threads_number;
      if (i_per_thread_ < MIN_GRAIN)
        i_per_thread_ = MIN_GRAIN;
      int j;
      for (j = 0; j < threads_number; ++j) {
        info_[j].from = i + j * i_per_thread_ * i;
        info_[j].to = i + (j + 1) * i_per_thread_ * i;
        info_[j].numbers = numbers_;
        info_[j].value = i;
        if (info_[j].to > max_value + 1)
          info_[j].to = max_value + 1;
        if (pthread_create(&threads_[j], NULL, checkPart, &info_[j]) != 0) {
          printf("Can not create thread.\n");
          return;
        }

        if (info_[j].to == max_value + 1)
          break;
      }
      for (; j >= 0; --j)
        if (pthread_join(threads_[j], NULL) != 0) {
          printf("Bad join.\n");
          return;
        }
    }
  }
  free(numbers_);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Not enough arguments. Use sieve \"number of threads\" \"max value\".\n");
    return 0;
  }
  if (argc == 2)
    sieve(atoi(argv[1]), MAX_VALUE);
  else
    sieve(atoi(argv[1]), atoi(argv[2]));
}
