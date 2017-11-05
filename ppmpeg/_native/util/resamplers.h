/* 
 * resamplers.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef RESAMPLERS_H
#define RESAMPLERS_H

#include <libswresample/swresample.h>

#ifdef __cplusplus
extern "C" {
#endif

void resamplers_init();
struct SwrContext* resamplers_get(int src_channel_layout, int src_rate, enum AVSampleFormat src_format,
                                    int dst_channel_layout, int dst_rate, enum AVSampleFormat dst_format);
void resamplers_dispose();

#ifdef __cplusplus
}
#endif

#endif /* RESAMPLERS_H */

