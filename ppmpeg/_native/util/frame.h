/* 
 * frame.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef FRAME_H
#define FRAME_H

#include <stdbool.h>
#include <stdint.h>
#include <libavutil/frame.h>

#ifdef __cplusplus
extern "C" {
#endif

bool frame_copy_pixels_from_buffer(AVFrame* frame, int width, int height, enum AVPixelFormat pixel_format, const uint8_t* pixels, size_t pixels_size);
uint8_t* frame_copy_pixels_to_buffer(const AVFrame* frame, uint8_t* dst, size_t dst_size);
bool frame_copy_samples_from_buffer(AVFrame* frame, int channels, int nb_samples, enum AVSampleFormat sample_format, const uint8_t* samples, size_t samples_size);
uint8_t* frame_copy_samples_to_buffer(const AVFrame* frame, uint8_t* dst, size_t dst_size);

#ifdef __cplusplus
}
#endif

#endif /* FRAME_H */

