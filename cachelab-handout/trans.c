/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose32(int M, int N, int A[N][M], int B[M][N]);
void transpose64(int M, int N, int A[N][M], int B[M][N]);
void transpose61(int M, int N, int A[N][M], int B[M][N]);
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){

    if(M == 32)
        transpose32(M, N, A, B);
    else if(M == 64)
        transpose64(M, N, A, B);
    else
        transpose61(M, N, A, B);
}

void transpose32(int M, int N, int A[N][M], int B[M][N]){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            for(int m = 0; m < 8; m++){
                int tmp, idx;
                int diagonal = 0;
                for(int n = 0; n < 8; n++){
                    if(n + j * 8 != m + i * 8)
                        B[n + j * 8][m + i * 8] = A[m + i * 8][n + j * 8];
                    else{
                        tmp = A[m + i * 8][n + j * 8];
                        idx = m + i * 8;
                        diagonal = 1;
                    }
                }
                if(diagonal)
                    B[idx][idx] = tmp;
            }
        }
    }
}


void transpose64(int M, int N, int A[N][M], int B[M][N]) {

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            for (int m = 0; m < 4; m++) {
                int tmp, idx;
                int diagonal = 0;
                for (int n = 0; n < 4; n++) {
                    if(n + j * 4 != m + i * 4){
                        B[n + j * 4][m + i * 4]  = A[m + i * 4][n + j * 4];
                    }else{
                        tmp = A[m + i * 4][n + j * 4];
                        idx = m + i * 4;
                        diagonal = 1;
                    }
                }
                if(diagonal)
                    B[idx][idx] = tmp;
            }
        }
    }
}

void transpose61(int M, int N, int A[N][M], int B[M][N]){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 3; j++){
            for(int m = 0; m < 16; m++){
                for(int n = 0; n < 16; n++){
                    B[n + j * 16][m + i * 16] = A[m + i * 16][n + j * 16];
                }
            }
        }
    }
    for(int i = 0; i < 67; i++){
        for(int j = 48; j < 61; j++){
            B[j][i] = A[i][j];
        }
    }
    for(int i = 64; i < 67; i++){
        for(int j = 0; j < 48; j++){
            B[j][i] = A[i][j];
        }
    }
}


/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

