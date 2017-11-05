/* 
 * scalars.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "scalars.h"
#include "array.h"
#include "buffer.h"

/** アイテム構造体 */
typedef struct Item {
    
    int src_width;                  ///< 変換元の幅
    int src_height;                 ///< 変換元の高さ
    enum AVPixelFormat src_format;  ///< 変換元のピクセルフォーマット
    
    int dst_width;                  ///< 変換先の幅
    int dst_height;                 ///< 変換先の高さ
    enum AVPixelFormat dst_format;  ///< 変換先のピクセルフォーマット
    
    int flags;                      ///< フラグ
    
    struct SwsContext* context;     ///< コンテキスト

} Item;

/**
 * スケーラーマネージャー構造体
 * @author ppmpeg@ovenworks.jp
 */
typedef struct Scalars {
    
    Item items[1024];       ///< アイテム
    int item_count;         ///< アイテム数
    uint8_t* safe_buffer;   ///< sws_scaleのセーフ処理用バッファ
    int safe_buffer_size;   ///< sws_scaleのセーフ処理用バッファのサイズ
    
} Scalars;

#define SWS_SCALE_SAFE_SIZE 64  ///< @bugfix sws_scale のオーバーランバグ回避用セーフ領域のサイズ

static Scalars instance;        ///< インスタンス

/** 初期化する */
void scalars_init() {

    memset(instance.items, 0, sizeof instance.items);
    instance.item_count = 0;
    instance.safe_buffer = NULL;
    instance.safe_buffer_size = 0;
}

/**
 * スケーラーコンテキストを返す
 * @param src_width     変換元の幅
 * @param src_height    変換元の高さ
 * @param src_format    変換元のピクセルフォーマット
 * @param dst_width     変換先の幅
 * @param dst_height    変換先の高さ
 * @param dst_format    変換先のピクセルフォーマット
 * @param flags         フラグ
 * @return              スケーラーコンテキスト。エラーが発生した場合はNULL
 */
struct SwsContext* scalars_get(int src_width, int src_height, enum AVPixelFormat src_format,
                                int dst_width, int dst_height, enum AVPixelFormat dst_format,
                                int flags) {
        
    for(int i = 0; i < instance.item_count; ++i) {
        Item* item = &instance.items[i];
        
        if(item->src_width == src_width && item->src_height == src_height && item->src_format == src_format &&
           item->dst_width == dst_width && item->dst_height == dst_height && item->dst_format == dst_format &&
           item->flags == flags) {
            return item->context;
        }
    }

    //@todo 可変長にする
    if(instance.item_count + 1 >= ARRAY_LENGTH_I(instance.items))
        return NULL;
    
    Item item;
    item.src_width = src_width;
    item.src_height = src_height;
    item.src_format = src_format;
    item.dst_width = dst_width;
    item.dst_height = dst_height;
    item.dst_format = dst_format;
    item.flags = flags;

    if((item.context = sws_getContext(src_width, src_height, src_format,
                                      dst_width, dst_height, dst_format,
                                      flags,
                                      NULL, NULL, NULL)) == NULL) {
        return NULL;
    }
    
    instance.items[instance.item_count++] = item;
        
    return item.context;
}

/**
 * @bugfix sws_scale にはフォーマットと環境（例: yuv420p/MacOS）によって変換元バッファをオーバーラン読み込みするバグがある。
 *          これをセーフ領域を持ったバッファを用いて変換することで回避するためのラップ関数。
 * @param src       変換元のバッファ
 * @param src_size  変換元のバッファのサイズ
 * @see sws_scale()
 */
int scalars_safe_scale(const uint8_t* src,
                         int src_size,
                         struct SwsContext* context,
                         const uint8_t* src_slice[],
                         const int src_stride[],
                         int src_slice_y,
                         int src_slice_h,
                         uint8_t* dst[],
                         const int dst_stride[]) {

    instance.safe_buffer_size = buffer_alloc(&instance.safe_buffer, instance.safe_buffer_size, src_size + SWS_SCALE_SAFE_SIZE);
    memcpy(instance.safe_buffer, src, src_size);

    const uint8_t* slice[4];
    slice[0] = instance.safe_buffer + (src_slice[0] - src);
    slice[1] = instance.safe_buffer + (src_slice[1] - src);
    slice[2] = instance.safe_buffer + (src_slice[2] - src);
    slice[3] = instance.safe_buffer + (src_slice[3] - src);

    return sws_scale(context, slice, src_stride, src_slice_y, src_slice_h, dst, dst_stride);
}

/** 破棄する */
void scalars_dispose() {

    buffer_freep(&instance.safe_buffer);
    
    for(int i = 0; i < instance.item_count; ++i) {
        sws_freeContext(instance.items[i].context);
    }

    memset(instance.items, 0, sizeof(instance.items));
    instance.item_count = 0;
}