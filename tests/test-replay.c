#include "ft9366-replay.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    const uint8_t good[] = {0,0,2,0x12,0x34,1,0x28,0,2,0,10};
    struct ft9366_record r;
    size_t offset = 0;
    assert(ft9366_replay_validate(good, sizeof good));
    assert(ft9366_replay_next(good,sizeof good,&offset,&r)==1);
    assert(r.operation==0 && r.length==2 && r.payload[0]==0x12);
    assert(ft9366_replay_next(good,sizeof good,&offset,&r)==1);
    assert(r.operation==1 && r.length==10240 && r.payload==NULL);
    assert(ft9366_replay_next(good,sizeof good,&offset,&r)==1);
    assert(r.operation==2 && r.length==10);
    assert(ft9366_replay_next(good,sizeof good,&offset,&r)==0);
    const uint8_t bad[][6] = {
        {3,0,1}, {0,0,0}, {1,0,0}, {1,0x28,1}, {2,4,0}, {0,0,4,1,2,3}
    };
    const size_t sizes[] = {3,3,3,3,3,6};
    for(size_t i=0;i<sizeof sizes/sizeof sizes[0];i++)
        assert(!ft9366_replay_validate(bad[i],sizes[i]));
    assert(!ft9366_replay_validate(good,1));
    assert(!ft9366_replay_validate(NULL,0));
    offset=sizeof good+1;
    assert(ft9366_replay_next(good,sizeof good,&offset,&r)==-1);
    puts("reference replay parser: bounds checks passed");
}
