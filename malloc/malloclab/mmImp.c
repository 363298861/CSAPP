/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "malloc",
    /* First member's full name */
    "zhiyuan",
    /* First member's email address */
    "363298861@qq.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define WSIZE 4
#define DSIZE 8
#define CHUNK (1 << 12)
#define PACK(size, alloc) ((size) | (alloc))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define GET(p) (*(unsigned int*)(p))
#define GETSIZE(p) ((GET(p)) & (~0x7))
#define VALID(p) ((GET(p)) & 1)
#define PUT(p, r) ((*(unsigned int*)(p)) = (r))
#define HADR(p) ((char*)(p) - WSIZE)
#define FADR(p) ((char*)(p) - DSIZE + GETSIZE(HADR(p)))
#define NEXT(p) ((char*) (p) + GETSIZE(HADR(p)))
#define PREV(p) ((char*) (p) - GETSIZE(((char*)(p) - DSIZE)))

#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

void* find_fit(size_t size);
void place(void* p, size_t size);
void* coalesce(void* ptr);
static void* extend_heap(size_t words);

static char* heap_listp;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1){
        printf("Memory initialization error");
        return -1;
    }
    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 2 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 3 * WSIZE, PACK(0, 1));
    heap_listp += 2 * WSIZE;
    if(extend_heap(CHUNK / WSIZE) == NULL)
        return -1;
    printf("The initial position is %p\n", heap_listp);
    return 0;
}

static void* extend_heap(size_t words){
    size_t size;
    char* bp;

    size = ALIGN(words * WSIZE);
    if((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    PUT(HADR(bp), PACK(size, 0));
    PUT(FADR(bp), PACK(size, 0));
    PUT(HADR(NEXT(bp)), PACK(0, 1));
    //printf("The boundary is extended to %p and size is %d\n", bp, GETSIZE(HADR(bp)));
    return coalesce(bp);
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

void *mm_malloc(size_t size)
{
    char* bp;
    int newsize = ALIGN(size + SIZE_T_SIZE);
    //printf("The newsize is %d\n", newsize);
    if((bp = find_fit(newsize)) != NULL){
        //printf("The position inside chunk is %p\n", bp);
        place(bp, newsize);
        return bp;
    }

    int ext = MAX(newsize, CHUNK);
    if((bp = extend_heap(ext / WSIZE)) == NULL) return NULL;
    //printf("The position outside chunk is %p\n", bp);
    place(bp, newsize);
    return bp;
}


void* find_fit(size_t size){
    char* t = heap_listp;
    while(GETSIZE(HADR(t))){
	//printf("Is valid %d at position %p, the value is %d\n", VALID(HADR(t)), t, GET(HADR(t)));
        if(VALID(HADR(t)) || GETSIZE(HADR(t)) < size)
            t = NEXT(t);
        else
            return (void*) t;
    }
    return NULL;
}

void place(void* p, size_t size){
    size_t psize = GETSIZE(HADR(p));
    PUT(HADR(p), PACK(size, 1));
    PUT(FADR(p), PACK(size, 1));
    if(psize != size){
        PUT(HADR(NEXT(p)), PACK(psize - size, 0));
        //printf("The new size for the block is %d - %d\n", psize, size);
        //printf("The next p is at %p\n", NEXT(p));
        PUT(FADR(NEXT(p)), PACK(psize - size, 0));
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GETSIZE(HADR(ptr));
    PUT(HADR(ptr), PACK(size, 0));
    PUT(FADR(ptr), PACK(size, 0));
    //printf("The value that is freed is %d, the new value is %d, the address is %p\n", GET(HADR(PREV(ptr))), PACK(size, 0), ptr);
    coalesce(ptr);
}

void* coalesce(void* ptr){
    size_t size;
    //printf("Previous is %d and next is %d\n", VALID(HADR(PREV(ptr))), VALID(HADR(NEXT(ptr))));
    //printf("The number in previous head is %d and the position is %p\n", GET(HADR(PREV(ptr))), HADR(PREV(ptr)));
    //printf("The current address is %p and previous address is %p, size is %d\n", ptr, PREV(ptr), GET((char*)ptr -8));
    if(VALID(HADR(PREV(ptr))) && VALID(HADR(NEXT(ptr))))
        return ptr;
    else if(!VALID(HADR(PREV(ptr))) && VALID(HADR(NEXT(ptr)))){
        size = GETSIZE(HADR(ptr)) + GETSIZE(HADR(PREV(ptr)));
        PUT(HADR(PREV(ptr)), PACK(size, 0));
        PUT(FADR(ptr), PACK(size, 0));
        ptr = PREV(ptr);
	//printf("1\n");
    }else if(VALID(HADR(PREV(ptr))) && !VALID(HADR(NEXT(ptr)))){
        size = GETSIZE(HADR(ptr)) + GETSIZE(HADR(NEXT(ptr)));
        PUT(HADR(ptr), PACK(size, 0));
        PUT(FADR(ptr), PACK(size, 0));
	//printf("2\n");
    }else{
        size = GETSIZE(HADR(ptr)) + GETSIZE(HADR(PREV(ptr))) + GETSIZE(HADR(NEXT(ptr)));
        PUT(HADR(PREV(ptr)), PACK(size, 0));
        PUT(FADR(NEXT(ptr)), PACK(size, 0));
        ptr = PREV(ptr);
	//printf("3\n");
    }
    return (void*) ptr;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}















