/* 
 * resamplers.c
 * @author: ppmpeg@ovenworks.jp
 */

#include "resamplers.h"
#include "array.h"

/** アイテム構造体 */
typedef struct Item {
    
    int src_channel_layout;         ///< 変換元のチャンネルレイアウト
    int src_rate;                   ///< 変換元のサンプルレート
    enum AVSampleFormat src_format; ///< 変換元のサンプルフォーマット
    
    int dst_channel_layout;         ///< 変換先のチャンネルレイアウト
    int dst_rate;                   ///< 変換先のサンプルレート
    enum AVSampleFormat dst_format; ///< 変換先のサンプルフォーマット
    
    struct SwrContext* context;     ///< コンテキスト

} Item;

/**
 * リサンプラーマネージャー構造体
 * @author ppmpeg@ovenworks.jp
 */
typedef struct Resamplers {
    
    Item items[1024];   ///< アイテム
    int item_count;     ///< アイテム数
    
} Resamplers;

static Resamplers instance; ///< インスタンス

/** 初期化する */
void resamplers_init() {

    memset(instance.items, 0, sizeof instance.items);
    instance.item_count = 0;
}

/**
 * リサンプラーコンテキストを返す
 * @param src_channel_layout    変換元のチャンネルレイアウト
 * @param src_rate              変換元のサンプルレート
 * @param src_format            変換元のサンプルフォーマット
 * @param dst_channel_layout    変換先のチャンネルレイアウト
 * @param dst_rate              変換先のサンプルレート
 * @param dst_format            変換先のサンプルフォーマット
 * @return                      リサンプラーコンテキスト。エラーが発生した場合はNULL
 */
struct SwrContext* resamplers_get(int src_channel_layout, int src_rate, enum AVSampleFormat src_format,
                                    int dst_channel_layout, int dst_rate, enum AVSampleFormat dst_format) {

    for(int i = 0; i < instance.item_count; ++i) {
        Item* item = &instance.items[i];
        
        if(item->src_channel_layout == src_channel_layout && item->src_rate == src_rate && item->src_format == src_format &&
           item->dst_channel_layout == dst_channel_layout && item->dst_rate == dst_rate && item->dst_format == dst_format) {
            return item->context;
        }
    }

    //@todo 可変長にする
    if(instance.item_count + 1 >= ARRAY_LENGTH_I(instance.items))
        return NULL;
    
    Item item;
    item.src_channel_layout = src_channel_layout;
    item.src_rate = src_rate;
    item.src_format = src_format;
    item.dst_channel_layout = dst_channel_layout;
    item.dst_rate = dst_rate;
    item.dst_format = dst_format;

    if((item.context = swr_alloc_set_opts(NULL,
                                          dst_channel_layout, dst_format, dst_rate,
                                          src_channel_layout, src_format, src_rate,
                                          0, NULL)) == NULL) {
        return NULL;
    }
        
    if(swr_init(item.context) < 0)
        return NULL;
    
    instance.items[instance.item_count++] = item;
        
    return item.context;
}

/** 破棄する */
void resamplers_dispose() {
    
    for(int i = 0; i < instance.item_count; ++i) {
        swr_free(&instance.items[i].context);
    }

    memset(instance.items, 0, sizeof(instance.items));
    instance.item_count = 0;
}