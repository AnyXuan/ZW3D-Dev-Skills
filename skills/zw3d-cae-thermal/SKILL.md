---
name: zw3d-cae-thermal
description: Use when implementing or debugging ZW3D 2026 steady thermal or transient thermal CAE workflows, including thermal tasks, temperature loads, heat flux, heat power, initial thermal conditions, thermal materials, mesh requirements, solve flow, and thermal result queries.
---

# ZW3D 热分析 Skill — v1.3.0

> **触发词**: "稳态热", "瞬态热", "温度载荷", "热通量", "热仿真", "thermal", "heat flux", "热传导", "热功率", "对流", "辐射"
> **适用**: ZW3D 2026 ZWMeshWorks 稳态/瞬态热仿真
> **维护者**: 韩天尊
> **依赖**: [[zw3d-plugin-base\|plugin-base]] (必需), [[zw3d-cae-mesh\|cae-mesh]] (必需), [[zw3d-ipc-comm\|ipc-comm]] (必需)

### 权威来源引用
| 内容 | 引用 |
|------|------|
| C++ API调用规范 | [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md]] |
| IPC消息协议 | [[.workbuddy/skills/zw3d-dev-team/docs/02-ipc-protocol\|02-ipc-protocol.md]] |
| 质量门控 | [[.workbuddy/skills/zw3d-dev-team/docs/04-quality-gate\|04-quality-gate.md]] |
| 完整命令码表 | [[zw3d-ipc-comm#七完整命令码表\|ipc-comm §七]] |
| 经验卡片原文 | [[知识库/02_CAE开发/工程经验_结构化\|工程经验_结构化.md]] |

---

## 一、任务类型

| 类型 | 字符串标识 | 说明 |
|------|-----------|------|
| 稳态热 | `"steady_thermal"` | 稳态温度场分析，与时间无关 |
| 瞬态热 | `"transient_thermal"` | 随时间变化的温度场 |

### 创建任务
```cpp
zwsDbId idxTask;
ZWSimEvxErrors err = czwsSimStCreateTask(ZW_ST_TASK_STEADY_THERMAL, &idxTask);
if (err != ZWSIM_API_NO_ERROR) {
    cvxMsgDisp("Failed to create thermal task");
    return;  /* idxTask未初始化，不可使用 */
}
```

---

## 二、材料要求

| 属性 | 稳态热 | 瞬态热 |
|------|--------|--------|
| 导热系数 | ✅ 必需 | ✅ 必需 |
| 比热容 | ❌ | ✅ 必需 |
| 密度 | ❌ | ✅ 必需 |

> 瞬态热必须设置比热容和密度，因为需要计算热容 C = ρ·Cp·V。
> 材料分配: `czwsSimGeomSetMaterial(simGeomId, materialId)`
> 默认材料: cmd=283，为未绑定材料的几何体创建默认稳态热材料

---

## 三、载荷类型 (核心)

### 3.1 温度载荷 (cmd=230)
```cpp
szwsTempLoadData loadData;
czwsSimStTemperatureLoadDataInit(&loadData);
loadData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
loadData.iEntNum = 1;
loadData.sEntities = &entityId;  // 面实体
loadData.sPoints.data = nullptr;
loadData.iNodeIds = nullptr;
loadData.dValue = 80.0;          // °C
strncpy_s(loadData.unitValue, sizeof(loadData.unitValue), "Celsius", _TRUNCATE);

zwsDbId loadIdx;
ZWSimEvxErrors err = czwsSimStCreateTemperatureLoad(&loadData, &loadIdx);
czwsSimStFreeTemperatureLoadData(&loadData);
if (err != ZWSIM_API_NO_ERROR) {
    cvxMsgDisp("Failed to create temperature load");
    return;
}
```

### 3.2 热通量载荷 (cmd=231)
```cpp
szwsHtFluxData loadData;
czwsSimStHeatFluxLoadDataInit(&loadData);
loadData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
loadData.iEntNum = 1;
loadData.sEntities = &entityId;
loadData.dValue = 1000.0;  // W/m²

zwsDbId loadIdx;
ZWSimEvxErrors err = czwsSimStCreateHeatFluxLoad(&loadData, &loadIdx);
czwsSimStFreeHeatFluxLoadData(&loadData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 3.3 热功率载荷 (cmd=232)
```cpp
szwsHtPowerData loadData;
czwsSimStHeatPowerLoadDataInit(&loadData);
loadData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
loadData.iEntNum = 1;
loadData.sEntities = &entityId;  // 体实体
loadData.dValue = 50.0;          // W

zwsDbId loadIdx;
ZWSimEvxErrors err = czwsSimStCreateHeatPowerLoad(&loadData, &loadIdx);
czwsSimStFreeHeatPowerLoadData(&loadData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 3.4 热传导接触 (cmd=240)
```cpp
szwsContactData param;
czwsSimStContactDataInit(&param);
param.contactType = ZW_ST_CONTACT_TYPE_HEATCONDUCTION;
param.conductance = 1000.0;
param.masterEntNum = 1;
param.sMasterEntities = &masterId;
param.slaveEntNum = 1;
param.sSlaveEntities = &slaveId;

zwsDbId contactIdx;
int err = czwsSimStCreateContact(&param, &contactIdx);
czwsSimStContactDataFree(&param);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 3.5 绑定接触 (cmd=239)
```cpp
szwsBondedContactData param;
czwsSimStBondedContactDataInit(&param);
param.masterEntNum = 1;
param.sMasterEntities = &masterId;
param.slaveEntNum = 1;
param.sSlaveEntities = &slaveId;

zwsDbId contactIdx;
ZWSimEvxErrors err = czwsSimStCreateBondedContact(&param, &contactIdx);
// 无对应 Free API，无需手动释放
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 3.6 初始温度 (cmd=233)
```cpp
szwsInitTempData loadData;
czwsSimStInitialTemperatureDataInit(&loadData);
loadData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
loadData.iEntNum = 1;
loadData.sEntities = &entityId;
loadData.dValue = 25.0;
strncpy_s(loadData.unitValue, sizeof(loadData.unitValue), "Celsius", _TRUNCATE);

zwsDbId idxInitTemp;
ZWSimEvxErrors err = czwsSimStCreateInitialTemperature(&loadData, &idxInitTemp);
czwsSimStFreeInitialTemperatureData(&loadData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

---

## 四、瞬态热特有

### 4.1 初始热效应
```cpp
szwsInitThermEffectData initThermData;
czwsSimStInitialThermalEffectDataInit(&initThermData);
initThermData.iStep = 0;

zwsDbId idxInitTherm;
int err = czwsSimStCreateInitialThermalEffect(&initThermData, &idxInitTherm);
czwsSimStFreeInitialThermalEffectData(&initThermData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 4.2 时间步设置
```cpp
czwsSimGeomSetSimProperty(simGeomId, "TimeStep", "0.1");
czwsSimGeomSetSimProperty(simGeomId, "TotalTime", "100.0");
```

### 4.3 瞬态热 vs 稳态热差异
| 特性 | 稳态热 | 瞬态热 |
|------|--------|--------|
| 温度载荷 | 固定值 | 可时间相关 |
| 初始温度 | 可选 | 必须设置(t=0) |
| 初始热效应 | 不需要 | 必需 |
| 材料要求 | 导热系数 | 导热系数+比热容+密度 |

---

## 五、稳态热完整工作流

```
Step 1  创建稳态热任务    czwsSimStCreateTask(ZW_ST_TASK_STEADY_THERMAL)
Step 2  查询仿真几何      czwsTaskInqSimGeoms
Step 3  分配材料         czwsSimGeomSetMaterial
Step 4  ⚠️ 预激活Group 10  ← 热载荷必须! 参见 §八 K-007
Step 5  设置温度载荷      czwsSimStCreateTempLoad
Step 6  设置热通量载荷    czwsSimStCreateHeatFlux
Step 7  设置热功率载荷    czwsSimStCreateHeatPower
Step 8  设置初始温度      czwsSimStCreateInitialTemp (可选)
Step 9  设置接触         czwsSimStCreateContact / CreateBondedContact
Step 10 网格划分         czwsMeshing3D (参见 [[zw3d-cae-mesh\|cae-mesh §三]])
Step 11 求解            czwsSimCalculate
Step 12 查询结果        czwsResultTypeInqData
```

> ⚠️ **关键**: Step 4 热载荷预激活是必须的，否则载荷设置静默失败。参见 §八 K-007。

### IPC命令对应 (完整表见 [[zw3d-ipc-comm#七完整命令码表\|ipc-comm §七]])
| 步骤 | cmd | 方法 |
|------|-----|------|
| 创建任务 | 200 | `CreateTask` |
| 选择实体 | 220/221/224 | `SelectEntities` |
| 温度载荷 | 230 | `SetTempLoad` |
| 热通量 | 231 | `SetHeatFlux` |
| 热功率 | 232 | `SetHeatPower` |
| 初始温度 | 233 | `SetInitialTemp` |
| 热传导接触 | 240 | `SetContact` |
| 绑定接触 | 239 | `SetBondedContact` |
| 网格划分 | 250 | `Mesh3D` |
| 求解 | 260 | `SolverRun` |
| 查询结果 | 261 | `QueryResults` |

---

## 六、瞬态热完整工作流

```
Step 1  创建瞬态热任务    czwsSimStCreateTask(ZW_ST_TASK_TRANSIENT_THERMAL)
Step 2  查询仿真几何      czwsTaskInqSimGeoms
Step 3  分配材料         czwsSimGeomSetMaterial (导热系数+比热容+密度)
Step 4  设置初始温度      czwsSimStCreateInitialTemp
Step 5  设置初始热效应    czwsSimStCreateInitialThermalEffect (iStep=0)
Step 6  ⚠️ 预激活Group 10  ← 热载荷必须! 参见 §八 K-007
Step 7  施加载荷         czwsSimStCreateTempLoad / HeatFlux / HeatPower
Step 8  设置时间步参数    czwsSimGeomSetSimProperty
Step 9  网格划分         czwsMeshing3D
Step 10 求解            czwsSimCalculate
Step 11 查询结果        czwsResultTypeInqData (含时间步索引)
```

---

## 七、结果查询

> 完整结果查询API见 [[zw3d-cae-post#二结果查询api\|cae-post §二]]。此处仅列热分析常用。

```cpp
// 查询结果类型
int numTypes = 0;
char** types = NULL;
int err = czwsResultInqTypes(256, &types, &numTypes);

// 查询温度数据
int count = 0;
void* pResults = NULL;
int err = czwsResultTypeInqData(taskId, "Temperature", 0, &count, &pResults);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }

szwResultNodalData* result = (szwResultNodalData*)pResults;
for (int i = 0; i < count; i++) {
    // result[i].entityId, result[i].value
}

cvxMemFree((void**)&pResults);
```

---

## 七B、热-结构耦合工作流 (cmd=295/296)

> 源码: `Zw3dPMBridge/CaeTaskOps.cpp` → `CopyAndConvertTask` + `SetThermalEffects`
> 模式来源: chihaya demo.txt → `CalculateStaticTask` / `SetThermalEffectsLoad`

### 7B.1 概述

热-结构耦合分析将热仿真结果作为结构仿真的温度载荷输入:
1. 先完成热分析 (稳态热/瞬态热)
2. 复制热任务并转换为结构任务 (cmd=295)
3. 导入热效应到结构任务 (cmd=296)

### 7B.2 复制并转换任务 (cmd=295)

```cpp
// 模式: czwsPartDuplicateTask → czwsRegenTree → czwsPartSetActiveTask → czwsSimStSwitchTask → czwsRegenTree
zwsDbId srcTask = /* 热任务ID */;
zwsDbId newTask = {};

// Step 1: 复制任务
czwsPartDuplicateTask(srcTask, &newTask);
czwsRegenTree();

// Step 2: 激活新任务
czwsPartSetActiveTask(newTask);

// Step 3: 切换分析类型 (0=Static, 5=Static in demo)
czwsSimStSwitchTask(static_cast<ezwsAnsysType>(0));
czwsRegenTree();
```

JSON 请求:
```json
{
  "cmd": 295,
  "data": {
    "sourceTaskId": 12345,
    "ansysType": 0
  }
}
```

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `sourceTaskId` | uintptr_t | 0 (使用当前激活任务) | 热任务ID |
| `ansysType` | int | 0 | 0=Static, 1=Nonlinear Static, 6=Frequency |

### 7B.3 导入热效应 (cmd=296)

```cpp
// 模式: czwsSimStThermalEffectsInit → 设置字段 → czwsSimStCreateThermalEffects → czwsSimStFreeThermalEffectsData
szwsThermalEffectData payload;
czwsSimStThermalEffectsInit(&payload);
payload.sTaskHandle = thermalTask;  // 热任务句柄
payload.itempStep = -1;             // -1=最后一步
payload.bMatchTimeStep = false;

zwsDbId resultId = {};
ZWSimEvxErrors err = czwsSimStCreateThermalEffects(&payload, &resultId);
czwsSimStFreeThermalEffectsData(&payload);
czwsRegenTree();
```

JSON 请求:
```json
{
  "cmd": 296,
  "data": {
    "thermalTaskId": 12345,
    "tempStep": -1,
    "matchTimeStep": false
  }
}
```

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `thermalTaskId` | uintptr_t | **必需** | 源热任务ID |
| `tempStep` | int | -1 | -1=最后一步, 0+=指定步 |
| `matchTimeStep` | bool | false | 是否匹配时间步 |

### 7B.4 完整耦合流程

```
Step 1  创建热任务        cmd=200 (type=steady_thermal)
Step 2  设置材料+载荷+网格   cmd=230/231/232 + cmd=250
Step 3  求解热分析        cmd=260
Step 4  复制并转换为结构    cmd=295 (ansysType=0)
Step 5  导入热效应        cmd=296 (thermalTaskId=热任务ID)
Step 6  设置结构载荷/约束   cmd=220 + 结构特有操作
Step 7  网格+求解结构      cmd=250 + cmd=260
Step 8  查询结构结果       cmd=261
```

---

## 八、经验卡片

### K-007: 热载荷必须预激活Group 10 (HIGH)

**现象**: `cvxCmdSend("!ZwThTempLoad")` 执行后ZW3D无任何反应。

**根因**: 热载荷命令属于UiCaeManager Group 10，必须先发送激活事件。

**解决方案**: 使用内联宏包含完整事件序列:
```cpp
char macro[512];
sprintf_s(macro, sizeof(macro),
    "[vxSendEvt,\"UiCaeManager\",1,10,25,4]\n"
    "[vxSendEvt,\"UiCaeManager\",1,10,4,4]\n"
    "[vxSend,\"!ZwThTempLoad\"]\n"
    "[vxSendEvt,\"UiCaeManager\",1,10,25,8]\n"
    "[vxSendEvt,\"UiCaeManager\",1,10,4,8]\n");
cvxCmdMacro(macro, &output);
```

**关键发现**:
- 网格命令(`ZwCeMeshing`) **不需要**预激活
- 热载荷命令 **必须** 先激活Group 10
- 同一Group内连续命令无需重复激活

### K-031: vxInitCmd参数编码未破解

参数70/71编码规则未破解(温度100°C→`<70,127><71,100>`)，先弹窗让用户手动输入。

### K-020: 网格error -32

API `czwsTaskMesh3D` 在某些环境返回error -32。fallback到`cvxCmdSend("ZwCeMeshing")`原生命令，成功率0%→90%+。详见 [[zw3d-cae-mesh#六经验卡片\|cae-mesh §六]]。

> 完整经验卡片原文见 [[知识库/02_CAE开发/工程经验_结构化\|工程经验_结构化.md]]

---

## 九、已验证原生命令

| 命令名 | cvxCmdSend | cvxCmdMacro | UiCaeManager |
|--------|-----------|-------------|-------------|
| `ZwCeMeshing` | ✅ 直接可用 | ✅ | 不需要 |
| `ZwThTempLoad` | ❌ 需预激活 | ✅ 推荐 | Group 10 |
| `ZwThHtFluxLoad` | ❌ 需预激活 | ✅ 推荐 | Group 10 |
| `ZwThHtPowLoad` | ❌ 需预激活 | ✅ 推荐 | Group 10 |

---

## 十、已知限制

| 限制 | 说明 |
|------|------|
| 对流/辐射载荷 | 需通过原生命令`ZwThConvLoad`/`ZwThRadLoad`实现 |
| 时间步设置API | 文档未完全暴露，可能需原生命令通道 |
| 非线性材料属性 | 温度相关的导热系数/比热容需确认API支持 |

---

## 十一、资源

| 资源 | 路径 |
|------|------|
| 稳态热模板 | `sample_templates/SteadyThermalAPI_wrapper.cpp` |
| 瞬态热模板 | `sample_templates/TransientThermalAPI_wrapper.cpp` |
| 开发计划 | [[知识库/02_CAE开发/稳态传热模块开发计划\|稳态传热模块开发计划.md]] |
| CAE功能清单 | [[知识库/02_CAE开发/ZWMeshWorks_CAE_插件_功能清单\|功能清单.md]] |
| 工程经验 | [[知识库/02_CAE开发/工程经验_结构化\|工程经验_结构化.md]] |
