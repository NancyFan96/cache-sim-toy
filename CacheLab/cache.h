#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include "storage.h"

#define HIT 0
#define COLD_MISS 1
#define CONFLICT_MISS 2


typedef struct CacheConfig_ {
    int size;
    int associativity;
    int set_num;              // Number of cache sets
    int write_through;        // for HIT: 0|1 for back(set dirty bit, only change cache)|through(write both cache and lower level)
    int write_allocate;       // for MISS: 0|1 for no-alc(write into lower level, no loading)|alc(load block into cache and rewrite)
    int *RPP_front;            // Replace Policy tag, LRU, RPP_front[SET_NUM],需初始化为0
    int *RPP_rear;             // Replace Policy tag, LRU, 同上
} CacheConfig;


typedef struct Block_{
    bool valid_bit;
    bool dirty_bit;
    unsigned int tag;
    int RPP_tag;            // Replace Policy tag
}Block;


class Cache: public Storage {
    public:
    Cache() {}                // luyao, 部分伪码见下，还要初始化config，lower（lower暂时不用在意）
    ~Cache() {}               // luyao
                              // 加上main的输入输出，参数设置，参考https://github.com/HolyLow/cache_simulator

    // Sets & Gets
    void SetConfig(CacheConfig cc);
    void GetConfig(CacheConfig cc);
    void SetLower(Storage *ll) { lower_ = ll; }
    // Main access process
    void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);       // yangkejing, 注意write_through, write_allocate

    private:
    // Bypassing
    int BypassDecision();                                       // Now, always false
    // Partitioning
    void PartitionAlgorithm(int &set, unsigned int & tag);      // luyao, decode
    // Replacement
    int ReplaceDecision(const int set,const unsigned int tag, int &target);   // fannaijia, return HIT, COLD_MISS or CONFLICT_MISS
    int ReplaceAlgorithm();                                     // fannaijia, return victim's way number
    void SetRPP(const int set_id, Block & block);               // fannaijia,每个新进入Cache的block都需要调用这个函数
    // Prefetching
    int PrefetchDecision();       // Diminish now
    void PrefetchAlgorithm();     // Diminish now

    CacheConfig config_;
    Storage *lower_;
    Block ** cache_;
    DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_ 


/*
 Block ** cache_LX = (Block **)malloc(sizeof(Block)*SET_NUM)
 for i = 0 - Associativity
    cache_LX[i] = (Block *)malloc(sizeof(Block)*Ass_NUM)
 memset(cache_LX, 0, sizeof(Block)*SET_NUM*ASS_NUM)
*/
