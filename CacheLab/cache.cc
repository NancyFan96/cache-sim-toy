#include "cache.h"
#include "def.h"
#include <time.h>

void Cache::HandleRequest(uint64_t addr, int bytes, int read,
                          char *content, int &hit, int &time) {
    hit = 0;
    time = 0;
    int set;
    unsigned int tag, offset;
    int target = -1;
    int condition = -1;

    // Bypass
    if (!BypassDecision()) {
        PartitionAlgorithm(set, tag, offset);
        condition = ReplaceDecision(set, tag, target);
        hit = (condition == HIT) ? 1:0;
        stats_.access_counter++;
        if (hit == 0) {
            stats_.miss_num++;
        }
        
        switch (condition) {
            case CONFLICT_MISS:
                if((read == false) && (config_.write_allocate == 0)){
                   // write miss, write no allc, lower_->HandleRequest
                    int lower_hit, lower_time;
                    lower_->HandleRequest(addr, bytes, read, content, lower_hit, lower_time);
                    time += latency_.bus_latency + lower_time;
                    stats_.access_time += latency_.bus_latency;
                    //no block evicted, no fetch from lower layer
                    break;
                }
                target = ReplaceAlgorithm(set);
                // evict, mind dirty bit(dirty bit is only valid under writeback policy)
                stats_.replace_num++;
                cache_[set][target].valid_bit = 0;
                if(config_.write_through == 0 && cache_[set][target].dirty_bit == 1){
                    int lower_hit, lower_time;
                    uint64_t victim_address = (tag<<1)||(set<<1);   // !!!!
                    char * victim_content;
                    victim_content[0] = 'v'; victim_content[1] = 0;
                    lower_->HandleRequest(victim_address, config_.block_size, 0, victim_content,
                                          lower_hit, lower_time);
                }

            case COLD_MISS:
                // load
                if (PrefetchDecision()) {
                    PrefetchAlgorithm(); // change some load arguments, no need for Lab3-1
                }
                if(read == true || (read == false && config_.write_allocate == 1)){
                    SetRPP(set, cache_[set][target]);
                }
                
                int lower_hit, lower_time;
                lower_->HandleRequest(addr, bytes, read, content, lower_hit, lower_time);
                time += latency_.bus_latency + lower_time;
                stats_.access_time += latency_.bus_latency;
                break;
                
            case HIT:
                // read hit, write hit(writeback or writethrough)
                // for HIT: 0|1 for back(set dirty bit, only change cache)|through(write both cache and lower level)
                if(read == true){
                    content[0] = 'r';
                    content[1] = 'h';
                    content[2] = 0;
                    cache_[set][target].valid_bit = 1;
                    cache_[set][target].tag = tag;
                    SetRPP(set, cache_[set][target]);
                 }
                else{
                    // write back, write through use a buffer, so no latency need to plus
                    cache_[set][target].valid_bit = 1;
                    cache_[set][target].dirty_bit = 1;
                    cache_[set][target].tag = tag;
                    SetRPP(set, cache_[set][target]);
                }
                
                time += latency_.bus_latency + latency_.hit_latency;
                stats_.access_time += time;
                break;
        }
    }
}

int Cache::BypassDecision() {
  return FALSE;
}

void Cache::PartitionAlgorithm(int &set, unsigned int & tag, unsigned int & offset) {
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
    
    if(target != -1) return COLD_MISS;       // set has slots, target refers to the first slot in this set
    
    return CONFLICT_MISS;
}

int Cache::ReplaceAlgorithm(const int set){
    int victim = -1;
    uint64_t min = UINT64_MAX;
    for(int i = 0; i < config_.associativity; i++){
        if(cache_[set][i].RPP_tag < min){
            min = cache_[set][i].RPP_tag;
            victim = i;
        }
    }
#ifdef DEBUG
    for(int j = 0; j < config_.associativity; j++){
        printf("%llu\t", cache_[set][j].RPP_tag);
    }
    printf("\n");
    printf("victim = %d\n", victim);
#endif
    
    return victim;
}

void Cache::SetRPP(const int set_id, Block & block){
    block.RPP_tag = clock();
}

int Cache::PrefetchDecision() {
  return FALSE;
}

void Cache::PrefetchAlgorithm() {
}

