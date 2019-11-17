# MIPS R3000 處理器模擬

類別：課程作業<br>
課號：NTHU, 10320 CS410001, 計算機結構 (Computer Architecture)<br>
課本：David A. Patterson and John L. Hennessy, Computer Organization and Design, Fifth Edition, 2014<br>
時程：2015 年 3 月 ~ 2015 年 6 月，全學期共有三個小專案，每個專案約耗時 1 個月完成<br>
簡介：大二下學期計算機結構課程模擬 MIPS R3000 處理器的 C/C++ 程式設計練習，三個專案的概要分別為：
* [Project 1 (Single Cycle)](https://github.com/alan23273850/Computer-Architecture/tree/master/Project1)：解讀每一個指令並執行，最後印出每個週期的暫存器與記憶體的狀態。
* [Project 2 (Pipeline)](https://github.com/alan23273850/Computer-Architecture/tree/master/Project2)：將執行一個指令的過程管線化為多個階段，任意時間的每個階段都執行不同指令以增進效率。
* [Project 3 (CMP)](https://github.com/alan23273850/Computer-Architecture/tree/master/Project3)：將記憶體擴充成多階層式儲存系統，並觀察儲存元件彼此之間的資料流向。

每個專案的封面都有非常詳盡的說明。

---

## 先備設定 (Prerequisites)

由於本專案有使用 [git-lfs](https://git-lfs.github.com/) 的技術存放測資檔，避免二進位檔也不小心加進 git 的比對系統以增進效率，使用者端必須先行安裝 git-lfs 才能下載到真正的測資檔，否則只會有指向測資的指針檔。在 Linux 底下安裝的指令：

1. 先下 `sudo apt update` 確保系統知道套件的下載位置。

2. 下 `sudo apt install git-lfs` 安裝此軟體。

3. 每台主機必須至少下一次 `git lfs install` 的初始化動作。

完成上述步驟之後，每次下 `git clone` 指令就能確保正確地下載由 git-lfs 管理的檔案！[這裡](https://blog.yowko.com/git-lfs/)有網友為這個套件給出淺顯易懂的使用介紹，值得一讀。
