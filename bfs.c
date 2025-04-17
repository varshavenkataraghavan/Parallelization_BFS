#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

#define MAX 2000

int graph[MAX][MAX];
int visited[MAX];
int queue[MAX];
int front = -1, rear = -1;
int n;

void enqueue(int v) {
    if (rear == MAX - 1) return;
    if (front == -1) front = 0;
    queue[++rear] = v;
}

int dequeue() {
    if (front == -1 || front > rear) return -1;
    return queue[front++];
}

void bfs_serial(int start) {
    FILE *log = fopen("web/thread_log.txt", "w");
    memset(visited, 0, sizeof(visited));
    front = rear = -1;

    visited[start] = 1;
    enqueue(start);
    fprintf(log, "Thread 0 visiting %d\n", start);

    while (front <= rear && front != -1) {
        int curr = dequeue();
        for (int i = 0; i < n; i++) {
            if (graph[curr][i] && !visited[i]) {
                visited[i] = 1;
                enqueue(i);
                fprintf(log, "Thread 0 visiting %d\n", i);
            }
        }
    }

    fclose(log);
}

void bfs_parallel(int start) {
    FILE *log = fopen("web/thread_log.txt", "w");
    memset(visited, 0, sizeof(visited));
    front = rear = -1;

    visited[start] = 1;
    enqueue(start);
    fprintf(log, "Thread 0 visiting %d\n", start);

    while (front <= rear && front != -1) {
        int curr = dequeue();

        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < n; i++) {
            if (graph[curr][i]) {
                #pragma omp critical
                {
                    if (!visited[i]) {
                        visited[i] = 1;
                        enqueue(i);
                        fprintf(log, "Thread %d visiting %d\n", omp_get_thread_num(), i);
                    }
                }
            }
        }
    }

    fclose(log);
}

void bfs_parallel_level(int start) {
    int curr_frontier[MAX], next_frontier[MAX];
    int curr_size = 0, next_size = 0;
    FILE *log = fopen("web/thread_log.txt", "w");

    memset(visited, 0, sizeof(visited));
    visited[start] = 1;
    curr_frontier[curr_size++] = start;
    fprintf(log, "Thread 0 visiting %d\n", start);

    while (curr_size > 0) {
        next_size = 0;

        #pragma omp parallel
        {
            int local_frontier[MAX];
            int local_size = 0;

            #pragma omp for schedule(dynamic)
            for (int i = 0; i < curr_size; i++) {
                int curr = curr_frontier[i];
                for (int j = 0; j < n; j++) {
                    if (graph[curr][j]) {
                        int expected = 0;
                        #pragma omp atomic capture
                        expected = visited[j]++;

                        if (expected == 0) {
                            local_frontier[local_size++] = j;

                            #pragma omp critical
                            fprintf(log, "Thread %d visiting %d\n", omp_get_thread_num(), j);
                        }
                    }
                }
            }

            #pragma omp critical
            {
                for (int k = 0; k < local_size; k++) {
                    next_frontier[next_size++] = local_frontier[k];
                }
            }
        }

        memcpy(curr_frontier, next_frontier, next_size * sizeof(int));
        curr_size = next_size;
    }

    fclose(log);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: ./bfs.out [serial|parallel|level]\n");
        return 1;
    }

    FILE *in = fopen("input.txt", "r");
    if (!in) {
        printf("Cannot open input.txt\n");
        return 1;
    }

    int start;
    fscanf(in, "%d", &n);
    fscanf(in, "%d", &start);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            fscanf(in, "%d", &graph[i][j]);
    fclose(in);

    double start_time = omp_get_wtime();

    if (strcmp(argv[1], "serial") == 0)
        bfs_serial(start);
    else if (strcmp(argv[1], "parallel") == 0)
        bfs_parallel(start);
    else if (strcmp(argv[1], "level") == 0)
        bfs_parallel_level(start);
    else {
        printf("Invalid mode: %s\n", argv[1]);
        return 1;
    }

    double end_time = omp_get_wtime();
    printf("Execution Time (%s BFS): %.6f seconds\n", argv[1], end_time - start_time);

    return 0;
}
