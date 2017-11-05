/* 
 * codec.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef CODEC_H
#define CODEC_H

#include <stdbool.h>
#include <libavcodec/avcodec.h>

#ifdef __cplusplus
extern "C" {
#endif

bool codec_is_support_pixel_format(AVCodec* codec, enum AVPixelFormat format);
bool codec_is_support_sample_format(AVCodec* codec, enum AVSampleFormat format);    

#ifdef __cplusplus
}
#endif

#endif /* BUFFER_H */

