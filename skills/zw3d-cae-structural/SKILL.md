---
name: zw3d-cae-structural
description: Use when implementing or debugging ZW3D 2026 structural CAE workflows, including static analysis, modal analysis, buckling, force loads, constraints, contacts, stress/strain/displacement results, and structural task setup through the bridge.
---

# ZW3D 结构分析 Skill — v1.3.0

> **触发词**: "结构分析", "静力学", "模态分析", "屈曲分析", "力载荷", "约束", "接触", "structural", "static analysis", "modal", "buckling", "应力", "位移"
> **适用**: ZW3D 2026 ZWMeshWorks 静力/模态/屈曲分析
> **维护者**: 韩天尊
> **依赖**: [[zw3d-plugin-base\|plugin-base]] (必需), [[zw3d-cae-mesh\|cae-mesh]] (必需), [[zw3d-ipc-comm\|ipc-comm]] (必需)

### 权威来源引用
| 内容 | 引用 |
|------|------|
| C++ API调用规范 | [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md]] |
| IPC消息协议 | [[.workbuddy/skills/zw3d-dev-team/docs/02-ipc-protocol\|02-ipc-protocol.md]] |
| 质量门控 | [[.workbuddy/skills/zw3d-dev-team/docs/04-quality-gate\|04-quality-gate.md]] |
| 完整命令码表 | [[zw3d-ipc-comm#七完整命令码表\|ipc-comm §七]] |

---

## 一、任务类型

| 类型 | 字符串标识 | 枚举值 | 说明 |
|------|-----------|--------|------|
| 静力分析 | `"static"` | `ZW_ST_TASK_STATIC` (0) | 线性静力学 |
| 模态分析 | `"frequency"` | `ZW_ST_TASK_FREQUENCY` (1) | 固有频率和振型 |
| 屈曲分析 | `"buckling"` | `ZW_ST_TASK_BUCKLING` (2) | 临界载荷屈曲 |

### 创建任务 (cmd=200)
```cpp
zwsDbId idxTask;
ZWSimEvxErrors err = czwsSimStCreateTask(ZW_ST_TASK_STATIC, &idxTask);
if (err != ZWSIM_API_NO_ERROR) {
    cvxMsgDisp("Failed to create static task");
    return;  /* idxTask未初始化 */
}
```

---

## 二、材料要求

| 分析类型 | 必需材料属性 |
|---------|------------|
| 静力分析 | **弹性模量 + 泊松比** (+ 密度用于自重) |
| 模态分析 | **弹性模量 + 泊松比 + 密度** |
| 屈曲分析 | **弹性模量 + 泊松比** |

---

## 三、载荷类型

### 3.1 力载荷 (cmd=236) 📐 设计草案
> ⚠️ cmd=236 未在 `Common/define.h` 中注册，以下API签名基于 CaeApiWrapper.h 推断，待实现后验证。

```cpp
szwsForceLoadData forceData;
czwsSimStForceLoadDataInit(&forceData);
forceData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
forceData.iEntNum = 1;
forceData.sEntities = &entityId;
forceData.dValue = 100.0;         // 力的大小 (N)
forceData.dDirection[0] = 1.0;    // X方向分量
forceData.dDirection[1] = 0.0;    // Y方向分量
forceData.dDirection[2] = 0.0;    // Z方向分量

zwsDbId idxLoad;
int err = czwsSimStCreateForceLoad(&forceData, &idxLoad);
czwsSimStFreeForceLoadData(&forceData);
if (err != ZWSIM_API_NO_ERROR) {
    cvxMsgDisp("Failed to create force load");
    return;
}
```

### 3.2 力矩载荷 (cmd=237) 📐 设计草案
> ⚠️ cmd=237 未在 `Common/define.h` 中注册，以下API签名基于 CaeApiWrapper.h 推断，待实现后验证。
```cpp
szwsMomentLoadData momentData;
czwsSimStMomentLoadDataInit(&momentData);
momentData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
momentData.iEntNum = 1;
momentData.sEntities = &entityId;
momentData.dX = 0.0;
momentData.dY = 50.0;  // N·m
momentData.dZ = 0.0;

zwsDbId idxLoad;
int err = czwsSimStCreateMomentLoad(&momentData, &idxLoad);
czwsSimStFreeMomentLoadData(&momentData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 3.3 线载荷/分布力 (cmd=238) 📐 设计草案
> ⚠️ cmd=238 未在 `Common/define.h` 中注册，以下API签名基于 CaeApiWrapper.h 推断，待实现后验证。
```cpp
szwsLineLoadData lineData;
czwsSimStLineLoadDataInit(&lineData);
lineData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
lineData.iEntNum = 1;
lineData.sEntities = &entityId;  // 边实体
lineData.dValue = 10.0;  // N/m

zwsDbId idxLoad;
int err = czwsSimStCreateLineLoad(&lineData, &idxLoad);
czwsSimStFreeLineLoadData(&lineData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 3.4 压力载荷
```cpp
szwsPressureLoadData pressureData;
czwsSimStPressureLoadDataInit(&pressureData);
pressureData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
pressureData.iEntNum = 1;
pressureData.sEntities = &entityId;  // 面实体
pressureData.dValue = 0.5;  // MPa

zwsDbId idxLoad;
int err = czwsSimStCreatePressureLoad(&pressureData, &idxLoad);
czwsSimStFreePressureLoadData(&pressureData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

---

## 四、约束类型

### 4.1 固定约束 (cmd=243) 📐 设计草案
> ⚠️ cmd=243 未在 `Common/define.h` 中注册，以下API签名基于 CaeApiWrapper.h 推断，待实现后验证。

```cpp
szwsFixedGeometryData fixData;
czwsSimStFixedGeometryDataInit(&fixData);
fixData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
fixData.iEntNum = 1;
fixData.sEntities = &entityId;

zwsDbId idxConstraint;
int err = czwsSimStCreateFixedGeometry(&fixData, &idxConstraint);
czwsSimStFreeFixedGeometryConstraintData(&fixData);
if (err != ZWSIM_API_NO_ERROR) {
    cvxMsgDisp("Failed to create fixed constraint");
    return;
}
```

### 4.2 铰支约束 (cmd=244) 📐 设计草案
> ⚠️ cmd=244 未在 `Common/define.h` 中注册，以下API签名基于 CaeApiWrapper.h 推断，待实现后验证。
```cpp
szwsFixedHingeConstraintData hingeData;
czwsSimStFixedHingeConstraintDataInit(&hingeData);
hingeData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
hingeData.iEntNum = 1;
hingeData.sEntities = &entityId;
hingeData.dX = 1.0;  // 铰链轴线方向
hingeData.dY = 0.0;
hingeData.dZ = 0.0;

zwsDbId idxConstraint;
int err = czwsSimStCreateFixedHingeConstraint(&hingeData, &idxConstraint);
czwsSimStFreeFixedHingeConstraintData(&hingeData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 4.3 弹性支撑 (cmd=245) 📐 设计草案
> ⚠️ cmd=245 未在 `Common/define.h` 中注册，以下API签名基于 CaeApiWrapper.h 推断，待实现后验证。
```cpp
szwsElasticSupportConstraintData elasticData;
czwsSimStElasticSupportConstraintDataInit(&elasticData);
elasticData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
elasticData.iEntNum = 1;
elasticData.sEntities = &entityId;
elasticData.dStiffness = 1000.0;  // N/m
elasticData.eDist = ZW_ST_SUPPORT_UNIFORM;

zwsDbId idxConstraint;
int err = czwsSimStCreateElasticSupport(&elasticData, &idxConstraint);
czwsSimStFreeElasticSupportConstraintData(&elasticData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 4.4 位移约束
```cpp
szwsDisplacementConstraintData dispData;
czwsSimStDisplacementConstraintDataInit(&dispData);
dispData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
dispData.iEntNum = 1;
dispData.sEntities = &entityId;
dispData.dX = 0.0;  // X方向位移(mm)
dispData.dY = 0.0;
dispData.dZ = 0.0;

zwsDbId idxConstraint;
int err = czwsSimStCreateDisplacementConstraint(&dispData, &idxConstraint);
czwsSimStFreeDisplacementConstraintData(&dispData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

---

## 五、接触类型

| 接触类型 | 枚举值 | 说明 |
|---------|--------|------|
| 标准接触 | `ZW_ST_CONTACT_TYPE_CONTACT` | 允许分离，摩擦/无摩擦 |
| 粗糙接触 | `ZW_ST_CONTACT_TYPE_ROUGH` | 无滑移(无限摩擦) |
| 粘合接触 | `ZW_ST_CONTACT_TYPE_ADHESIVE` | 粘接，不允许分离 |
| 通用接触 | `ZW_ST_CONTACT_TYPE_GENERAL` | 自定义接触定义 |

---

## 六、静力分析工作流

```
Step 1  创建静力任务    czwsSimStCreateTask(ZW_ST_TASK_STATIC)
Step 2  查询仿真几何    czwsTaskInqSimGeoms
Step 3  分配材料       czwsSimGeomSetMaterial (弹性模量+泊松比)
Step 4  施加固定约束    czwsSimStCreateFixedGeometry
Step 5  施加铰支约束    czwsSimStCreateFixedHingeConstraint
Step 6  施加弹性支撑    czwsSimStCreateElasticSupport
Step 7  施加力载荷     czwsSimStCreateForceLoad
Step 8  施加力矩载荷    czwsSimStCreateMomentLoad
Step 9  施加压力载荷    czwsSimStCreatePressureLoad
Step 10 设置接触       czwsSimStCreateContact
Step 11 网格划分      czwsMeshing3D (参见 [[zw3d-cae-mesh\|cae-mesh §三]])
Step 12 求解         czwsSimCalculate
Step 13 查询结果     czwsResultTypeInqData
```

### IPC命令对应 (完整表见 [[zw3d-ipc-comm#七完整命令码表\|ipc-comm §七]])
| 步骤 | cmd | 方法 | 状态 |
|------|-----|------|------|
| 创建任务 | 200 | `CreateTask` | |
| 选择实体 | 220/221/224 | `SelectEntities` | |
| 力载荷 | 236 | `SetForceLoad` | 📐 设计草案 |
| 力矩载荷 | 237 | `SetMomentLoad` | 📐 设计草案 |
| 线载荷 | 238 | `SetLineLoad` | 📐 设计草案 |
| 固定约束 | 243 | `SetFixedConstraint` | 📐 设计草案 |
| 铰支约束 | 244 | `SetHingeConstraint` | 📐 设计草案 |
| 弹性支撑 | 245 | `SetElasticSupport` | 📐 设计草案 |
| 网格划分 | 250 | `Mesh3D` | |
| 求解 | 260 | `SolverRun` | |
| 查询结果 | 261 | `QueryResults` | |

---

## 七、模态分析

### 关键规则
- **不需要施加力/力矩载荷** — 只关心固有特性
- **需要约束** — 否则刚体模态，频率为0
- 需要材料弹性属性(弹性模量、泊松比) + 密度

### 工作流
```
Step 1  创建模态任务    czwsSimStCreateTask(ZW_ST_TASK_FREQUENCY)
Step 2  查询仿真几何    czwsTaskInqSimGeoms
Step 3  分配材料       czwsSimGeomSetMaterial
Step 4  施加约束       czwsSimStCreateFixedGeometry (必须有!)
Step 5  设置模态阶数    (通过仿真属性或原生命令)
Step 6  网格划分      czwsMeshing3D
Step 7  求解         czwsSimCalculate
Step 8  查询频率     czwsResultTypeInqData("Frequency")
Step 9  查询振型     czwsResultTypeInqData("Mode Shape")
```

### 结果类型
| 结果 | 说明 |
|------|------|
| Frequency | 固有频率 (Hz) |
| Mode Shape | 振型(位移云图) |
| Modal Stress | 模态应力(可选) |

---

## 八、屈曲分析

### 关键规则
- 线性屈曲(特征值屈曲)
- 需要先施加参考载荷
- 屈曲因子 = 临界载荷 / 参考载荷

### 工作流
```
Step 1  创建屈曲任务    czwsSimStCreateTask(ZW_ST_TASK_BUCKLING)
Step 2  查询仿真几何    czwsTaskInqSimGeoms
Step 3  分配材料       czwsSimGeomSetMaterial
Step 4  施加约束       czwsSimStCreateFixedGeometry
Step 5  施加参考载荷    czwsSimStCreateForceLoad (与静力分析相同)
Step 6  设置屈曲阶数    (通过仿真属性或原生命令)
Step 7  网格划分      czwsMeshing3D
Step 8  求解         czwsSimCalculate
Step 9  查询屈曲因子  czwsResultTypeInqData("Buckle Factor")
Step 10 查询屈曲振型  czwsResultTypeInqData("Buckle Mode")
```

### 结果类型
| 结果 | 说明 |
|------|------|
| Buckle Factor | 屈曲因子(临界载荷 = 参考载荷 × 因子) |
| Buckle Mode | 屈曲振型 |

---

## 九、结果类型

### 静力分析
| 结果类型 | 子类型 | 说明 |
|---------|--------|------|
| Displacement | Total/X/Y/Z | 位移 |
| Stress | Von Mises/Principal/Max/Min | 应力 |
| Strain | Equivalent/Principal/Max/Min | 应变 |
| Reaction Force | X/Y/Z/Total | 支反力 |

> 完整结果查询API见 [[zw3d-cae-post#二结果查询api\|cae-post §二]]

---

## 十、实体类型映射

| entityType | 含义 | ZW3D类型 |
|-----------|------|---------|
| 0 | 面 | `ZW_INPUT_FACE` |
| 1 | 边 | `ZW_INPUT_EDGE` |
| 2 | 顶点 | `ZW_INPUT_VERTEX` |
| 3 | 体 | `ZW_INPUT_SHAPE` |
| 4 | 网格节点 | `ENTITY_TYPE_MESH_NODE` |
| 5 | 网格单元 | `ENTITY_TYPE_MESH_ELEMENT` |

---

## 十一、已知限制

| 限制 | 说明 |
|------|------|
| 非线性接触 | 当前API主要支持线性接触 |
| 大变形 | 需确认是否支持几何非线性 |
| 塑性材料 | Von Mises等塑性模型需确认API暴露程度 |
| 模态阶数设置 | 前n阶模态的参数设置方法待确认 |

---

## 十二、热-结构耦合

> 热-结构耦合将热仿真温度场结果导入结构分析作为温度载荷。完整工作流和API详见 [[zw3d-cae-thermal#七b热-结构耦合工作流|thermal §七B]]。

**快速流程**:
1. 完成热分析 (稳态热/瞬态热) → 求解
2. cmd=295: 复制热任务并转换为结构任务 (`czwsPartDuplicateTask` → `czwsSimStSwitchTask`)
3. cmd=296: 导入热效应 (`czwsSimStCreateThermalEffects`)
4. 设置结构载荷/约束 → 网格 → 求解 → 查询结果

| cmd | 功能 | 关键API |
|-----|------|---------|
| 295 | 复制并转换任务 | `czwsPartDuplicateTask`, `czwsSimStSwitchTask` |
| 296 | 导入热效应 | `czwsSimStThermalEffectsInit`, `czwsSimStCreateThermalEffects` |

---

## 十三、资源

| 资源 | 路径 |
|------|------|
| 结构分析模板 | `sample_templates/StructuralAnalysisAPI_template.cpp` |
| CAE功能清单 | [[知识库/02_CAE开发/ZWMeshWorks_CAE_插件_功能清单\|功能清单.md]] |
| 工程经验 | [[知识库/02_CAE开发/工程经验_结构化\|工程经验_结构化.md]] |
| 代码架构 | [[知识库/02_CAE开发/代码架构\|代码架构.md]] |

---

*文档版本: v1.3.0 | 维护者: 韩天尊*
