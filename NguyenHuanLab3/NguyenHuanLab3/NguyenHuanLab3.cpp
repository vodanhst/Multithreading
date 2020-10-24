// NguyenHuanLab3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include<iostream>
#include<math.h>
#include<chrono>
#include<fstream>
#include"mpi.h"
#include<stdio.h>
#include<stdlib.h>
#include<algorithm>
using namespace std;

int PARTITION(int* arr, int left, int right) {
    int pivot = arr[right];
    int i = (left - 1);

    for (int j = left; j <= right - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[right]);
    return (i + 1);
}

void QUICK_SORT(int* arr, int left, int right) {
    if (left < right) {
        int pi = PARTITION(arr, left, right);
        QUICK_SORT(arr, left, pi - 1);
        QUICK_SORT(arr, pi + 1, right);

    }
}

int  SORT_RECURSIVE(int* arr, int size, int pr_rank, int max_rank, int rank_index) {
    MPI_Status status;
    int share_pr = pr_rank + pow(2, rank_index);
    rank_index++;

    if (share_pr > max_rank) {
        QUICK_SORT(arr, 0, size - 1);
        return 0;
    }
    int id = 0, piv_id;
    do {
        piv_id = PARTITION(arr, id, size - 1);
        id++;
    } while (piv_id == id - 1 && id <= size - 1);
    if (id > size - 1) {
        return 0;
    }
    if (piv_id <= size - piv_id) {
        MPI_Send(arr, piv_id - 1, MPI_INT, share_pr, piv_id, MPI_COMM_WORLD);
        SORT_RECURSIVE((arr + piv_id + 1), (size - piv_id - 1), pr_rank, max_rank, rank_index);
        MPI_Recv(arr, piv_id - 1, MPI_INT, share_pr, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
    else {
        MPI_Send((arr + piv_id + 1), size - piv_id - 1, MPI_INT, share_pr, piv_id + 1, MPI_COMM_WORLD);
        SORT_RECURSIVE(arr, piv_id + 1, pr_rank, max_rank, rank_index);
        MPI_Recv((arr + piv_id + 1), size - piv_id - 1, MPI_INT, share_pr, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }
}
int Aarr[1000000], Barr[1000000];

int main(int argc, char* argv[])
{
	ifstream inf("test.csv");

	char c;
	string ss; ss.clear();
	while (inf.get(c))
	{
		ss.push_back(c);
	}
	inf.close();
	ofstream of("test.csv");
	of << ss << endl;
	int size, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int rank_index = 0;
	while (pow(2, rank_index) <= rank)rank_index++;
	int n = 10;
	of << " using " << size << " of threads\n";
	while (n <= 1000000) {
		if (rank == 0) {

			/*freopen("input.txt", "r", stdin);

			int n; std::cin >> n;
			for (int i = 0; i < n; i++)std::cin >> A[i];*/
			//int n = 100000;
			//for (int i = 0; i < n; i++)A[i] = rand() % 100 +1;

			for (int i = 0; i < n; i++) {
				Aarr[i] = rand() % 10000 + 1;
			}
			cout << n << " stopped here " << rank << " " << size << endl;
			auto start = std::chrono::steady_clock::now();
			SORT_RECURSIVE(Aarr, n, rank, size - 1, rank_index);
			auto finish = std::chrono::steady_clock::now();
			of << (std::chrono::duration<double, std::milli>(finish - start).count()) << " ms " << std::endl;
			n *= 10;

			//freopen("output.txt", "w", stdout);
			//std::cout << n << std::endl;
			//for (int i = 0; i < n; i++)std::cout << A[i]<<" ";
			//fclose(stdout);			
		}
		else {
			MPI_Status status;
			int arsize;
			MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			MPI_Get_count(&status, MPI_INT, &arsize);
			int source = status.MPI_SOURCE;
			int* ar = new int[arsize];
			MPI_Recv(ar, arsize, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
			SORT_RECURSIVE(ar, arsize, rank, size - 1, rank_index);
			MPI_Send(ar, arsize, MPI_INT, source, 0, MPI_COMM_WORLD);
		}

	}
	of.close();
	MPI_Finalize();
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu


