/* 
 * codec.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "codec.h"

/**
 * コーデックが指定されたピクセルフォーマットをサポートしているかを返す
 * @param codec     コーデック
 * @param format    ピクセルフォーマット
 * @return          サポートしているか
 */
bool codec_is_support_pixel_format(AVCodec* codec, enum AVPixelFormat format) {
    
    if(codec->pix_fmts == NULL)
        return false;
    
    for(const enum AVPixelFormat* fmt = codec->pix_fmts; *fmt != -1; ++fmt) {
        if(*fmt == format)
            return true;
    }
    
    return false;
}

/**
 * コーデックが指定されたサンプルフォーマットをサポートしているかを返す
 * @param codec     コーデック
 * @param format    サンプルフォーマット
 * @return          サポートしているか
 */
bool codec_is_support_sample_format(AVCodec* codec, enum AVSampleFormat format) {
    
    if(codec->sample_fmts == NULL)
        return false;
    
    for(const enum AVSampleFormat* fmt = codec->sample_fmts; *fmt != -1; ++fmt) {
        if(*fmt == format)
            return true;
    }
    
    return false;
}
