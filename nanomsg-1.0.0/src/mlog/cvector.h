#ifndef CVECTOR_H
#define CVECTOR_H


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/////////结点存放自己的数据结构//////////
//typedef struct
//{
//    char name[10];
//    int id;
//}node;

#include "memorylog.h"

typedef T_MLOG node;




#pragma pack(1)
typedef struct VectorSt {
    int size;
    int maxSize;
    node *data;
} *Vector;
#pragma pack()


Vector VectorNew(void);
void VectorPushBack(Vector v, node e);
node VectorPopBack(Vector v);
node VectorGet(Vector v, int index);
int VectorSize(Vector v);
int VectorMaxSize(Vector v);
void VectorRm(Vector v, int index);
void VectorDelete(Vector v);


int test_cvector();
#endif // CVECTOR_H
