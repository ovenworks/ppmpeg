/* 
 * buffer.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int buffer_alloc(uint8_t** bufp, int cur_size, int need_size);
void buffer_freep(uint8_t** bufp);

#ifdef __cplusplus
}
#endif

#endif /* BUFFER_H */

