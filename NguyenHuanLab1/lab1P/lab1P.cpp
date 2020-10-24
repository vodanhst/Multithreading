// lab1P.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



#include <iostream>
#include <fstream>
#include <windows.h>
#include <omp.h>

using namespace std;

void createFileOfRandomNums(char* name, int rows, int columns);
double** readMatrix(char* name);
void multiMatrix(double** a, double** b, double** c);
void printMatrix(double** c, int rows, int columns);
void writeMatrixInFile(char* name, double** result, int rows, int columns);
double** createResultMatrixEmpty();

ULONGLONG dwStart;
int rows1 = 2000;
int columns2 = 2000;
int transfer12 = 2000;
int maxThreadNum = 10;
int chunk = 10;

int main(int argc, char* argv[]) {

	double** a = nullptr, ** b = nullptr, ** c;
	dwStart = GetTickCount64();
	createFileOfRandomNums((char*)("matrix1.txt"), rows1, transfer12);
	createFileOfRandomNums((char*)("matrix2.txt"), transfer12, columns2);

	a = readMatrix((char*)"matrix1.txt");
	b = readMatrix((char*)"matrix2.txt");
	c = createResultMatrixEmpty();
	multiMatrix(a, b, c);
	writeMatrixInFile((char*)"result.txt", c, rows1, columns2);
	

	return (0);
}

void createFileOfRandomNums(char* name, int rows, int columns) {
	omp_set_num_threads(maxThreadNum);
#pragma omp parallel 
	{
		ofstream f1(name, ios::out | ios::app);
		f1 << rows << " " << columns << endl;
#pragma omp for schedule (static, chunk)
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < columns; j++) {
				f1 << (double)(rand()) / rand() << " ";
			}
			f1 << endl;
		}
		f1.close();
	}

}

void writeMatrixInFile(char* name, double** result, int rows, int columns) {
	omp_set_num_threads(maxThreadNum);
#pragma omp parallel 
	{	
		ofstream f1(name, ios::out | ios::app);
		f1 << rows << " " << columns << endl;
#pragma omp for schedule (static, chunk)
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < columns; j++) {
				f1 << result[i][j] << " ";
			}
			f1 << endl;
		}
		f1.close();
	}
	
}

double** readMatrix(char* name) {
	ifstream file(name);
	int rows, columns;
	double** matrix;
	file >> rows >> columns;
	if (name == "matrix1.txt") {
		rows1 = rows;
		transfer12 = columns;
	}
	else {
		transfer12 = rows;
		columns2 = columns;
	}
	matrix = new double* [rows];
	omp_set_num_threads(maxThreadNum);
#pragma omp parallel 
	{
#pragma omp for schedule (static, chunk)
		for (int row = 0; row < rows; row++) {
			matrix[row] = new double[columns];
			for (int column = 0; column < columns; column++) {
				file >> matrix[row][column];
			}
		}
	}

	file.close();
	return matrix;
}

double** createResultMatrixEmpty() {
	double** result;
	result = new double* [rows1];
	for (int row = 0; row < rows1; row++) {
		result[row] = new double[columns2];
	}
	return result;
}

void multiMatrix(double** a, double** b, double** c) {
	cout << "Begin multiplay" << endl;
	omp_set_num_threads(maxThreadNum);
#pragma omp parallel 
	{
#pragma omp for schedule (static, chunk)
		for (int row = 0; row < rows1; row++) {
			for (int column = 0; column < columns2; column++) {
				c[row][column] = 0;
				for (int transfer = 0; transfer < transfer12; transfer++) {
					c[row][column] += a[row][transfer] * b[transfer][column];
				}
			}
		}
	}
	
	printf_s("For multiply two matrixs: %dx%d on %dx%d, spent %I64d milliseconds\n", rows1, transfer12, transfer12, columns2, (GetTickCount64() - dwStart));
}

void printMatrix(double** c, int rows, int columns) {

	cout << "matrix" << endl;
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
			cout << c[i][j] << " ";
		cout << endl;
	}
}
