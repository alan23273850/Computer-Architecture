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

#include "global.h" // cycle
#include "cache.h" // CACHE

int CACHE::find_corresponding_set(int index, int tag)
{
    for (int s=0; s<(this->sizeS); s++)
        if (this->entry[index].set[s].valid && this->entry[index].set[s].tag==tag) {
            this->entry[index].set[s].time = cycle;
            this->hit++;
            return s;
        }
    this->miss++;
    return -1;
}

int CACHE::find_replace_set(int index)
{
    /* find least indexed invalid set first */
    for (int s=0; s<(this->sizeS); s++)
        if (!this->entry[index].set[s].valid)
            return s;

    /* then find least recently used set */
    int min_time = 2147483647, set_record;
    for (int s=0; s<(this->sizeS); s++)
        if (this->entry[index].set[s].time < min_time) {
            min_time = this->entry[index].set[s].time;
            set_record = s;
        }
    return set_record;
}

void CACHE::disable(int target_ppn)
{
    for (int index=0; index<(this->sizeE); index++)
        for (int s=0; s<(this->sizeS); s++) {
            int tag = this->entry[index].set[s].tag;
            int ppn = (tag * this->sizeE + index) * this->sizeB / this->m->sizeP;
            if (ppn == target_ppn)
                this->entry[index].set[s].valid = 0;
        }
}
