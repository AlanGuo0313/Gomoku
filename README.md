# 五子棋 Gomoku

使用 C++17 與 Windows 原生視窗製作的五子棋，支援三種難度的電腦對手與本機雙人模式。介面採用暖木色棋盤、霧綠背景與漸層棋子，無須瀏覽器、網路或額外圖片素材。

## 開始遊戲

在 Windows 雙擊 `gomoku.exe`，即可開啟獨立遊戲視窗。

1. 預設為「玩家對電腦／普通」：你執黑棋先行，電腦執白棋；也可在右側選擇本機雙人模式。
2. 以滑鼠左鍵點選棋盤交叉點落子；人機模式下，電腦會自動回應，思考時不接受玩家落子。
3. 橫向、直向或任一斜線連續五顆以上同色棋子即獲勝。
4. 棋盤填滿且無人獲勝時為平手。
5. 遊戲結束後不再接受落子，可按「重新開始」或「悔棋」。

本作採自由五子棋規則，沒有三三、四四或長連禁手；目前沒有網路連線或存檔功能。

## 電腦難度

| 難度 | 策略 |
| --- | --- |
| 簡單 | 在評分較好的六個候選位置中隨機挑選，適合入門 |
| 普通 | 評估連線、空缺與對手威脅，選擇單步攻防評分最高的位置 |
| 困難 | 使用相同攻防評估，再以 Alpha-Beta 剪枝及逐層加深搜索，最多推演三手 |

所有難度都會優先直接取勝，或阻擋對手下一手的五連。困難模式限制候選數、搜索節點與約 650 毫秒的搜索時間，並非必勝或專業棋力；實際回應時間依電腦效能而異。

電腦在背景執行緒計算，視窗仍可操作。切換模式或難度會立即清空棋局、重新開始；雙人模式下會停用難度選單。

## 操作與畫面資訊

| 操作或標示 | 說明 |
| --- | --- |
| 滑鼠左鍵 | 在空交叉點落子，棋盤外或已有棋子的位置不接受落子 |
| 滑鼠移至空位 | 顯示落子位置預覽 |
| 重新開始 | 清空棋盤與歷史紀錄，回到黑棋先行；立即生效 |
| 悔棋 | 人機模式撤回電腦與玩家的一輪；雙人模式撤回最後一手；可連續使用 |
| 電腦思考中悔棋 | 撤回玩家剛下的一手，舊計算結果不會套用到新棋局 |
| 對弈模式／電腦難度 | 變更設定並立即重新開始 |
| 離開遊戲 | 關閉視窗，棋局不會儲存 |
| 紅點 | 標記最後一手 |
| A～O／1～15 | 棋盤欄位／列號，右側會顯示最後落子座標 |
| 右側狀態卡 | 顯示目前回合、已下手數及勝負結果 |

沒有棋步紀錄時，悔棋按鈕會停用。也能以 Tab 切換按鈕，再按空白鍵執行。

## 編譯環境

- Windows 桌面環境。
- 支援 C++17 的 MinGW-w64 G++，專案設定使用 `C:/mingw64/bin/g++.exe`。
- Windows SDK 相容標頭與系統函式庫：User32、GDI、GDI+（一般 MinGW-w64 發行版已包含）。
- 原始碼以 UTF-8 儲存，中文介面使用寬字元 API。

### 命令列編譯

在專案根目錄執行：

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -O2 -finput-charset=UTF-8 gomoku.cpp -o gomoku.exe -mwindows -lgdi32 -lgdiplus -luser32 -static-libgcc -static-libstdc++
.\gomoku.exe
```

若找不到 `g++`，請將 MinGW-w64 的 `bin` 資料夾加入 PATH，或使用編譯器的完整路徑。重新編譯前請先關閉舊的遊戲視窗，以免執行檔被鎖定。

`-mwindows` 讓程式以 GUI 模式啟動，不另開命令列視窗；`-lgdiplus` 用於平滑與漸層繪圖。靜態連結選項會包含 GCC 與 C++ 執行階段，但程式仍依賴 Windows 系統 DLL。

### VS Code

1. 開啟整個專案資料夾。
2. 確認已安裝 Microsoft C/C++ 擴充套件及 MinGW-w64。
3. 按 Ctrl+Shift+B，執行預設的 G++ 建置工作。
4. 雙擊產生的執行檔，或按 F5 使用既有 GDB 設定偵錯。

若 MinGW-w64 安裝位置不同，請調整 [建置設定](.vscode/tasks.json) 與 [偵錯設定](.vscode/launch.json) 的編譯器／GDB 路徑。

## 程式碼導讀

| 檔案 | 用途 |
| --- | --- |
| [gomoku.cpp](gomoku.cpp) | Windows 視窗、畫面繪製、按鈕與滑鼠事件 |
| [gomoku_game.h](gomoku_game.h) | 不依賴 Windows 的棋盤狀態、落子、勝負判定、悔棋及重置 |
| [gomoku_ai.h](gomoku_ai.h) | 三種電腦策略、棋型評分與有時間限制的搜索 |
| [tests/gomoku_tests.cpp](tests/gomoku_tests.cpp) | 遊戲規則的回歸測試 |
| [tests/gomoku_ai_tests.cpp](tests/gomoku_ai_tests.cpp) | 電腦落子、勝利／防守策略與人機悔棋的測試 |
| [.vscode/tasks.json](.vscode/tasks.json) | VS Code 建置工作 |
| [.vscode/launch.json](.vscode/launch.json) | GDB 偵錯設定 |

建議先閱讀 `Gomoku::makeMove()`，了解一次落子如何更新狀態，再閱讀視窗程式中的以下區塊：

- **版面設定**：集中管理棋盤位置、格距與按鈕識別碼。
- **基本繪圖工具**：文字、圓形、面板與棋子。
- **畫面內容**：`drawBoard()`、`drawStones()`、`drawSidebar()`。
- **控制項與輸入處理**：建立按鈕、滑鼠座標轉換及落子預覽。
- **事件分派**：`windowProcedure()` 將 Windows 訊息交給對應功能。
- **程式入口**：`WinMain()` 初始化 GDI+、建立視窗並啟動訊息迴圈。

遊戲資料由 `Gomoku` 類別封裝，介面透過唯讀存取函式取得狀態；改變棋局則呼叫 `makeMove()`、`undoMove()` 或 `resetGame()`。

人機模式使用 `undoToPlayer()` 撤回一整輪。`updateComputer()` 以 `std::async` 計算棋局副本，再由視窗計時器收取結果；版本編號確保重開、悔棋或切換設定後，不會套用舊結果。畫面和實際棋局始終只由視窗執行緒更新。

## 測試

在專案根目錄執行：

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic tests/gomoku_tests.cpp -o gomoku_tests.exe
.\gomoku_tests.exe
g++ -std=c++17 -O2 -Wall -Wextra -pedantic tests/gomoku_ai_tests.cpp -o gomoku_ai_tests.exe
.\gomoku_ai_tests.exe
```

測試涵蓋越界與重複落子、黑白輪替、四方向勝利、白棋勝利、長連、平手、結束後禁止落子，以及悔棋和重新開始。

電腦測試涵蓋三種難度的合法落子、立即取勝、阻擋五連、補缺口防守、簡單模式隨機性、計算不修改原棋局，以及思考中／回應後／勝局後的人機悔棋。

## 目前限制

- 僅支援 Windows，介面使用固定版面，不支援自由縮放。
- 以系統 DPI 感知顯示；跨不同縮放比例螢幕時不會重新配置版面。
- 關閉視窗或重新開始會清除目前棋局。
