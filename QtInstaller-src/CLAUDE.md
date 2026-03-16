# Project Rules (C++/Qt)

对本仓库进行任何代码修改时，必须遵守以下规则。

## Naming

| 类型 | 格式 | 示例 |
|------|------|------|
| 类名 / 结构体名 / 枚举类型名 | `UpperCamelCase` | `RobotStatusWidget` |
| 函数名 / 普通变量名 | `lowerCamelCase` | `loadRobotConfig` |
| 成员变量 | `_lowerCamelCase` | `_currentRobotId` |
| 静态成员变量 | `s_lowerCamelCase` | `s_instance` |
| 常量 | `kUpperCamelCase` | `kDefaultTimeoutMs` |
| 宏 | `UPPER_SNAKE_CASE` | `MAX_BUFFER_SIZE` |
| 枚举值 | `E_` + `UpperCamelCase` | `E_Disconnected` |
| Qt 信号 | `sigXxx` | `sigConnectionChanged` |
| Qt 槽 | `slotXxx` | `slotReconnect` |

## File Layout

- 一个主要类对应一对 `.h/.cpp`，文件名与类名一致。
- 头文件使用 `#pragma once` 或 include guard，同一子模块保持一致。
- 头文件中禁止 `using namespace xxx;`。
- include 顺序：本文件头文件 → Qt → 标准库 → 项目内其他头文件。
- 能前置声明就前置声明，减少头文件耦合。

## Functions

- 职责单一，优先早返回，减少嵌套层级。
- 单函数不超过 80 行，超过时拆分私有辅助函数。
- 参数超过 4 个时，封装为结构体或配置对象。
- 避免魔法数字，提取为具名常量。
- 任何可能失败的操作必须有返回值检查或错误日志。

## Qt

- 信号槽连接使用新式语法：`connect(a, &A::sigXxx, b, &B::slotXxx);`
- QObject 对象优先通过父子关系管理生命周期。
- 非 QObject 资源使用 RAII（智能指针或栈对象）。
- UI 线程不做耗时阻塞，耗时任务放工作线程。

## Comments

- public API 必须有 Doxygen 注释。
- 注释解释"为什么"，不只写"做了什么"。
- 复杂算法补充输入、输出、边界条件说明。

## Logging & Error Handling

- 错误路径必须记录可定位日志（含关键参数）。
- 日志内容可检索，禁止无意义日志（如只打印 `"failed"`）。
- 不得吞异常或忽略错误码。

## Safety

- 禁止在未确认线程安全的情况下跨线程共享可变对象。
- 禁止引入未使用代码、未使用 include、未使用变量。
- 修改旧逻辑时保持行为兼容，除非需求明确要求变更。

## Commit Checklist

- [ ] 命名符合本规范
- [ ] 编译通过
- [ ] 关键路径已做最小验证（启动、主流程、异常路径）
- [ ] 无无用代码、无调试残留

## Priority

- 与历史代码风格冲突 → 新改动按本规范执行。
- 与外部硬性规范冲突（编译器、框架、接口协议）→ 以硬性规范为准，并在注释中说明原因。
