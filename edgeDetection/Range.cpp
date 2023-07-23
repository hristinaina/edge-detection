#include "Range.h"
#include "Parameters.h"

/**
*@brief Function that sets starting values to class attributes
*@param width width of the image
*@param height height of the image
*/
void Range::initRange(int width, int height)
{
	int cut = (FILTER_SIZE -1) / 2;

	x_start = cut;
	y_start = cut;
	x_end = height - cut;
	y_end = width - cut;

	picture_width = width;
	picture_height = height;
}

/**
*@brief Function that sets the range for the upper left quarter of the matrix
*@param r object from which values are copied
*/
void  Range::setUpperLeftRange(Range r)
{
	x_start = r.x_start;
	x_end = r.x_start + (r.x_end - r.x_start) / 2;
	y_start = r.y_start;
	y_end = r.y_start + (r.y_end - r.y_start) / 2;

	setSize(r);
}

/**
*@brief Function that sets the range for the bottom left quarter of the matrix
*@param r object from which values are copied
*/
void  Range::setBottomLeftRange(Range r)
{
	x_start = r.x_start;
	x_end = r.x_start + (r.x_end - r.x_start) / 2;
	y_start = r.y_start + (r.y_end - r.y_start) / 2;
	y_end = r.y_end;

	setSize(r);
}

/**
*@brief Function that sets the range for the upper right quarter of the matrix
*@param r object from which values are copied
*/
void  Range::setUpperRightRange(Range r)
{
	x_start = r.x_start + (r.x_end - r.x_start) / 2;
	x_end = r.x_end;
	y_start = r.y_start;
	y_end = r.y_start + (r.y_end - r.y_start) / 2;

	setSize(r);
}

/**
*@brief Function that sets the range for the bottom right quarter of the matrix
*@param r object from which values are copied
*/
void  Range::setBottomRightRange(Range r)
{
	x_start = r.x_start + (r.x_end - r.x_start) / 2;
	x_end = r.x_end;
	y_start = r.y_start + (r.y_end - r.y_start) / 2;
	y_end = r.y_end;

	setSize(r);
}

//@brief Function that sets picture_width and picture_height attributes
void  Range::setSize(Range r)
{
	picture_width = r.picture_width;
	picture_height = r.picture_height;
}