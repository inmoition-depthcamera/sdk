#include "Matrix.h"


void Matrix::SetMatrix(int r, int c, REAL* p){
	if ((rows != r) || (cols != c))
	{
		rows = r;
		cols = c;
		data.assign(rows, vector<REAL>(cols, 0));
	}
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			if (p == NULL)
				data[i][j] = 0;
			else
				data[i][j] = p[i*cols + j];
		}
	}
}

Matrix Matrix::operator+(Matrix &ad) const{
	Matrix res(rows, cols);
	if (this->rows != ad.rows || this->cols != ad.cols){
		cerr << "Matrix dimensions must agree." << endl;
		exit(-1);
	}
	else{
		for (int i = 0; i<rows; i++)
		for (int j = 0; j<cols; j++)
			res.data[i][j] = this->data[i][j] + ad.data[i][j];
	}
	return res;
}

Matrix Matrix::operator-(Matrix &su) const{
	Matrix res(rows, cols);
	if (this->rows != su.rows || this->cols != su.cols){
		cerr << "Matrix dimensions must agree." << endl;
		exit(-1);
	}
	else{
		for (int i = 0; i<rows; i++)
		for (int j = 0; j<cols; j++)
			res.data[i][j] = this->data[i][j] - su.data[i][j];
	}
	return res;
}

Matrix Matrix::operator*(Matrix &mul) const{
	if (mul.rows != cols){
		cerr << "Matrix dimensions must agree." << endl;
		exit(-1);
	}
	Matrix res(rows, mul.Getcols());

	for (int i = 0; i < rows; i++)
	{
		for (int k = 0; k < cols; k++)
		{
			if (data[i][k] == 0) continue;
			for (int j = 0; j < mul.Getcols(); j++)
			{
				{
					res.data[i][j] += data[i][k] * mul.data[k][j];
				}
			}
		}
	}
	return res;
}

Matrix Matrix::operator*(const REAL &mul) const{

	Matrix res(rows, cols);
	for (int i = 0; i != rows; i++){
		for (int j = 0; j != cols; j++){
			res.data[i][j] = this->data[i][j] * mul;
		}
	}
	return res;
}

Matrix Matrix::operator/(const REAL &div)const{
	Matrix res(rows, cols);
	for (int i = 0; i<rows; i++)
	for (int j = 0; j<cols; j++)
		res.data[i][j] = this->data[i][j] / div;
	return res;
}

Matrix Matrix::m_sqrt(){
	Matrix res(rows, cols);
	vector<vector<REAL>> data_t(rows, vector<REAL>(cols, 0));
	for (int i = 0; i<rows; i++)
	for (int j = 0; j<cols; j++)
		data_t[i][j] = sqrt(data[i][j]);
	res.setdata(data_t);
	return res;
}

Matrix Matrix::trans(){
	Matrix res(cols, rows);
	vector<REAL> t;
	for (int i = 0; i != cols; i++)
	for (int j = 0; j != rows; j++)
		res.data[i][j] = this->data[j][i];
	return res;
}

vector<REAL> Matrix::mean(int option){
	vector<REAL> res;
	switch (option){
	case 1:{
			   //Mean on rows
			   REAL s = 0;
			   for (int j = 0; j<cols; j++){
				   for (int i = 0; i<rows; i++)
					   s += this->data[i][j];
				   res.push_back(s / rows);
				   s = 0;
			   }
			   break;
	}
	case 2:{
			   //Mean on rows
			   REAL s = 0;
			   for (int j = 0; j<rows; j++){
				   for (int i = 0; i<cols; i++)
					   s += this->data[j][i];
				   res.push_back(s / cols);
				   s = 0;
			   }
			   break;
	}
	default:{
				cerr << "WRONG Input arguments" << endl;
				exit(-1);
	}
	}
	return res;
}

Matrix Matrix::sort(vector<int>& id, int option){
	switch (option){
	case 1:{
			   //on rows
			   Matrix res(rows, cols);
			   if (id.size() != rows){
				   cerr << "Matrix dimensions must agree." << endl;
				   exit(-1);
			   }
			   for (vector<REAL>::size_type i = 0; i != id.size(); i++)
				   res.data[i] = data[id[i]];
			   return res;
	}
	case 2:{
			   //on columns
			   Matrix res = trans();
			   res = res.sort(id, 1);
			   res = res.trans();
			   return res;
	}
	default:{
				cerr << "WRONG Input arguments" << endl;
				exit(-1);
	}
	}

}

Matrix Matrix::trunc(int num, int option){
	switch (option){
	case 1:{
			   //on rows
			   Matrix res(num, cols);
			   vector<vector<REAL>> d;
			   for (int i = 0; i<num; i++)
				   d.push_back(data[i]);
			   res.setdata(d);
			   return res;
	}
	case 2:{
			   //on column
			   Matrix res = trans();
			   res = res.trunc(num, 1);
			   res = res.trans();
			   return res;
	}
	default:{
				cerr << "WRONG Input arguments" << endl;
				exit(-1);
	}
	}

}

Matrix Matrix::invert(){
	if (rows != cols){
		cerr << "Matrix dimensions must agree." << endl;
		exit(-1);
	}
	Matrix res(rows, rows);
	int actualsize = rows;
	int maxsize = rows;
	vector<vector<REAL>> data_t = data;
	for (int i = 1; i<actualsize; i++)
		data_t[0][i] /= data_t[0][0];
	for (int i = 1; i < actualsize; i++){
		for (int j = i; j < actualsize; j++){
			REAL sum = 0.0;
			for (int k = 0; k < i; k++)
				sum += data_t[j][k] * data_t[k][i];
			data_t[j][i] -= sum;
		}
		if (i == actualsize - 1)
			continue;
		for (int j = i + 1; j < actualsize; j++){
			REAL sum = 0.0;
			for (int k = 0; k < i; k++)
				sum += data_t[i][k] * data_t[k][j];
			data_t[i][j] = (data_t[i][j] - sum) / data_t[i][i];
		}
	}
	for (int i = 0; i < actualsize; i++)
	for (int j = i; j < actualsize; j++){
		REAL x = 1.0;
		if (i != j) {
			x = 0.0;
			for (int k = i; k < j; k++)
				x -= data_t[j][k] * data_t[k][i];
		}
		data_t[j][i] = x / data_t[j][j];
	}
	for (int i = 0; i < actualsize; i++)
	for (int j = i; j < actualsize; j++)  {
		if (i == j) continue;
		REAL sum = 0.0;
		for (int k = i; k < j; k++)
			sum += data_t[k][j] * ((i == k) ? 1.0 : data_t[i][k]);
		data_t[i][j] = -sum;
	}
	for (int i = 0; i < actualsize; i++)
	for (int j = 0; j < actualsize; j++)  {
		REAL sum = 0.0;
		for (int k = ((i>j) ? i : j); k < actualsize; k++)
			sum += ((j == k) ? 1.0 : data_t[j][k])*data_t[k][i];
		data_t[j][i] = sum;
	}
	res.setdata(data_t);
	return res;
}



void MatDiff(Matrix *A, Matrix *B, Matrix *C)
{
	if (A->rows != B->rows || A->cols != B->cols || A->rows != C->rows || A->cols != C->cols)
	{
		cerr << "Matrix dimensions must agree." << endl;
		exit(-1);
	}
	for (int i = 0; i < C->rows; i++)
	{
		for (int j = 0; j < C->cols; j++)
		{
			C->data[i][j] = A->data[i][j] - B->data[i][j];
		}
	}
	return;
}

void MatMul(Matrix *A, Matrix *B, Matrix *C)
{
	if ((C->rows != A->rows) || (C->cols != B->cols) || (A->cols != B->rows))
	{
		cerr << "Matrix dimensions must agree." << endl;
		exit(-1);
	}

	for (int i = 0; i < A->rows; i++)
	{
		for (int j = 0; j < C->cols; j++)
		{
			C->data[i][j] = 0;
		}
	}

	for (int i = 0; i < A->rows; i++)
	{
		for (int k = 0; k < B->rows; k++)
		{
			if (A->data[i][k] == 0) continue;
			for (int j = 0; j < C->cols; j++)
			{
				{
					C->data[i][j] += A->data[i][k] * B->data[k][j];
				}
			}
		}
	}
	return;
}



void MatDiv(Matrix *A, float B, Matrix *C)
{
	if (A->rows != C->rows || A->cols != C->cols)
	{
		cerr << "Matrix dimensions must agree." << endl;
		exit(-1);
	}
	for (int i = 0; i < C->rows; i++)
	{
		for (int j = 0; j < C->cols; j++)
		{
			C->data[i][j] = A->data[i][j] / B;
		}
	}
	return;
}

void MatNumMul(Matrix *A, float B, Matrix *C)
{
	if (A->rows != C->rows || A->cols != C->cols)
	{
		cerr << "Matrix dimensions must agree." << endl;
		exit(-1);
	}
	for (int i = 0; i < C->rows; i++)
	{
		for (int j = 0; j < C->cols; j++)
		{
			C->data[i][j] = A->data[i][j] * B;
		}
	}
	return;
}

unsigned short  Convolution(Matrix *A, Matrix *B, Matrix *C, Matrix *D)
{
	float SumF = 0,SumD=0,tmp;
	for (int i = 0; i < A->rows; i++)
	{
		for (int j = 0; j < A->cols; j++)
		{
			tmp = A->data[i][j] * B->data[i][j] * C->data[i][j];
			SumF += tmp;
			SumD += tmp*D->data[i][j];
		}
	}
	SumD /= SumF;
	//if (SumD > 0)
	//{
	//	cout << SumD << endl;
	//}
	return (unsigned short(SumD + 0.5));
}

void ROI55(Matrix *A, Matrix *B, int x, int y, int w, int h)
{
	if (B->rows != h || B->cols != w)
	{
		cerr << "Matrix dimensions must agree." << endl;
		exit(-1);
	}
	for (int i = y; i < h + y; i++)
	{
		for (int j = x; j < w + x; j++)
		{
			B->data[i - y][j - x] = A->data[i][j];
		}
	}
}

