# 结构分析 API 模板（静力/模态/屈曲）

> 基于 ZWMeshWorks ZWSim SDK 的结构分析能力

---

## 1. 任务类型枚举

| 任务类型 | 枚举值 | 说明 |
|---------|--------|------|
| 静力分析 | `ZW_ST_TASK_STATIC` (0) | 线性静力学 |
| 模态分析 | `ZW_ST_TASK_FREQUENCY` (1) | 固有频率和振型 |
| 屈曲分析 | `ZW_ST_TASK_BUCKLING` (2) | 临界载荷屈曲 |

### 1.1 创建任务

```cpp
// 静力分析
czwsSimStCreateTask(ZW_ST_TASK_STATIC, &idxTask);

// 模态分析
czwsSimStCreateTask(ZW_ST_TASK_FREQUENCY, &idxTask);

// 屈曲分析
czwsSimStCreateTask(ZW_ST_TASK_BUCKLING, &idxTask);
```

---

## 2. 载荷类型

### 2.1 力载荷

```cpp
struct szwsForceLoadData {
    ezwsEntityType eEntityType;      // 实体类型（几何/网格节点）
    int iEntNum;                     // 实体数量
    szwEntityHandle *sEntities;      // 实体 handle 列表
    double dX, dY, dZ;              // ★ 力分量 (N)
    char unitValue[256];             // 单位字符串
};
```

**配套 API**：
| 函数 | 用途 |
|------|------|
| `czwsSimStCreateForceLoad` | 创建力载荷 |
| `czwsSimStEditForceLoad` | 编辑力载荷 |
| `czwsSimStInqForceLoad` | 查询力载荷列表 |
| `czwsSimStForceLoadInqData` | 查询载荷数据 |
| `czwsSimStForceLoadDataInit` | 初始化数据结构 |
| `czwsSimStFreeForceLoadData` | 释放内存 |

### 2.2 力矩载荷

```cpp
struct szwsMomentLoadData {
    ezwsEntityType eEntityType;
    int iEntNum;
    szwEntityHandle *sEntities;
    double dX, dY, dZ;              // ★ 力矩分量 (N·m)
    char unitValue[256];
};
```

### 2.3 线载荷（分布力）

```cpp
struct szwsLineLoadData {
    ezwsEntityType eEntityType;
    int iEntNum;
    szwEntityHandle *sEntities;
    double dValue;                   // ★ 线载荷值 (N/m)
    char unitValue[256];
};
```

---

## 3. 约束类型

### 3.1 固定约束（Fixed）

```cpp
struct szwsFixedGeometryConstraintData {
    ezwsEntityType eEntityType;
    int iEntNum;
    szwEntityHandle *sEntities;
    // 固定所有自由度
};
```

### 3.2 铰支约束（Hinge）

```cpp
struct szwsFixedHingeConstraintData {
    ezwsEntityType eEntityType;
    int iEntNum;
    szwEntityHandle *sEntities;
    double dX, dY, dZ;              // 固定方向
    // 铰链轴线方向
};
```

### 3.3 弹性支撑（Elastic Support）

```cpp
struct szwsElasticSupportConstraintData {
    ezwsEntityType eEntityType;
    int iEntNum;
    szwEntityHandle *sEntities;
    double dStiffness;              // ★ 刚度值 (N/m)
    ezwsSupportDistribution eDist;  // 分布方式
};
```

---

## 4. 接触类型（结构分析）

| 接触类型 | 枚举值 | 说明 |
|---------|--------|------|
| 标准接触 | `ZW_ST_CONTACT_TYPE_CONTACT` | 允许分离，摩擦/无摩擦 |
| 粗糙接触 | `ZW_ST_CONTACT_TYPE_ROUGH` | 无滑移（无限摩擦） |
| 粘合接触 | `ZW_ST_CONTACT_TYPE_ADHESIVE` | 粘接，不允许分离 |
| 通用接触 | `ZW_ST_CONTACT_TYPE_GENERAL` | 自定义接触定义 |

---

## 5. 静力分析典型工作流

```
Step 1  创建静力任务         czwsSimStCreateTask(ZW_ST_TASK_STATIC)
Step 2  查询仿真几何         czwsTaskInqSimGeoms
Step 3  分配材料（必需弹性属性） czwsSimGeomSetMaterial
Step 4  施加固定约束         czwsSimStCreateFixedGeometry
Step 5  施加铰支约束         czwsSimStCreateFixedHingeConstraint
Step 6  施加弹性支撑         czwsSimStCreateElasticSupport
Step 7  施加力载荷          czwsSimStCreateForceLoad
Step 8  施加力矩载荷         czwsSimStCreateMomentLoad
Step 9  施加线载荷          czwsSimStCreateLineLoad
Step 10 设置接触           czwsSimStCreateContact
Step 11 网格划分           czwsMeshing3D
Step 12 求解             czwsSimCalculate
Step 13 查询结果          czwsResultTypeInqData
```

---

## 6. 模态分析特点

### 6.1 无需外部载荷

模态分析只关心结构的固有特性：
- 不需要施加力/力矩载荷
- **需要约束**（否则刚体模态，频率为0）
- 需要材料弹性属性（弹性模量、泊松比）

### 6.2 结果类型

| 结果 | 说明 |
|------|------|
| Frequency | 固有频率 (Hz) |
| Mode Shape | 振型（位移云图） |
| Modal Stress | 模态应力（可选） |

### 6.3 查询前 n 阶模态

```cpp
// 可能需要设置模态阶数参数
// 通过原生命令或仿真属性
```

---

## 7. 屈曲分析特点

### 7.1 需要预载荷

屈曲分析是线性屈曲（特征值屈曲）：
1. 先施加参考载荷（静力分析级别的载荷）
2. 求解特征值问题得到临界载荷因子

### 7.2 结果类型

| 结果 | 说明 |
|------|------|
| Buckle Factor | 屈曲因子（临界载荷 = 参考载荷 × 因子） |
| Buckle Mode | 屈曲振型 |

---

## 8. 材料要求差异

| 分析类型 | 必需材料属性 |
|---------|------------|
| 稳态热 | 导热系数 |
| 瞬态热 | 导热系数 + 比热容 + 密度 |
| **静力分析** | **弹性模量 + 泊松比** |
| **模态分析** | **弹性模量 + 泊松比** |
| **屈曲分析** | **弹性模量 + 泊松比** |

---

## 9. IPC 命令扩展

| cmd | 名称 | 说明 |
|-----|------|------|
| 200 | `CAE_CREATE_TASK` | 支持 `taskType="static"/"frequency"/"buckling"` |
| 236 | `CAE_SET_FORCE_LOAD` | 设置力载荷 |
| 237 | `CAE_SET_MOMENT_LOAD` | 设置力矩载荷 |
| 238 | `CAE_SET_LINE_LOAD` | 设置线载荷 |
| 243 | `CAE_SET_FIXED_CONSTRAINT` | 设置固定约束 |
| 244 | `CAE_SET_HINGE_CONSTRAINT` | 设置铰支约束 |
| 245 | `CAE_SET_ELASTIC_SUPPORT` | 设置弹性支撑 |

---

## 10. 结果类型扩展

### 10.1 静力分析结果

| 结果类型 | 子类型 | 说明 |
|---------|--------|------|
| Displacement | Total/X/Y/Z | 位移 |
| Stress | Von Mises/Principal/... | 应力 |
| Strain | Equivalent/Principal/... | 应变 |
| Reaction Force | - | 支反力 |

### 10.2 模态分析结果

| 结果类型 | 子类型 | 说明 |
|---------|--------|------|
| Frequency | - | 固有频率 |
| Mode Shape | Total/X/Y/Z | 振型位移 |

### 10.3 屈曲分析结果

| 结果类型 | 子类型 | 说明 |
|---------|--------|------|
| Buckle Factor | - | 屈曲因子 |
| Buckle Mode | Total/X/Y/Z | 屈曲振型 |

---

## 11. 模板代码

```cpp
// 创建静力任务 + 施加载荷
nlohmann::json CreateStaticAnalysis(const nlohmann::json& data) {
    nlohmann::json resp;
    
    // 1. 创建任务
    zwsDbId idxTask;
    ZWSimEvxErrors err = czwsSimStCreateTask(ZW_ST_TASK_STATIC, &idxTask);
    if (err != ZWSIM_API_NO_ERROR) {
        resp["status"] = "error";
        resp["error"] = "Failed to create static task";
        return resp;
    }
    
    resp["status"] = "success";
    resp["data"]["taskId"] = idxTask;
    resp["data"]["taskType"] = "static";
    
    return resp;
}

// 设置力载荷
nlohmann::json SetForceLoad(const nlohmann::json& data) {
    nlohmann::json resp;
    
    std::vector<int> cacheIds = data["cacheIds"].get<std::vector<int>>();
    double fx = data.value("fx", 0.0);
    double fy = data.value("fy", 0.0);
    double fz = data.value("fz", 0.0);
    
    // 获取实体句柄
    auto entities = getCachedEntities(cacheIds);
    
    // 构建力载荷数据
    szwsForceLoadData forceData;
    czwsSimStForceLoadDataInit(&forceData);
    forceData.eEntityType = ZW_ST_ENTITY_TYPE_GEOMETRY;
    forceData.iEntNum = entities.size();
    forceData.sEntities = entities.data();
    forceData.dX = fx;
    forceData.dY = fy;
    forceData.dZ = fz;
    strcpy(forceData.unitValue, "N");
    
    // 创建载荷
    zwsDbId idxLoad;
    ZWSimEvxErrors err = czwsSimStCreateForceLoad(&forceData, &idxLoad);
    
    czwsSimStFreeForceLoadData(&forceData);
    clearCachedEntities(cacheIds);
    
    if (err == ZWSIM_API_NO_ERROR) {
        resp["status"] = "success";
        resp["data"]["loadId"] = idxLoad;
    } else {
        resp["status"] = "error";
        resp["error"] = "Failed to create force load";
    }
    
    return resp;
}

// 设置固定约束
nlohmann::json SetFixedConstraint(const nlohmann::json& data) {
    std::vector<int> cacheIds = data["cacheIds"].get<std::vector<int>>();
    auto entities = getCachedEntities(cacheIds);
    
    szwsFixedGeometryConstraintData fixData;
    // 初始化并设置实体
    // ...
    
    zwsDbId idxConstraint;
    czwsSimStCreateFixedGeometry(&fixData, &idxConstraint);
    
    clearCachedEntities(cacheIds);
    // 返回结果...
}
```

---

## 12. 已知限制

| 限制 | 说明 |
|------|------|
| 非线性接触 | 当前 API 主要支持线性接触 |
| 大变形 | 需确认是否支持几何非线性 |
| 塑性材料 | Von Mises 等塑性模型需确认 API 暴露程度 |
| 模态阶数设置 | 前 n 阶模态的参数设置方法待确认 |