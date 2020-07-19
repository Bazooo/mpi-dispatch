#include <chrono>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <mpi.h>

// Number of maximum operations to be distributed between processes
#define MAX_OPS 100

using std::chrono::milliseconds;

void dispatcher(int size);
void worker(int rank);
void doWork();

int main() {
    int rank, size;

    MPI_Init(nullptr, nullptr);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    srand(rank + time(nullptr));

    if (rank == 0) {
        dispatcher(size);
    } else {
        worker(rank);
    }

    MPI_Finalize();

    return 0;
}

void dispatcher(int size) {
    int opsLeft = MAX_OPS;
    int values[MAX_OPS];
    int doneWorkers = 0;
    int freeWorkerRank;
    MPI_Status status;

    // wait till all workers are done (there are no more tasks left)
    while (doneWorkers < size - 1) {
        if (0 < opsLeft && opsLeft < MAX_OPS) {
            printf("%d operations left... last one completed by %d\n", opsLeft, freeWorkerRank);
        }

        // wait for results from a worker that finished his task
        MPI_Recv(&freeWorkerRank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // do something with the data the worker returned
        values[MAX_OPS - opsLeft] = freeWorkerRank;
        --opsLeft;

        // if there are no more operations, mark worker as done
        if (opsLeft <= 0) {
            ++doneWorkers;
        }

        // Send number of operations left to the worker (and possibly his new task to work on)
        MPI_Send(&opsLeft, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD);
    }

    printf("[");
    for (int i = 0; i < MAX_OPS; ++i) {
        printf("%d%s", values[i], i != MAX_OPS - 1 ? ", " : "");
    }
    printf("]\n");
}

void worker(int rank) {
    int opsLeft = MAX_OPS;
    int nbTasks = 0;

    // continue working until there are no more operations left
    while (opsLeft > 0) {
        // do some operation
        doWork();
        ++nbTasks;

        // send results to the root process
        MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

        // wait for next task (or the end of its job)
        MPI_Recv(&opsLeft, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, nullptr);
    }

    printf("Worker %d finished and did a total of %d tasks \n", rank, nbTasks);
}

void doWork() {
    long duration = (rand() % 500) + 1;

    std::this_thread::sleep_for(milliseconds(duration));
}
