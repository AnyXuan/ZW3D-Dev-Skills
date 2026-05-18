---
name: zw3d-cae-thermal
description: Use when implementing or debugging ZW3D 2026 steady thermal or transient thermal CAE workflows, including thermal tasks, temperature loads, heat flux, heat power, initial thermal conditions, thermal materials, mesh requirements, solve flow, and thermal result queries.
---

# ZW3D 热分析 Skill — v1.2.0

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
szwsTempLoadData param;
czwsSimStTempLoadDataInit(&param);
param.temperature = 80.0;  // °C
param.iStep = 1;
param.entNum = 1;
param.entities = &entityId;  // 面实体

zwsDbId loadIdx;
int err = czwsSimStCreateTempLoad(&param, &loadIdx);
czwsSimStTempLoadDataFree(&param);
if (err != ZWSIM_API_NO_ERROR) {
    cvxMsgDisp("Failed to create temp load");
    return;
}
```

### 3.2 热通量载荷 (cmd=231)
```cpp
szwsHtFluxData param;
czwsSimStHeatFluxDataInit(&param);
param.flux = 1000.0;  // W/m²
param.entNum = 1;
param.entities = &entityId;

zwsDbId loadIdx;
int err = czwsSimStCreateHeatFlux(&param, &loadIdx);
czwsSimStHeatFluxDataFree(&param);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 3.3 热功率载荷 (cmd=232)
```cpp
szwsHtPowerData param;
czwsSimStHeatPowerDataInit(&param);
param.power = 50.0;  // W
param.entNum = 1;
param.entities = &entityId;  // 体实体

zwsDbId loadIdx;
int err = czwsSimStCreateHeatPower(&param, &loadIdx);
czwsSimStHeatPowerDataFree(&param);
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

### 3.5 绑定接触 (cmd=241)
```cpp
szwsBondedContactData param;
czwsSimStBondedContactDataInit(&param);
param.masterEntNum = 1;
param.sMasterEntities = &masterId;
param.slaveEntNum = 1;
param.sSlaveEntities = &slaveId;

zwsDbId contactIdx;
int err = czwsSimStCreateBondedContact(&param, &contactIdx);
czwsSimStBondedContactDataFree(&param);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 3.6 初始温度 (cmd=233)
```cpp
szwsInitTempData param;
czwsSimStInitTempDataInit(&param);
param.temperature = 25.0;
param.entNum = 1;
param.entities = &entityId;

zwsDbId idxInitTemp;
int err = czwsSimStCreateInitialTemp(&param, &idxInitTemp);
czwsSimStFreeInitTempData(&param);
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
| 绑定接触 | 241 | `SetBondedContact` |
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
