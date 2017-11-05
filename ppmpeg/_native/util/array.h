/* 
 * array.h
 * @author: ppmpeg@ovenworks.jp
 */

#ifndef ARRAY_H
#define ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 配列の長さを返す
 * @param array 配列
 * @return      長さ
 */
#define ARRAY_LENGTH(array) \
    (sizeof(array) / sizeof(array[0]))

/**
 * 配列の長さを int 型で返す
 * @param array 配列
 * @return      長さ
 */
#define ARRAY_LENGTH_I(array) \
    ((int)(sizeof(array) / sizeof(array[0])))

#ifdef __cplusplus
}
#endif

#endif /* ARRAY_H */

