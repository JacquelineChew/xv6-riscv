# include "../../kernel/types.h"
# include "../../kernel/stat.h"
# include "../../user/user.h"
// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h" 

typedef struct task_t {
  int priority;
  int x;
  int y;
  char* op; // Supports "+", "-", "*", "/"
  int result;
  int error;
} task_t;

int calc(int x, int y, char* op, int *result) {
  // Action: calc task similar to the one in homework 1, but in the user space

  switch (op[0]) {
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

// Server calculates result for clients' tasks
void server(int read_fd, int write_fd) {

  task_t task;

  printf("hello!\n");

  while (read(read_fd, &task, sizeof(task)) == sizeof(task)) {
    // Action: execute calc();
    printf("Client task: %d, %d, %s, %d, %d\n",task.x, task.y, task.op, task.result, task.error);
    
    task.error = calc(task.x, task.y, task.op, &task.result);

    // Action: write the result to write_fd; (send result back to client)
    if (write(write_fd, &task, sizeof(task)) != sizeof(task)){
      printf("Write syscall failed.");
      exit(1);
    }
  }

  // for (int i=0; i<4; i++){
  //   if (read(read_fd, &task, sizeof(task)) != sizeof(task)) {
  //     printf("Server: read failed\n");
  //     exit(1);
  //   }

  //   printf("Client task: %d, %d, %s, %d, %d\n",task.x, task.y, task.op, task.result, task.error);

  //   task.error = calc(task.x, task.y, task.op, &task.result);

  //   if (write(write_fd, &task, sizeof(task)) != sizeof(task)) {
  //     printf("Server: write failed\n");
  //     exit(1);
  //   }
  // }
  
  exit(0);
}

// Client sends calculation tasks to server, then reads back result
void client(int write_fd, int read_fd, task_t *task) {
  // Action: write task to write_fd;
  task_t client_task = *task;

  if (write(write_fd, task, sizeof(*task)) != sizeof(*task)){
    printf("Write syscall failed.");
    exit(1);
  }

  // Action: read from read_fd and get result;
  task_t result_task;
  if (read(read_fd, &result_task, sizeof(result_task)) < 0){
    printf("Read syscall failed.");
    exit(1);
  }

  printf("Task %d: (%d %s %d). Received: %d, %d.\n", 
    client_task.priority, client_task.x, client_task.op, client_task.y, result_task.result, result_task.error);
  
  exit(0);
}

int main() {
  // Action: create two pipes for input and output respectively;
  int pipe_task[2];
  int pipe_res[2];

  // Use pipe system call to create pipe for client to server, and server to client
  if (pipe_rt(pipe_task) < 0 || pipe(pipe_res) < 0) {
    printf("Pipe creation failed.");
    exit(1);
  }

  printf("pipe-task: %d, %d\n", pipe_task[0], pipe_task[1]);
  printf("pipe-res: %d, %d\n", pipe_res[0], pipe_res[1]);

  task_t test_cases[] = {
    {0, 10, 4, "-", 0, 0},
    {0, 34, 9, "+", 0, 0},
    {0, 56, 6, "&", 0, 0}, // Invalid operator
    {0, 5, 0, "/", 0, 0},  // Division by zero
  };

  for (int i=0; i<4; i++){
    if (fork() == 0) { // Client
      close(pipe_task[0]);     // Close read end of client->server
      close(pipe_res[1]);   // Close write end of server->client

      // Action: create a task;
      //task_t task = { .x = 5, .y = 3, .op = "*", .result = 0, .error = 0 };
      task_t task = test_cases[i];
      task.priority = getpid();      // Set task priority

      client(pipe_task[1], pipe_res[0], &task);

      close(pipe_task[1]);     // Close write end of client->server
      close(pipe_res[0]);   // Close read end of server->client
      exit(0);
    }
  }
  

  // Server
  close(pipe_task[1]);     // Close write end of client->server
  close(pipe_res[0]);   // Close read end of server->client

  server(pipe_task[0], pipe_res[1]);

  close(pipe_task[0]);
  close(pipe_res[1]);

  wait(0);  // Wait for client processes to exit
  exit(0);
}