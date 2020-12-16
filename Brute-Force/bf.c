#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<time.h>
typedef struct Data_point{
	double x;
	double y;
	double dist;
	int index;
}Data_point;
Data_point s_data[100001];
int data_size;
void range_query(double range);
void knn_query(int k);

void load_data(int filetype,double point_x,double point_y){
	FILE *fp;
	int i=0;
    char v;
    double x,y;

	switch(filetype){
		case 1:
				fp=fopen("../geospatial_data/clustered_dataset.txt","r");
				break;
		case 2:
				fp=fopen("../geospatial_data/gaussian_dataset.txt","r");
				break;
		case 3:
				fp=fopen("../geospatial_data/uniformed_dataset.txt","r");
				break;
	}

	while(fscanf(fp,"%lf%c%c%lf",&x,&v,&v,&y)!=EOF){
		s_data[i].x=x;
		s_data[i].y=y;
		s_data[i].dist=sqrt(pow(x-point_x,2)+pow(y-point_y,2));
		i+=1;
	}
	data_size=i;
}
int main(){
		int query_type,file_type,k;
		double point_x, point_y,range;
		printf("choose query\n");
		printf("1. Range query\n");
		printf("2. KNN query\n");
		scanf("%d",&query_type);
		printf("choose file\n");
		printf("1. clustered_dataset\n");
		printf("2. gaussian_dataset\n");
		printf("3. uniformed_dataset\n");
		scanf("%d",&file_type);

		if (query_type!=1 && query_type!=2){
			printf("Error: Invalid query type");
			return -1;
		}
		printf("Please enter the query point:  \n");
		printf("point x: ");
		scanf("%lf",&point_x);
		printf("point y: ");
		scanf("%lf",&point_y);
		load_data(file_type,point_x,point_y);
		
		clock_t start, end;
		if (query_type==1){
		
			printf("Please enter the range: ");
			scanf("%lf",&range);
			start=clock();
			range_query(range);
			end=clock();
			printf("Execution time: %.5lf\n\n",(double)(end-start));
		}
		else if(query_type==2){
			printf("Please enter k: ");
			scanf("%d",&k);
			start=clock();
			knn_query(k);
			end=clock();
			printf("Execution time: %.5lf\n\n",(double)(end-start));
		}
}

void range_query(double range){
	int count=0;
	for(int i=0;i<data_size;i++){
		if (s_data[i].dist<range){
			//printf("index: %d : %.2lf, %.2lf\n", i, s_data[i].x,s_data[i].y);
			count+=1;
		}
	}
	printf("total_count: %d\n",count);
}
void knn_query(int k){
	int cur_num,cur_max,j;
	cur_num=0;
	cur_max=s_data[0].dist;
	Data_point *knn=malloc(sizeof(int*)*k);
	for(int i=0;i<data_size;i++){
		
		if(cur_num==k && knn[k-1].dist<s_data[i].dist)
				continue;
		
		if(cur_num==k){
			if (knn[k-1].dist>s_data[i].dist)
				cur_num-=1;
			else
				continue;
		}

		//printf("%d\n",i);	
		for(j=cur_num-1;j>=0 && knn[j].dist>s_data[i].dist;j--){
			knn[j+1].index=knn[j].index;
			knn[j+1].dist=knn[j].dist;
			knn[j+1].x=knn[j].x;
			knn[j+1].y=knn[j].y;
		}
		knn[j+1].dist=s_data[i].dist;
		knn[j+1].x=s_data[i].x;
		knn[j+1].y=s_data[i].y;
		knn[j+1].index=i;
		cur_num+=1;
	}
	printf("BRUTE FORCE KNN QUERY RESULT\n");
	for(int i=0;i<k;i++){
		printf("(%.2lf, %.2lf) dist:  %lf \n",knn[i].x,knn[i].y,knn[i].dist);
	
	}
}
