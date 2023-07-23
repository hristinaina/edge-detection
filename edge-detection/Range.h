
/**
Class that stores range for x and y coordinates (and height and width of the image)
*/
struct Range
{
	int x_start, x_end;
	int y_start, y_end;
	int picture_width, picture_height;

	//@brief Function that sets starting values to class attributes
	//@param width width of the image
	//@param height height of the image
	void initRange(int width, int height);

	//@brief Function that sets the range for the upper left quarter of the matrix
	//@param r object from which values are copied
	void setUpperLeftRange(Range r);

	//@brief Function that sets the range for the bottom left quarter of the matrix
	//@param r object from which values are copied
	void setBottomLeftRange(Range r);

	//@brief Function that sets the range for the upper right quarter of the matrix
	//@param r object from which values are copied
	void setUpperRightRange(Range r);

	//@brief Function that sets the range for the bottom right quarter of the matrix
	//@param r object from which values are copied
	void setBottomRightRange(Range r);

	//@brief Function that sets picture_width and picture_height attributes
	void setSize(Range r);
};