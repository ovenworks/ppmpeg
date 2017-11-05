/* 
 * buffer.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "buffer.h"

#include <libavutil/avutil.h>

#define SAFE_AREA_SIZE  64  ///< セーフ領域のサイズ

/**
 * バッファを確保する
 * @param bufp      現在のバッファアドレスへのポインタ
 * @param cur_size  現在のサイズ
 * @param need_size 必要なサイズ。cur_size 以下なら再確保は行わない
 * @return          バッファのサイズ
 */
int buffer_alloc(uint8_t** bufp, int cur_size, int need_size) {

    if(*bufp == NULL || cur_size <= 0) {
        *bufp = av_malloc(need_size + SAFE_AREA_SIZE);
        return need_size;
    }
    else if(cur_size >= need_size) {
        return cur_size;
    }
    else {
        av_freep(bufp);
        *bufp = av_malloc(need_size + SAFE_AREA_SIZE);

        return need_size;
    }
}

/**
 * バッファを解放する
 * @param bufp  バッファアドレスへのポインタ
 */
void buffer_freep(uint8_t** bufp) {
    av_freep(bufp);
}