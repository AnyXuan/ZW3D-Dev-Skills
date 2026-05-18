---
name: zw3d-ipc-comm
description: Use when working on ZW3D Qt EXE to ZW3D DLL communication, Named Pipe IPC, JSON command protocols, cacheId/entity handle handoff, PostMessage worker dispatch, target resolution, or frontend-to-bridge command routing.
---

# ZW3D 双进程通信 Skill — v1.2.0

> **触发词**: "IPC", "Named Pipe", "双进程", "Qt EXE", "JSON协议", "实体解析", "cacheId", "通信", "ipc communication", "消息分发", "PostMessage"
> **适用**: ZW3D 2026 Qt EXE ↔ ZW3D DLL 双进程通信架构
> **维护者**: 韩天尊
> **依赖**: [[zw3d-plugin-base\|plugin-base]] (必需)

> ⚠️ **本文件是IPC命令码表的唯一真相源**。各domain skill中的号段表只列出本域条目，此处为完整表。

### 权威来源引用
| 内容 | 引用 |
|------|------|
| C++ API调用规范 | [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md]] |
| IPC消息协议 | [[.workbuddy/skills/zw3d-dev-team/docs/02-ipc-protocol\|02-ipc-protocol.md]] |
| 质量门控 | [[.workbuddy/skills/zw3d-dev-team/docs/04-quality-gate\|04-quality-gate.md]] |
| 开发工作流 | [[.workbuddy/skills/zw3d-dev-team/docs/03-workflow\|03-workflow.md]] |
| 代码架构 | [[知识库/02_CAE开发/代码架构\|代码架构.md]] |

---

## 一、架构概述

```
┌─────────────────────────────────────────────────────────────────┐
│ Z3ParametricModeling.exe (Qt6, 客户端)                          │
│                                                                 │
│   SteadyThermalWidget ─── ZW3DHelper ─── NamePipeIPC (Client)  │
│   StructSimWidget       (单例)       \\.\pipe\Zw3dPMBridgePipe  │
│   ParamOptimiWidget                                              │
│   AppMainPage                                                    │
└───────────────────────────────┬─────────────────────────────────┘
                                │ 命名管道 IPC (JSON + \n)
┌───────────────────────────────┴─────────────────────────────────┐
│ Zw3dPMBridge.dll (ZW3D插件, 服务端)                            │
│                                                                 │
│   vxApiInit() ─── WorkerWndProcA ─── WorkHandle::HandleIPCMsg  │
│   (DLL入口)    (消息循环)         (cmd路由分发)                  │
│                                     ├── CAE: CaeHandle          │
│                                     └── CAD: cvx*/Zw*           │
└─────────────────────────────────────────────────────────────────┘
```

### 三个核心模块
| 模块 | 职责 | 关键类 |
|------|------|--------|
| **Qt EXE** | UI交互、用户输入、结果显示 | SteadyThermalWidget, StructSimWidget, ZW3DHelper |
| **ZW3D.exe** | CAD/CAE引擎，通过`-newinstance -silent`静默启动 | — |
| **Zw3dPMBridge.dll** | 桥接插件，Named Pipe服务端，ZW3D API调用 | WorkHandle, CaeHandle |

### 线程安全保障
| 机制 | 作用 | 说明 |
|------|------|------|
| `PostMessage` + 隐藏窗口 | 将API调用调度到ZW3D主线程 | 解决ZW3D API非线程安全问题 |
| `condition_variable` | 同步等待API执行完成 | IPC线程阻塞等待，执行完后notify |
| `mutex` | 保护共享数据(g_currentTask) | HandleIPCMsg写入加锁，WndProc读取也需加锁 |

**关键设计**:
```
IPC线程收到命令 → 存入g_currentTask(加锁) → PostMessage到隐藏窗口
→ ZW3D主线程WndProc执行(cvxPartVarSet/cvxUpdate等)
→ 设置g_currentTask.finished=true → condition_variable.notify_one()
```

---

## 二、IPC消息协议

> 完整协议规范见 [[.workbuddy/skills/zw3d-dev-team/docs/02-ipc-protocol\|02-ipc-protocol.md]]

### 2.1 传输层
- **管道名**: `\\.\pipe\Zw3dPMBridgePipe`
- **DLL侧**: Server (`Initialize(true)`)
- **Qt侧**: Client
- **消息格式**: JSON + `\n` 行分隔

### 2.2 请求/响应格式
```json
// 请求 (Qt → DLL)
{ "cmd": <int>, "data": { <key-value pairs> } }

// 成功响应 (DLL → Qt)
{ "cmd": <int>, "status": "success", "data": {...} }

// 失败响应
{ "cmd": <int>, "status": "error", "error": "<msg>", "errorCode": <int>, "debug": {...} }
```

### 2.3 回包铁律
1. `cmd`字段必须与请求**完全一致**
2. `status`只能是`"success"`或`"error"`
3. 错误时必须同时提供`error`(文本)和`errorCode`(数字)
4. 响应必须以`\n`结尾

> ⚠️ **K-002**: 分发层必须先把当前任务cmd回填: `j["cmd"] = static_cast<int>(g_currentTask.type);`

---

## 三、Qt侧发送

```cpp
QJsonObject j;
j["cmd"] = 230;
QJsonObject data;
data["taskId"] = 1;
data["temperature"] = 80.0;
data["cacheId"] = 3;  // 使用cacheId，非原始句柄
j["data"] = data;

QJsonDocument doc(j);
QString msg = doc.toJson(QJsonDocument::Compact) + "\n";
NamePipeIPC::Instance().Send(msg.toUtf8());
```

> ⚠️ **K-041**: UI端保持数值类型发送(`2.0`而不是`"2"`)，DLL端做容错解析。

---

## 四、DLL侧接收与分发

```cpp
// 使用无异常模式解析JSON
json j = json::parse(message, nullptr, false);
if (j.is_discarded()) {
    resp["status"] = "error";
    resp["error"] = "Invalid JSON";
    SendResponse(resp);
    return;
}

int cmd = j["cmd"];

// 分发处理 (WorkHandle::HandleIPCMsg)
switch (cmd) {
    case 200: resp = CaeHandle::CreateTask(j); break;
    case 220: resp = CaeHandle::SelectEntities(j); break;
    case 230: resp = CaeHandle::SetTempLoad(j); break;
    case 250: resp = CaeHandle::Mesh3D(j); break;
    case 260: resp = CaeHandle::SolverRun(j); break;
    default: resp = {{"status","error"},{"error","unknown cmd"}}; break;
}

// 回填cmd并发送
resp["cmd"] = cmd;
SendResponse(resp);
```

---

## 五、实体选择机制

### 5.1 核心API
```cpp
int count = 0;
szwEntityHandle* list = NULL;
int err = ZwEntityListGetByPick(prompt.c_str(), zwInputType, allowMulti, &count, &list);
if (err != 0) {
    // Fallback: 单选API (K-010)
    err = ZwEntityGetByPick(prompt.c_str(), zwInputType, &count, &list);
}

for (int i = 0; i < count; i++) {
    char name[256] = {0};
    ZwEntityNameGet(list[i], 256, name);
}
ZwMemoryFree((void**)&list);
```

### 5.2 ZW3D原生选择类型映射
| entityType | ZW3D类型 | 说明 |
|-----------|---------|------|
| 0 | `ZW_INPUT_FACE` | 面 |
| 1 | `ZW_INPUT_EDGE` | 边 |
| 2 | `ZW_INPUT_VERTEX` | 顶点 |
| 3 | `ZW_INPUT_SHAPE` | 体 |

### 5.3 非交互Fallback (K-011)
当拾取API返回-10000(无命令上下文)时:
```cpp
// 通过czwsTaskInqSimGeoms查询所有仿真几何体
// 支持entityType=0(面)/3(体)/5(网格)
```

---

## 六、cacheId机制

### 6.1 为什么用cacheId
- `szwEntityHandle`是不透明结构体，不能跨进程传输
- DLL侧建立`cacheId → vector<szwEntityHandle>`缓存
- UI后续请求只传`cacheId`，由DLL取回真实句柄

### 6.2 使用模式
```cpp
// DLL侧: 选择实体 → 建立缓存 → 返回cacheId
int cacheId = g_entityCache.size();
g_entityCache[cacheId] = entities;
resp["cacheId"] = cacheId;

// Qt侧: 后续操作用cacheId
data["cacheIds"] = {1, 2, 3};

// DLL侧: 从缓存获取句柄
auto entities = g_entityCache[cacheId];
// ... 使用完毕后清理
g_entityCache.erase(cacheId);
```

---

## 七、完整命令码表 (唯一真相源)

> 各domain skill中的号段表只列出本域条目，此处为完整表。新增命令必须在此登记。

| cmd | 功能 | CaeHandle方法 | 号段 |
|-----|------|--------------|------|
| 101 | 打开文件 | `OpenFile` | 参数化建模 |
| 102 | 查询参数 | `GetParameters` | 参数化建模 |
| 103 | 参数化建模 | `ModelOnce` | 参数化建模 |
| 104 | 保存 | `SaveFile` | 参数化建模 |
| 105 | 批量建模 | `BatchModel` | 参数化建模 |
| 200 | 创建仿真任务 | `CreateTask` | CAE任务管理 |
| 201 | 删除任务 | `DeleteTask` | CAE任务管理 |
| 202 | 查询任务列表 | `QueryTasks` | CAE任务管理 |
| 220 | 选择实体 | `SelectEntities` | CAE实体选择 |
| 230 | 设置温度载荷 | `SetTempLoad` | CAE载荷 |
| 231 | 设置热通量 | `SetHeatFlux` | CAE载荷 |
| 232 | 设置热功率 | `SetHeatPower` | CAE载荷 |
| 233 | 设置初始温度 | `SetInitialTemp` | CAE载荷 |
| 236 | 设置力载荷 | `SetForceLoad` | CAE载荷 |
| 237 | 设置力矩载荷 | `SetMomentLoad` | CAE载荷 |
| 238 | 设置线载荷 | `SetLineLoad` | CAE载荷 |
| 240 | 热传导接触 | `SetContact` | CAE约束/接触 |
| 241 | 绑定接触 | `SetBondedContact` | CAE约束/接触 |
| 243 | 固定约束 | `SetFixedConstraint` | CAE约束/接触 |
| 244 | 铰支约束 | `SetHingeConstraint` | CAE约束/接触 |
| 245 | 弹性支撑 | `SetElasticSupport` | CAE约束/接触 |
| 250 | 3D网格划分 | `Mesh3D` | CAE网格 |
| 251 | 网格质量检查 | `MeshQuality` | CAE网格 |
| 260 | 求解 | `SolverRun` | CAE求解 |
| 261 | 查询结果 | `QueryResults` | CAE求解 |
| 262 | 查询结果类型 | `QueryResultTypes` | CAE求解 |
| 280 | 原生命令 | `RunNativeCmd` | CAE材料/原生 |
| 281 | 查询仿真几何 | `QuerySimGeoms` | CAE材料/原生 |
| 282 | 查询材料 | `QueryMaterial` | CAE材料/原生 |
| 283 | 分配默认材料 | `AssignDefaultMaterial` | CAE材料/原生 |
| 300 | 查询变量列表 | `QueryVariables` | 参数化建模 |
| 301 | 批量设置变量并重建 | `SetVariablesAndRegen` | 参数化建模 |
| 302 | 查询变量状态 | `QueryVariableStatus` | 参数化建模 |
| 310 | 查询可解析目标 | `QueryTargets` | 目标解析 |
| 311 | 按规则解析目标 | `ResolveTarget` | 目标解析 |
| 313 | 校验目标规则 | `ValidateRule` | 目标解析 |
| 400 | 定义设计变量 | `OptDefineVariable` | 参数优化 |
| 401 | 定义优化目标 | `OptDefineObjective` | 参数优化 |
| 402 | 定义约束条件 | `OptDefineConstraint` | 参数优化 |
| 403 | 设置优化算法 | `OptSetAlgorithm` | 参数优化 |
| 404 | 执行优化 | `OptRun` | 参数优化 |
| 405 | 查询优化结果 | `OptQueryResult` | 参数优化 |
| 406 | 查询优化历史 | `OptQueryHistory` | 参数优化 |
| 500 | 云图PNG导出 | `ExportContourPNG` | 结果处理 |
| 501 | CSV数据导出 | `ExportResultCSV` | 结果处理 |
| 502 | 曲线导出 | `ExportResultCurve` | 结果处理 |
| 503 | 报告生成 | `GenerateReport` | 结果处理 |
| 504 | 结果查询 | `QueryResultData` | 结果处理 |
| 505 | 动画导出 | `ExportAnimation` | 结果处理 |
| 506 | 多工况查询 | `QueryMultiCase` | 结果处理 |

---

## 八、经验卡片

| 卡片 | 问题 | 关键结论 |
|------|------|---------|
| **K-001** | 跨进程实体句柄 | 用cacheId而非原始`szwEntityHandle` |
| **K-002** | IPC回包cmd混乱 | 分发层先回填`j["cmd"] = g_currentTask.type` |
| **K-003** | Named Pipe断线重连线程泄漏 | 重连前检查`m_ioThread.joinable()`并join |
| **K-004** | g_currentTask数据竞争 | WndProc读取时必须加锁 |
| **K-005** | JSON结构路径不一致 | 统一传递外层JSON，子函数内部取`j["data"]` |
| **K-006** | cvxCmdSend不识别前缀 | 区分三种模式: macro_file/macro_inline/command |
| **K-010** | 实体选择间歇失败 | 双通路降级: ZwEntityListGetByPick → ZwEntityGetByPick |
| **K-011** | 拾取返回-10000 | 检查cvxCmdFuncLoad，或fallback到查询所有SimGeom |
| **K-040** | 日志规范 | 关键调用回传: 入参、上下文、实体来源、错误码 |

> 完整经验卡片原文见 [[知识库/02_CAE开发/工程经验_结构化\|工程经验_结构化.md]]

---

## 九、编码转换

ZW3D API使用本地编码(GBK/CP_ACP)，Qt使用UTF-8:
```cpp
// DLL → ZW3D: UTF-8 → 本地编码
const char* acpStr = Utf8ToLocalAcp(utf8Str);
// ZW3D → DLL: 本地编码 → UTF-8
std::string utf8Str = LocalAcpToUtf8(acpStr);
```

---

## 十、资源

| 资源 | 路径 |
|------|------|
| IPC模板 | `sample_templates/ipc_message_template.cpp` |
| 实体解析模板 | `sample_templates/entity_target_resolver_template.cpp` |
| 代码架构 | [[知识库/02_CAE开发/代码架构\|代码架构.md]] |
| 技术路线详解 | [[知识库/02_CAE开发/Z3ParametricModeling_技术路线详解\|技术路线详解.md]] |
| 工程经验 | [[知识库/02_CAE开发/工程经验_结构化\|工程经验_结构化.md]] |
