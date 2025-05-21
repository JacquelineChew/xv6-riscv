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
  int id; // ID of client sending task
  int x;
  int y;
  char op; // Supports "+", "-", "*", "/"
  int  act_result;
  int  act_error;
  int  exp_result;
  int  exp_error;
  int status;   // synchronization flag: 0 = waiting, 1 = ready to read
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
  { 2, 3, '+',  5,  0},
  { 7, 4, '-',  3,  0},
  { 6, 2, '*',  12,  0},
  {20, 3, '*', 60,  0},
  {14, 2, '+', 16,  0},
  {99, 3, '/', 33,  0},
  { 1, 1, '+',  2,  0},
  { 4, 8, 'e',  0, -1},
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
void server(int read_fd) {
  
  // Server allocates page in physical memory; Return va
  uint64 shmaddr = shmget(0, SHM_SIZE);
  if ((int)shmaddr == -1) {
    printf("server: shmget failed\n");
    exit(1);
  }
  // shm points to start of shared page
  task_t *shm = (task_t *)shmaddr; // Cast from uint64 to task_t pointer

  show_vm_areas();  // VM areas after shmget is called

  //printf("Server PID %d: shm = %p\n", getpid(), (void *)shmaddr);

  task_t t;
  while (read(read_fd, &t, sizeof(t)) == sizeof(t)) { // Read from pipe
    t.act_error = calc(t.x, t.y, t.op, &t.act_result);

    task_t *slot = &shm[t.id]; // Pointer to client's slot in shared page
    printf("Client %d: Slot address %p\n", t.id, (void*)slot);
    *slot = (task_t)t;   // Put task in slot
    slot->status = 1;
  }

  exit(0);
}
  

// Client sends calculation tasks to server, then reads back result
void client(int id, int write_fd, int logfd) {
  test_case tc = cases[id];
  task_t t = { id, tc.x, tc.y, tc.op, 0, 0, cases[id].expect_res, cases[id].expect_err, 0};

  // Write task through pipe
  if (write(write_fd, &t, sizeof(t)) != sizeof(t)){
    printf("Write syscall failed.");
    exit(1);
  }

  // Clients allocate page to same place as server in physical memory; Return va
  uint64 shmaddr = shmget(0, SHM_SIZE); 
  if ((int)shmaddr == -1) {
    printf("client %d: shmget failed\n", id);
    exit(1);
  }
  // shm points to start of shared page
  task_t *shm = (task_t *)shmaddr; // Cast from uint64 to task_t pointer

  task_t *slot = &shm[id];  // Pointer to specific location (slot) in shared page

  // *slot = (task_t)t;
  // slot->status = 1;

  // Wait for server to set result
  while (slot->status != 1) {
    sleep(1);
  }

  // Write result to buffer (Debugging)
  char buf[128];
  int len = 0;

  strcpy(buf, "Task "); len = 5;
  int_to_str(buf + len, id); len = strlen(buf);
  strcpy(buf + len, ": ("); len = strlen(buf);
  int_to_str(buf + len, slot->x); len = strlen(buf);
  buf[len++] = ' '; buf[len++] = slot->op; buf[len++] = ' ';
  int_to_str(buf + len, slot->y); len = strlen(buf);
  strcpy(buf + len, "). Expected: "); len = strlen(buf);
  int_to_str(buf + len, slot->exp_result); len = strlen(buf);
  strcpy(buf + len, ", "); len = strlen(buf);
  int_to_str(buf + len, slot->exp_error); len = strlen(buf);
  strcpy(buf + len, ". Received: "); len = strlen(buf);
  int_to_str(buf + len, slot->act_result); len = strlen(buf);
  strcpy(buf + len, ", "); len = strlen(buf);
  int_to_str(buf + len, slot->act_error); len = strlen(buf);
  strcpy(buf + len, ". "); len = strlen(buf);
  strcpy(buf + len, (slot->act_result == slot->exp_result && slot->act_error == slot->exp_error) ? "PASS\n" : "FAIL\n");

  int n = write(logfd, buf, strlen(buf));

  if (n <= 0) {
    printf("client %d: failed to write to result.txt\n", id);
  }

  close(logfd);

  exit(0);
}


int main() {
  // Create one pipe for Client to Server
  int toSrv[2];

  if (pipe(toSrv) < 0) {  // Create pipe
    printf("main: cannot create pipes\n");
    exit(1);
  }

  int logfd = open("result.txt", O_CREATE | O_WRONLY);
  if (logfd < 0) {
    printf("main: cannot open result.txt\n");
    exit(1);
  }

  // fork all clients
  for (int i = 0; i < N; i++) {
    if (fork() == 0) {  // Client
      close(toSrv[0]);
      client(i, toSrv[1], dup(logfd));
    }
  }

  close(toSrv[1]);
  server(toSrv[0]);

  close(logfd);

  wait(0);

  exit(0);
}
