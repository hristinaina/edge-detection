#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "BitmapRawConverter.h"
#include "tbb/tick_count.h"
#include "tbb/task_group.h"
#include "Parameters.h"
#include "Range.h"

using namespace std;
using namespace tbb;


//@brief Function that gets corresponding horizontal prewitt filter
const int* getFilterHor()
{
	if (FILTER_SIZE == 3)
	{
		const int filterHor[3 * 3] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
		return filterHor;
	}
	else if (FILTER_SIZE == 5) {
		const int filterHor[5 * 5] = { 9, 9, 9, 9, 9,
							   9, 5, 5, 5, 9,
							  -7, -3, 0, -3, -7,
							  -7, -3, -3, -3, -7,
							  -7, -7, -7, -7, -7 };
		return filterHor;
	}
}

//@brief Function that gets corresponding vertical prewitt filter
const int* getFilterVer()
{
	if (FILTER_SIZE == 3)
	{
		const int filterVer[3 * 3] = { -1, -1, -1, 0, 0, 0, 1, 1, 1 };
		return filterVer;
	}
	else if (FILTER_SIZE == 5) {
		const int filterVer[5 * 5] = { 9, 9, -7, -7, -7,
									9, 5, -3, -3, -7,
									9, 5, 0, -3, -7,
									9, 5, -3, -3, -7,
									9, 9, -7, -7, -7 };
		return filterVer;
	}
}


/**
* @brief Function for filtering, used in prewitt edge detection algorithm
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param x horizontal coordinate of a point/pixel
* @param y vertical coordinate of a point/pixel
* @param width image width
*/
void multiply_prewitt(int* inBuffer, int* outBuffer, int x, int y, int width)
{
	int cut = (FILTER_SIZE -1) / 2;
	int Gx = 0;
	int Gy = 0;

	for (int n = 0; n < FILTER_SIZE; n++) {
		for (int m = 0; m < FILTER_SIZE; m++)
		{
			Gx += inBuffer[(x - cut + m) * width + y - cut + n] * getFilterHor()[m * FILTER_SIZE + n];
			Gy += inBuffer[(x - cut + m) * width + y - cut + n] * getFilterVer()[m * FILTER_SIZE + n];
		}
	}
	int G = abs(Gx) + abs(Gy);
	outBuffer[x * width + y] = (G > THRESHOLD) ? 255 : 0;
}

/**
* @brief Function for filtering, used in special edge detection algorithm
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param x horizontal coordinate of a point/pixel
* @param y vertical coordinate of a point/pixel
* @param width image width
* @param height image height
*/
void search_neighbour_points(int* inBuffer, int* outBuffer, int x, int y, int width, int height)
{
	int P = 0, O = 1;

	for (int x_step = -POINT_RANGE; x_step <= POINT_RANGE; x_step++)
	{
		for (int y_step = -POINT_RANGE; y_step <= POINT_RANGE; y_step++)
		{
			// provjera na greske tj eliminisati granicne slucajeve (sve uz okvir)
			if (!x_step && !y_step) continue;
			if (x + x_step < 0 || y + y_step < 0) continue;
			if (x + x_step >= height || y + y_step >= width) continue;

			int value = inBuffer[(x + x_step) * width + y + y_step];
			if (value == 1) P = 1;
			if (value == 0) O = 0;
		}
	}
	int result;
	result = abs(P - O) == 0 ? 0 : 255;
	outBuffer[x * width + y] = result;
}

/**
* @brief Serial version of edge detection algorithm implementation using Prewitt operator
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param r object that stores range for x and y coordinates
*/
void filter_serial_prewitt(int* inBuffer, int* outBuffer, Range r)
{
	for (int i = r.x_start; i < r.x_end; i++)
	{
		for (int j = r.y_start; j < r.y_end; j++)
		{
			multiply_prewitt(inBuffer, outBuffer, i, j, r.picture_width);
		}
	}
}

/**
* @brief Serial version of edge detection algorithm
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param r object that stores range for x and y coordinates
*/
void filter_serial_edge_detection(int* inBuffer, int* outBuffer, Range r)
{
	for (int i = r.x_start; i < r.x_end; i++)
	{
		for (int j = r.y_start; j < r.y_end; j++)
		{
			search_neighbour_points(inBuffer, outBuffer, i, j, r.picture_width, r.picture_height);
		}
	}
}

/**
* @brief Parallel version of both edge detection and prewitt algorithm
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param r object that stores range for x and y coordinates
* @param fn_serial name of the serial function
*/
template <typename Function_name>
void filter_parallel(int* inBuffer, int* outBuffer, Range r, Function_name fn_serial)
{
	if (r.x_end - r.x_start < CUTOFF || r.y_end - r.y_start < CUTOFF)
		fn_serial(inBuffer, outBuffer, r);

	else {
		task_group tg;
		Range r_temp;

		r_temp.setUpperLeftRange(r);
		tg.run([=] {filter_parallel(inBuffer, outBuffer, r_temp, fn_serial); });

		r_temp.setUpperRightRange(r);
		tg.run([=] {filter_parallel(inBuffer, outBuffer, r_temp, fn_serial); });

		r_temp.setBottomLeftRange(r);
		tg.run([=] {filter_parallel(inBuffer, outBuffer, r_temp, fn_serial); });

		r_temp.setBottomRightRange(r);
		tg.run([=] {filter_parallel(inBuffer, outBuffer, r_temp, fn_serial); });

		tg.wait();
	}
}

/**
* @brief Function that filters inBuffer and calls serial or parallel edge detection function
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param r object that stores range for x and y coordinates
* @param serial parameter that determines whether to call serial or parallel version of edge detection algorithm
*/
void init_edge_detection(int* inBuffer, int* outBuffer, Range r, bool serial)
{
	for (int i = 0; i < r.picture_width * r.picture_height; i++)
		inBuffer[i] = (inBuffer[i] > THRESHOLD) ? 1 : 0;

	if (serial) filter_serial_edge_detection(inBuffer, outBuffer, r);
	else filter_parallel(inBuffer, outBuffer, r, filter_serial_edge_detection);
}

/**
* @brief Function for running test.
* @param testNr test identification, 1,3: for serial version, 2,4: for parallel version
* @param ioFile input/output file, firstly it's holding buffer from input image and than to hold filtered data
* @param outFileName output file name
* @param outBuffer buffer of output image
* @param r object that stores range for x and y coordinates
*/
void run_test_nr(int testNr, BitmapRawConverter* ioFile, char* outFileName, int* outBuffer, Range r)
{
	tick_count startTime = tick_count::now();

	switch (testNr)
	{
	case 1:
		cout << "Running serial version of edge detection using Prewitt operator" << endl;
		filter_serial_prewitt(ioFile->getBuffer(), outBuffer, r);
		break;
	case 2:
		cout << "Running parallel version of edge detection using Prewitt operator" << endl;
		filter_parallel(ioFile->getBuffer(), outBuffer, r, filter_serial_prewitt);
		break;
	case 3:
		cout << "Running serial version of edge detection" << endl;
		init_edge_detection(ioFile->getBuffer(), outBuffer, r, true);
		break;
	case 4:
		cout << "Running parallel version of edge detection" << endl;
		init_edge_detection(ioFile->getBuffer(), outBuffer, r, false);
		break;
	default:
		cout << "ERROR: invalid test case, must be 1, 2, 3 or 4!";
		break;
	}

	tick_count endTime = tick_count::now();
	cout << "Time: \t\t\t" << (endTime - startTime).seconds() << " seconds\n";

	ioFile->setBuffer(outBuffer);
	ioFile->pixelsToBitmap(outFileName);
}

/**
* @brief Print program usage.
*/
void usage()
{
	cout << "\n\ERROR: call program like: " << endl << endl;
	cout << "ProjekatPP.exe";
	cout << " input.bmp";
	cout << " outputSerialPrewitt.bmp";
	cout << " outputParallelPrewitt.bmp";
	cout << " outputSerialEdge.bmp";
	cout << " outputParallelEdge.bmp" << endl << endl;
}

int main(int argc, char* argv[])
{
	if (argc != __ARG_NUM__)
	{
		usage();
		return 0;
	}

	BitmapRawConverter inputFile(argv[1]);
	BitmapRawConverter outputFileSerialPrewitt(argv[1]);
	BitmapRawConverter outputFileParallelPrewitt(argv[1]);
	BitmapRawConverter outputFileSerialEdge(argv[1]);
	BitmapRawConverter outputFileParallelEdge(argv[1]);

	unsigned int width, height;

	int test;
	cout << "Filter size: " << FILTER_SIZE << endl;
	cout << "Point range: " << POINT_RANGE << endl;

	width = inputFile.getWidth();
	height = inputFile.getHeight();

	Range r;
	r.initRange(width, height);

	int* outBufferSerialPrewitt = new int[width * height];
	int* outBufferParallelPrewitt = new int[width * height];

	memset(outBufferSerialPrewitt, 0x0, width * height * sizeof(int));
	memset(outBufferParallelPrewitt, 0x0, width * height * sizeof(int));

	int* outBufferSerialEdge = new int[width * height];
	int* outBufferParallelEdge = new int[width * height];

	memset(outBufferSerialEdge, 0x0, width * height * sizeof(int));
	memset(outBufferParallelEdge, 0x0, width * height * sizeof(int));

	// serial version Prewitt
	run_test_nr(1, &outputFileSerialPrewitt, argv[2], outBufferSerialPrewitt, r);

	// parallel version Prewitt
	run_test_nr(2, &outputFileParallelPrewitt, argv[3], outBufferParallelPrewitt, r);

	// serial version special
	run_test_nr(3, &outputFileSerialEdge, argv[4], outBufferSerialEdge, r);

	// parallel version special
	run_test_nr(4, &outputFileParallelEdge, argv[5], outBufferParallelEdge, r);

	// verification
	cout << "Verification: ";
	test = memcmp(outBufferSerialPrewitt, outBufferParallelPrewitt, width * height * sizeof(int));

	if (test != 0)
	{
		cout << "Prewitt FAIL!" << endl;
	}
	else
	{
		cout << "Prewitt PASS." << endl;
	}

	test = memcmp(outBufferSerialEdge, outBufferParallelEdge, width * height * sizeof(int));

	if (test != 0)
	{
		cout << "Edge detection FAIL!" << endl;
	}
	else
	{
		cout << "Edge detection PASS." << endl;
	}

	// clean up
	delete outBufferSerialPrewitt;
	delete outBufferParallelPrewitt;

	delete outBufferSerialEdge;
	delete outBufferParallelEdge;

	return 0;
}