#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define PORT 5009
#define MAX_LISTEN_QUEUE_SIZE 100
#define MAX_USERS 200
#define MAX_USERNAME_SIZE 10
#define MAX_STRING_SIZE 100
#define true 1
#define false 0

const char username_request[] = "What is your username?\n";
const char username_exists[] = "This username already exists. Try again.\n";
const char username_empty[] = "This username in empty. Try again.\n";
const char username_space[] = "Username must not contain space. Try again.\n";
const char long_username[] = "Too long user name, it must be up to 10 characters long. Try again.\n";
const char killed_notification[] = "You are killed.\n";
const char no_user[] = "No such user exists.\n";

int number_of_users = 0;
char *users[MAX_USERS];
pthread_mutex_t users_mtx = PTHREAD_MUTEX_INITIALIZER;
int sockets[MAX_USERS];
char messages_to_users[MAX_USERS][MAX_STRING_SIZE];
pthread_mutex_t messages_to_users_mtx = PTHREAD_MUTEX_INITIALIZER;
int users_hp[MAX_USERS];
int kill_requests[MAX_USERS];
int heal_requests[MAX_USERS];
int kill_requested[MAX_USERS];
int heal_requested[MAX_USERS];
pthread_mutex_t hp_mtx = PTHREAD_MUTEX_INITIALIZER;
int program_finished = false;

void delete_n_in_the_end(char *str) {
  size_t len = strlen(str);
  if (len > 0 && str[len - 1] == '\n') {
    str[len - 1] = '\0';
    if (len > 1 && str[len - 2] == '\r') {
      str[len - 2] = '\0';
    }
  }
}

void client_read(int this_thread_id) {
  int this_socket = sockets[this_thread_id];
  char buf[MAX_STRING_SIZE];
  ssize_t read_size;
  while ((read_size = read(this_socket, buf, MAX_STRING_SIZE)) != -1) {
    buf[read_size] = '\0';
    delete_n_in_the_end(buf);

    char *p = strtok(buf, " ");
    while (p != NULL) {
      if (strcmp(p, "who") == 0) {
        pthread_mutex_lock(&users_mtx);
        for (int i = 0; i < number_of_users; ++i) {
          if (users[i] != NULL) {
            char out[MAX_STRING_SIZE] = " ";
            strcat(out, users[i]);
            strcat(out, " ");
            char hp[MAX_STRING_SIZE] = " ";
            sprintf(hp, "%d", users_hp[i]);
            strcat(out, hp);
            strcat(out, "\n");
            write(this_socket, out, sizeof(out));
          }
        }
        pthread_mutex_unlock(&users_mtx);
      }

      else if (strcmp(p, "say") == 0) {
        p = strtok(NULL, " ");
        pthread_mutex_lock(&users_mtx);
        int message_if_written = false;
        for (int i = 0; i < number_of_users; ++i) {
          if (strcmp(users[i], p) == 0) {
            p = strtok(NULL, "\n");
            pthread_mutex_lock(&messages_to_users_mtx);
            strcat(messages_to_users[i], users[this_thread_id]);
            strcat(messages_to_users[i], " says: ");
            strcat(messages_to_users[i], p);
            strcat(messages_to_users[i], "\n");
            pthread_mutex_unlock(&messages_to_users_mtx);
            message_if_written = true;
            break;
          }
        }
        pthread_mutex_unlock(&users_mtx);
        if(message_if_written == false)
          write(this_socket, no_user, sizeof(no_user));
      }

      else if (strcmp(p, "wall") == 0) {
        p = strtok(NULL, "\n");
        pthread_mutex_lock(&users_mtx);
        pthread_mutex_lock(&messages_to_users_mtx);
        for (int i = 0; i < number_of_users; ++i) {
          if (users[i] != NULL) {
            strcat(messages_to_users[i], users[this_thread_id]);
            strcat(messages_to_users[i], " shouts: ");
            strcat(messages_to_users[i], p);
            strcat(messages_to_users[i], "\n");
          }
        }
        pthread_mutex_unlock(&users_mtx);
        pthread_mutex_unlock(&messages_to_users_mtx);
        write(this_socket, no_user, sizeof(no_user));
      }

      else if (strcmp(p, "kill") == 0) {
        p = strtok(NULL, " ");
        pthread_mutex_lock(&users_mtx);
        if (kill_requested[this_thread_id] == false) {
          kill_requested[this_thread_id] = true;
          for (int i = 0; i < number_of_users; ++i) {
            if (strcmp(users[i], p) == 0) {
              pthread_mutex_lock(&messages_to_users_mtx);
              strcat(messages_to_users[i], users[this_thread_id]);
              strcat(messages_to_users[i], " attacks you!\n");
              pthread_mutex_unlock(&messages_to_users_mtx);
              pthread_mutex_lock(&hp_mtx);
              ++kill_requests[i];
              pthread_mutex_unlock(&hp_mtx);
              break;
            }
          }
        }
        pthread_mutex_unlock(&users_mtx);
      }

      else if (strcmp(p, "heal") == 0) {
        p = strtok(NULL, " ");
        pthread_mutex_lock(&users_mtx);
        if (heal_requested[this_thread_id] == false) {
          kill_requested[this_thread_id] = true;
          for (int i = 0; i < number_of_users; ++i) {
            if (strcmp(users[i], p) == 0) {
              pthread_mutex_lock(&hp_mtx);
              ++heal_requests[i];
              pthread_mutex_unlock(&hp_mtx);
              break;
            }
          }
        }
        pthread_mutex_unlock(&users_mtx);
      }

      p = strtok(NULL, " ");
    }
  }
  close(this_socket);
}

void client(int this_socket) {
  int this_thread_id;
  while (true) {
    write(this_socket, username_request, sizeof(username_request));
    char username[MAX_STRING_SIZE];
    ssize_t read_size = recv(this_socket, username, MAX_STRING_SIZE, 0);//потести
    if (read_size == -1) {
      close(this_socket);
      return;
    }
    username -= read_size;
    username[read_size] = '\0';
    delete_n_in_the_end(username);
    if (strlen(username) == 0) {
      write(this_socket, username_empty, sizeof(username_empty));
      continue;
    }
    if (strchr(username, ' ') != NULL) {
      write(this_socket, username_space, sizeof(username_space));
      continue;
    }
    if (strlen(username) > MAX_USERNAME_SIZE) {
      write(this_socket, long_username, sizeof(long_username));
      continue;
    }

    pthread_mutex_lock(&users_mtx);
    for (int i = 0; i < number_of_users; ++i) {
      if (strcmp(users[i], username) == 0) {
        write(this_socket, username_exists, sizeof(username_exists));
        pthread_mutex_unlock(&users_mtx);
        continue;
      }
    }
    users[number_of_users] = malloc(sizeof(char) * (strlen(username) + 1));
    strcpy(users[number_of_users], username);
    this_thread_id = number_of_users - 1;
    ++number_of_users;
    users_hp[this_thread_id] = 100;
    pthread_mutex_unlock(&users_mtx);
    break;
  }
  sockets[this_thread_id] = this_socket;
  pthread_create(NULL, NULL, client_read, this_thread_id);

  while (true) {
    if (users_hp[this_thread_id] < 1) {
      write(this_socket, killed_notification, sizeof(killed_notification));
      break;
    }
    if (strlen(messages_to_users[this_thread_id]) != 0) {
      pthread_mutex_lock(&messages_to_users_mtx);
      if (write(this_socket, messages_to_users[this_thread_id], sizeof(messages_to_users[this_thread_id])) == -1)
        break;
      messages_to_users[this_thread_id][0] = '\0';
      pthread_mutex_unlock(&messages_to_users_mtx);
    }
    sleep(1);
  }

  pthread_mutex_lock(&users_mtx);
  free(users[this_thread_id]);
  users[this_thread_id] = NULL;
  pthread_mutex_unlock(&users_mtx);
  close(this_socket);
}

void every_20_sec() {
  while(!program_finished){
    printf("Yet another round started.\n");
    pthread_mutex_lock(&users_mtx);
    pthread_mutex_lock(&messages_to_users_mtx);
    pthread_mutex_lock(&hp_mtx);
    for(int i = 0; i < number_of_users; ++i) {
      heal_requested[i] = false;
      kill_requested[i] = false;
      for(int j = 0; j < heal_requests[i]; ++i) {
        users_hp[i] += 1 + rand() % 10;
      }
      for(int j = 0; j < kill_requests[i]; ++i) {
        users_hp[i] -= 1 + rand() % 10;
      }
      if(users_hp[i] > 0){
        strcat(messages_to_users[i], "Your current hp is ");
        char hp[MAX_STRING_SIZE] = " ";
        sprintf(hp, "%d", users_hp[i]);
        strcat(messages_to_users[i], hp);
        strcat(messages_to_users[i], ". You are welcome to yet another round.\n");
      }
      heal_requests[i] = 0;
      kill_requests[i] = 0;
    }
    pthread_mutex_unlock(&hp_mtx);
    pthread_mutex_unlock(&messages_to_users_mtx);
    pthread_mutex_unlock(&users_mtx);
    sleep(20);
  }
}

int main(int argc, char **argv) {
  printf("Hello.\n");
  struct sockaddr_in server;
  server.sin_family = PF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(PORT);
  printf("This is server %s:%d.\n", inet_ntoa(server.sin_addr), htons(server.sin_port));
  int server_descriptor = socket(PF_INET, SOCK_STREAM, 0);
  if (bind(server_descriptor, (struct sockaddr *) &server, sizeof(server))) {
    printf("Can not assign parameters to socket.\n");
    return 0;
  }
  if (listen(server_descriptor, MAX_LISTEN_QUEUE_SIZE)) {
    printf("Can not listen.\n");
    return 0;
  }
  srand(time(NULL));
  pthread_t every_20_sec_thread;
  if(pthread_create(&every_20_sec_thread, NULL, &every_20_sec, NULL)){
    printf("Can not create thread.\n");
    return 0;
  }
  printf("Server is ready to work.\n");
  while (true) {
    int descriptor = accept(server_descriptor, NULL, NULL);
    if (descriptor == -1)
      break;
    pthread_t new_thread;
    if(pthread_create(&new_thread, NULL, &client, descriptor)){
      printf("Can not create thread.\n");
      break;
    }
  }
}

