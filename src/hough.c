/*
 * This file is part of HoughTransform.
 *
 * The program HoughTransform is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * «Copyright 2012 Salvador de la Puente»
 */

/*
 * hough.c
 *
 *  Created on: 21/01/2012
 *      Author: Salvador de la Puente González
 */

#include "include/hough.h"
#include <stdio.h>

/**
 * Initializes every data structure related with Hough transformation.
 * -- Caches for sine and cosine
 * -- Accumulator
 * -- Classifier
 */
void setup_hough(
		POINT_TYPE* points,	// Array of points
		sizep_t num_points,	// Number of points in the array
		sizep_t threshold,	// Threshold to consider line intersection
		double tolerance_t,	// Tolerance (in grades) to consider two lines are the same
		double tolerance_r,	// Distance (in pixels) to consider two lines are the same
		sizep_t precision,	// Precision for degrees (10 = decimals, 100 = cents...)
		SIZE_TYPE size,		// Size of the image
		sizep_t max_lines)	// Lines to look for (value -1 falls back to 500)
{
	// Default values
	if (!max_lines)
		max_lines = 500;

	// Input image globals
	_input_points = points;
	_num_points = num_points;

	_dimensions = size;
	_dimensions.diagonal = (sizep_t) (ceil(sqrt((size.width*size.width) + (size.height*size.height))));
	_dimensions.semi_diagonal = (sizep_t) (ceil(_dimensions.diagonal/2.0));

	// Theta related globals
	_precision = precision;
	_circumference = 360*_precision;
	_semi_circumference = 180*_precision;

	// Line recognition globals
	_threshold = threshold;
	_tolerance_t = tolerance_t*_precision;
	_tolerance_r = tolerance_r;
	_max_lines = max_lines;

	// Allocate caches and accumulator
	size_t cache_size = sizeof(double) * _circumference;
	_SIN_CACHE = (double*) malloc(cache_size);
	_COS_CACHE = (double*) malloc(cache_size);
	_accumulator = (_CELL_TYPE**) malloc(sizeof(_CELL_TYPE*)*_circumference);

	// Initialize caches and accumulator
	grade_t t; dim_t r;
	for(t=0; t<_circumference; t++){

		// Allocate and initialize each accumulator's row
		_accumulator[t] = (_CELL_TYPE*) malloc(sizeof(_CELL_TYPE)*_dimensions.semi_diagonal);
		for(r=0; r<_dimensions.semi_diagonal; r++) {
			_accumulator[t][r].t = t;
			_accumulator[t][r].r = r;
			_accumulator[t][r].count = 0;
			_accumulator[t][r].pindex = (sizep_t*) malloc(sizeof(sizep_t)*_num_points);
			_accumulator[t][r].processed = 0;

			// Set to 0
			memset(_accumulator[t][r].pindex, 0, sizeof(sizep_t)*_num_points);
		}

		// Set caches
		_SIN_CACHE[t] = sin(t*PI/_semi_circumference);
		_COS_CACHE[t] = cos(t*PI/_semi_circumference);
	}

	// Allocate and setup classifier
	_classifier.centers = (_CENTER_TYPE*) malloc(sizeof(_CENTER_TYPE)*_max_lines);
	_classifier.length = 0;
}

/**
 * Free memory of Hough transformation related data structures.
 */
void finish_hough(void) {
	grade_t t; sizep_t i;
	for(i=0; i<_classifier.length; i++){
		free(_classifier.centers[i].cells);
	}
	free(_classifier.centers);
	for(t=0; t<_circumference; t++){
		free(_accumulator[t]);
	}
	free(_accumulator);
	free(_COS_CACHE);
	free(_SIN_CACHE);
}

/**
 * Return the cached value of theta's sine
 */
inline double _cached_sin(grade_t theta) {
	return _SIN_CACHE[theta];
}

/**
 * Return the cached value of theta's cosine
 */
inline double _cached_cos(grade_t theta) {
	return _COS_CACHE[theta];
}

/**
 * Compute r(x, y, theta) = x*cos(theta)+y*sin(theta)
 */
inline dim_t _r(dim_t x, dim_t y, grade_t theta) {
	return (dim_t) (x*_cached_cos(theta) + y*_cached_sin(theta));
}

/**
 * Compute distance between to ratios
 */
double _distance_r(double r1, double r2) {
	return abs(r1-r2);
}

/**
 * Compute distance between to angles
 * NOTE: if distance is over 180º, take the "other" distance (short path)
 */
double _distance_t(double t1, double t2) {
	double t = abs(t1-t2);
	if (t>_semi_circumference)
		t = _circumference-t;
	return t;
}

/**
 * Classify a cell.
 * NOTE: when averaging angles, note it is done following:
 * http://en.wikipedia.org/wiki/Mean_of_circular_quantities
 */
void _classify(_CELL_TYPE* cell) {

	// Obtain minimal distance to each center
	size_t i = 0, min_i = 0;
	double min_dt, min_dr, min_distance = -1;
	for (i = 0; i < _classifier.length; i++) {
		double r1 = _classifier.centers[i].r;
		double t1 = _classifier.centers[i].t;
		double r2 = cell->r;
		double t2 = cell->t;
		double dr = _distance_r(r1, r2);
		double dt = _distance_t(t1, t2);
		double distance = dr < dt ? dt : dr;
		if (distance < min_distance || min_distance == -1) {
			min_i = i;
			min_dt = dt;
			min_dr = dr;
			min_distance = distance;
		}
	}

	// Minimal distance is not close enough of any center --> new center
	if (min_dt > _tolerance_t || min_dr > _tolerance_t || !_classifier.length) {

		// More lines than expected
		if (_classifier.length == _max_lines) {
			printf("_classifier FULL!\n");
			return;
		}

		int free_index = _classifier.length++;

		// TODO: consider another limit
		_classifier.centers[free_index].cells = (_CELL_TYPE**) malloc(sizeof(_CELL_TYPE*)*_max_lines);

		_classifier.centers[free_index].cells[0] = cell;
		_classifier.centers[free_index].lenght = 1;
		_classifier.centers[free_index].r = cell->r;
		_classifier.centers[free_index].t = cell->t;

	// close enough --> group with
	} else {

		// Too many points per group. Skip.
		sizep_t length = _classifier.centers[min_i].lenght;
		if (length >= _max_lines) {
			printf("Impossible to keep more points for group %i!\nSkipping...\n", (int) min_i);
			return;
		}
		_classifier.centers[min_i].cells[_classifier.centers[min_i].lenght++] = cell;

		// Compute average
		sizep_t i;
		double sum_r, sum_sin_t, sum_cos_t;
		sum_r = sum_sin_t = sum_cos_t = 0.0;
		for (i=0; i<length; i++) {
			sum_r += _classifier.centers[min_i].cells[i]->r;
			sum_sin_t += _cached_sin(_classifier.centers[min_i].cells[i]->t);
			sum_cos_t += _cached_cos(_classifier.centers[min_i].cells[i]->t);
		}

		// Means
		_classifier.centers[min_i].r = sum_r/length;
		_classifier.centers[min_i].t = atan2(sum_sin_t/length, sum_cos_t/length)*_semi_circumference/PI;
	}
}

/**
 * Prints the contents of the classifier as:
 * (theta, radius) from # lines
 * Parametric line: y = m*x + b
 */
void print_classifier(void) {
	int i;
	for (i = 0; i < _classifier.length; i++) {
		double r = _classifier.centers[i].r;
		double t = _classifier.centers[i].t;
		sizep_t l = _classifier.centers[i].lenght;
		printf("(%.2fº, %.2f) from %i lines\n", t/_precision, r, l);
		printf("Parametric line: ");
		double radians = t*PI/_semi_circumference;
		printf("y = %.2f*x+%.2f\n\n", -cos(radians)/sin(radians), r/sin(radians));
	}
}

/**
 * Fill lines with lines result of Hough transformation.
 * Returns the number of lines.
 */
sizep_t get_lines(LINE_TYPE** lines) {
	size_t i, count = _classifier.length;
	LINE_TYPE* output = (LINE_TYPE*) malloc(sizeof(LINE_TYPE)*count);
	for (i=0; i<count; i++) {
		output[i].r = _classifier.centers[i].r;
		output[i].t = _classifier.centers[i].t*PI/_semi_circumference;
	}

	(*lines) = output;
	return count;
}

/**
 * Plots the point over the accumulator. You need to call setup_hough()
 * before using this function.
 */
void plot_point(sizep_t index) {
	// Project the point over the accumulator.

	grade_t t;
	POINT_TYPE cpoint = _input_points[index];

	// Going through 180º is enough to cover all directions
	for(t=0; t<_semi_circumference; t++) {

		// Calculate r and use the positive representation
		dim_t r = _r(cpoint.x, cpoint.y, t);
		grade_t theta = t;
		if (r<0) {
			r = -r;
			theta = _semi_circumference+t;
		}

		// Count another point
		_CELL_TYPE* ccell = &_accumulator[theta][r];
		if (!ccell->pindex[index]) {
			ccell->pindex[index] = 1;
			ccell->count++;
		}

		// If not yet processed, and cell over the threshold, classify it!
		if (!ccell->processed && ccell->count >= _threshold) {
			//printf("%i, %i\n", theta, r);
			ccell->processed = 1;
			_classify(ccell);
			//_print_classifier();
			//printf("--\n\n");
		}//*/
	}
}
