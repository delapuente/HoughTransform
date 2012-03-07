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
 ============================================================================
 Name        : hought_transform.c
 Author      : Salvador de la Puente González
 Version     :
 Copyright   : GNU3
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "include/hough.h"
#include <wand/magick_wand.h>

/**
 * Extracts from a binary image white points and consider them as information
 * filling sample array and setting size structure.
 *
 * NOTE: When extracting points origin is moved to the center of the image:
 *
 *                     +semi_height
 *     Image Origin
 *              +-----------|
 *              |           |
 *              |           |
 *              |           | Hough Origin
 *  -semi_width ------------+------------- +semi_width
 *                          |
 *                          |
 *                          |
 *                          |
 *
 *                     -semi_height
 */
sizep_t get_points(char* name, POINT_TYPE **sample, SIZE_TYPE *size) {
	MagickWand *mw = NULL;
	PixelWand **pmw = NULL;
	PixelIterator *imw = NULL;
	MagickWandGenesis();

	mw = NewMagickWand();
	MagickReadImage(mw, name);

	unsigned long width, semi_width, height, semi_height;
    width = MagickGetImageWidth(mw);
    semi_width = ceil(width/2.0);
    height = MagickGetImageHeight(mw);
    semi_height = ceil(height/2.0);
    imw = NewPixelIterator(mw);

    sizep_t count = 0;
    POINT_TYPE *aux = (POINT_TYPE*) malloc(sizeof(POINT_TYPE)*width*height);

    // Extract white points
    int y, x;
    for (y=0; y<height; y++) {
    	pmw = PixelGetNextIteratorRow(imw, &width);
    	for (x=0; x< (long) width; x++) {
    		if (PixelGetBlack(pmw[x])) {
    			aux[count].x = x-semi_width;
    			aux[count].y = (height-y)-semi_height;
    			count++;
    		}
    	}
    }

	POINT_TYPE* output = (POINT_TYPE*) malloc(sizeof(POINT_TYPE)*count);
	memcpy(output, aux, sizeof(POINT_TYPE)*count);
	free(aux); aux = NULL;

	if(mw)
		mw = DestroyMagickWand(mw);

	MagickWandTerminus();

	(*sample) = output;
	size->width = width;
	size->height = height;
	return count;
}

/**
 * Helper function to provide a visual interpretation of the accumulator.
 */
void print_accumulator(char* name) {
	MagickWand *mw = NULL;
	PixelWand **pmw = NULL;
	PixelIterator *imw = NULL;
	MagickWandGenesis();

	unsigned long width, height;
	width = _circumference;
	height = _dimensions.semi_diagonal;

	mw = NewMagickWand();
	MagickSetSize(mw, width, height);
	MagickReadImage(mw, "xc:black");
    imw = NewPixelIterator(mw);

    int y, x;
    for (y=0; y<height; y++) {
    	pmw = PixelGetNextIteratorRow(imw, &width);
    	for (x=0; x< (long) width; x++) {
    		_CELL_TYPE* cell = &_accumulator[x][height-y-1];
    		PixelSetRed(pmw[x], 1.0/20.0 * cell->count);
    	}
    	PixelSyncIterator(imw);
    }
    MagickWriteImage(mw, name);

	if(mw)
		mw = DestroyMagickWand(mw);

	MagickWandTerminus();
}

/**
 * Prints count lines on output with background bg. Lines can be scaled by using factor.
 * NOTE: This performs some filtering removing lines passing through origin
 */
void print_lines(char* output, char* bg, LINE_TYPE* lines, sizep_t count, double factor) {
	MagickWand *mw = NULL;
	DrawingWand *dw = NULL;
	PixelWand *pmw = NULL;
	unsigned long width, semi_width, height, semi_height;

	MagickWandGenesis();
	mw = NewMagickWand();
	dw = NewDrawingWand();
	pmw = NewPixelWand();

	MagickReadImage(mw, bg);
	width = MagickGetImageWidth(mw);
	semi_width = ceil(width/2.0);
	height = MagickGetImageHeight(mw);
	semi_height = ceil(height/2.0);
	PixelSetColor(pmw,"red");
	DrawSetStrokeColor(dw, pmw);
	DrawSetStrokeWidth(dw, .5*factor);
	DrawSetStrokeAntialias(dw, 0);

	sizep_t n;
	for(n=0; n<count; n++) {
		LINE_TYPE cline = lines[n];

		double m = -cos(cline.t)/sin(cline.t);
		double b = cline.r/sin(cline.t);

		if ((-0.5 < m && m < 0.5) || (-1 < b && b < 1)) continue; // remove lines too horizontal

		double x0 = - ((long) semi_width); double y0 = x0*m+b;
		double x1 = ((long) semi_width); double y1 = x1*m+b;

		// Apply factor
		x0 *= factor; y0 *= factor; x1 *= factor; y1 *= factor;

		// Fix coordinates and plot over the image
		DrawLine(dw, x0+semi_width, height-y0-semi_height, x1+semi_width, height-y1-semi_height);
	}

	MagickDrawImage(mw,dw);
	MagickWriteImage(mw, output);

	pmw = DestroyPixelWand(pmw);
	mw = DestroyMagickWand(mw);
	dw = DestroyDrawingWand(dw);

	MagickWandTerminus();
}

POINT_TYPE sample [] = {
		{50, -50},
		{100, -50},
		{100, -100}
};
SIZE_TYPE sample_size = {800, 600};

/**
 * USE: Provide sample points and how many to setup_hough() transformation and plot those points
 * using plot_point() with each index of points you want to convert. The use print_classifier()
 * or get_lines() to see the lines extracted.
 */
int main(int argc, char **argv) {

	// Get points from image
	SIZE_TYPE size;
	POINT_TYPE *points = NULL;
	sizep_t count = get_points("sample_small.bmp", &points, &size);

	/*
	 *  points 		: Array of points
	 *  num_points	: Number of points in the array
	 *  threshold	: Threshold to consider line intersection
	 *  tolerance_t	: Tolerance (in grades) to consider two lines are the same
	 *  tolerance_r	: Distance (in pixels) to consider two lines are the same
	 *  precision	: Precision for degrees (10 = decimals, 100 = cents...)
	 *  size		: Size of the image
	 *  max_lines	: Lines to look for (value -1 falls back to 500)
	 */
	clock_t cp = clock();
	setup_hough(points, count, 12, 15.0, 5.0, 1, size, 10000);

	// Plot every point and measure clock ticks
	int n;
	for (n=0; n<_num_points; n++) {
		plot_point(n);
	}
	printf("_plot_point() ticks: %.4f\n", ((double)(clock()-cp))/1000000l);
	print_accumulator("accumulator.bmp");
	print_classifier();//*/

	// Get lines from classifier
	LINE_TYPE *lines;
	sizep_t clines = get_lines(&lines);

	// Print lines
	print_lines("output.bmp", "sample.bmp", lines, clines, 10);
	print_lines("output_small.bmp", "sample_small.bmp", lines, clines, 1);

	// Free memory
	finish_hough();
	free(lines);
	free(points);
	printf("All clear!");

	return EXIT_SUCCESS;
}
