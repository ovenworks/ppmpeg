/* 
 * frame.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "frame.h"

#include <libavutil/imgutils.h>

/**
 * ピクセルデータをバッファからコピーする
 * @param frame         フレーム
 * @param width         幅
 * @param height        高さ
 * @param pixel_format  ピクセルフォーマット
 * @param pixels        ピクセルデータ
 * @param pixels_size   ピクセルデータのサイズ
 * @return              コピーできたか
 */
bool frame_copy_pixels_from_buffer(AVFrame* frame, int width, int height, enum AVPixelFormat pixel_format, const uint8_t* pixels, size_t pixels_size) {

    uint8_t* buffers[4];
    int linesizes[4];
    
    int need_size = av_image_fill_arrays(buffers, linesizes, pixels, pixel_format, width, height, 1);
    
    if(need_size < 0 || (size_t)need_size > pixels_size)
        return false;
    
    av_image_copy(frame->data,
                  frame->linesize,
                  (const uint8_t**)buffers,
                  linesizes,
                  pixel_format,
                  width,
                  height);
    
    return true;
}

/**
 * ピクセルデータをバッファにコピーする
 * @param frame     フレーム
 * @param dst       コピー先のバッファ
 * @param dst_size  コピー先のバッファのサイズ
 * @return          コピーしたピクセルデータの先頭アドレス。コピーできなかった場合はNULL
 */
uint8_t* frame_copy_pixels_to_buffer(const AVFrame* frame, uint8_t* dst, size_t dst_size) {
    
    uint8_t* buffers[4];
    int linesizes[4];

    int need_size = av_image_fill_arrays(buffers,
                                         linesizes,
                                         dst,
                                         frame->format,
                                         frame->width,
                                         frame->height,
                                         1);
    
    if(need_size < 0 || (size_t)need_size > dst_size)
        return NULL;
        
    av_image_copy(buffers,
                  linesizes,
                  (const uint8_t**)frame->data,
                  frame->linesize,
                  frame->format,
                  frame->width,
                  frame->height);
    
    return buffers[0];
}

/**
 * バッファからサンプルデータをコピーする
 * @param frame         フレーム
 * @param channels      チャンネル数
 * @param nb_samples    チャンネルあたりのサンプル数
 * @param sample_format サンプルフォーマット
 * @param samples       サンプルデータ
 * @param samples_size  サンプルデータのサイズ
 * @return              コピーできたか
 */
bool frame_copy_samples_from_buffer(AVFrame* frame, int channels, int nb_samples, enum AVSampleFormat sample_format, const uint8_t* samples, size_t samples_size) {
    
    uint8_t* buffers[channels];
    int linesize;
    
    int need_size = av_samples_fill_arrays(buffers, &linesize, samples, channels, nb_samples, sample_format, 1);
    
    if(need_size < 0 || (size_t)need_size > samples_size)
        return false;
    
    av_samples_copy(frame->data,
                    buffers,
                    0,
                    0,
                    nb_samples,
                    channels,
                    sample_format);
    
    return true;
}

/**
 * サンプルデータをバッファにコピーする
 * @param frame     フレーム
 * @param dst       コピー先のバッファ
 * @param dst_size  コピー先のバッファのサイズ
 * @return          コピーしたサンプルデータの先頭アドレス
 */
uint8_t* frame_copy_samples_to_buffer(const AVFrame* frame, uint8_t* dst, size_t dst_size) {
    
    uint8_t* buffers[frame->channels];
    int linesize;

    int need_size = av_samples_fill_arrays(buffers,
                                           &linesize,
                                           dst,
                                           frame->channels,
                                           frame->nb_samples,
                                           frame->format,
                                           1);
    
    if(need_size < 0 || (size_t)need_size > dst_size)
        return NULL;

    av_samples_copy(buffers,
                    frame->extended_data,
                    0,
                    0,
                    frame->nb_samples,
                    frame->channels,
                    frame->format);
    
    return buffers[0];
}
