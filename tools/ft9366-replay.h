#ifndef FT9366_REPLAY_H
#define FT9366_REPLAY_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
struct ft9366_record { uint8_t operation; uint16_t length; const uint8_t *payload; };
/* Host-side replay metadata only, not FT9366 command framing. */
int ft9366_replay_next(const uint8_t *data, size_t size, size_t *offset, struct ft9366_record *record);
bool ft9366_replay_validate(const uint8_t *data, size_t size);
#endif
