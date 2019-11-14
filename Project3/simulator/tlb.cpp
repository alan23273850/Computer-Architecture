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

#include "tlb.h" // TLB
#include "global.h" // cycle

int TLB::find_ppn(int vpn)
{
    for (int e=0; e<(this->sizeE); e++)
        if (this->entry[e].valid && this->entry[e].tag==vpn) {
            this->entry[e].time = cycle;
            this->hit++;
            return this->entry[e].ppn;
        }
    this->miss++;
    return -1;
}

void TLB::disable(int ppn)
{
    for (int e=0; e<(this->sizeE); e++)
        if (this->entry[e].ppn == ppn)
            this->entry[e].valid = 0;
}

void TLB::update(int vpn, int ppn)
{
    /* find least indexed invalid entry first */
    for (int e=0; e<(this->sizeE); e++)
        if (!this->entry[e].valid) {
            this->entry[e].valid = 1;
            this->entry[e].tag = vpn;
            this->entry[e].ppn = ppn;
            return;
        }

    /* then find valid LRU entry */
    int min_time = 2147483647, entry_record;
    for (int e=0; e<(this->sizeE); e++) {
        if (this->entry[e].time < min_time) {
            min_time = this->entry[e].time;
            entry_record = e;
        }
    }
    this->entry[entry_record].tag = vpn;
    this->entry[entry_record].ppn = ppn;
}
