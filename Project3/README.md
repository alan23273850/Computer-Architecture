# Project 3 (CMP)

本專案目的是用 C++ 去實作一個能趁處理器在執行 (精簡版) MIPS R3000 指令集的時候去模擬多階層式儲存系統 (hierarchical storage system) 的元件之間資料互相存取的過程，專案名稱的三個字母 CMP 分別是 cache、memory、pagetable 的開頭。與第二個專案不同，本專案的架構和第一個專案 (single cycle) 極為類似，可以直接修改而來，只要在 (a) 指令讀取 (instruction fetch) 與 (b) 存取資料 (data read/write) 的地方安插和儲存元件互動的程式碼，就能練習到這次專案的目標。

---

## 專案概要

#### # 處理器架構 (和 Project 1 比較)

<img src="README/structure.PNG" width="100%">
簡單來說就是少了錯誤偵測，任何會發生位址不對齊 (misalignment) 或 溢出 (overflow) 的測資都是不允許的，並且多了模擬多階層式儲存系統的過程以及回報每個儲存單元各自發生存取命中 (hit) 與失誤 (miss) 的次數。

---

#### # 儲存元件介紹

上圖的 hierarchical storage system 包含了五個元件，分別是轉譯後備緩衝區 (translation lookaside buffer，簡稱 TLB)、分頁表 (page table，簡稱 PT)、快取 (cache)、實體記憶體 (physical memory)、虛擬記憶體 / 硬碟 (virtual memory / disk)。

* Disk：整個多階層式儲存系統裡面最大、最底層、離 CPU 最遠、速度最慢的最基本元件，可以看成是資料正本 (original) 儲存的地方，所有的值都會從這裡開始讀取，最終也會存回此處。注意到另外一個詞「虛擬記憶體」指的其實不是記憶體元件，而是每支程式以自己的視角所看到的虛擬空間。在本專案為了設計的方便與概念的簡單，虛擬記憶體位址 (virtual address) 就直接假設為硬碟位址，不過在實際的系統上未必是如此，有人在 stackoverflow 論壇上詢問[作業系統如何得知硬碟位址](https://stackoverflow.com/questions/27579864/)。

* Memory：可視為 disk 的「快取」，所謂快取就是一個離 CPU 比較近、負責儲存相對於自己較為低階的儲存元件資料之複本 (copy) 的地方，如此一來當 CPU 想存取低階元件的資料時便可以直接從較近的元件抓取，以節省指令執行的時間。具體的實作方式是把 disk 分成許多 pages，而其中的幾個 page 寄放在 memory 裡，就達到了快取的效果，也因此 disk page 大小必須和 memory page 相同。

* Cache：注意到 cache 一詞除了有功能性的意義之外，在這裡還專指「memory 的快取」這個元件。具體的實作方式也是把這個元件切成許多小資料塊 (block)，等到需要資料的時候就把 memory 的一個 page 裡面的一小部分搬移到 cache 的一個 block 裡面，通常 block size 不會超過 page size。

* PT：負責記錄每個虛擬位址 (virtual address) 所對應的資料是否已存在 memory 之中，若否稱為分頁錯誤 (page fault)，必須從 memory 裡面選一頁容納虛擬位址在虛擬記憶體所對應到的 page。所以嚴格說起來 PT 並不具快取功能。

* TLB：其實就是 PT 的快取。和其他元件不同的地方在於它不具有 index 欄位，這是因為 TLB 在現實世界中就預設為硬體實作，要檢查某個 VPN 是否已有對應的 PPN 可同時比對所有欄位的緣故，那到了軟體的世界中就只能一個欄位一個欄位慢慢比對了。

---

#### # 讀寫流程圖
<br>
<img src="README/flow.png" width="100%">

---

#### # 元件間資料流

<img src="README/element.png" width="100%">

---

#### # 檔案內容

<img src="README/file.PNG" width="100%">

---

## 實作細節 (需要注意的地方)

* Valid bit 的意義：架構圖中的五個元件裡面除了硬碟之外其他都有 valid bit，而這幾種 valid bit 的意義也不盡相同，只有 PT 的 valid bit 意思是「虛擬位址所對應到的資料是否已存在 memory 中」，而其他元件 valid bit 的意思都是當前的欄位 (entry) 是否正在使用的意思。值得一提的是，當 valid bit 是採用後者的意義時，以軟體程式實作的角度上來說其實根本就可以把它和 time 欄位 (負責記錄最近存取的 cycle 數) 整合起來，沒在使用的欄位最近存取時間一律初始化為 0，因為執行中的 cycle 數必定為正。只是說因為留有 valid bit 才能維持一定的可讀性，所以沒有把它拿掉。

* Cache 的回寫 (write-back) 和透寫 (write-through) 策略：在元件介紹一節有提到 cache 是 memory 的快取，我們只知道快取至少要節省讀取的時間，那對於寫入的操作自然就有兩種選擇，「更新快取時是否應該連同較低階的 memory 一同更新」呢？答是就是「透寫」，答否就是「回寫」。如果採取回寫策略的話就還需要 dirty bit 負責去記錄該欄位是否已經被覆寫，如果已經被覆寫，那麼該欄位要被取代之時就得先把資料寫回 memory page 才行。因為本專案只是個模擬器，毋須考慮效能，因此直接採取透寫策略在撰寫程式碼上會方便許多。另外也可以去思考為什麼 memory 就沒有分成兩種策略分別討論呢？

---

## 編譯與執行

* 由於課程的評分系統是在 Linux 上進行，本專案僅適合在 Linux 作業系統底下運行。
* 本程式只能用 g++ 編譯，因為裡面有用到 C++ 的語法 (把 struct 當 class 在用)。

1. 在 simulator 資料夾底下先 make clean，把該清除的檔案清掉。
2. 從 testcases 資料夾底下隨意挑選一位同學 (學號) 的 dimage.bin 和 iimage.bin 測資，並放進 simulator 資料夾。
3. 下 make 指令，編譯出我們的模擬器作品。
4. 下 ./CMP [P1] [P2] ... [P10] 指令，開始執行 iimage.bin 裡面的指令，並產生出 snapshot.rpt 和 report.rpt 兩個檔案。其中十個參數的順序如下：

    * [P1]：The instruction memory (I memory) size, in number of bytes
    * [P2]：The data memory (D memory) size, in number of bytes
    * [P3]：The page size of instruction memory (I memory), in number of bytes
    * [P4]：The page size of data memory (D memory), in number of bytes
    * [P5]：The total size of instruction cache (I cache), in number of bytes
    * [P6]：The block size of I cache, in number of bytes
    * [P7]：The set associativity of I cache
    * [P8]：The total size of data cache (D cache), in number of bytes
    * [P9]：The block size of D cache, in number of bytes
    * [P10]：The set associativity of D cache
   
   如果不給參數的話預設值如下：64, 32, 8, 16, 16, 4, 4, 16, 4, 1。

5. 這兩個輸出檔案理論上要和該同學 (學號) 底下的 snapshot.rpt 和 report.rpt / report1.rpt (預設參數)、report2.rpt (參數：256, 256, 32, 32, 16, 4, 4, 16, 4, 4)、report3.rpt (參數：512, 1024, 128, 64, 64, 4, 8, 32, 4, 4) 完全相符，可用 diff 檢驗。

* 在 simulator 資料夾底下還有另一個有用的 test.sh 腳本，它會把上述步驟對每位同學的測資都跑過一遍，如果沒輸出錯誤代表這個模擬器的實作和助教的有一定程度的相符。

---

## 專案展示 (Demo)

* 以下是本人的測資跑出來的檔案截圖 (只擷取其中一部份)。

1. report.rpt
    <div><img src="README/report.PNG" width="40%"></div>

2. snapshot.rpt
    <div><img src="README/snapshot.PNG" width="40%"></div>
