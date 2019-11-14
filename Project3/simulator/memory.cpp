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

#include <cstring> // memcpy
#include "pagetable.h" // this->pt
#include "memory.h" // MEMORY
#include "disk.h" // this->d

/* return the refreshed page number */
int MEMORY::read_disk_into_memory(int va, int &old_is_valid)
{
    int new_ppn; /* new selected */
    int new_vpn = va / this->sizeP;

    /** find least indexed invalid page first */
    for (int new_ppn=0; new_ppn<(this->sizeE); new_ppn++)
        if (!this->page[new_ppn].valid) {
            memcpy(this->page[new_ppn].byte, /* select the desired memory page */
                   this->d->page[new_vpn].byte, /* copy data from disk */
                   this->sizeP * sizeof(char));
            this->page[new_ppn].valid = 1; /* this page is in use */
            old_is_valid = 0;
            return new_ppn;
        }

    /** then find least recently used page */
    int min_time = 2147483647;
    for (int ppn=0; ppn<(this->sizeE); ppn++)
        if (min_time > this->page[ppn].time) {
            min_time = this->page[ppn].time;
            new_ppn = ppn;
        }

    /* if adopting write-back policy, we should remember to save the dirty blocks
     * of cache belonging to the to-be-replaced memory page back to it. however
     * this step can be omitted if write-through policy is adopted. */

    /* To make new use of this pre-occupied memory page, we should remember
     * to save its content back to the original disk location "first." */
    int old_vpn; /* find old vpn from target ppn */
    for (int vpn=0; vpn<(this->pt->sizeE); vpn++)
        if (this->pt->entry[vpn].valid && this->pt->entry[vpn].ppn==new_ppn)
            old_vpn = vpn;
    memcpy(this->d->page[old_vpn].byte, /* back to original disk location */
           this->page[new_ppn].byte, /* save from pre-occupied memory */
           this->sizeP * sizeof(char));
    memcpy(this->page[new_ppn].byte, /* to this desired memory page */
           this->d->page[new_vpn].byte, /* load from new disk location */
           this->sizeP * sizeof(char));
    old_is_valid = 1;
    return new_ppn;
}
