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
 * hough.h
 *
 *  Created on: 21/01/2012
 *      Author: Salvador de la Puente González
 */

#ifndef MTOOLS_H_
#define MTOOLS_H_

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265

typedef int dim_t;
typedef int grade_t;
typedef unsigned int sizep_t;

typedef struct {
	double t;
	double r;
} LINE_TYPE;

typedef struct {
	dim_t x;
	dim_t y;
} POINT_TYPE;

typedef struct {
	dim_t width;
	dim_t height;
	dim_t diagonal;
	dim_t semi_diagonal;
} SIZE_TYPE;

typedef struct {
	sizep_t* pindex;
	sizep_t count;
	dim_t r;
	grade_t t;
	short processed;
} _CELL_TYPE;

typedef struct {
	double t;
	double r;
	_CELL_TYPE** cells;
	sizep_t lenght;
} _CENTER_TYPE;

typedef struct {
	sizep_t length;
	_CENTER_TYPE* centers;

} _CLASSIFIER_TYPE;

sizep_t _precision;
unsigned int _circumference;
unsigned int _semi_circumference;

SIZE_TYPE _dimensions;
_CELL_TYPE** _accumulator;
POINT_TYPE* _input_points;
sizep_t _num_points;

sizep_t _threshold;
sizep_t _tolerance_t;
sizep_t _tolerance_r;
sizep_t _max_lines;
_CLASSIFIER_TYPE _classifier;

double *_SIN_CACHE;
double *_COS_CACHE;

void setup_hough(POINT_TYPE*, sizep_t, sizep_t, double, double, sizep_t, SIZE_TYPE, sizep_t);
void finish_hough(void);

double _distance_r(double, double);
double _distance_t(double, double);
void _classify(_CELL_TYPE*);
void print_classifier(void);

inline double _cached_sin(grade_t);
inline double _cached_cos(grade_t);
inline dim_t _r(dim_t, dim_t, grade_t);


void plot_point(sizep_t);
sizep_t get_lines(LINE_TYPE**);
#endif /* MTOOLS_H_ */
