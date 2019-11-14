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
#include "pagetable.h" // PAGETABLE

int PAGETABLE::find_ppn(int vpn)
{
    if (this->entry[vpn].valid) {
        this->hit++;
        int ppn = this->entry[vpn].ppn;
        this->m->page[ppn].time = cycle;
        return ppn;
    }
    this->miss++;
    return -1;
}

void PAGETABLE::disable(int ppn)
{
    for (int e=0; e<(this->sizeE); e++)
        if (this->entry[e].ppn == ppn)
            this->entry[e].valid = 0;
}

void PAGETABLE::update(int vpn, int ppn)
{
    this->entry[vpn].valid = 1;
    this->entry[vpn].ppn = ppn;
}
