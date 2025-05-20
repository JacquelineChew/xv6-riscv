// # include "../../kernel/types.h"
// # include "../../user/user.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h" 
#include "kernel/fcntl.h"

#define CLIENTS 10
#define SLOT_SIZE sizeof(task_t)
#define SHM_SIZE 4096

typedef struct task_t {
  int x;
  int y;
  char op; // Supports "+", "-", "*", "/"
  int  act_result;
  int  act_error;
  int  exp_result;
  int  exp_error;
} task_t;

typedef struct {
  int  x, y;
  char op;
  int  expect_res;
  int  expect_err;
} test_case;

static test_case cases[] = {
  {10, 4, '-',  6,  0},
  {34, 9, '+', 43,  0},
  {56, 6, '&',  0, -1}, // Invalid operator
  { 5, 0, '/',  0, -1}, // Division by zero
};
static int N = sizeof cases / sizeof cases[0];

int calc(int x, int y, char op, int *result) {
  // Action: calc task similar to the one in homework 1, but in the user space

  switch (op) {
    case '+': *result = x + y; break;
    case '-': *result = x - y; break;
    case '*': *result = x * y; break;
    case '/':
      if (y == 0)
        return -1;
      *result = x / y; break;
    default:
      return -1;
  }
  return 0;
}

// Used for printing task elements to file
void int_to_str(char *buf, int x) {
  char tmp[16];
  int i = 0, j = 0, neg = 0;
  if (x < 0) { neg = 1; x = -x; }
  do {
    tmp[i++] = '0' + (x % 10);
    x /= 10;
  } while (x);
  if (neg) tmp[i++] = '-';
  while (i--) buf[j++] = tmp[i];
  buf[j] = '\0';
}

// Server calculates result for clients' tasks, then sends back to clients
void server(int read_fd, int write_fd) {
  task_t task;

  while (read(read_fd, &task, sizeof(task)) == sizeof(task)) {
    // Action: execute calc();
    task.act_error = calc(task.x, task.y, task.op, &task.act_result);

    // Action: write the result to write_fd; (send result back to client)
    if (write(write_fd, &task, sizeof(task)) != sizeof(task)){
      printf("Write syscall failed.");
      exit(1);
    }
  }
  
  exit(0);
}

// Client sends calculation tasks to server, then reads back result
void client(int id, int write_fd, int read_fd, int logfd) {
  test_case tc = cases[id];
  task_t t = { tc.x, tc.y, tc.op, 0, 0, cases[id].expect_res, cases[id].expect_err};

  // Action: write task to write_fd;
  if (write(write_fd, &t, sizeof(t)) != sizeof(t)){
    printf("Write syscall failed.");
    exit(1);
  }

  // Action: read from read_fd and get result;
  if (read(read_fd, &t, sizeof(t)) != sizeof(t)){
    printf("Read syscall failed.");
    exit(1);
  }

  // Write result to buffer (Debugging)
  char buf[128];
  int len = 0;

  strcpy(buf, "Task "); len = 5;
  int_to_str(buf + len, id); len = strlen(buf);
  strcpy(buf + len, ": ("); len = strlen(buf);
  int_to_str(buf + len, t.x); len = strlen(buf);
  buf[len++] = ' '; buf[len++] = t.op; buf[len++] = ' ';
  int_to_str(buf + len, t.y); len = strlen(buf);
  strcpy(buf + len, "). Expected: "); len = strlen(buf);
  int_to_str(buf + len, t.exp_result); len = strlen(buf);
  strcpy(buf + len, ", "); len = strlen(buf);
  int_to_str(buf + len, t.exp_error); len = strlen(buf);
  strcpy(buf + len, ". Received: "); len = strlen(buf);
  int_to_str(buf + len, t.act_result); len = strlen(buf);
  strcpy(buf + len, ", "); len = strlen(buf);
  int_to_str(buf + len, t.act_error); len = strlen(buf);
  strcpy(buf + len, ". "); len = strlen(buf);
  strcpy(buf + len, (t.act_result == t.exp_result && t.act_error == t.exp_error) ? "PASS\n" : "FAIL\n");

  write(logfd, buf, strlen(buf));

  close(logfd);
  
  exit(0);
}

int main() {
  // Create two pipes for Client to Server, and Server to Client respectively
  int toSrv[2], fromSrv[2];

  if (pipe(toSrv) < 0 || pipe(fromSrv) < 0) {  // Create pipe
    printf("pipe-test: cannot create pipes\n");
    exit(1);
  }

  int logfd = open("result.txt", O_CREATE | O_WRONLY);
  if (logfd < 0) {
    printf("pipe-test: cannot open result.txt\n");
    exit(1);
  }

  for (int i = 0; i < N; i++) {
    if (fork() == 0) {  // Client
      close(toSrv[0]);
      close(fromSrv[1]);

      client(i, toSrv[1], fromSrv[0], dup(logfd));
    }
  }
  // Server
  close(toSrv[1]);
  close(fromSrv[0]);
  server(toSrv[0], fromSrv[1]);

  close(toSrv[0]);
  close(fromSrv[1]);
  wait(0);  // Wait for client processes to exit
  exit(0);
}