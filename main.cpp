#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "windows.h"
#include "io.h"
#include "fstream"
#include "math.h"
#include "time.h"
#define MAX 30000          //最大把一张图片划分为30000块
#define MAXTILES 500        //最多读取500张马赛克图片

using namespace cv;
using namespace std;


typedef struct pic_headinfo
{
	double blue;
	double green;
	double red;
}pic_headinfo;

typedef struct tilefile //定义一种马赛克图像文件
{
	pic_headinfo head_tile;
	char name[80];//文件名字
	Mat tile;//图像信息
	
}tilefile;

pic_headinfo coloravg(Mat src,int height,int width)
{
	pic_headinfo head_tile;
	int i,j;
	Vec3b intensity;
	double blue  =0 ;
	double green =0;
	double red   =0 ;
		for(i=0;i<width;i++)
		{
			for(j=0;j<height;j++)
			{
				intensity=src.at<Vec3b>(i,j);
				blue += intensity.val[0];
				green+= intensity.val[1];
				red  += intensity.val[2];
			}
		}
	head_tile.blue=blue/(width*height);
	head_tile.green=green/(width*height);
	head_tile.red=red/(width*height);
	return head_tile;
}

void creat_piclist_txt(char *path)//创建一个txt,记录本文件所有图片路径
{
	FILE *fp=fopen(path,"w+");
	struct _finddata_t fa;
    long fileHandle;
	
    if( (fileHandle=_findfirst( ".\\tilepic\\*.bmp", &fa ))==-1L )
    {
        printf( "无bmp文件\n");
    }
	else
    {
		do
		{
			printf( "找到%s\n", fa.name );
			fprintf(fp,".\\tilepic\\%s\n",fa.name);
		}while( _findnext(fileHandle,&fa)==0 );
		_findclose( fileHandle );
	}
	
    if( (fileHandle=_findfirst( ".\\tilepic\\*.jpg", &fa ))==-1L )
    {
        printf( "无jpg文件\n");
    }
	else
    {
		do
		{
			printf( "找到%s\n", fa.name );
			fprintf(fp,".\\tilepic\\%s\n",fa.name);
		}while( _findnext(fileHandle,&fa)==0 );
		_findclose( fileHandle );
	}
    if( (fileHandle=_findfirst( ".\\tilepic\\*.png", &fa ))==-1L )
    {
        printf( "无png文件\n");
    }
	else
    {
		do
		{
			printf( "找到%s\n", fa.name );
			fprintf(fp,".\\tilepic\\%s\n",fa.name);
		}while( _findnext(fileHandle,&fa)==0 );
		_findclose( fileHandle );
	}
	fclose(fp);  
}

int read_piclist(const string& filename,tilefile tilefilelist[])//读取piclist.txt中所有图片路径，并根据图片路径读取图片
{
	int i=0;
	ifstream file(filename,ifstream::in);//创建一个输入流对象
	if (!file)//如果输入流创建失败，则报错，程序终止
	{
		return -1;
	}
	string path;//string变量，用以存储读出的路径

	while (getline(file,path))//从文件中读出一行的内容，保存在变量path中
	{
		if (!path.empty())
		{
			tilefilelist[i].tile=imread(path,1);	
			if(tilefilelist[i].tile.empty())
			{
				printf("%d.bmp不存在",i);
				//tilefilelist[i].tile=imread("sample.bmp",1);
			}
			else
			{
				printf("已读取%d张图片\r",i);
				i++;
			}
		}
	}
	printf("读取所有图片完毕。\n");
	return i;
}


double get_min_distance(pic_headinfo pa,pic_headinfo pb)
{
	double min_distance=sqrt((pa.blue - pb.blue) 
			* (pa.blue - pb.blue) 
			+ (pa.green - pb.green)  
			* (pa.green - pb.green) 
			+ (pa.red - pb.red) 
			* (pa.red - pb.red));
	return min_distance;
}
int main()
{
	char inFileName[200];
	int m,n;
	int num_of_pic;
	int i=0,j,k=0;
	tilefile tilefilelist[MAXTILES];//存放图像的数组


	creat_piclist_txt(".\\tilepic\\piclist.txt");
	num_of_pic=read_piclist(".\\tilepic\\piclist.txt",tilefilelist);

	printf( "请输入文件名: ");
	scanf("%s",inFileName);
	Mat src = imread(inFileName,1);
	if(src.empty())
	{   printf( "读取失败，请检查文件\n");
		system("pause");
		return 0;
	}
	printf("请输入每行嵌入马赛克块数：");
	scanf("%d",&m);
	int tile_width=src.cols/m;//求出每块区域的宽和高
	int tile_height=tile_width;
	n=((double)m)/((double)src.cols/src.rows);//求出n
	
	if((n*m)>MAX)
	{
		printf("超出%d张图片\n",MAX);
		system("pause");
		return 0;
	}
	else{
		clock_t start=clock();
		Size cell_size=Size(tile_width*5,tile_height*5);//组成新图的每一块小图片大小,
		Size new_size=Size(m*cell_size.width,n*cell_size.height);//调整后的新图片尺寸
		printf("tile width=%d tile height=%d n=%d\n",tile_width,tile_height,n);
		printf("src.cols=%d src.rows=%d\n",src.cols,src.rows);
		
		//resize(src,src,new_size);
		Mat dst;
		resize(src,dst,new_size);
		for(i=0;i<num_of_pic;i++)
		{  
			resize(tilefilelist[i].tile,tilefilelist[i].tile,cell_size);
			tilefilelist[i].head_tile=coloravg(tilefilelist[i].tile,
				tilefilelist[i].tile.cols,tilefilelist[i].tile.rows);
			
			
		}

		pic_headinfo src_head[MAX];
		int best_tile_name[MAX]={0};
		for(i=0;i<n;i++)
		{
			for(j=0;j<m;j++)
			{
				Mat partofsrc(dst,Rect(j*cell_size.width,i*cell_size.height,
					cell_size.width,cell_size.height));
				src_head[k]=coloravg(partofsrc,partofsrc.cols,partofsrc.rows);
				k=k+1;
			}
		}   
		srand((unsigned)time(NULL));
	for(i=0;i<(n*m);i++)
	{	
		int rn=rand()%160;
		double min_distance=get_min_distance(src_head[i],tilefilelist[rn].head_tile);
		best_tile_name[i]=rn;
		int flag=0;
		for(j=0;j<num_of_pic;j++)
		{   //尽量避免某一范围内全是同样图片
			for(int k=i-3;k<i && k>0;k++)
				{if(j==best_tile_name[k])
						flag=1;}
			for(int k=i-m-3;k<i-m+4 && k>0;k++)
				{if(j==best_tile_name[k])
						flag=1;}
			for(int k=i-2*m+3;k<i-2*m+4 &&k>0;k++)
				{if(j==best_tile_name[i-2*m])
						flag=1;}
			if(flag == 0)
			{
			double distance=get_min_distance(src_head[i],tilefilelist[j].head_tile);
			if(distance<=min_distance)
			{
							min_distance=distance;
							best_tile_name[i]=j;
			}}
			flag=0;//重置为0
		}
		
	}
	
	k=0;
	for(i=0;i<n;i++)
	{
			for(j=0;j<m;j++)
			{
				
				Mat partofdst(dst,Rect(j*cell_size.width,i*cell_size.height,
					cell_size.width,cell_size.height));
				tilefilelist[best_tile_name[k]].tile.copyTo(partofdst);
				k++;
			}
	}
	clock_t finish=clock();
	printf("运行时间 %f ms\n",(double(finish-start)));
	//imshow("原图", src);
	//imshow("新图", dst);
	//imwrite("mosaic_gakki.bmp",dst);
	int result;
	result=cvSaveImage("mosaic_gakki.jpg",&IplImage(dst));
	if(result=1)
		system("mosaic_gakki.jpg");
	}
	system("pause");
}