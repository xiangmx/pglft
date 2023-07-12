#ifndef __TS_H__
#define __TS_H__


int get_touchscreen_index(int *x,int *y);

typedef struct node
{
    char s[100];
	struct node *next,*prev;
}Node;

typedef struct head
{
	Node*first;
	Node*last;
	int node_num;
}Head;

Node* picture_cell(char*s);


Node *create_node(char *pathname);

int get_touchscreen_index(int *x,int *y);


void printf_list(Node*p);


#endif
