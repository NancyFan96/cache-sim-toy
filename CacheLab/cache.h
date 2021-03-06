#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include "storage.h"
#include "def.h"

#define HIT 1
#define MISS 0

#define ONES(x,y)       (uint64_t) ((((uint64_t)1<<x)-1)+((uint64_t)1<<x) -(((uint64_t)1<<y)-1))


#define GETSET(addr, offset_tag, offset_set)    ((addr & ONES((offset_tag-1),offset_set)) >> offset_set)
#define GETTAG(addr, offset_tag)                ((addr & ONES(63,offset_tag)) >> offset_tag)
#define GETOFFSET(addr, offset_set)             (addr & ONES((offset_set-1), 0))

typedef struct CacheConfig_ {
    int size;
    int associativity;
    int set_num;              // Number of cache sets
    int block_size;
    int write_through;        // for HIT: 0|1 for back(set dirty bit, only change cache)|through(write both cache and lower level)
    int write_allocate;       // for MISS: 0|1 for no-alc(write into lower level, no loading)|alc(load block into cache and rewrite)
} CacheConfig;

#ifdef RW
typedef struct Block_{
    bool valid_bit;
    bool dirty_bit;
    unsigned int tag;
    uint64_t RPP_tag;            // Replace Policy tag
    char * block_content;
}Block;
#endif

#ifndef RW
typedef struct Block_{
    bool valid_bit;
    bool dirty_bit;
    unsigned int tag;
    uint64_t RPP_tag;            // Replace Policy tag
}Block;
#endif

class Cache: public Storage {
public:
    Cache();
    ~Cache(){};
    
    void cachehelp();
    // Sets & Gets
    void SetConfig(CacheConfig cc);
    void GetConfig(CacheConfig cc);
    void SetLower(Storage *ll) { lower_ = ll; }
    // Main access process
    void HandleRequest(uint64_t addr, int bytes, int read,
                       char *content, int &hit, int &time);
private:
    int BypassDecision();                                                    // Now, always false
    void PartitionAlgorithm();                                               // What is it for ? Diminish now
    int ReplaceDecision(const int set,const unsigned int tag, int &target);  // return HIT(set target),
    // COLD_MISS(set target) or CONFLICT_MISS
    int ReplaceAlgorithm(const int set);                                     // return victim's way number
    void SetRPP(const int set_id, Block & block);                            // set Replace Policy tag
    int PrefetchDecision();                                                  // Diminish in Lab3-1
    void PrefetchAlgorithm();                                                // Diminish in Lab3-1
    
    CacheConfig config_;
    Storage *lower_;
    Block ** cache_;
    DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_

