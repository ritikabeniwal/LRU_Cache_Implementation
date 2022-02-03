#include <cstdio>
#include <cmath>
#include <cstdint>
#include <sys/stat.h>

#define K 1024
#define log2(_f) (log(_f)/log(2.0))

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
    uint32_t timestamp;
    // no need to save data in this simulator   
    // unsigned int data[16]; 
};

struct cache_monitor
{
    uint32_t u4HitCnt;
    uint32_t u4MissCnt;
};

void simulate(int cache_size, int block_size, int assoc_way, const char* FileName)
{   
    struct stat st = {0};
    FILE *fpInput, *fpOutput, *fpDump;
    char dumpFieName[256];
    uint32_t tag, index, mem_addr;
    uint32_t offset_bits, index_bits, line;
    uint32_t u4TotalHitCnt = 0, u4TotalMissCnt = 0;
    uint32_t u4Timestampe = 0;
    uint32_t line_lru;
    uint32_t i;
    bool to_be_replaced = false;
    struct cache_content *cache;
    struct cache_monitor *monitor;

    /* Init input/output */
    if (stat("output", &st) == -1) {
        mkdir("output", 0755);
    }
    sprintf(dumpFieName, "output/Dump_%dway_%s_Cache%dK_Block%d.txt", 
                            assoc_way, FileName, cache_size/K, block_size);
    fpInput = fopen(FileName, "r");     // read file
    fpOutput = fopen("output/_Result_nway_assoc_cache.txt", "a+"); // result file
    fpDump = fopen(dumpFieName, "w+");  // dump file
    if(fpInput == NULL || fpOutput == NULL || fpDump == NULL) {
        printf("Open file file!, %d, %d, %d\n", fpInput, fpOutput, fpDump);
        return;
    }

    /* Set cache configuration */
    line = cache_size/block_size; //block_number
    offset_bits = log2(block_size); //index in block
    index_bits  = log2(line/assoc_way); 

    /* Init cache*/
        cache = new struct cache_content[line];
        for(i = 0; i < line; i++) {
            cache[i].v = false;
            cache[i].timestamp = 0;
        }

    /* Init cache monitor */
    monitor = new struct cache_monitor[line];
    for(i=0; i<line; i++) {
        monitor[i].u4HitCnt = 0; 
        monitor[i].u4MissCnt = 0;
    }

    /* Start cache simulation */
    while(fscanf(fpInput, "%x", &mem_addr) != EOF)
    {
        u4Timestampe++;

        index = (mem_addr >> offset_bits) & (line/assoc_way - 1);
        tag   = mem_addr >> (index_bits + offset_bits);

        /* Check if index is in the cache block consider set concept */
        /* 
            if is two sets, you need to exam two locaiton 
            So this is why the for loop is here
        */
    
        
        for(i = assoc_way * index ;i < index * assoc_way+assoc_way;i++)
        {
            if(cache[i].v && cache[i].tag == tag){
               to_be_replaced = false;

                /*  if Cache hit, update timestamp */
             cache[i].timestamp = u4Timestampe;
             line_lru=-1;

                /* Add to statistic result. */
                if(monitor[i].u4HitCnt == 0xFFFFFFFF) {
                    fprintf(stderr, "Hit Counter Overflow @ line %d, %s\n", i, FileName);
                    return;
                } else
                    monitor[i].u4HitCnt++;
            
                break;
            }
            else {
                
               
               if (cache[i].v==false)
               {
                  
                  line_lru = i;
                  to_be_replaced = true;
                  break;
                  
               }
               
               else
               {
                   if (line_lru == -1){
                       line_lru=i;
                   }
                  else{
                  if (cache[line_lru].timestamp > cache[i].timestamp )
                  
                  {
                     
                     line_lru = i;
                  }
                 
                  }
                  
                  
               }
     to_be_replaced = true;
            }   
        }


                
        /* Else if cache not hit */  
        /* need to find the next replace location */
        // (1) keep compare the timestamp while traverse inside the set
        // (2) find which location in set is going to be replace by compare timestamp
        // hint: besides timestamp, if you see valid tag is not set, this is the location you want to replace

        /* LRU Algorithm:
        *   1st priority: the invalid cahce line(cache block)
        *   2st priority: the least used cahce line (cache block)
        */
    
        /* Put the data into cache if is cache miss */
        // (0) if is a cache miss
        // (1) use the location you find above
        // (2) set cache[location].v , cache[location].tag, cache[location].timestamp
        if( to_be_replaced== true ) {
            /* Cache Miss, used line_lru to store data */
            cache[line_lru].v = true;
            cache[line_lru].tag = tag;
            cache[line_lru].timestamp = u4Timestampe++;

            /* Add to statistic result. */
            if(monitor[line_lru].u4MissCnt == 0xFFFFFFFF) {
                fprintf(stderr, "Miss Counter Overflow @ line %d, %s\n", line_lru, FileName);
                return;
            } else
                monitor[line_lru].u4MissCnt++;
        }
    }

    /* Write result to output file */ // No Need to Edit
    for(int i=0; i<line; i++) {
        uint32_t u4HitCnt  = monitor[i].u4HitCnt;
        uint32_t u4MissCnt = monitor[i].u4MissCnt;

        u4TotalHitCnt  += u4HitCnt;
        u4TotalMissCnt += u4MissCnt;

        if(i % assoc_way == 0)
            fprintf(fpDump, "\n === index %d ===\n", i/assoc_way);
        fprintf(fpDump, "Line[%d], HitCnt=%d, MissCnt=%d, MissRate=%f\n", 
            i, u4HitCnt, u4MissCnt, ((double)u4MissCnt)/(u4HitCnt+u4MissCnt));
    }
    fprintf(fpOutput, "%s, CacheSize=%dK, BlcokSize=%d\n", FileName, cache_size/K, block_size);
    fprintf(fpOutput, " - %d way associative cache\n", assoc_way);
    fprintf(fpOutput, " - TotalBits: %d ( line*(tag+valid)=%d*(%d+1) )\n", 
            line * (32 - (index_bits + offset_bits) + 1), 
            line, 32 - (index_bits + offset_bits));
    fprintf(fpOutput, " - Total Miss Rate = %f%% (%d/%d)\n", 
            (((double)u4TotalMissCnt)/(u4TotalHitCnt+u4TotalMissCnt))*100.0, 
            u4TotalMissCnt, u4TotalHitCnt+u4TotalMissCnt);
    fprintf(fpOutput, "============================================\n\n");

    /* De-Init */
    fclose(fpInput);
    fclose(fpOutput);
    fclose(fpDump);

    delete [] cache;
    delete [] monitor;
}

int main()
{
#if DBG
    simulate(1*K, 64, 4, "RADIX.txt");
    printf("start cache simulator");
#else
    int i, j, k, l;
    int testCacheSize[] = {1, 2, 4, 8, 16, 32};
    int testBlockSize[] = {64};
    int testAssocWay[] = {1, 2, 4, 8};
    const char *testFile[] = {"LU.txt", "RADIX.txt"};

    // remove "output dir if existed"
    system("rm -rf output");
    printf("start cache simulator");
    for(i=0; i< (sizeof(testFile)/sizeof(char*)); i++)
        for(j=0; j<(sizeof(testCacheSize)/sizeof(int)); j++)
            for(k=0; k<(sizeof(testBlockSize)/sizeof(int)); k++)
                for(l=0; l<(sizeof(testAssocWay)/sizeof(int)); l++)
                    simulate(testCacheSize[j]*K, testBlockSize[k], testAssocWay[l], testFile[i]);
#endif
}





