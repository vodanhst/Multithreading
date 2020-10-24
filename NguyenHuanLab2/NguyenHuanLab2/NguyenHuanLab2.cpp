#include<stdio.h>
#include<iostream>
#include<cmath>
#include<mpi.h>
#include<chrono>
#include<fstream>
#include<string>
#define lld long double
using namespace std;
#define MAX 2005
typedef lld Matrix[MAX][MAX];

Matrix Al;
lld xl[MAX];
lld bl[MAX];


lld dis(lld x[], lld y[], int n) {
	lld sum = 0.0;
	for (int i = 0; i < n; i++) {

		sum += ((x[i] - y[i]) * (x[i] - y[i]));
	}
	return sqrt(sum);
}

void READMATRIX(int n, int myTask, int nTasks) {

	Matrix temp; int nBar = n / nTasks;
	for (int i = 0; i < n; i++) {
		for (int j = n; j < MAX; j++)temp[i][j] = 0;
	}
	if (myTask == 0) {
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				cin >> temp[i][j];
			}
		}
	}
	MPI_Scatter(temp, nBar * MAX, MPI_DOUBLE, Al, nBar * MAX, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

Matrix temp1;
void GENMATRIX(int n, int myTask, int nTasks) {

	int nBar = n / nTasks;
	for (int i = 0; i < n; i++) {
		for (int j = n; j < MAX; j++)temp1[i][j] = 0;
	}

	if (myTask == 0) {
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				(i == j) ? temp1[i][j] = (1.0 + rand() % 10) : temp1[i][j] = (1.0 + rand() % 10) / 10000;
			}
		}
	}

	MPI_Scatter(temp1, nBar * MAX, MPI_DOUBLE, Al, nBar * MAX, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

lld temp[MAX];
void READVECTOR(int n, int myTask, int nTasks) {

	int nBar = n / nTasks;
	if (myTask == 0) {
		for (int i = 0; i < n; i++) {
			cin >> temp[i];
		}
	}
	MPI_Scatter(temp, nBar, MPI_DOUBLE, xl, nBar, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

lld Temp[MAX];
void GENVECTOR(int n, int myTask, int nTasks) {
	int nBar = n / nTasks;
	if (myTask == 0) {
		for (int i = 0; i < n; i++) {
			Temp[i] = 1.0 + rand() % 10;
		}
	}
	MPI_Scatter(Temp, nBar, MPI_DOUBLE, bl, nBar, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}


void PRINTVECTOR(int n, int myTask, int nTasks) {
	lld temp[MAX];
	int nBar = n / nTasks;
	MPI_Gather(xl, nBar, MPI_DOUBLE, temp, nBar, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	if (myTask == 0) {
		for (int i = 0; i < n; i++)cout << temp[i] << " ";
		cout << endl;
	}
}

lld xtemp1[MAX];
lld xtemp2[MAX];
int parallelJacobi(int n, lld EPS, int max_iter, int nTasks, int myTask) {
	int il, ig, j, nBar, iterId;

	lld* xold, * xnew;
	nBar = n / nTasks;
	MPI_Allgather(bl, nBar, MPI_DOUBLE, xtemp1, nBar, MPI_DOUBLE, MPI_COMM_WORLD);
	xnew = xtemp1;
	xold = xtemp2;
	iterId = 0;

	lld sum = 0.0;
	for (il = 0; il < nBar; il++) {
		ig = il + myTask * nBar;
		sum = 0.0;
		for (j = 0; j < ig; j++) {
			sum += Al[il][j];
		}
		for (j = ig + 1; j < n; j++) {
			sum += Al[il][j];
		}
		if (sum > Al[il][ig])
		{
			if (myTask == 0)
				cout << n << " The Spectral Radius of the Input Matrix is Bigger than 1\n";
			return 0;
		}
	}


	while (((iterId < max_iter) && (dis(xnew, xold, n)) >= EPS) || iterId == 0) {
		iterId++;
		swap(xold, xnew);
		for (il = 0; il < nBar; il++) {
			ig = il + myTask * nBar;
			xl[il] = bl[il];
			for (j = 0; j < ig; j++) {
				xl[il] = xl[il] - Al[il][j] * xold[j];
			}
			for (j = ig + 1; j < n; j++) {
				xl[il] = xl[il] - Al[il][j] * xold[j];
			}
			xl[il] = xl[il] / Al[il][ig];
		}
		MPI_Allgather(xl, nBar, MPI_DOUBLE, xnew, nBar, MPI_DOUBLE, MPI_COMM_WORLD);
	}
	return ((dis(xnew, xold, n) < EPS) ? 1 : 0);
}

int main(int argc, char* argv[]) {
	int nTasks;
	int myTask;


	int n;
	lld EPS;
	int max_iteration;
	int converged;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nTasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myTask);

	if (myTask == 0) {
		//cin >> n >> EPS >> max_iteration;
		n = 1700, EPS = 0.000001, max_iteration = 300;
	}


	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&EPS, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	MPI_Bcast(&max_iteration, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//while (n <= 2000)
	//{
	while (n % nTasks != 0)n++;
	GENMATRIX(n, myTask, nTasks);

	GENVECTOR(n, myTask, nTasks);

	auto start = std::chrono::steady_clock::now();

	converged = parallelJacobi(n, EPS, max_iteration, nTasks, myTask);
	auto finish = std::chrono::steady_clock::now();

	if (converged);
	//PRINTVECTOR(xl, n, myTask, nTasks);
	//printf("converged");

	else
		if (myTask == 0)printf("Failed to converge in iteration %d\n", max_iteration);

	if (myTask == 0) { cout << (std::chrono::duration<double, std::milli>(finish - start).count()) << "," << n << ',' << nTasks << ',' << converged << ','; cout << "DONE" << endl; }
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	//}
	//of.close();
	MPI_Finalize();
	return 0;
}