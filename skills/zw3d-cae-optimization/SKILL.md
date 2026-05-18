---
name: zw3d-cae-optimization
description: Use when implementing or debugging ZW3D 2026 CAE parameter optimization workflows, including design variables, optimization objectives, constraints, sensitivity, MMFD, SLP, SQP, and optimization flows connected to thermal or structural simulation.
---

# ZW3D 参数优化 Skill — v1.2.0

> **触发词**: "参数优化", "optimization", "MMFD", "SLP", "SQP", "设计变量", "优化目标", "约束条件", "灵敏度"
> **适用**: ZW3D 2026 ZWMeshWorks 参数优化模块
> **维护者**: 韩天尊
> **依赖**: [[zw3d-plugin-base\|plugin-base]] (必需), [[zw3d-ipc-comm\|ipc-comm]] (必需), [[zw3d-cae-thermal\|cae-thermal]] 或 [[zw3d-cae-structural\|cae-structural]] (至少一个)

### 权威来源引用
| 内容 | 引用 |
|------|------|
| C++ API调用规范 | [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md]] |
| IPC消息协议 | [[.workbuddy/skills/zw3d-dev-team/docs/02-ipc-protocol\|02-ipc-protocol.md]] |
| 质量门控 | [[.workbuddy/skills/zw3d-dev-team/docs/04-quality-gate\|04-quality-gate.md]] |
| 完整命令码表 | [[zw3d-ipc-comm#七完整命令码表\|ipc-comm §七]] |
| CAE接口设计(一期) | [[知识库/02_CAE开发/CAE自动化接口设计_一期\|接口设计_一期.md]] |

---

## 一、优化算法

| 算法 | 枚举值 | 说明 | 适用场景 |
|------|--------|------|---------|
| 罚函数法 | `ZW_PO_ALGORITHM_TYPE_PENALTY_METHOD` | 约束转化为罚函数加到目标函数 | 简单约束优化 |
| 修正可行方向法 | `ZW_PO_ALGORITHM_TYPE_MMFD` | Modified Method of Feasible Directions | 工程优化常用 |
| 序列线性规划 | `ZW_PO_ALGORITHM_TYPE_SLP` | Sequential Linear Programming | 线性近似优化 |
| 序列二次规划 | `ZW_PO_ALGORITHM_TYPE_SQP` | Sequential Quadratic Programming | 高精度非线性优化 |

---

## 二、核心数据结构

### 2.1 设计变量
```cpp
struct szwsParamOptVariableData {
    char name[256];          // 变量名称 (与ZW3D变量名对应)
    double initialValue;     // 初始值
    double lowerBound;       // 下界
    double upperBound;       // 上界
    int variableType;        // 0=连续, 1=离散
};
```

### 2.2 优化目标
```cpp
struct szwsParamOptObjectiveData {
    int objectiveType;           // 0=最小化, 1=最大化
    char resultType[256];        // "Temperature", "VonMises", "Mass", "Frequency"
    char subType[256];           // "Max", "Min", "Average", "Total"
    int targetEntityCount;
    szwEntityHandle *targetEntities;
};
```

### 2.3 优化约束
```cpp
struct szwsParamOptConstraintData {
    int constraintType;          // 0=不等式(≤), 1=等式(=)
    char resultType[256];
    char subType[256];
    double boundValue;           // 约束边界值
    int targetEntityCount;
    szwEntityHandle *targetEntities;
};
```

### 2.4 优化设置
```cpp
struct szwsParamOptSettingData {
    int algorithmType;           // 算法类型(见§一)
    int maxIterations;           // 最大迭代次数
    double convergenceTolerance; // 收敛容差
    int objectiveWeight;         // 多目标权重
};
```

---

## 三、优化工作流

```
Step 1  定义设计变量    szwsParamOptVariableData (名称、初值、上下界)
Step 2  定义优化目标    szwsParamOptObjectiveData (最小化/最大化某结果)
Step 3  定义约束条件    szwsParamOptConstraintData (不等式/等式)
Step 4  选择优化算法    szwsParamOptSettingData (算法、迭代次数、容差)
Step 5  执行优化       自动迭代：改参数→重建→分析→评估→改参数
Step 6  获取结果       最优变量值、约束满足情况、目标变化曲线
```

---

## 四、优化循环内部流程

```
for iteration = 1 to maxIterations:
    1. 优化器计算新的设计变量值
    2. 调用 cmd=301 (SetVariablesAndRegen) 更新ZW3D模型参数
       → ZwVariableListSet() 修改变量
       → ZwEntityAutoRegen() 重建模型
    3. 执行CAE分析流程 (cmd=200~260)
       → 网格划分 → 求解
    4. 提取目标值和约束值 (cmd=261)
       → czwsResultTypeInqData() (参见 [[zw3d-cae-post\|cae-post §二]])
    5. 优化器判断收敛
       → 收敛: 返回最优解
       → 未收敛: 继续下一迭代
```

---

## 五、典型应用场景

### 5.1 热分析优化
| 目标 | 设计变量 | 约束 |
|------|---------|------|
| 最小化最高温度 | 散热片厚度、翅片数量 | 质量 ≤ 某值 |
| 最小化热应力 | 材料选择、几何尺寸 | 最高温度 ≤ 限值 |
| 最大化散热效率 | 接触面积、导热路径 | 成本 ≤ 某值 |

### 5.2 结构分析优化
| 目标 | 设计变量 | 约束 |
|------|---------|------|
| 最大化固有频率 | 梁截面尺寸、材料 | 质量 ≤ 限值 |
| 最小化最大位移 | 壁厚、加强筋位置 | 应力 ≤ 许用值 |
| 最小化质量 | 板厚、材料 | 频率 ≥ 某值, 应力 ≤ 限值 |

### 5.3 多目标优化
```cpp
// 多目标需要设置权重
szwsParamOptObjectiveData objectives[2];
objectives[0] = {MINIMIZE, "VonMises", "Max", ...};  // 最小化最大应力
objectives[1] = {MINIMIZE, "Mass", "Total", ...};    // 最小化质量
```

---

## 六、IPC命令号段 (完整表见 [[zw3d-ipc-comm#七完整命令码表\|ipc-comm §七]])

| cmd | 名称 | 说明 |
|-----|------|------|
| 400 | OPT_DEFINE_VARIABLE | 定义设计变量 |
| 401 | OPT_DEFINE_OBJECTIVE | 定义优化目标 |
| 402 | OPT_DEFINE_CONSTRAINT | 定义约束条件 |
| 403 | OPT_SET_ALGORITHM | 设置优化算法和参数 |
| 404 | OPT_RUN | 执行优化 |
| 405 | OPT_QUERY_RESULT | 查询优化结果 |
| 406 | OPT_QUERY_HISTORY | 查询优化历史 |

---

## 七、JSON请求/响应格式

### 7.1 定义设计变量 (cmd=400)
```json
{
  "cmd": 400,
  "data": {
    "variables": [
      { "name": "plate_thickness", "initialValue": 5.0, "lowerBound": 1.0, "upperBound": 20.0, "variableType": 0 },
      { "name": "fin_count", "initialValue": 10, "lowerBound": 5, "upperBound": 50, "variableType": 1 }
    ]
  }
}
```

### 7.2 定义优化目标 (cmd=401)
```json
{
  "cmd": 401,
  "data": {
    "objectiveType": 0,
    "resultType": "Temperature",
    "subType": "Max",
    "targetEntity": "hot_spot_face",
    "resolver": { "type": "nameTag", "value": "HOT_ZONE" }
  }
}
```

### 7.3 定义约束 (cmd=402)
```json
{
  "cmd": 402,
  "data": {
    "constraintType": 0,
    "resultType": "VonMises",
    "subType": "Max",
    "boundValue": 250.0,
    "unit": "MPa"
  }
}
```

### 7.4 执行优化响应 (cmd=404)
```json
{
  "cmd": 404,
  "status": "success",
  "data": {
    "optimizationId": 1,
    "finalIteration": 23,
    "converged": true,
    "optimalVariables": { "plate_thickness": 8.5, "fin_count": 18 },
    "objectiveValue": 45.2,
    "constraintStatus": "satisfied"
  }
}
```

---

## 八、与参数化建模的集成

### 变量查询 (cmd=300)
```json
{ "cmd": 300, "data": { "includeStatus": 1 } }
```
响应:
```json
{
  "cmd": 300,
  "status": "success",
  "data": {
    "count": 3,
    "variables": [
      { "name": "L", "expression": "120", "status": 1 },
      { "name": "W", "expression": "40", "status": 1 },
      { "name": "T", "expression": "6" }
    ]
  }
}
```

### 批量设置变量并重建 (cmd=301)
```json
{ "cmd": 301, "data": { "variables": { "L": "100", "W": "50", "T": "8" } } }
```

---

## 九、目标实体解析

> 实体解析参见 [[zw3d-ipc-comm#五实体选择机制\|ipc-comm §五]]。优化中的目标实体通过nameTag/feature/几何规则定位。

解析优先级: `nameTag` → `feature_child` → `entityName` → `face_rule/body_rule` → `index`

---

## 十、已知限制

| 项目 | 状态 | 说明 |
|------|------|------|
| 优化API完全暴露 | ❓ | 部分可能在文档中未完全暴露 |
| 多目标Pareto前沿 | ❓ | 是否支持待确认 |
| 梯度计算方法 | ❓ | 内置有限差分还是需用户指定 |
| 灵敏度分析 | ❓ | 是否提供灵敏度输出 |

---

## 十一、资源

| 资源 | 路径 |
|------|------|
| 优化模板 | `sample_templates/ParameterOptimizationAPI_template.cpp` |
| CAE功能清单 | [[知识库/02_CAE开发/ZWMeshWorks_CAE_插件_功能清单\|功能清单.md]] |
| CAE接口设计 | [[知识库/02_CAE开发/CAE自动化接口设计_一期\|接口设计_一期.md]] |
