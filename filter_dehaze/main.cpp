#include<iostream>
#include <opencv2/opencv.hpp>
#include<vector> 
#include <algorithm> 
using namespace cv; 
using namespace std; //求暗通道

int min_(int a, int b, int c)
{
	int res = 0;
	res = a<b?a:b;
	res = res<c?res:c;
	return res;
}

Mat minfilter(Mat src,int size)//最小值滤波
{
	int row = src.rows;
	int col = src.cols;
	Mat dst = Mat::zeros(row,col,CV_8UC1);
	for (int i=1;i<row-1;i++)
		for (int j=1;j<col-1;j++)
		{
			int min = 1000;
			int tmp = 1000;
			for (int x=0;x<size;x++)
			{
				for (int y=0;y<size;y++)
				{
					tmp = src.at<uchar>(i-size/2+x,j-size/2+y);
					if (min>tmp)
						min = tmp;
				}
			}
			dst.at<uchar>(i,j) = min;
		}
	//int minAtomsLight = 220;//经验值
	//double maxval = 0;
	//minMaxLoc(dst,NULL,&maxval); 
	//double A = min(minAtomsLight, (int)maxval);//得到A
	return dst;
	//namedWindow("dst",0);
	//imshow("dst",dst);
	//waitKey(0);
}

Mat getantongdao(Mat &src,int size)//得到暗通道
{
	int row = src.rows;
	int col = src.cols;
	Mat tmp(src.size(),CV_8UC1,Scalar(0));
	Mat dst(src.size(),CV_8UC1,Scalar(0));
	for (int i=0;i<row;i++)
	{
		for (int j=0;j<col;j++)
		{
            int a = src.at<Vec3b>(i,j)[0];
            int b = src.at<Vec3b>(i,j)[1];
            int c = src.at<Vec3b>(i,j)[2];
            tmp.at<uchar>(i,j) = saturate_cast<int>(min_(a,b,c));
		}
	}
	//Mat dst = Mat::zeros(row,col,CV_8UC1);
	for (int i=1;i<row-1;i++)
		for (int j=1;j<col-1;j++)
		{
			int min = 1000;
			int tmp = 1000;
			for (int x=0;x<size;x++)
			{
				for (int y=0;y<size;y++)
				{
					tmp = src.at<uchar>(i-size/2+x,j-size/2+y);
					if (min>tmp)
						min = tmp;
				}
			}
			dst.at<uchar>(i,j) = min;
		}
	return dst;
}



Mat darkChannel(Mat src) 
{ 
	Mat rgbmin = Mat::zeros(src.rows, src.cols, CV_8UC1);
	Mat dark = Mat::zeros(src.rows, src.cols, CV_8UC1); 
	Vec3b intensity; 
	for (int m = 0; m<src.rows; m++) 
	{ 
		for (int n = 0; n<src.cols; n++) 
		{ 
			intensity = src.at<Vec3b>(m, n); 
			rgbmin.at<uchar>(m, n) = min(min(intensity.val[0], intensity.val[1]), intensity.val[2]); 
		} 
	} //模板尺寸 
	int scale = 31; //
	//cout << "Please enter the mask scale: " << endl; //
	//cin >> scale; //边界扩充 
	int radius = (scale - 1) / 2; 
	Mat border; //由于要求最小值，所以扩充的边界可以用复制边界填充 
	copyMakeBorder(rgbmin, border, radius, radius, radius, radius, BORDER_REPLICATE); //最小值滤波 
	for (int i = 0; i < src.cols; i++) 
	{ 
		for (int j = 0; j < src.rows; j++) 
		{ //选取兴趣区域 
			Mat roi; 
			roi = border(Rect(i, j, scale, scale)); //求兴趣区域的最小值 
			double minVal = 0; 
			double maxVal = 0; 
			Point minLoc; 
			Point maxLoc; 
			minMaxLoc(roi, &minVal, &maxVal, &minLoc, &maxLoc, noArray());
			dark.at<uchar>(Point(i, j)) = (uchar)minVal;
		}
	}
	return dark; 
} 
uchar light(vector<uchar> inputIamgeMax) 
{ 
	uchar maxA=0;
	int sum = 0;
	for (int i = 0; i < inputIamgeMax.size() - 1; i++) 
	{ 
		sum+=inputIamgeMax[i];
		//cout<<(int)inputIamgeMax[i]<<"  ";
		//if (maxA < inputIamgeMax[i + 1])
		//{ 
		//	maxA = inputIamgeMax[i + 1];
		//} 
	}
	int ave = sum/inputIamgeMax.size();
	if (ave>220)
		ave=220;
	return ave;
}
int main(int argc, char* argv[]) 
{ 
	Mat image = imread(argv[1]); 
	imshow("image",image);
	Mat darkChannel1(image.size(),CV_8UC1);
	darkChannel1 = darkChannel(image);
	int size = 21;
	//darkChannel1 = getantongdao(image,size);
	imshow("darkChannel1", darkChannel1);
	namedWindow("dehazed"); //估计大气光 
	Mat temp; 
	darkChannel1.copyTo(temp); 
	vector<Point> darkMaxPoint; 
	vector<uchar> inputMax; 
	for (long i = 0; i < ((darkChannel1.rows*darkChannel1.cols) / 1000); i++) 
	{ 
		double minVal = 0;
		double maxVal = 0; 
		Point minLoc; 
		Point maxLoc; 
		minMaxLoc(temp, &minVal, &maxVal, &minLoc, &maxLoc, noArray()); 
		darkMaxPoint.push_back(maxLoc); 
		inputMax.push_back(image.at<uchar>(maxLoc));
		circle(temp, maxLoc,5, Scalar(0), 1, 8, 0); 
		temp.at<uchar>(maxLoc) = temp.at<uchar>(minLoc); 
	}
	//imshow("temp",temp);
	uchar A = light(inputMax); 
	cout<<endl;
	cout<<(int)A<<endl;
	double w = 0.65; //createTrackbar("w1", "dehazed", &w1, 100, NULL); 
	//求折射率
	Mat T = Mat::zeros(image.rows, image.cols, CV_8UC3); 
	Scalar intensity; 
	for (int m = 0; m<image.rows; m++) 
	{
		for (int n = 0; n<image.cols; n++) 
		{ 
			intensity = darkChannel1.at<uchar>(m, n); 
			T.at<Vec3b>(m, n)[0] = (1 - w * intensity.val[0] / A) * 255; 
			T.at<Vec3b>(m, n)[1] = (1 - w * intensity.val[0] / A) * 255; 
			T.at<Vec3b>(m, n)[2] = (1 - w * intensity.val[0] / A) * 255; 
		} 
	} //去雾
	imshow("T",T);
	Mat J(image.rows, image.cols, CV_8UC3, Scalar(180, 120, 50)); 
	Mat temp1(image.rows, image.cols, CV_8UC3, Scalar(180, 120, 50)); 
	//subtract(image, Scalar(A, A, A), temp1); 
	//temp1 = abs(image - Scalar(A, A, A)); 
	double t0 = 0.1;
	Scalar T1; 
	Vec3b intsrc; 
	for (int i = 0; i < image.cols; i++)
	{ 
		for (int j = 0;
				j < image.rows; j++) 
		{ 
			T1 = T.at<uchar>(Point(i, j));
			intsrc = image.at<Vec3b>(Point(i, j)); 
			double tmax = (T1.val[0] / 255) < t0 ? t0 : (T1.val[0] / 255);
			for (int k = 0; k < 3; k++)
			{ 
				J.at<Vec3b>(Point(i, j))[k] = abs((intsrc.val[k] - A) / tmax + A) > 255 ? 255 : 
					abs((intsrc.val[k] - A) / tmax + A);
			} 
		} 
	} 
	imshow("dehazed", J); 
	while (char(waitKey(1)) != 'q') {} 
	return 0; 
}

