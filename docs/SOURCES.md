# 参考资料与贡献来源

- Intel Software Developer Manuals（指令与架构规范的查证入口）：https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
- CMake/CTest add_test 官方说明：https://cmake.org/cmake/help/latest/command/add_test.html
- Genspark QA，X86-QA-001 v0.1：经本项目群聊提供的源码 ZIP；2026-10-18 取得。原始档案 SHA-256：`7cbe3f95c9422cb8f9b7735c57b5ef05ea3b4f3c2466a8c001fb2f89c156fbb4`。包内 SHA256SUMS 已逐文件验证。

external/astervm_qa 保留原始交付内容与来源说明。其中 evidence/ 是原贡献者的自报参考记录；本次重新执行的产品结果在本项目根 evidence/ 内。下载会话令牌不随源码归档。

- v0.2 栈语义依据：Intel SDM Vol.2B 的 PUSH/POP 指令说明（286+ PUSH SP取旧值、POP SP在隐含增量后写目标、实模式栈段限）。官方PDF：https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2b-manual.pdf 。M0只实现状态变换及fault回滚，不声称实现规范中的异常投递链。
