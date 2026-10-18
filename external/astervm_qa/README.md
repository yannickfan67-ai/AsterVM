# AsterVM — X86-QA-001 v0.1

職責：Genspark QA／測試與正確性驗證工程師。

交付：40 個具名、手工指定預期值的真實 x86 機器碼案例；單步契約草案；JSON 子程序轉接執行器；外部 Unicorn 參考轉接器；框架故障注入自測。

## 執行

需要 Python 3.10 以上；框架自測不需第三方套件。

```sh
python generate_vectors.py
python -m unittest -v test_harness
python -m pip install -r requirements-reference.txt
python runner.py --label reference --report reference-report.json -- python unicorn_adapter.py
```

產品核心接入：將最後的命令換成團隊實作的轉接器，並使用 `--label astervm`。預期值不傳入受測轉接器。每例建立獨立程序，任何失敗最終回傳狀態碼 1。參考實作通過不等於 AsterVM 通過。

## v0.1 單步契約（待 CPU 團隊對齊，非強制產品 ABI）

標準輸入是一個 JSON 物件：`id`、`mode=real16`、`code` 十六進位字串、`initial.registers`、`initial.memory`、`budget`。標準輸出只能是一個 JSON 物件：`reason=step`、`registers`、`memory_hex`；診斷請寫標準錯誤。

暫存器名稱：ax bx cx dx si di bp sp cs ds es ss ip flags，數值為無號整數。FLAGS 以低 16 位元比較。每例從乾淨 1 MiB 零值 RAM 開始，套用初始實體記憶體區塊後，最後將 code 放到 CS*16+IP。預設 CS=DS=ES=SS=0、IP=0x1000、SP=0x8000、FLAGS=2；完整初始值已展開於 vectors.json。

只退休一條指令；預算 5 秒。回應必須包含所有暫存器，以及完整 1 MiB 實體 RAM 的十六進位字串。執行器會比對所有暫存器及所有 RAM，包括本應不變的欄位，避免漏掉額外寫入。記憶體區塊採實體位址、小端序位元組；預期區塊是在初始 RAM 上的變更，不是完整影像。

每例的 `expected.registers` 是完整期望終態，與 initial 的差就是預期暫存器變更；`expected.memory` 是期望記憶體寫入；`flags_mask` 為可比較旗標位元。算術遮罩 0x8d5；邏輯遮罩 0x8c5，排除未定義 AF；其餘本切片使用 0xffff 檢查旗標保持。預期值由測試作者指定，不由 Unicorn 產生；Unicorn 只提供獨立實跑比對，並非完整架構正確性的證明。

失敗條件：非預期停止、暫存器差異、有效旗標差異、任何 RAM 差異、程序錯誤、無效輸出、逾時。v0.1 不涵蓋 A20 跨界、保護模式、例外投遞、中斷、裝置、BIOS、客體系統、HLT 或無效 opcode 語義。所有案例使用 CS=0；非零 DS/SS 有覆蓋。

## 驗收界線

- 框架自測：合成輸出只檢查比較器，絕不作為 CPU 成功證據。
- 外部參考驗證：Unicorn 2.1.4 在 Linux 實際執行案例。
- AsterVM 核心：尚未取得、尚未驗證。
- Windows：程式未使用平台專用 shell，但尚未完成 Windows 原生實跑。

請見 evidence/ 內的實跑日誌、機器環境及 JSON 報告。此版本不內附其他成員的程式或啟動影像，也未核驗其群聊成果聲明。
