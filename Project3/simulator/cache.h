/*
//
//                       _oo0oo_
//                      o8888888o
//                      88" . "88
//                      (| -_- |)
//                      0\  =  /0
//                    ___/`---'\___
//                  .' \\|     |// '.
//                 / \\|||  :  |||// \
//                / _||||| -:- |||||- \
//               |   | \\\  -  /// |   |
//               | \_|  ''\---/''  |_/ |
//               \  .-\__  '-'  ___/-. /
//             ___'. .'  /--.--\  `. .'___
//          ."" '<  `.___\_<|>_/___.' >' "".
//         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
//         \  \ `_.   \_ __\ /__ _/   .-` /  /
//     =====`-.____`.___ \_____/___.-`___.-'=====
//                       `=---='
//
//
//     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//               佛祖保佑         永無BUG
//
//
//
*/

#ifndef CACHE_H
#define CACHE_H

typedef struct memory MEMORY;

typedef struct {
    int hit, miss;
    int sizeE, sizeS, sizeB;
    struct {
        struct {
            int valid, tag, time;
            unsigned char byte[1024];
        } set[256];
    } entry[256];
    MEMORY *m;

    int find_corresponding_set(int index, int tag);
    int find_replace_set(int index);
    void disable(int target_ppn);
} CACHE;

#endif // CACHE_H
