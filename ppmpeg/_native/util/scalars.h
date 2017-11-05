/* 
 * scalars.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef SCALARS_H
#define SCALARS_H

#include <libswscale/swscale.h>

#ifdef __cplusplus
extern "C" {
#endif

void scalars_init();
struct SwsContext* scalars_get(int src_width, int src_height, enum AVPixelFormat src_format,
                                int dst_width, int dst_height, enum AVPixelFormat dst_format,
                                int flags);

int scalars_safe_scale(const uint8_t* src,
                         int src_size,
                         struct SwsContext* context,
                         const uint8_t* src_slice[],
                         const int src_stride[],
                         int src_slice_y,
                         int src_slice_h,
                         uint8_t* dst[],
                         const int dst_stride[]);

void scalars_dispose();

#ifdef __cplusplus
}
#endif

#endif /* SCALARS_H */

