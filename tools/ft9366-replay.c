#include "ft9366-replay.h"
int ft9366_replay_next(const uint8_t *data, size_t size, size_t *offset, struct ft9366_record *record)
{
    if (!data || !offset || !record || *offset > size) return -1;
    if (*offset == size) return 0;
    size_t position = *offset;
    if (size-position < 3) return -1;
    uint8_t operation = data[position];
    uint16_t length = (uint16_t)((unsigned)data[position+1]*256u+data[position+2]);
    position += 3;
    if (operation > 2 || length == 0 ||
        (operation == 0 && (length > 64 || length > size-position)) ||
        (operation == 1 && length > 10240) || (operation == 2 && length > 1000)) return -1;
    record->operation=operation; record->length=length;
    record->payload=operation==0 ? data+position : NULL;
    if(operation==0) position+=length;
    *offset=position;
    return 1;
}
bool ft9366_replay_validate(const uint8_t *data, size_t size)
{
    if (!data || !size || size > 65536) return false;
    size_t offset=0, records=0, delays=0;
    struct ft9366_record record;
    int result;
    while ((result=ft9366_replay_next(data,size,&offset,&record))==1) {
        if(++records>1024) return false;
        if(record.operation==2) delays+=record.length;
        if(delays>10000) return false;
    }
    return result==0;
}
