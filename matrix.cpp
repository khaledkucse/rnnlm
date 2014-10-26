#include <stdlib.h>

#include "word_class.h"
#include "matrix.h"

void Matrix::initialize(int rows, int columns) {
	_synapses = (Synapse *)calloc(rows * columns, sizeof(Synapse));
	if (_synapses == NULL) {
		printf("%s line %d: Memory allocation failed\n", __FILE__, __LINE__);
		exit(1);
	}
	_rows = rows;
	_columns = columns;
}

void Matrix::copy(const Matrix &src) {
	for (int b = 0; b < _rows; b++)
		for (int a = 0; a < _columns; a++) 
			_synapses[a + b * _columns].weight = src._synapses[a + b * _columns].weight;	
}

void Matrix::print(FILE *fo) {
	for (long long b = 0; b < _rows; b++) {
		for (int a = 0; a < _columns; a++) {
			_synapses[a + b * _columns].printWeight(fo);
		}
	}
}

void Matrix::write(FILE *fo) {
	for (long long b = 0; b < _rows; b++) {
		for (int a = 0; a < _columns; a++) {
			_synapses[a + b * _columns].writeWeight(fo);
		}
	}
}

void Matrix::scan(FILE *fi) {
	for (int b = 0; b < _rows; b++) {
		for (int a = 0; a < _columns; a++) {
			_synapses[a + b * _columns].scanWeight(fi);
		}
	}
}

void Matrix::read(FILE *fi) {
	for (int b = 0; b < _rows; b++) {
		for (int a = 0; a < _columns; a++) {
			_synapses[a + b * _columns].readWeight(fi);
		}
	}
}

/* static */
real Matrix::random(real min, real max)
{
	return rand()/(real)RAND_MAX*(max-min)+min;
}

void Matrix::randomize()
{
	for (int dest_index = 0; dest_index < _rows; dest_index++) 
		for (int src_index = 0; src_index < _columns; src_index++)
			_synapses[src_index + dest_index * _columns].weight = random(-0.1, 0.1) + random(-0.1, 0.1) + random(-0.1, 0.1);
}

void Matrix::adjustRowWeights(int row, real alpha, const Layer &row_layer, const Layer &column_layer) {
	for (int column = 0; column < _columns; column++) 
		_synapses[row + column * _rows].weight += alpha * column_layer.getError(column) * row_layer.getActivation(row);
}

void Matrix::adjustColumnWeights(int column, real alpha, const Layer &row_layer, const Layer &column_layer) {
	for (int row = 0; row < _rows; row++) 
		_synapses[row + column * _rows].weight += alpha * column_layer.getError(column) * row_layer.getActivation(row);
}

void Matrix::adjustWeights(real alpha, const Layer &row_layer, const Layer &column_layer) {
	for (int row = 0; row < _rows; row++)
		adjustRowWeights(row, alpha, row_layer, column_layer);
}

void Matrix::adjustRowWeightsBeta2(int row, real alpha, real beta2, const Layer &row_layer, const Layer &column_layer) {
	for (int column = 0; column < _columns; column++) 
		_synapses[row + column * _rows].weight += alpha * column_layer.getError(column) * row_layer.getActivation(row) - _synapses[row + column * _rows].weight * beta2;
}

void Matrix::adjustColumnWeightsBeta2(int column, real alpha, real beta2, const Layer &row_layer, const Layer &column_layer) {
	for (int row = 0; row < _rows; row++) 
		_synapses[row + column * _rows].weight += alpha * column_layer.getError(column) * row_layer.getActivation(row) - _synapses[row + column * _rows].weight * beta2;
}

void Matrix::learnForWords(int word, int counter, real alpha, real beta2, const Vocabulary &vocab, const WordClass &wordClass, const Layer &layer1, const Layer &layer2) {
	for (int c = 0; c < wordClass.wordCount(vocab.getWord(word).class_index); c++) {
		int column = wordClass.getWord(vocab.getWord(word).class_index, c);
		if ((counter % 10) == 0)	//regularization is done every 10 steps
			adjustColumnWeightsBeta2(column, alpha, beta2, layer1, layer2);
		else
			adjustColumnWeights(column, alpha, layer1, layer2);
	}
}

void Matrix::learnForClasses(int counter, real alpha, real beta2, const Vocabulary &vocab, const Layer &layer1, const Layer &layer2) {
	for (int column = vocab.getSize(); column < _columns; column++) {
		if ((counter % 10) == 0)	//regularization is done every 10 steps
			adjustColumnWeightsBeta2(column, alpha, beta2, layer1, layer2);
		else
			adjustColumnWeights(column, alpha, layer1, layer2);
	}
}

void MatrixBackup::initialize(int rows, int columns) {
	Matrix::initialize(rows, columns);
	_backup.initialize(rows, columns);
}

void MatrixBackup::backup() {
	_backup.copy(*this);
}

void MatrixBackup::restore() {
	this->copy(_backup);
}