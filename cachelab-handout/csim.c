#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

static int hit, miss, evict;

typedef struct cache_line{
    unsigned int valid;
    int tag;
    size_t counter;
} cache_line;


void printHelp(){
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options: \n");
    printf("-h: Optional help ﬂag that prints usage info\n");
    printf("-v: Optional verbose ﬂag that displays trace info\n");
    printf("-s <s>: Number of set index bits (S = 2 s is the number of sets)\n");
    printf("-E <E>: Associativity (number of lines per set)\n");
    printf("-b <b>: Number of block bits (B = 2 b is the block size)\n");
    printf("-t <tracefile>: Name of the valgrind trace to replay\n");
}

void simulateCache(int s, int e, int b, char* tracefile, int verb){
    FILE* pFile;
    pFile = fopen(tracefile, "r");
    unsigned set = 1 << s;

    cache_line cache[set][e];

    for(int i = 0; i < set; i++){
        for(int j = 0; j < e; j++){
            cache[i][j].tag = -1;
            cache[i][j].counter = 0;
            cache[i][j].valid = 0;
        }
    }

    char identifier;
    unsigned address;
    int size;

    while(fscanf(pFile, " %c %x,%d", &identifier, &address, &size) > 0){
        int hitThisline = 0;
        int missThisline = 0;
        int evictThisline = 0;
        if(identifier == 'I')
            continue;
        unsigned line = (address >> b) & (set - 1);
        int tag = (address >> (b + s)) << (b + s);

        int exist = 0;
        for(int i = 0; i < e; i++){
            if(cache[line][i].tag == tag && cache[line][i].valid){
                hit++;
                hitThisline = 1;
                cache[line][i].counter = 0;
                for(int j = 0; j < e; j++){
                    if(j != i && cache[line][j].valid)
                        cache[line][j].counter++;
                }
                exist = 1;
                break;
            }
        }

        if(!exist){
            miss++;
            missThisline++;
            int inserted = 0;
            for(int i = 0; i < e; i++){
                if(!cache[line][i].valid){
                    cache[line][i].valid = 1;
                    cache[line][i].tag = tag;
                    inserted = i + 1;
                    break;
                }
            }
            if(inserted){
                for(int i = 0; i < e; i++){
                    if(i != inserted - 1 && cache[line][i].valid)
                        cache[line][i].counter++;
                }
            }

            if(!inserted){
                size_t max = 0;
                int idx = 0;
                for(int i = 0; i < e; i++){
                    if(cache[line][i].counter >= max && cache[line][i].valid){
                        max = cache[line][i].counter;
                        idx = i;
                    }
                }
                cache[line][idx].tag = tag;
                cache[line][idx].counter = 0;
                evict++;
                evictThisline++;
                for(int i = 0; i < e; i++){
                    if(i != idx && cache[line][i].valid)
                        cache[line][i].counter++;
                }
            }
        }
        if(identifier == 'M'){
            hit++;
            hitThisline++;
        }
        if(verb){
            printf("%c %x,%d ", identifier, address, size);
            if(hitThisline)
                printf("hit ");
            if(missThisline)
                printf("miss ");
            if(evictThisline)
                printf("eviction");
            printf("\n");
        }
    }
    printSummary(hit, miss, evict);
}

int main(int argc, char *argv[]){
    int ar;
    int s, e, b;
    int verb = 0;
    char* tracefile;

    while((ar = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch(ar){
            case 'h':
                printHelp();
                break;
            case 'v':
                verb = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                e = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                tracefile = optarg;
                break;
            case '?':
                printf("Unknown option: %c\n",(char)optopt);
                break;
        }
    }

    simulateCache(s, e, b, tracefile, verb);
    return 0;
}
