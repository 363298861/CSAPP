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
#define GETADDR(p) (*(unsigned long*)(p))
#define PREDADDR(p) ((char*)(p))
#define SUCCADDR(p) ((char*)(p) + DSIZE)
#define PUTADDR(p, addr) ((*(unsigned long*)(p)) = (addr))
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define ADDR_SIZE (ALIGN(sizeof(unsigned int*) * 2))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

static char* heap_listp;
static char* free_listp;
static size_t line;
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //char* fp;
    if((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1){
        printf("Memory initialization error");
        return -1;
    }
    PUT(heap_listp, 0);
    PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 2 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 3 * WSIZE, PACK(0, 1));
    heap_listp += 2 * WSIZE;
    free_listp = NULL;
    if((free_listp = extend_heap(CHUNK / WSIZE)) == NULL)
        return -1;
    printf("The initial position is %p\n", heap_listp);
    line = 0;
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
//    PUTADDR(SUCCADDR(bp), free_listp);
//    PUTADDR(PREDADDR(bp), 0);
//    free_listp = bp;
    printf("The boundary is extended to %p and size is %d\n", NEXT(bp), GETSIZE(HADR(bp)));
    return coalesce(bp);
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    printf("Allocating memory of size %lu at line %u $$$$$$$$$$$$$$$\n", size, line++);
    char* bp;
    int newsize;
    if(size < 16)
        newsize = 24;
    else
        newsize = ALIGN(size + SIZE_T_SIZE);
    if((bp = find_fit(newsize)) != NULL){
        printf("The position inside chunk is %p\n", bp);
        place(bp, newsize);
        return bp;
    }

    int ext = MAX(newsize, CHUNK);
    if((bp = extend_heap(ext / WSIZE)) == NULL) return NULL;
    printf("The position outside chunk is %p\n", bp);
    place(bp, newsize);
    return bp;
}

void* find_fit(size_t size){
    printf("Looking for position from %p\n", free_listp);
    char* t = free_listp;
    if(t == NULL || GET(HADR(t)) == 1)
        return NULL;
    while (t){
        printf("Is valid %d at position %p, the value is %d\n", VALID(HADR(t)), t, GET(HADR(t)));
        if(GETSIZE(HADR(t)) < size) {
            printf("The predecessor address is %p and succ address is %p\n", (char*) GETADDR(PREDADDR(t)), (char*) GETADDR(SUCCADDR(t)));
            t = (char *) GETADDR(SUCCADDR(t));
        }
        else {
            printf("The predecessor address is %p and succ address is %p\n", (char*) GETADDR(PREDADDR(t)), (char*) GETADDR(SUCCADDR(t)));
            return (void *) t;
        }
    }
    return NULL;
}

void place(void* p, size_t size){
    size_t psize = GETSIZE(HADR(p));
    printf("The size needed to be allocated is %lu\n within %lu\n", size, psize);
    unsigned long pred = GETADDR(PREDADDR(p));
    unsigned long succ = GETADDR(SUCCADDR(p));
    printf("The pred and succ is %x and %x\n", pred, succ);
    PUT(HADR(p), PACK(size, 1));
    PUT(FADR(p), PACK(size, 1));
    if(psize != size){
        PUT(HADR(NEXT(p)), PACK(psize - size, 0));
        PUT(FADR(NEXT(p)), PACK(psize - size, 0));
    }
    printf("%lu is put at %p and %p\n", size, HADR(p), FADR(p));
    if(psize - size > 16) {
        PUTADDR(PREDADDR(NEXT(p)), pred);
        PUTADDR(SUCCADDR(NEXT(p)), succ);
        printf("%lu is put at %p and %p\n", psize - size, HADR(NEXT(p)), FADR(NEXT(p)));
        if(pred && succ){
            PUTADDR(SUCCADDR(pred), NEXT(p));
            PUTADDR(PREDADDR(succ), NEXT(p));
        }else if(pred){
            PUTADDR(SUCCADDR(pred), NEXT(p));
        }else if(succ){
            PUTADDR(PREDADDR(succ), NEXT(p));
            free_listp = NEXT(p);
        }else{
            free_listp = NEXT(p);
        }
    }else{
        if(pred && succ){
            PUTADDR(SUCCADDR(pred), succ);
            PUTADDR(PREDADDR(succ), pred);
        }else if(pred){
            PUTADDR(SUCCADDR(pred), 0);
        }else if(succ){
            free_listp = succ;
            PUTADDR(PREDADDR(succ), 0);
        }else{
            free_listp = 0;//        unsigned long pred = GETADDR(PREDADDR(NEXT(ptr)));
//        unsigned long succ = GETADDR(SUCCADDR(NEXT(ptr)));
//        printf("The pred and succ is %x and %x\n", pred, succ);
        }
    }
    //char* tmp = free_listp;
    //printf("The free list pointer in place is at %p and the value is %u and %u\n", free_listp, GET(tmp), GET(tmp));
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GETSIZE(HADR(ptr));
    printf("The size that is freed is %lu and the position is at %p at line %lu fffffffff\n", size, ptr, line++);
    PUT(HADR(ptr), PACK(size, 0));
    PUT(FADR(ptr), PACK(size, 0));
    coalesce(ptr);
}

void* coalesce(void* ptr){
    size_t size;
    //printf("Previous is %p and next is %p\n", HADR(PREV(ptr)), HADR(NEXT(ptr)));
    //printf("The number in previous head is %d and the position is %p\n", GET(HADR(PREV(ptr))), HADR(PREV(ptr)));

    if(free_listp == NULL){//case 1
        free_listp = ptr;
        PUTADDR(PREDADDR(ptr), 0);
        PUTADDR(SUCCADDR(ptr), 0);
        printf("case 1\n");
        return ptr;
    }
    printf("The value in next pointer %p is %d\n",NEXT(ptr), GET(HADR(NEXT(ptr))));
    if(VALID(HADR(PREV(ptr))) && VALID(HADR(NEXT(ptr)))){//case 2
        PUTADDR(PREDADDR(ptr), 0);
        PUTADDR(SUCCADDR(ptr), free_listp);
        if(free_listp != 0)
            PUTADDR(PREDADDR(free_listp), ptr);
        free_listp = ptr;
        printf("case 2\n");
    }else if(!VALID(HADR(PREV(ptr))) && VALID(HADR(NEXT(ptr)))){//case 3
        size_t size1 = GETSIZE(HADR(PREV(ptr)));
        size = GETSIZE(HADR(ptr)) + size1;
//        unsigned long pred = GETADDR(PREDADDR(ptr));
//        unsigned long succ = GETADDR(SUCCADDR(ptr));
        printf("The value in previous %p header is %u and in previous foot is %u\n", PREV(ptr), GETSIZE(HADR(PREV(ptr))), GETSIZE(FADR(PREV(ptr))));
        printf("The size of ptr is %lu and previous is %lu\n", GETSIZE(HADR(ptr)), GETSIZE(HADR(PREV(ptr))));
        PUT(HADR(PREV(ptr)), PACK(size, 0));
        PUT(FADR(ptr), PACK(size, 0));
        ptr = PREV(ptr);
        if(size1 < 24){
            PUTADDR(PREDADDR(ptr), 0);
            PUTADDR(SUCCADDR(ptr), free_listp);
            if(free_listp != 0)
                PUTADDR(PREDADDR(free_listp), ptr);
            free_listp = ptr;
        }
        printf("case 3\n");
    }else if(VALID(HADR(PREV(ptr))) && !VALID(HADR(NEXT(ptr)))){//case 4
        int nextSize = GETSIZE(HADR(NEXT(ptr)));
        size = GETSIZE(HADR(ptr)) + nextSize;
        char* nextp = NEXT(ptr);
        PUT(HADR(ptr), PACK(size, 0));
        PUT(FADR(ptr), PACK(size, 0));
        if(nextSize < 24){
            PUTADDR(PREDADDR(ptr), 0);
            PUTADDR(SUCCADDR(ptr), free_listp);
            if(free_listp != 0)
                PUTADDR(PREDADDR(free_listp), ptr);
            free_listp = ptr;
        }else {
            unsigned long pred = GETADDR(PREDADDR(nextp));
            unsigned long succ = GETADDR(SUCCADDR(nextp));
            printf("The pred and succ is %p and %p\n", (char *)pred, (char *)succ);
            PUTADDR(PREDADDR(ptr), pred);
            PUTADDR(SUCCADDR(ptr), succ);
            if (pred && succ) {
                PUTADDR(SUCCADDR(pred), ptr);
                PUTADDR(PREDADDR(succ), ptr);
            } else if (pred) {
                PUTADDR(SUCCADDR(pred), ptr);
            } else if (succ) {
                free_listp = ptr;
                PUTADDR(PREDADDR(succ), ptr);
            } else {
                free_listp = ptr;
            }
        }
        printf("case 4\n");
    }else{//case 5
        size_t size1 = GETSIZE(HADR(PREV(ptr)));
        size_t size2 = GETSIZE(HADR(NEXT(ptr)));
        size = GETSIZE(HADR(ptr)) + size1 + size2;
        char* np = NEXT(ptr);
        PUT(HADR(PREV(ptr)), PACK(size, 0));
        PUT(FADR(NEXT(ptr)), PACK(size, 0));
        ptr = PREV(ptr);
        if(size1 < 24 && size2 < 24){
            PUTADDR(PREDADDR(ptr), 0);
            PUTADDR(SUCCADDR(ptr), free_listp);
            if(free_listp != 0)
                PUTADDR(PREDADDR(free_listp), ptr);
            free_listp = ptr;
        }else if(size1 < 24){
            unsigned long pred = GETADDR(PREDADDR(np));
            unsigned long succ = GETADDR(SUCCADDR(np));
            PUTADDR(PREDADDR(ptr), pred);
            PUTADDR(SUCCADDR(ptr), succ);
            if (pred && succ) {
                PUTADDR(SUCCADDR(pred), ptr);
                PUTADDR(PREDADDR(succ), ptr);
            } else if (pred) {
                PUTADDR(SUCCADDR(pred), ptr);
            } else if (succ) {
                free_listp = ptr;
                PUTADDR(PREDADDR(succ), ptr);
            } else {
                free_listp = ptr;
            }
        }else if(size2 >= 24 && size1 >= 24) {
            unsigned long pred = GETADDR(PREDADDR(np));
            unsigned long succ = GETADDR(SUCCADDR(np));
            if (pred && succ) {
                PUTADDR(SUCCADDR(pred), succ);
                PUTADDR(PREDADDR(succ), pred);
            } else if (pred) {
                PUTADDR(SUCCADDR(pred), 0);
            } else if (succ) {
                free_listp = succ;
                PUTADDR(PREDADDR(succ), 0);
            } else {
                exit(1);
            }
        }
        printf("case 5\n");
    }
    printf("The free list pointer in coalesce is at %p\n", free_listp);
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















