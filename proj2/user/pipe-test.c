#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

typedef struct task_t {
  int  priority;
  int  x, y;
  char op;
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

static int
calc(int a, int b, char op, int *out)
{
  switch(op){
    case '+': *out = a + b; return 0;
    case '-': *out = a - b; return 0;
    case '*': *out = a * b; return 0;
    case '/': if (b == 0) return -1; *out = a / b; return 0;
    default : return -1;
  }
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
static void server(int rfd, int wfd) {
  task_t t;

  while (read(rfd, &t, sizeof t) == sizeof t) {
    t.act_error = calc(t.x, t.y, t.op, &t.act_result);
    write(wfd, &t, sizeof t);
  }

  exit(0);
}

// Client sends calculation tasks to server, then reads back result
static void client(int id, int wfd, int rfd, int logfd) {
  test_case tc = cases[id];
  task_t t = { id, tc.x, tc.y, tc.op, 0, 0, cases[id].expect_res, cases[id].expect_err};

  write(wfd, &t, sizeof t);  // write to server

  read(rfd, &t, sizeof t);   // read from server   

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

int main(void) {
  // Create two pipes for Client to Server, and Server to Client respectively
  int toSrv[2], fromSrv[2];

  if (pipe_rt(toSrv) < 0 || pipe(fromSrv) < 0) {  // Create pipe
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
