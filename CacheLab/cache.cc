#include "cache.h"
#include "def.h"

void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {
    hit = 0;
    time = 0;
    int set;
    unsigned int tag;
    int target = -1;
    int condition = -1;

    // Bypass
    if (!BypassDecision()) {
        PartitionAlgorithm(set, tag);
        condition = ReplaceDecision(set, tag, target);
        switch (condition) {
            case CONFLICT_MISS:
                // evict
                ReplaceAlgorithm();
            case COLOD_MISS:
                // load
                if (PrefetchDecision()) {
                    PrefetchAlgorithm(); // change some load arguments, like 
                }
                
                // read miss, write miss
                
                int lower_hit, lower_time;
                lower_->HandleRequest(addr, bytes, read, content,
                                      lower_hit, lower_time);
                time += latency_.bus_latency + lower_time;
                stats_.access_time += latency_.bus_latency;
                hit = 0;
                break;
            case HIT:
                
                // read hit, write hit
                
                time += latency_.bus_latency + latency_.hit_latency;
                stats_.access_time += time;
                hit = 1;
                break;
        }
    }
}

int Cache::BypassDecision() {
  return FALSE;
}

void Cache::PartitionAlgorithm(int &set, unsigned int & tag) {
}

int Cache::ReplaceDecision(const int set, const unsigned int tag, int & target) {
    Block * cur_set = cache_[set];
    target = -1;
    for(int i = 0; i < config_.associativity; i++){
        if(cur_set[i].valid_bit == true && cur_set[i].tag == tag){
            target = i;
            return HIT;
        }
        if(target == -1 && cur_set[i].valid_bit == false){
            target = i;
        }
    }
    
    if(target != -1) return COLOD_MISS;       // set has slots, target refers to the first slot in this set
    
    return CONFLICT_MISS;
}

int Cache::ReplaceAlgorithm(){
    return -1;
}

void Cache::SetRPP(const int set_id, Block & block){
    
}

int Cache::PrefetchDecision() {
  return FALSE;
}

void Cache::PrefetchAlgorithm() {
}

