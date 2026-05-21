---
name: zw3d-ipc-comm
description: Use when working on ZW3D Qt EXE to ZW3D DLL communication, Named Pipe IPC, JSON command protocols, cacheId/entity handle handoff, PostMessage worker dispatch, target resolution, or frontend-to-bridge command routing.
---

# ZW3D 双进程通信 Skill — v1.3.0

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
> 命令ID来源: `Common/define.h` TaskType 枚举。
> 标注 `📐 设计草案` 的条目尚未在 `define.h` 中注册，仅作为设计参考。

### 参数化建模 (100-199)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 101 | `TASK_OPEN_FILE` | 打开文件 | `OpenFile` |
| 102 | `TASK_MODEL_PARAMS` | 查询参数 | `GetParameters` |
| 103 | `TASK_MODEL_ONCE` | 参数化建模 | `ModelOnce` |
| 104 | `TASK_MODEL_SAVE` | 保存 | `SaveFile` |
| 105 | `TASK_BATCH_MODEL` | 批量建模 | `BatchModel` |
| 106 | `TASK_BATCH_MODEL_END` | 批量建模结束 | `BatchModelEnd` |

### CAE 任务管理 (200-209)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 200 | `CAE_CREATE_TASK` | 创建仿真任务 | `CreateTask` |
| 201 | `CAE_DELETE_TASK` | 删除任务 | `DeleteTask` |
| 202 | `CAE_QUERY_TASKS` | 查询任务列表 | `QueryTasks` |
| 203 | `CAE_QUERY_ACTIVE_TASK` | 查询当前激活任务 | `QueryActiveTask` |

### CAE 仿真几何与网格查询 (210-219)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 210 | `CAE_QUERY_SIMGEOMS` | 查询仿真几何体 | `QuerySimGeoms` |
| 211 | `CAE_QUERY_MESHES` | 查询网格列表 | `QueryMeshes` |
| 212 | `CAE_QUERY_ACTIVE_MESH` | 查询当前激活网格 | `QueryActiveMesh` |

### CAE 实体选择 (220-225)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 220 | `CAE_SELECT_ENTITIES` | 选择通用实体 | `SelectEntities` |
| 221 | `CAE_SELECT_FACES` | 选择面 | `SelectFaces` |
| 222 | `CAE_SELECT_EDGES` | 选择边 | `SelectEdges` |
| 223 | `CAE_SELECT_VERTICES` | 选择顶点 | `SelectVertices` |
| 224 | `CAE_SELECT_BODIES` | 选择体 | `SelectBodies` |

### CAE 热载荷 (230-239)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 230 | `CAE_SET_TEMP_LOAD` | 设置温度载荷 | `SetTempLoad` |
| 231 | `CAE_SET_HEAT_FLUX` | 设置热通量 | `SetHeatFlux` |
| 232 | `CAE_SET_HEAT_POWER` | 设置热功率 | `SetHeatPower` |
| 233 | `CAE_SET_INITIAL_TEMP` | 设置初始温度 | `SetInitialTemp` |
| 234 | `CAE_SET_FIXED_TEMP` | 设置固定温度边界 | `SetFixedTemp` |
| 239 | `CAE_SET_BONDED_CONTACT` | 设置绑定接触 | `SetBondedContact` |

### CAE 接触与约束 (240-249)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 240 | `CAE_SET_CONTACT` | 热传导接触 | `SetContact` |
| 241 | `CAE_SET_BONDED_CONTACT_LEGACY` | 绑定接触(旧) | `SetBondedContactLegacy` |

> ⚠️ 注意: cmd=239 和 cmd=241 都是绑定接触，cmd=239 为 `define.h` 中的当前定义。

### CAE 网格 (250-259)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 250 | `CAE_MESH_3D` | 3D网格划分 | `Mesh3D` |
| 251 | `CAE_MESH_QUALITY` | 网格质量检查 | `MeshQuality` |
| 252 | `CAE_MESH_FIX` | 网格修复 | `MeshFix` |
| 253 | `CAE_MESH_MERGE_NODES` | 合并重合节点 | `MeshMergeNodes` |

### CAE 求解与结果 (260-269)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 260 | `CAE_SOLVER_RUN` | 求解 | `SolverRun` |
| 261 | `CAE_RESULT_QUERY` | 查询结果 | `QueryResults` |
| 262 | `CAE_RESULT_TYPES` | 查询结果类型 | `QueryResultTypes` |
| 263 | `CAE_RESULT_SUBTYPES` | 查询结果子类型 | `QueryResultSubtypes` |
| 264 | `CAE_RESULT_EXPORT` | 结果导出 | `ResultExport` |

### CAE 网格自适应 (270-279)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 270 | `CAE_MESH_ADAPTIVE` | 自适应网格 | `MeshAdaptive` |

### CAE 材料与原生命令 (280-289)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 280 | `CAE_OPEN_NATIVE_HEAT_LOAD` | 打开原生热载荷面板 | `RunNativeCmd` |
| 281 | `CAE_QUERY_ALL_FACES` | 查询所有面 | `QueryAllFaces` |
| 282 | `CAE_QUERY_MATERIALS` | 查询材料 | `QueryMaterials` |
| 283 | `CAE_ASSIGN_DEFAULT_MATERIAL` | 分配默认材料 | `AssignDefaultMaterial` |
| 284 | `CAE_CREATE_MATERIAL` | 创建材料 | `CreateMaterial` |
| 285 | `CAE_DELETE_MATERIAL` | 删除材料 | `DeleteMaterial` |
| 286 | `CAE_SET_MATERIAL` | 设置材料 | `SetMaterial` |

### CAE 热效应与时间步 (290-296)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 290 | `CAE_SET_INITIAL_THERMAL_EFFECT` | 设置初始热效应 | `SetInitialThermalEffect` |
| 291 | `CAE_SET_TIME_STEP` | 设置时间步 | `SetTimeStep` |
| 295 | `CAE_COPY_CONVERT_TASK` | 复制并转换任务(热→结构耦合) | `CopyAndConvertTask` |
| 296 | `CAE_SET_THERMAL_EFFECTS` | 导入热效应(热→结构耦合) | `SetThermalEffects` |

### 参数化建模自动化 (300-313)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 300 | `AUTO_QUERY_VARIABLES` | 查询变量列表 | `QueryVariables` |
| 301 | `AUTO_SET_VARIABLES` | 批量设置变量并重建 | `SetVariablesAndRegen` |
| 302 | `AUTO_QUERY_VARIABLE_STATUS` | 查询变量状态 | `QueryVariableStatus` |
| 303 | `AUTO_QUERY_CAD_MATERIALS` | 查询CAD材料 | `QueryCADMaterials` |
| 310 | `AUTO_QUERY_TARGET_OVERVIEW` | 查询可解析目标 | `QueryTargets` |
| 311 | `AUTO_RESOLVE_TARGET` | 按规则解析目标 | `ResolveTarget` |
| 312 | `AUTO_HIGHLIGHT_TARGET` | 高亮目标 | `HighlightTarget` |
| 313 | `AUTO_VALIDATE_TARGET` | 校验目标规则 | `ValidateRule` |

### 静力学特有 (320-338)

| cmd | 枚举名 | 功能 | CaeHandle方法 |
|-----|--------|------|--------------|
| 320 | `STATIC_CREATE_SIM` | 创建静力学仿真 | `StaticCreateSim` |
| 321 | `STATIC_SELECT_PART` | 选择部件 | `StaticSelectPart` |
| 322 | `STATIC_MESH_3D` | 静力学网格 | `StaticMesh3D` |
| 323 | `STATIC_CREATE_FORCE` | 创建力载荷 | `StaticCreateForce` |
| 324 | `STATIC_CREATE_FIXED` | 创建固定约束 | `StaticCreateFixed` |
| 325 | `STATIC_GET_PARAM` | 获取参数 | `StaticGetParam` |
| 326 | `STATIC_SET_PARAM` | 设置参数 | `StaticSetParam` |
| 327 | `STATIC_GET_ENTITY` | 获取实体 | `StaticGetEntity` |
| 328 | `STATIC_SELECT_FORCE_ENTITY` | 选择力实体 | `StaticSelectForceEntity` |
| 329 | `STATIC_CREATE_MATERIAL` | 创建材料 | `StaticCreateMaterial` |
| 330 | `STATIC_DELETE_MATERIAL` | 删除材料 | `StaticDeleteMaterial` |
| 331 | `STATIC_SET_MATERIAL` | 设置材料 | `StaticSetMaterial` |
| 332 | `STATIC_START_SIMULATE` | 开始仿真 | `StaticStartSimulate` |
| 333 | `STATIC_VERIFY_SETUP` | 验证设置 | `StaticVerifySetup` |
| 334 | `STATIC_CREATE_BONDED` | 创建绑定接触 | `StaticCreateBonded` |
| 335 | `STATIC_SELECT_MASTER` | 选择主面 | `StaticSelectMaster` |
| 336 | `STATIC_SELECT_SLAVE` | 选择从面 | `StaticSelectSlave` |
| 337 | `STATIC_DELETE_BONDED` | 删除绑定接触 | `StaticDeleteBonded` |
| 338 | `STATIC_INQ_BONDED` | 查询绑定接触 | `StaticInqBonded` |

### 📐 设计草案 — 参数优化 (400-406)

> ⚠️ 以下命令尚未在 `define.h` 中注册，仅作为设计参考。

| cmd | 功能 | CaeHandle方法 |
|-----|------|--------------|
| 400 | 定义设计变量 | `OptDefineVariable` |
| 401 | 定义优化目标 | `OptDefineObjective` |
| 402 | 定义约束条件 | `OptDefineConstraint` |
| 403 | 设置优化算法 | `OptSetAlgorithm` |
| 404 | 执行优化 | `OptRun` |
| 405 | 查询优化结果 | `OptQueryResult` |
| 406 | 查询优化历史 | `OptQueryHistory` |

### 📐 设计草案 — 结果处理 (500-506)

> ⚠️ 以下命令尚未在 `define.h` 中注册，仅作为设计参考。

| cmd | 功能 | CaeHandle方法 |
|-----|------|--------------|
| 500 | 云图PNG导出 | `ExportContourPNG` |
| 501 | CSV数据导出 | `ExportResultCSV` |
| 502 | 曲线导出 | `ExportResultCurve` |
| 503 | 报告生成 | `GenerateReport` |
| 504 | 结果查询 | `QueryResultData` |
| 505 | 动画导出 | `ExportAnimation` |
| 506 | 多工况查询 | `QueryMultiCase` |

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

---

*文档版本: v1.3.0 | 维护者: 韩天尊*
