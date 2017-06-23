////////////cvector.c////////////////////////////////
#include "cvector.h"
#include <assert.h>

/*============================================
* FuncName    : VectorNew
* Description :
* @--         :
* Author      :
* Time        : 2017-06-12
============================================*/
Vector VectorNew(void)
{
    Vector v = (Vector)malloc(sizeof(struct VectorSt));
    assert(v != NULL);
//    assert(0);//false会断住，1不会
    v->size = 0;
    v->maxSize = 32;
    v->data = (node *)malloc(sizeof(node) * v->maxSize);
//    v->data = (node *)malloc(sizeof(node)* v->maxSize);
    assert(v->data != NULL);
    return v;
}

/*============================================
* FuncName    : VectorPushBack
* Description :
* @v          :
* @e          :
* Author      :
* Time        : 2017-06-12
============================================*/
void VectorPushBack(Vector v, node e)
{
    assert(v != NULL);
    if (v->size >= v->maxSize) {

        v->maxSize *= 2;
        v->data = (node *)realloc(v->data, v->maxSize * sizeof(node));
        assert(v->data != NULL);
    }

    v->data[v->size++] = e;
}

/*============================================
* FuncName    : VectorPopBack
* Description :
* @v          :
* Author      :
* Time        : 2017-06-12
============================================*/
node VectorPopBack(Vector v)
{
    assert(v != NULL && v->size > 0);
    return v->data[--v->size];
}

/*============================================
* FuncName    : VectorGet
* Description :
* @v          :
* @index      :
* Author      :
* Time        : 2017-06-12
============================================*/
node VectorGet(Vector v, int index)
{
    assert(v != NULL && index >= 0 && index < v->size);
    return v->data[index];
}

/*============================================
* FuncName    : VectorSize
* Description :
* @v          :
* Author      :
* Time        : 2017-06-12
============================================*/
int VectorSize(Vector v)
{
    assert(v != NULL);
    return v->size;
}

/*============================================
* FuncName    : VectorMaxSize
* Description :
* @v          :
* Author      :
* Time        : 2017-06-12
============================================*/
int VectorMaxSize(Vector v)
{
    assert(v != NULL);
    return v->maxSize;
}

/*============================================
* FuncName    : VectorRm
* Description :
* @v          :
* @index      :
* Author      :
* Time        : 2017-06-12
============================================*/
void VectorRm(Vector v, int index)
{
    assert(v != NULL || index <= v->size - 1);
    int i;
    if (index< v->size - 1){
        for (i = index; i < v->size - 1; ++i)
            v->data[i] = v->data[i + 1];
        v->size--;
    }
    else{
        v->size--;
    }
}

/*============================================
* FuncName    : VectorDelete
* Description :
* @v          :
* Author      :
* Time        : 2017-06-12
============================================*/
void VectorDelete(Vector v)
{
    assert(v != NULL && v->data != NULL);
    free(v->data);
    free(v);
}







/*============================================
* FuncName    : test_cvector
* Description :
* @           :
* Author      :
* Time        : 2017-06-12
============================================*/
int test_cvector()
{
    Vector v = VectorNew();
    node e1 = {"lili" , 1111};
    node e2 = { "Mario", 2222 };
    node e3 = { "Lucy", 3333 };

    VectorPushBack(v , e1);
    VectorPushBack(v, e2);
    VectorPushBack(v, e3);
    printf("the size of v is :%d\n", VectorSize(v));

    int i;
    printf("The id value of all nodes in v is :");
    for (i = 0; i < VectorSize(v); ++i){
        printf("%d ", VectorGet(v , i).msglen);
    }
    printf("\n");

    VectorDelete(v);

    return 0;
}










