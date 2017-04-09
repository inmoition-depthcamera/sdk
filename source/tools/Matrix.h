#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <iostream>

using namespace std;

#define USINGFLOAT
//#define USINGDOUBLE


#ifdef USINGFLOAT 
typedef float REAL;
#define MATDATATYPE CV_32F
#define READTYEP "%f"
#endif

#ifdef USINGDOUBLE
typedef double REAL;
#define MATDATATYPE CV_64F
#define READTYPE "%lf"
#endif

#include <vector>
#include <string>
#include <fstream>


using namespace std;

class Matrix{
private:

public:
	int rows;
	int cols;
	vector<vector<REAL>> data;
	Matrix(){
		rows = 0;
		cols = 0;
	}
	Matrix(int r, int c){
		rows = r;
		cols = c;
		data.assign(rows, vector<REAL>(cols, 0));
	}
	Matrix(int r, int c, REAL* p){
		rows = r;
		cols = c;
		data.assign(rows, vector<REAL>(cols, 0));
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				data[i][j] = p[i*cols + j];
			}
		}
	}
	Matrix(int r, int c, unsigned short* p) {
		rows = r;
		cols = c;
		data.assign(rows, vector<REAL>(cols, 0));
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				data[i][j] = (REAL)p[i*cols + j];
			}
		}
	}
	Matrix(int r, int c, unsigned char* p) {
		rows = r;
		cols = c;
		data.assign(rows, vector<REAL>(cols, 0));
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				data[i][j] = (REAL)p[i*cols + j];
			}
		}
	}
	
	void SetMatrix(int r, int c, REAL* p);
	Matrix operator*(Matrix &mul) const;
	Matrix operator*(const REAL &mul) const;
	Matrix operator+(Matrix &ad) const;
	Matrix operator-(Matrix &su) const;
	Matrix operator/(const REAL &div) const;
	Matrix trans();
	Matrix m_sqrt();
	vector<REAL> mean(int option);
	Matrix sort(vector<int>& id, int option);
	Matrix trunc(int num, int option);
	Matrix invert();
	Matrix copy(){
		Matrix res(rows, cols);
		res.setdata(data);
		return res;
	}
	Matrix ROI(int x, int y, int w, int h)
	{
		Matrix ROI(h, w);
		for (int i = y; i < y + h; i++)
		{
			for (int j = x; j < x + w; j++)
			{
				ROI.data[i - y][j - x] = data[i][j];
			}
		}
		return(ROI);

	}
	REAL &operator()(int x, int y){
		return data[x][y];
	}
	const REAL &operator()(int x, int y) const{
		return data[x][y];
	}
	int Getrows() const{
		return this->rows;
	}
	int Getcols() const{
		return this->cols;
	}
	void setdata(vector<vector<REAL>> d){
		if (data.size() != d.size()){
			cout << "Matrix dimensions must agree." << endl;
			exit(-1);
		}
		data.assign(d.begin(), d.end());
	}
};

inline Matrix zeros(int r, int c){
	Matrix res(r, c);
	return res;
}
inline Matrix ones(int r, int c){
	Matrix res(r, c);
	vector<vector<REAL>> data_t(r, vector<REAL>(c, 1));
	res.setdata(data_t);
	return res;
}
inline Matrix diag(vector<REAL> d){
	int n = (int)d.size();
	Matrix res(n, n);
	vector<vector<REAL>> data_t(n, vector<REAL>(n, 0));
	for (int i = 0; i<n; i++)
		data_t[i][i] = d[i];
	res.setdata(data_t);
	return res;
}

void MatMul(Matrix *A, Matrix *B, Matrix *C);
//void MatMul(Matrix *A, Matrix *B, Matrix *C, int thread_num);
void MatDiff(Matrix *A, Matrix *B, Matrix *C);
void MatDiv(Matrix *A, float B, Matrix *C);
void MatNumMul(Matrix *A, float B, Matrix *C);
unsigned short  Convolution(Matrix *A, Matrix *B, Matrix *C, Matrix *D);
void ROI55(Matrix *A, Matrix *B, int x, int y, int w, int h);