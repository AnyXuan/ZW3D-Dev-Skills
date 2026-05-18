# 参数优化 API 模板

> 基于 ZWMeshWorks ZWSim SDK 的参数优化（Parameter Optimization）能力

---

## 1. 优化算法类型

| 算法 | 枚举值 | 说明 |
|------|--------|------|
| 罚函数法 | `ZW_PO_ALGORITHM_TYPE_PENALTY_METHOD` | 将约束转化为罚函数加到目标函数 |
| 修正可行方向法 | `ZW_PO_ALGORITHM_TYPE_MMFD` | Modified Method of Feasible Directions |
| 序列线性规划 | `ZW_PO_ALGORITHM_TYPE_SLP` | Sequential Linear Programming |
| 序列二次规划 | `ZW_PO_ALGORITHM_TYPE_SQP` | Sequential Quadratic Programming |

---

## 2. 核心数据结构

### 2.1 优化变量（设计变量）

```cpp
struct szwsParamOptVariableData {
    char name[256];              // 变量名称
    double initialValue;         // 初始值
    double lowerBound;           // 下界
    double upperBound;           // 上界
    int variableType;            // 变量类型（连续/离散）
};
```

### 2.2 优化目标

```cpp
struct szwsParamOptObjectiveData {
    int objectiveType;           // 最小化(0) 或 最大化(1)
    char resultType[256];        // 目标结果类型（如 "Temperature"、"VonMises"）
    char subType[256];           // 子类型（如 "Max"、"Min"、"Average"）
    int targetEntityCount;       // 目标实体数量
    szwEntityHandle *targetEntities;  // 目标实体列表
};
```

### 2.3 优化约束

```cpp
struct szwsParamOptConstraintData {
    int constraintType;          // 不等式(0) 或 等式(1)
    char resultType[256];        // 约束结果类型
    char subType[256];           // 子类型
    double boundValue;           // 约束边界值
    int targetEntityCount;
    szwEntityHandle *targetEntities;
};
```

### 2.4 优化设置

```cpp
struct szwsParamOptSettingData {
    int algorithmType;           // 算法类型
    int maxIterations;           // 最大迭代次数
    double convergenceTolerance; // 收敛容差
    int objectiveWeight;         // 目标权重（多目标时）
};
```

---

## 3. 参数优化工作流程

```
┌─────────────────────────────────────────────────────────────┐
│  1. 定义设计变量                                              │
│     → szwsParamOptVariableData (名称、初值、上下界)          │
├─────────────────────────────────────────────────────────────┤
│  2. 定义优化目标                                              │
│     → szwsParamOptObjectiveData (最小化/最大化某结果)        │
│     例: 最小化最高温度, 最大化第一阶频率, 最小化最大应力      │
├─────────────────────────────────────────────────────────────┤
│  3. 定义约束条件                                              │
│     → szwsParamOptConstraintData (不等式/等式)              │
│     例: 最高温度 ≤ 100°C, 最大应力 ≤ 200 MPa, 质量 ≤ 1 kg   │
├─────────────────────────────────────────────────────────────┤
│  4. 选择优化算法                                              │
│     → szwsParamOptSettingData (算法、迭代次数、容差)         │
├─────────────────────────────────────────────────────────────┤
│  5. 执行优化                                                  │
│     → 调用参数优化 API                                        │
│     → 自动迭代：改参数→重建→分析→评估→改参数                 │
├─────────────────────────────────────────────────────────────┤
│  6. 获取优化结果                                              │
│     → 最优设计变量值                                          │
│     → 满足约束情况                                            │
│     → 目标函数变化曲线                                        │
└─────────────────────────────────────────────────────────────┘
```

---

## 4. 典型应用场景

### 4.1 热分析优化

| 目标 | 设计变量 | 约束 |
|------|---------|------|
| 最小化最高温度 | 散热片厚度、翅片数量 | 质量 ≤ 某值 |
| 最小化热应力 | 材料选择、几何尺寸 | 最高温度 ≤ 限值 |
| 最大化散热效率 | 接触面积、导热路径 | 成本 ≤ 某值 |

### 4.2 结构分析优化

| 目标 | 设计变量 | 约束 |
|------|---------|------|
| 最大化固有频率 | 梁截面尺寸、材料 | 质量 ≤ 限值 |
| 最小化最大位移 | 壁厚、加强筋位置 | 应力 ≤ 许用值 |
| 最小化质量 | 板厚、材料 | 频率 ≥ 某值, 应力 ≤ 限值 |

### 4.3 多目标优化

```cpp
// 多目标需要设置权重或使用 Pareto 前沿方法
// (具体 API 支持程度待确认)
szwsParamOptObjectiveData objectives[2];
objectives[0] = {MINIMIZE, "VonMises", "Max", ...};
objectives[1] = {MINIMIZE, "Mass", "Total", ...};
```

---

## 5. IPC 命令扩展

| cmd | 名称 | 说明 |
|-----|------|------|
| 400 | `OPT_DEFINE_VARIABLE` | 定义设计变量 |
| 401 | `OPT_DEFINE_OBJECTIVE` | 定义优化目标 |
| 402 | `OPT_DEFINE_CONSTRAINT` | 定义约束条件 |
| 403 | `OPT_SET_ALGORITHM` | 设置优化算法和参数 |
| 404 | `OPT_RUN` | 执行优化 |
| 405 | `OPT_QUERY_RESULT` | 查询优化结果 |
| 406 | `OPT_QUERY_HISTORY` | 查询优化历史 |

---

## 6. 命令请求/响应格式

### 6.1 定义设计变量（cmd=400）

**请求**：
```json
{
  "cmd": 400,
  "data": {
    "variables": [
      {
        "name": "plate_thickness",
        "initialValue": 5.0,
        "lowerBound": 1.0,
        "upperBound": 20.0,
        "variableType": 0
      },
      {
        "name": "fin_count",
        "initialValue": 10,
        "lowerBound": 5,
        "upperBound": 50,
        "variableType": 1
      }
    ]
  }
}
```

**响应**：
```json
{
  "cmd": 400,
  "status": "success",
  "data": {
    "variableCount": 2,
    "variableIds": [1, 2]
  }
}
```

### 6.2 定义优化目标（cmd=401）

**请求**：
```json
{
  "cmd": 401,
  "data": {
    "objectiveType": 0,
    "resultType": "Temperature",
    "subType": "Max",
    "targetEntity": "hot_spot_face",
    "resolver": {
      "type": "nameTag",
      "value": "HOT_ZONE"
    }
  }
}
```

### 6.3 定义约束（cmd=402）

**请求**：
```json
{
  "cmd": 402,
  "data": {
    "constraintType": 0,
    "resultType": "VonMises",
    "subType": "Max",
    "boundValue": 250.0,
    "unit": "MPa",
    "targetEntity": "critical_section"
  }
}
```

### 6.4 设置算法（cmd=403）

**请求**：
```json
{
  "cmd": 403,
  "data": {
    "algorithmType": 3,
    "maxIterations": 50,
    "convergenceTolerance": 1.0e-4,
    "objectiveWeight": [1.0, 0.5]
  }
}
```

### 6.5 执行优化（cmd=404）

**请求**：
```json
{
  "cmd": 404,
  "data": {}
}
```

**响应**：
```json
{
  "cmd": 404,
  "status": "success",
  "data": {
    "optimizationId": 1,
    "finalIteration": 23,
    "converged": true,
    "optimalVariables": {
      "plate_thickness": 8.5,
      "fin_count": 18
    },
    "objectiveValue": 45.2,
    "constraintStatus": "satisfied"
  }
}
```

### 6.6 查询优化历史（cmd=406）

**响应**：
```json
{
  "cmd": 406,
  "status": "success",
  "data": {
    "history": [
      {
        "iteration": 1,
        "variables": {"plate_thickness": 5.0, "fin_count": 10},
        "objective": 85.3,
        "maxConstraintViolation": 50.0
      },
      {
        "iteration": 2,
        "variables": {"plate_thickness": 6.2, "fin_count": 12},
        "objective": 68.7,
        "maxConstraintViolation": 25.0
      },
      // ...
      {
        "iteration": 23,
        "variables": {"plate_thickness": 8.5, "fin_count": 18},
        "objective": 45.2,
        "maxConstraintViolation": 0.0
      }
    ]
  }
}
```

---

## 7. 与参数化建模的集成

参数优化的核心是**自动迭代改参数→重建→分析**：

```
优化循环：
  for iteration = 1 to maxIterations:
    1. 获取当前设计变量值 (from optimizer)
    2. 调用 cmd=301 (SetVariablesAndRegen) 更新模型参数
    3. 执行 CAE 分析流程 (cmd=200~260)
    4. 提取目标值和约束值 (cmd=261)
    5. 传递给优化器判断收敛
```

---

## 8. 模板代码

```cpp
// 定义设计变量
nlohmann::json OptDefineVariable(const nlohmann::json& data) {
    nlohmann::json resp;
    
    auto vars = data["variables"];
    std::vector<szwsParamOptVariableData> varList(vars.size());
    
    for (size_t i = 0; i < vars.size(); i++) {
        strcpy(varList[i].name, vars[i]["name"].get<std::string>().c_str());
        varList[i].initialValue = vars[i]["initialValue"];
        varList[i].lowerBound = vars[i]["lowerBound"];
        varList[i].upperBound = vars[i]["upperBound"];
        varList[i].variableType = vars[i].value("variableType", 0);
    }
    
    // 调用优化器 API 注册变量
    // (具体 API 待确认)
    
    resp["status"] = "success";
    resp["data"]["variableCount"] = vars.size();
    
    return resp;
}

// 执行优化
nlohmann::json OptRun(const nlohmann::json& data) {
    nlohmann::json resp;
    
    // 1. 检查变量/目标/约束是否已定义
    // 2. 调用优化求解器
    // 3. 内部自动迭代：
    //    for each iteration:
    //      - 改参数 (调用 ZwVariableListSet)
    //      - 重建 (ZwEntityAutoRegen)
    //      - 网格 (czwsMeshing3D)
    //      - 求解 (czwsSimCalculate)
    //      - 提取结果 (czwsResultTypeInqData)
    //      - 检查收敛
    // 4. 返回最优解
    
    resp["status"] = "success";
    resp["data"]["optimizationId"] = 1;
    resp["data"]["converged"] = true;
    resp["data"]["iterations"] = 23;
    
    return resp;
}
```

---

## 9. 已知限制与待确认

| 项目 | 状态 | 说明 |
|------|------|------|
| 优化 API 是否完全暴露 | ❓ | 部分可能在文档中未完全暴露 |
| 多目标优化支持 | ❓ | Pareto 前沿方法是否支持待确认 |
| 梯度计算方法 | ❓ | 是内置有限差分还是需用户指定 |
| 灵敏度分析 | ❓ | 是否提供灵敏度输出 |
| 优化结果导出 | ❓ | 最优设计如何保存/导出 |