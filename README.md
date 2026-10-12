# log-task-service

一个使用 C++17 开发的批量日志分析学习项目。

项目从单行日志解析开始，逐步实现命令行分析、后台任务处理、HTTP 接口和数据库存储。

## 当前进度

目前已完成：

- 使用 CMake 管理解析库、命令行入口和测试程序。
- 解析单条日志，校验字段数量、日志级别、服务名和耗时。
- 使用 `std::variant` 返回解析成功的记录或明确的错误。
- 使用 CTest 运行解析器测试，覆盖正常输入、非法输入和边界情况。

当前 `log_cli` 仅输出工程就绪信息。批次读取和统计功能将在后续实现。

## 日志格式

每条日志包含三个字段：

```text
LEVEL service latency_ms
```

例如：

```text
INFO api 12
WARN worker 35
ERROR database 120
```

字段约定：

| 字段 | 规则 |
|---|---|
| `LEVEL` | 仅接受 `INFO`、`WARN`、`ERROR`，区分大小写 |
| `service` | 长度为 1～32 个字符，仅包含 ASCII 字母、数字、下划线和连字符 |
| `latency_ms` | 十进制非负整数，范围为 0～60000，单位为毫秒 |

补充规则：

- 字段使用 ASCII 空白字符分隔，例如空格或 Tab。
- 允许连续分隔符和首尾空白。
- 服务名区分大小写。
- 耗时允许前导零，例如 `0012` 表示 12。
- 耗时不允许正负号、小数点或单位后缀。
- 空行和全空白行返回解析错误。

详细需求约定记录在 `docs/requirements.md` 中。

## 开发环境

当前开发和验证环境为 WSL Ubuntu。

构建需要：

- 支持 C++17 的 C++ 编译器，当前使用 GCC。
- CMake 3.20 或更高版本。
- Ninja。

测试使用 CTest，无需额外安装测试框架。

## 构建

在项目根目录执行：

```bash
cmake -S . -B build/week1-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTING=ON

cmake --build build/week1-debug
```

生成的主要目标：

| 目标 | 用途 |
|---|---|
| `log_parser` | 单行日志解析的静态库 |
| `log_cli` | 命令行程序入口 |
| `parser_test` | 解析器测试程序 |

## 运行测试

构建完成后执行：

```bash
ctest --test-dir build/week1-debug --output-on-failure
```

通过时可以看到：

```text
100% tests passed, 0 tests failed out of 1
```

CTest 注册了一个测试程序，该程序内部包含多项检查：

- 正常日志的所有字段是否解析正确。
- 空行、字段数量错误和非法字段是否返回对应错误码。
- 服务名长度和耗时范围的边界是否正确。
- 返回记录中的服务名是否独立保存，不受原输入字符串修改影响。

## 运行命令行入口

```bash
./build/week1-debug/log_cli
```

当前输出：

```text
Log analysis project is ready
```

后续命令行入口将负责读取输入、调用解析与统计模块，以及展示结果或错误。

## 工程结构

```text
.
├── CMakeLists.txt            # 构建与测试配置
├── README.md                # 项目介绍与复现步骤
├── include/
│   └── logsvc/
│       └── parser.h         # 解析器的公共接口和数据类型
├── src/
│   ├── parser.cpp           # 单行解析实现
│   └── main.cpp             # 命令行入口
├── tests/
│   └── parser_test.cpp      # 解析器测试
└── docs/
    └── requirements.md      # 输入规则与项目需求
```

解析库负责将日志文本转换为记录或错误。命令行入口负责与使用者交互；后续读取文件和报告行号的逻辑放在入口或批次处理模块中。

## 后续计划

1. 实现批次读取，遇到非法日志时报告行号。
2. 实现各级别数量、各服务请求数、错误数、平均耗时和 TopK 统计。
3. 加入本地任务管理、有界队列和工作线程。
4. 提供 HTTP 提交与查询接口，并使用 MySQL 保存任务和结果。
