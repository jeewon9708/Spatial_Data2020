/**
   rtree lib usage example app.
*/

#include <stdio.h>
#include<stdlib.h>
#include<math.h>
#include "rtree.h"
#include<time.h>
/*
RTREEMBR rects[] = {
    { {0, 0, 0, 2, 2, 0} },   xmin, ymin, zmin, xmax, ymax, zmax (for 3 dimensional RTree) 
    { {5, 5, 0, 7, 7, 0} },
    { {8, 5, 0, 9, 6, 0} },
    { {7, 1, 0, 9, 2, 0} }
};

*/
//int nrects = sizeof(rects) / sizeof(rects[0]);
//RTREEMBR search_rect = {
//    {6, 4, 0, 10, 6, 0}   /* search will find above rects that this one overlaps */
//};

RTREENODE* root;
int MySearchCallback(int id, void* arg) 
{
    /* Note: -1 to make up for the +1 when data was inserted */
//    fprintf (stdout, "Hit data mbr %d ", id-1);
    return 1; /* keep going */
}
static inline double MinDist(RTREEMBR *m, RTREEMBR *p){
	double dist=0;

    if(m->bound[3] < p->bound[0]) //mbr_right_x < point_ x
		dist += pow(p->bound[0] - m->bound[3],2);
  	else if(m->bound[0] > p->bound[0]) //mbr_xmin > point_x
		dist += pow(p->bound[0] - m->bound[0],2);
	if(m->bound[4] < p->bound[1]) //mbr_ymax < point_y
    	dist += pow(p->bound[1] - m->bound[4],2);
   	else if(m->bound[1] > p->bound[1]) //mbr_ymin > point_y
    	dist += pow(p->bound[1] - m->bound[1],2);

  return sqrt(dist);

}

void load_data(int filetype){
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
		RTREEMBR point = {{x, y, 0, x, y, 0}};
		RTreeInsertRect(&point,i+1,&root,0);
        i+=1;
    }
}
//---create queue------//
struct queue_node{
	RTREEBRANCH node;
	struct queue_node* next;
	int level;
	double dist;
};
struct priority_queue{
	struct queue_node* head;
	int size;
};

struct priority_queue queue;
void print_queue();
void enqueue(struct priority_queue *queue,struct queue_node *newnode, RTREEMBR *q){
	struct queue_node* new;
	
	new=(struct queue_node *)malloc(sizeof(struct queue_node));
	*new=*newnode;
	new->dist=MinDist(&new->node.mbr,q);
	new->next=NULL;
	if( queue->size ==0){
		queue->head=new;
		queue->head->next=NULL;
		queue->size+=1;
		return;
	}
	struct queue_node *ptr;
	struct queue_node *prev_ptr;

	ptr=queue->head;

	if(ptr->dist>=new->dist){
		new->next=ptr;
		queue->head=new;
		queue->size+=1;
		return;
	}
	while(ptr->dist < new->dist){	
		if(!ptr->next) break;
		if(ptr->next->dist > new->dist) break;
		ptr=ptr->next;
	}
	new->next=ptr->next;
	ptr->next=new;
	queue->size+=1;
}

void init_queue(RTREEMBR *q){
    queue.size=0;
    for(int i=0;i<MAXCARD;i++){
        if(&root->branch[i]){
            struct queue_node newnode;
            newnode.node=root->branch[i];
            newnode.level=root->level;
            enqueue(&queue,&newnode,q);
        }
    }
	
}
void print_queue(){
	struct queue_node *ptr;
	ptr=queue.head;
	for(int i=0;i<queue.size;i++){
		printf("%lf ",ptr->dist);
		ptr=ptr->next;
		if(!ptr)
			break;
	}
	printf("\n");
}

void RTreeKNN(struct priority_queue *queue, RTREEMBR *q, int k){
	int count=0;

	init_queue(q);
	printf("RTREE KNN QUERY RESULT\n");
	while(queue->size>0){
		struct queue_node *n;
		n=queue->head;
		queue->head=queue->head->next;
		
		if(n->level ==0){
			printf("(%.2lf %.2lf)	dist:%lf \n",n->node.mbr.bound[0],n->node.mbr.bound[1],n->dist);
			count+=1;
		}
		else{
			for(int i=0;i<MAXCARD;i++){
				if(&n->node.child->branch[i]){
					struct queue_node newnode;
            		newnode.node=n->node.child->branch[i];
           			newnode.level=n->node.child->level;
           		    enqueue(queue,&newnode,q);
	
				}
				else
					break;
			}

		
		}
		if(count == k)	break;

	}
}


int main()
{
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
    
	RTREEMBR qpoint={};
	
	root = RTreeCreate();
   	load_data(file_type);
	
	clock_t start, end;

    if (query_type==1){
		printf("Please enter the range: ");
		scanf("%lf",&range);
        qpoint.bound[0]=point_x-range;
        qpoint.bound[1]=point_y-range;
        qpoint.bound[2]=0;//z
        qpoint.bound[3]=point_x+range;
        qpoint.bound[4]=point_y+range;
        qpoint.bound[5]=0;//z
		start=clock();
		int count = RTreeSearch(root,&qpoint,MySearchCallback,0);
		end=clock();
		printf("range query counts: %d\n",count);
		printf("Execution time: %lf",(double)(end-start));
    }
    else if(query_type==2){
        printf("Please enter k: ");
        scanf("%d",&k);
        qpoint.bound[0]=point_x;
        qpoint.bound[1]=point_y;
        qpoint.bound[2]=0;//z
        qpoint.bound[3]=point_x;
        qpoint.bound[4]=point_y;
        qpoint.bound[5]=0;//z
		start=clock();
		RTreeKNN(&queue,&qpoint,k);
		end=clock();
		printf("Execution time: %lf\n",(double)(end-start));
    }







    
    

    RTreeDestroy (root);

    return 0;
}
