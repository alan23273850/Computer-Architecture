# Project 1 (single cycle)

本專案目的是用 C 語言去實作一個能執行 (精簡版) MIPS R3000 指令集的「單時脈週期」模擬器，所謂「單時脈週期」是為了和 Project 2 的「管線化」方式作區別，在本專案每個指令都只會花費剛好一個時脈週期去執行，這只是為了實作上的方便，能把重點放在「指令的解讀」上。

---

## 專案概要

#### # 處理器架構

<div><img src="README/structure.PNG" width="100%"></div>

#### # 檔案內容

<div><img src="README/file.PNG" width="100%"></div>

#### # 暫存器與指令集

<div><img src="README/register.png" width="70%"></div>
<div><img src="README/instruction.png" width="100%"></div>

---

## 實作細節 (需要注意的地方)

1. 為了模擬處理器的硬體運算，減法 a-b 必須改成「加上一個減數的負數 a+(-b)」。由於在 32-bit 整數環境下 -(-2^32) = -2^32，如果不按照規定的方法去實作，就會不小心改變了減法結果是否溢位 (overflow) 的判定。不得不說 -(-2^32) = -2^32 真的是一件神奇的事！

---

## 編譯與執行

* 由於課程的評分系統是在 Linux 上進行，本專案僅適合在 Linux 作業系統底下運行。
* 而且本程式只能用 gcc 編譯，因為裡面有用到 C++ 的關鍵字 (and, or, ..)。

1. 在 simulator 資料夾底下先 make clean，把該清除的檔案清掉。
2. 從 testcases 資料夾底下隨意挑選一位同學 (學號) 的 dimage.bin 和 iimage.bin 測資，並放進 simulator 資料夾。
3. 下 make 指令，編譯出我們的模擬器作品。
4. 下 ./single_cycle 指令，開始執行 iimage.bin 裡面的指令，並產生出 snapshot.rpt 和 error_dump.rpt 兩個檔案。
5. 這兩個輸出檔案理論上要和該同學 (學號) 底下的 snapshot.rpt 和 error_dump.rpt 完全相符，可用 diff 指令檢驗。

* 在 simulator 資料夾底下還有另一個有用的 test.sh 腳本，它會把上述步驟對每位同學的測資都跑過一遍，如果沒輸出錯誤代表這個模擬器的實作和助教的有一定程度的相符。

---

## 專案展示 (Demo)

* 以下是本人的測資跑出來的檔案截圖 (只擷取其中一部份)。

1. error_dump.rpt
    <div><img src="README/error_dump.PNG" width="40%"></div>

2. snapshot.rpt
    <div><img src="README/snapshot.PNG" width="40%"></div>
