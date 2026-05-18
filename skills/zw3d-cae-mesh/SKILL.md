---
name: zw3d-cae-mesh
description: Use when implementing or debugging ZW3D 2026 geometry cleanup and meshing workflows, including face/edge/body filtering, hole and fillet cleanup, mid-surface extraction, mesh sizing, mesh generation, mesh quality checks, and CAE mesh preparation.
---

# ZW3D 几何清理与网格 Skill — v1.2.0

> **触发词**: "几何清理", "网格划分", "mesh", "geometry cleanup", "删除小面", "移除圆角", "中面抽取", "网格尺寸", "网格质量"
> **适用**: ZW3D 2026 ZWMeshWorks 几何清理、简化、网格划分
> **维护者**: 韩天尊
> **依赖**: [[zw3d-plugin-base\|plugin-base]] (必需)

### 权威来源引用
| 内容 | 引用 |
|------|------|
| C++ API调用规范 | [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md]] |
| 质量门控 | [[.workbuddy/skills/zw3d-dev-team/docs/04-quality-gate\|04-quality-gate.md]] |
| CAE功能清单(84+) | [[知识库/02_CAE开发/ZWMeshWorks_CAE_插件_功能清单\|功能清单.md]] |

---

## 一、几何清理 (18个功能)

### 1.1 几何检测
| # | 功能 | 核心API | 输入 | 输出 |
|---|------|---------|------|------|
| 1 | 按尺寸筛选几何体 | `czwsFindShapesBySize` | 实体列表、体积/面积范围 | 符合条件的实体 |
| 2 | 按尺寸筛选面 | `czwsFindFacesBySize` | 面列表、面积范围 | 符合条件的面 |
| 3 | 按尺寸筛选边 | `czwsFindEdgesBySize` | 边列表、长度范围 | 符合条件的边 |
| 4 | 孔洞检测 | `czwsFindHolesBySize` | 实体列表、孔径范围 | 孔洞信息 |
| 5 | 平面查找(平行) | `czwsFindPlanesByParallelDistance` | 参考面、距离 | 平行平面列表 |
| 6 | 平面查找(角度) | `czwsFindPlanesByDihedralAngle` | 参考面、角度 | 成角平面列表 |
| 7 | 圆角检测 | `czwsFindFillets` | 实体列表、半径范围 | 圆角面信息 |
| 8 | 倒角检测 | `czwsFindChamfers` | 实体列表、角度/宽度 | 倒角面信息 |

### 1.2 几何修复
| # | 功能 | 核心API | 说明 |
|---|------|---------|------|
| 9 | 删除微小边 | `czwsDeleteTinyEdges` | 清理短边，防止网格畸形 |
| 10 | 删除开放边 | `czwsDeleteOpenEdges` | 修复开放边界 |
| 11 | 删除小面 | `czwsDeleteSmallFaces` | 清理碎面 |
| 12 | 删除狭长面 | `czwsDeleteSliverFaces` | 清理细长比过大的面 |
| 13 | 删除自相交面 | `czwsDeleteSelfIntersectingFaces` | 修复自相交 |

### 1.3 特征移除
| # | 功能 | 核心API | 说明 |
|---|------|---------|------|
| 14 | 移除3D孔 | `czwsRemove3DHoles` | 填充实体孔洞 |
| 15 | 移除2D孔 | `czwsRemove2DHoles` | 填充面内孔洞 |
| 16 | 移除Logo/刻字 | `czwsRemoveLogos` | 移除刻字、商标 |
| 17 | 移除圆角 | `czwsRemoveFillets` | 批量移除圆角 |
| 18 | 移除倒角 | `czwsRemoveChamfers` | 批量移除倒角 |

---

## 二、几何简化

| # | 功能 | 核心API | 说明 |
|---|------|---------|------|
| 19 | 自动抽取中面 | `czwsAutoMidSurface` | 薄壁件中面，壳单元前处理 |
| 20 | 自动延伸曲面 | `czwsAutoExtendSurface` | 曲面延伸到目标面，消除间隙 |
| 21 | 几何简化 | `czwsSimplifyGeometry` | 降低模型复杂度 |

---

## 三、网格划分

### 3.1 网格划分API
```cpp
// 3D网格划分
int err = czwsMeshing3D(globalSize, minSize, order);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }

// 2D网格划分
int err = czwsMeshing2D(globalSize, minSize, order);

// 1D网格划分
int err = czwsMeshing1D(globalSize, order);

// 3D扫掠网格(六面体)
int err = czwsMeshing3DSweep(globalSize, minSize, order);
```

### 3.2 网格质量检查 (cmd=251)
```cpp
// 网格质量检查: 雅可比、长宽比、翘曲等
int err = czwsMeshQuality(meshId, &qualityData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }

// 网格缺陷检测: 退化/相交/非流形
int err = czwsMeshFlawCheck(meshId, &flawData);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 3.3 网格修复
```cpp
// 修复2D/3D交叉
int err = czwsMeshFix2DIntersection(meshId);
int err = czwsMeshFix3DIntersection(meshId);

// 合并重合节点(按容差)
int err = czwsMeshMergeVertices(meshId, tolerance);

// 更改单元阶次(一阶↔二阶)
int err = czwsMeshChangeOrder(meshId, newOrder);
```

### 3.4 网格划分参数建议
| 场景 | globalSize | minSize | order |
|------|-----------|---------|-------|
| 快速验证 | 模型尺寸的1/5 | globalSize/5 | 1 |
| 常规分析 | 模型尺寸的1/10 | globalSize/10 | 1 |
| 精确分析 | 模型尺寸的1/20 | globalSize/20 | 2 |

> ⚠️ 关键区域局部加密建议不超过globalSize/20，否则可能产生数百万单元导致内存溢出。

---

## 四、网格-几何关联

| 功能 | API | 说明 |
|------|-----|------|
| 几何→网格映射 | `czwsMeshInqEntityByGeometry` | 获取几何实体对应的网格 |
| 网格→几何映射 | `czwsMeshInqGeometryByEntity` | 获取网格节点所属几何 |
| 网格节点查询 | `czwsMeshInqVertices` | 查询节点坐标、连接关系 |
| 网格单元查询 | `czwsMeshInqElements` | 查询单元类型、节点列表 |

### 查询仿真几何和网格
```cpp
// 查询任务关联的仿真几何体
int simGeomCount = 0;
zwsDbId* simGeomIds = NULL;
int err = czwsTaskInqSimGeoms(taskId, &simGeomCount, &simGeomIds);
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }

// 查询任务下的所有网格
int meshCount = 0;
zwsDbId* meshIds = NULL;
err = czwsTaskInqMeshes(taskId, &meshCount, &meshIds);

// 查询当前激活的网格
zwsDbId activeMeshId;
err = czwsTaskInqActiveMesh(taskId, &activeMeshId);
```

---

## 五、CAE前处理标准流程

```
Step 1  导入/创建几何模型
Step 2  几何检测 (发现问题)
        czwsFindShapesBySize / FindFillets / FindHolesBySize
Step 3  几何清理 (修复问题)
        czwsDeleteTinyEdges / DeleteSmallFaces / RemoveFillets
Step 4  几何简化 (可选)
        czwsAutoMidSurface / AutoExtendSurface
Step 5  分配材料
        czwsSimGeomSetMaterial
Step 6  网格划分
        czwsMeshing3D(globalSize, minSize, order)
Step 7  网格质量检查
        czwsMeshQuality / czwsMeshFlawCheck
Step 8  进入仿真设置 (载荷/约束/求解)
```

> 仿真设置参见: [[zw3d-cae-thermal\|cae-thermal]] / [[zw3d-cae-structural\|cae-structural]]

---

## 六、经验卡片

### K-020: 网格error -32 (HIGH)

**现象**: API `czwsTaskMesh3D` 在某些环境返回vendor特定错误码-32，但ZW3D UI手动点击网格按钮正常。

**解决方案**: 双通路fallback
```cpp
// 1. 先调用官方API
int err = czwsTaskMesh3D(taskId, globalSize, minSize, order);
if (err != ZWSIM_API_NO_ERROR) {
    // 2. Fallback到原生命令
    cvxCmdSend("ZwCeMeshing");
    // 3. 校验网格数量确认成功
    int meshCount = 0;
    czwsTaskInqMeshes(taskId, &meshCount, NULL);
    if (meshCount == 0) { /* 仍然失败 */ }
}
```

**效果**: 网格划分成功率从0%→90%+。

> 完整经验卡片原文见 [[知识库/02_CAE开发/工程经验_结构化\|工程经验_结构化.md]]

---

## 七、IPC命令号段 (完整表见 [[zw3d-ipc-comm#七完整命令码表\|ipc-comm §七]])

| cmd | 名称 | 说明 |
|-----|------|------|
| 250 | CAE_MESH_3D | 3D网格划分 |
| 251 | CAE_MESH_QUALITY | 网格质量检查 |
| 252 | CAE_MESH_2D | 2D网格划分 |
| 253 | CAE_MESH_1D | 1D网格划分 |
| 320-399 | 几何清理 | 预留80个空位 |

---

## 八、资源

| 资源 | 路径 |
|------|------|
| CAE功能清单 | [[知识库/02_CAE开发/ZWMeshWorks_CAE_插件_功能清单\|功能清单.md]] |
| 稳态热开发计划 | [[知识库/02_CAE开发/稳态传热模块开发计划\|稳态传热模块开发计划.md]] |
| 工程经验 | [[知识库/02_CAE开发/工程经验_结构化\|工程经验_结构化.md]] |
