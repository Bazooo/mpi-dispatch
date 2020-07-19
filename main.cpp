#include <chrono>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <mpi.h>

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

    while (opsLeft > 0 || doneWorkers < size - 1) {
        if (0 < opsLeft && opsLeft < MAX_OPS) {
            printf("%d operations left... last one completed by %d\n", opsLeft, freeWorkerRank);
        }

        MPI_Recv(&freeWorkerRank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        values[MAX_OPS - opsLeft] = freeWorkerRank;
        --opsLeft;

        if (opsLeft <= 0) {
            ++doneWorkers;
        }

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

    while (opsLeft > 0) {
        doWork();
        ++nbTasks;
        MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&opsLeft, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, nullptr);
    }

    printf("Worker %d finished and did a total of %d tasks \n", rank, nbTasks);
}

void doWork() {
    long duration = (rand() % 500) + 1;

    std::this_thread::sleep_for(milliseconds(duration));
}
