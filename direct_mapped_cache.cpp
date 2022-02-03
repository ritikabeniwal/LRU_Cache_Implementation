#include <cstdio>
#include <cmath>
#include <cstdint>
#include <sys/stat.h>

#define K 1024
#define log2(_f) (log(_f)/log(2.0))
struct cache_content
{
    bool v;
    uint32_t tag;
    // no need to save data in this simulator   
    // unsigned int data[16]; 
};

struct cache_monitor
{
    uint32_t u4HitCnt;
    uint32_t u4MissCnt;
};

void simulate(int cache_size, int block_size, const char* FileName)
{   
    struct stat st = {0};
    FILE *fpInput, *fpOutput, *fpDump;
    char dumpFieName[256];
    uint32_t tag, index, mem_addr;
    uint32_t offset_bits, index_bits, line;
    uint32_t u4TotalHitCnt = 0, u4TotalMissCnt = 0;
    struct cache_content *cache;
    struct cache_monitor *monitor;

    /* Init input/output */ 
    if (stat("output", &st) == -1) {
        mkdir("output", 0755);
    }
    sprintf(dumpFieName, "output/Dump_direct_%s_Cache%dK_Block%d.txt", FileName, cache_size/K, block_size);
    fpInput = fopen(FileName, "r");     // read file
    fpOutput = fopen("output/_Result_direct_mapped_cache.txt", "a+"); // result file
    fpDump = fopen(dumpFieName, "w+");  // dump file
    if(fpInput == NULL || fpOutput == NULL || fpDump == NULL) {
        printf("Open file failed!, %d, %d, %d\n", fpInput, fpOutput, fpDump);
        return;
    }

    // Edit the equation !!!!!!!!!!!!!!!!!!!!!!!!!!!
    /* Set cache configuration */ 
    line =  cache_size/block_size;                    // what is equation for  the cache block (aka cache line)?
    offset_bits =  log2(block_size);          // what is equation for  the offset bits 
    index_bits  =  log2(line);            // what is equation for  index bits

    /* Init cache*/
    cache = new struct cache_content[line];
    for(int i = 0; i < line; i++)
        cache[i].v = false;

    /* Init cache monitor */
    monitor = new struct cache_monitor[line];
    for(int i=0; i<line; i++) {
        monitor[i].u4HitCnt = 0; monitor[i].u4MissCnt = 0;
    }

    
    /* Start cache simulation */
    while(fscanf(fpInput, "%x", &mem_addr) != EOF)
    {
        index = (mem_addr >> offset_bits) & (line - 1);
        tag   = mem_addr >> (index_bits + offset_bits);

        if(cache[index].v && cache[index].tag == tag)
        {
            /* Cache Hit, Do nothing */

            /* Add to statistic result. */
            if(monitor[index].u4HitCnt == 0xFFFFFFFF) {
                fprintf(stderr, "Hit Counter Overflow @ line %d, %s\n", index, FileName);
                return;
            } else
                monitor[index].u4HitCnt++;
        }
        else
        {
            /* Cache Miss, Set the corresponding cache line to valid. */
            cache[index].v = true;
            cache[index].tag = tag;

            /* Add to statistic result. */
            if(monitor[index].u4MissCnt == 0xFFFFFFFF) {
                fprintf(stderr, "Miss Counter Overflow @ line %d, %s\n", index, FileName);
                return;
            } else
                monitor[index].u4MissCnt++;
        }
    }
    
    /* Write result to output file */
    for(int i=0; i<line; i++) {
        uint32_t u4HitCnt  = monitor[i].u4HitCnt;
        uint32_t u4MissCnt = monitor[i].u4MissCnt;

        u4TotalHitCnt  += u4HitCnt;
        u4TotalMissCnt += u4MissCnt;

        fprintf(fpDump, "Line[%d], HitCnt=%d, MissCnt=%d, MissRate=%f\n", 
            i, u4HitCnt, u4MissCnt, ((double)u4MissCnt)/(u4HitCnt+u4MissCnt));
    }
    
    

    fprintf(fpOutput, "%s, CacheSize=%dK, BlcokSize=%d\n", FileName, cache_size/K, block_size);
    fprintf(fpOutput, " - Total Miss Rate = %f%% (%d/%d)\n", 
            (((double)u4TotalMissCnt)/(u4TotalHitCnt+u4TotalMissCnt))*100.0, 
            u4TotalMissCnt, u4TotalHitCnt+u4TotalMissCnt);
    fprintf(fpOutput, "============================================\n\n");
    fclose(fpInput);
    fclose(fpOutput);
    fclose(fpDump);

    delete [] cache;
    delete [] monitor;
}

int main()
{
    //For example : simulate(Cashe Size: 4KByte, Block sise: 16Byte, "ICACHE.txt");
    int i, j, k;
    int testCacheSize[] = {4, 16, 64, 256};
    int testBlockSize[] = {16, 32, 64, 128, 256};
    const char *testFile[] = {"ICACHE.txt", "DCACHE.txt"};

    // remove "output dir if existed"
    system("rm -rf output");

    for(i=0; i< (sizeof(testFile)/sizeof(char*)); i++)
        for(j=0; j<(sizeof(testCacheSize)/sizeof(int)); j++)
            for(k=0; k<(sizeof(testBlockSize)/sizeof(int)); k++)
                simulate(testCacheSize[j]*K, testBlockSize[k], testFile[i]);
}
