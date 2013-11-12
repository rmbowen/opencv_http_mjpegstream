#include <iostream>
#include <sstream>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{		
	VideoCapture capture("http://localhost:8082");		

	Mat raw;
	
	capture >> raw;
	
	namedWindow("stream",1);
	
	
	while(1){
		
		capture >> raw;
		
		imshow("stream",raw);
		
		waitKey(5);		
		
	}
	
	return 0;
}
