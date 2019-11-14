# Project 2 (pipeline)

本專案目的是用 C 語言去實作一個能執行 (精簡版) MIPS R3000 指令集的「管線化執行」模擬器，所謂「管線化執行」指的是把一個指令執行的過程切成許多階段，由於每個階段所使用到的硬體資源不同，當該指令 I<sub>1</sub> 在某階段已經沒在使用硬體資源 R 的時候，下一道指令 I<sub>2</sub> 就能趁 I<sub>1</sub> 還在其他階段執行的時候先行使用這個硬體資源 R，增進整體程式效率。而管線化會遇到的問題在於當我的下一道指令要取用上一道指令產生的結果時，因為結果尚未跑回暫存器而取用不到，解決這個問題 (hazard) 的方法才是這個專案最精華的地方。

---

## 專案概要

#### # 指令管線化 (可將網頁縮放以獲得最佳閱覽大小)

<img src="README/pipeline.png" width="100%">
<img src="README/cycle.png" width="100%">

#### # 週期 (cycle) 的定義

從一組暫存器開始，經過一系列的硬體運算之後將結果存到另一組暫存器的這個過程稱為一個週期。從上面的圖可以觀察到，資料流的方向未必是從左到右，例如在 IF 階段除了根據 PC 暫存器讀取適當的指令之外，也要同時更新自己 PC 的值，在 ID 階段如有遇到跳躍指令也得更新 PC，而 WB 階段主要就是把資料寫回放在 ID 階段的 register file，更不用說還有紅色和綠色的 forwarding path。
* IF (instruction fetch) 階段：根據 PC 暫存器的值從 instruction memory 抓取適當的指令執行，同時更新下個週期要使用的 PC 值。
* ID (instruction decode) 階段：解讀這個指令想要處理器做什麼事，由控制單元 (control unit，簡稱 CU) 主導，如遇跳躍指令會在此階段更新 PC 值，同時清空尚未考慮跳躍指令時已預先讀取的指令。
* EX (execute) 階段：如果指令需要算術邏輯單元 (Arithmetic Logic Unit，簡稱 ALU)，會在此階段進行並產生計算結果。
* MEM (data memory) 階段：任何寫入與讀取記憶體的動作會在此階段執行，由於欲動作的位址必須由 ALU 產生，所以 MEM 階段必須放在 EX 階段之後。
* WB (write back) 階段：把 ALU 的運算結果或者從記憶體讀取出來的資料寫回 register file。

#### # 檔案內容

<img src="README/file.PNG" width="100%">
在 snapshot.rpt 裡的 cycle N 是印出「已經執行完第 N 個 cycle」的暫存器狀態，再加上各階段「預計執行第 N+1 個 cycle」的指令，旁邊會搭配「執行完第 N+1 個 cycle」之後所有與 hazard 相關的警告訊息。

---

## HDU (Hazard Detection Unit) 的推導

在一開始的專案概要有提到，管線化會造成當下一道指令要取用上一道指令產生的結果時，因為結果尚未跑回「暫存器」而有取用不到的情況，如何解決這個問題是這個專案的核心所在。先觀察一下所有階段的暫存器讀寫狀況，有重要結論：

1. 資料產生 (generate) 的時機只可能是 MEM 階段的前半週期 gen<sub>M</sub> 或 WB 階段的前半週期 gen<sub>W</sub>。
2. 資料使用 (use) 的時機只可能是 ID 階段的後半週期 use<sub>D</sub> 或 EX 階段的後半週期 use<sub>E</sub>。

如果我們把產生和使用的時機配對一下會發現總共有 4 種情況，其中只有第 2 種情況是正常的 datapath，其他的情況都必須倚賴 forwarding 來完成，所謂 forwarding 就是把後面的階段產生的結果直接拉回給前面的階段使用，因為資料是在前半週期產生、後半週期使用，便能解決取用不到的問題。

<a> | ID | EX | MEM | WB | 結果
:---:|:---:|:---:|:---:|:---:|:---:
-| use<sub>D</sub> | use<sub>E</sub> | gen<sub>M</sub> | gen<sub>W</sub> | -
1| use<sub>D</sub> | | gen<sub>M</sub> | | fwd
2| use<sub>D</sub> | | | gen<sub>W</sub> |
3| | use<sub>E</sub> | gen<sub>M</sub> | | fwd
4| | use<sub>E</sub> | | gen<sub>W</sub>| fwd

事實上 forwarding 只能解決「資料已產生但還來不及跑回暫存器的情況」，如果連「資料都還沒產生就要使用」的話勢必要先暫停取用的指令，讓產生資料的指令跑到能有正式產出的階段後，取用的指令方能繼續向前，這個動作我們稱之為 stall。

<a> | ID | EX | MEM | WB | 結果
:---:|:---:|:---:|:---:|:---:|:---:
-| use<sub>D</sub> | use<sub>E</sub> | gen<sub>M</sub> | gen<sub>W</sub> | -
5| use<sub>D</sub> | gen<sub>M</sub> → | | | stall
6| use<sub>D</sub> | gen<sub>W</sub> → | | | stall
7| use<sub>D</sub> | | gen<sub>W</sub> → | | stall
8| | use<sub>E</sub> | gen<sub>W</sub> → | | stall

比較特別的是第 8 種情況的 stall 其實可以提前，因為 stall 的目的只是為了拉開資料產生階段與讀取階段的時間差距，與什麼時候執行無關，如此一來就可以和第 5 ~ 7 種情況統一起來，方便實作。

<a> | ID | EX | MEM | WB | 結果
:---:|:---:|:---:|:---:|:---:|:---:
8| use<sub>E</sub> | gen<sub>W</sub> → | | | stall

---

## 實作細節 (需要注意的地方)

* 關於一個週期裡面各個元件彼此之間運作的順序如下：

    1. 先跑 CU 運算出其他各個元件在這個週期內會需要的控制輸入。
    2. 以 CU 的輸出作為輸入跑 HDU，得到「這個週期」forwarding 的選擇和「下個週期一開始」stall 的控制結果。
    3. 其他各階段同時 (平行地) 根據前兩個步驟的輸出作為輸入去執行。

  如同之前所說，週期的定義是從一組暫存器開始作一系列的硬體運算之後再將結果存回另一組暫存器，由於這五個階段是平行處理，而 C 語言無法做到這件事，所以一般會建議從最後一個階段開始往回模擬 (WB → MEM → EX → ID → IF)，才不會發生想要取用某暫存器卻已經先被洗除的狀況。每模擬一個階段，除了 (A) 根據起始暫存器更新目標暫存器外，也要記得 (B) 先計算並儲存那些在前面的階段「和起始暫存器有相依性」的暫時性線路之運算結果，不然也會發生取用值先被洗除的問題。<br>
  　　　　　　　　　　　　　<img src="README/stage.png" width="50%">

* 本專案以軟體去模擬硬體實作，有些地方的硬體設計很繁雜，改成軟體就會變得簡單，例如多工器 (multiplexer) 其實就是程式語言的 if-else 或 switch-case，這便不必拘泥於一個位元一個位元 (bitwise) 的讀取，而 ALU 的運算也能更多元更彈性 (例如任意置換兩運算元的順序)。另外，關於在 HDU 裡面是否想使用暫存器 (use<sub>D</sub> 或 use<sub>E</sub>) 的判定，原先理論上可以直接根據多工器的選擇來作決定，只不過由於指令集內有個非常特別的 lui $t,C ($t = C << 16) 指令，它只吃一個運算元 C，在另一個運算元的多工器選項無法呈現出是否使用的前提之下，處理器便會誤以為它也有被使用到，就因為這搞怪的指令，使得我在這邊的條件判定必須直接寫死 (hard-coded) 每個指令的 opcode 和 funct。雖然說也可以在 ALU 外面多弄個常數16再讓它作單純的 << 運算，不過這樣就會使得多工器的選擇變複雜，所以青菜蘿蔔各有所好，為了多工器的規律性以及電路圖的美觀 (把常數 16 放進 ALU 內)，我選擇前者的作法。

---

## 編譯與執行

* 由於課程的評分系統是在 Linux 上進行，本專案僅適合在 Linux 作業系統底下運行。
* 而且本程式只能用 gcc 編譯，因為裡面有用到 C++ 的關鍵字 (and, or, ..)。

1. 在 simulator 資料夾底下先 make clean，把該清除的檔案清掉。
2. 從 testcases 資料夾底下隨意挑選一位同學 (學號) 的 dimage.bin 和 iimage.bin 測資，並放進 simulator 資料夾。
3. 下 make 指令，編譯出我們的模擬器作品。
4. 下 ./pipeline 指令，開始執行 iimage.bin 裡面的指令，並產生出 snapshot.rpt 和 error_dump.rpt 兩個檔案。
5. 這兩個輸出檔案理論上要和該同學 (學號) 底下的 snapshot.rpt 和 error_dump.rpt 完全相符，可用 diff 指令檢驗。

* 在 simulator 資料夾底下還有另一個有用的 test.sh 腳本，它會把上述步驟對每位同學的測資都跑過一遍，如果沒輸出錯誤代表這個模擬器的實作和助教的有一定程度的相符。

---

## 專案展示 (Demo)

* 以下是本人的測資跑出來的檔案截圖 (只擷取其中一部份)。

1. error_dump.rpt
    <div><img src="README/error_dump.PNG" width="40%"></div>

2. snapshot.rpt
    <div><img src="README/snapshot.PNG" width="40%"></div>
