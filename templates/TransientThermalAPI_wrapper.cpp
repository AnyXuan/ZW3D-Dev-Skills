# 瞬态热仿真 API 模板

> 基于 ZWMeshWorks ZWSim SDK 的瞬态传热（Transient Thermal）分析能力

---

## 1. 任务类型枚举

```cpp
// 任务类型：ZW_TASK_Structure = 2
// 子类型：ZW_ST_TASK_TRANSIENT_THERMAL = 4
```

创建瞬态热任务：
```cpp
zwsDbId idxTask;
ZWSimEvxErrors err = czwsSimStCreateTask(ZW_ST_TASK_TRANSIENT_THERMAL, &idxTask);
```

---

## 2. 瞬态热特有数据结构

### 2.1 初始热效应（Initial Thermal Effect）

瞬态分析特有，用于定义时间步初始状态：

```cpp
struct szwsInitThermEffectData {
    zwsDbId sTaskHandle;     // 关联的任务句柄
    int iStep;               // ★ 分析步号（瞬态分析的多步特性）
};
```

配套 API：
| 函数 | 用途 |
|------|------|
| `czwsSimStCreateInitialThermalEffect` | 创建初始热效应 |
| `czwsSimStEditInitialThermalEffect` | 编辑初始热效应 |
| `czwsSimStInqInitialThermalEffect` | 查询初始热效应列表 |
| `czwsSimStInitialThermalEffectInqData` | 查询数据详情 |
| `czwsSimStInitialThermalEffectDataInit` | 初始化数据结构 |
| `czwsSimStFreeInitialThermalEffectData` | 释放内存 |

### 2.2 时间步设置

瞬态分析需设置时间相关参数（通过原生命令或仿真属性）：

```cpp
// 示例：设置时间步长和总时间
// 可能需要通过 czwsSimGeomSetSimProperty 或原生命令
czwsSimGeomSetSimProperty(simGeomId, "TimeStep", "0.1");  // 时间步长(s)
czwsSimGeomSetSimProperty(simGeomId, "TotalTime", "100.0"); // 总时间(s)
```

---

## 3. 载荷设置差异

瞬态热载荷与稳态热载荷结构相同，但**值可以是时间的函数**：

| 载荷类型 | 稳态热 | 瞬态热 |
|---------|--------|--------|
| 温度载荷 | 固定值 | 可时间相关 |
| 热通量载荷 | 固定值 | 可时间相关 |
| 热功率载荷 | 固定值 | 可时间相关 |
| 初始温度 | 全局/局部 | 必须设置（作为 t=0 状态） |
| **初始热效应** | 不需要 | **必需**（定义各时间步初始状态） |

---

## 4. 瞬态热典型工作流

```
Step 1  创建瞬态热任务         czwsSimStCreateTask(ZW_ST_TASK_TRANSIENT_THERMAL)
Step 2  查询仿真几何           czwsTaskInqSimGeoms
Step 3  分配材料              czwsSimGeomSetMaterial（必须含比热容）
Step 4  设置初始温度          czwsSimStCreateInitialTemperature
Step 5  设置初始热效应         czwsSimStCreateInitialThermalEffect（iStep=0）
Step 6  施加温度载荷          czwsSimStCreateTemperatureLoad
Step 7  施加热通量载荷         czwsSimStCreateHeatFluxLoad
Step 8  施加热功率载荷         czwsSimStCreateHeatPowerLoad
Step 9  设置时间步参数        (原生命令或仿真属性)
Step 10 网格划分             czwsMeshing3D
Step 11 求解               czwsSimCalculate
Step 12 查询结果            czwsResultTypeInqData（可查询不同时间步的结果）
```

---

## 5. IPC 命令扩展（推荐）

在现有 cmd 体系基础上，瞬态热需新增或扩展：

| cmd | 名称 | 说明 |
|-----|------|------|
| 200 | `CAE_CREATE_TASK` | 支持 `taskType="transient_thermal"` |
| 234 | `CAE_SET_INITIAL_THERMAL_EFFECT` | 设置初始热效应（瞬态特有） |
| 235 | `CAE_SET_TIME_STEP` | 设置时间步参数 |
| 261 | `CAE_QUERY_RESULTS` | 扩展支持时间步索引参数 |

### cmd=234 请求示例

```json
{
  "cmd": 234,
  "data": {
    "step": 0,
    "taskType": "transient_thermal"
  }
}
```

### cmd=235 请求示例

```json
{
  "cmd": 235,
  "data": {
    "timeStep": 0.1,
    "totalTime": 100.0,
    "outputInterval": 1.0
  }
}
```

---

## 6. 材料要求差异

| 属性 | 稳态热必需 | 瞬态热必需 |
|------|-----------|-----------|
| 导热系数 | ✅ | ✅ |
| **比热容** | ❌ | **✅ 必需** |
| 密度 | ❌ | **✅ 必需** |
| 弹性模量 | ❌ | ❌ |
| 泊松比 | ❌ | ❌ |

> 瞬态热必须设置比热容和密度，因为需要计算热容 C = ρ·Cp·V。

---

## 7. 结果查询扩展

瞬态热结果包含时间维度：

```cpp
// 查询结果类型
czwsResultInqTypes(256, &types, &numTypes);
// 返回可能包含: "Temperature", "Heat Flux", "Temperature Gradient", 
//               以及带时间标签的版本如 "Temperature @ t=10s"

// 查询结果数据时可能需要指定时间步
czwsResultTypeInqData("Temperature", "Total", &num, &results, &ids);
```

---

## 8. 已知限制

| 限制 | 说明 |
|------|------|
| 对流/辐射载荷 | 需通过原生命令 `ZwThConvLoad`/`ZwThRadLoad` 实现 |
| 时间步设置 API | 文档未完全暴露，可能需原生命令通道 |
| 非线性材料属性 | 温度相关的导热系数/比热容需确认 API 支持 |
| 结果时间步索引 | 查询特定时刻结果的方法待确认 |

---

## 9. 模板代码

```cpp
// 创建瞬态热任务 + 初始热效应
nlohmann::json CreateTransientThermalTask(const nlohmann::json& data) {
    nlohmann::json resp;
    
    double totalTime = data.value("totalTime", 100.0);
    double timeStep = data.value("timeStep", 0.1);
    
    // 1. 创建任务
    zwsDbId idxTask;
    ZWSimEvxErrors err = czwsSimStCreateTask(ZW_ST_TASK_TRANSIENT_THERMAL, &idxTask);
    if (err != ZWSIM_API_NO_ERROR) {
        resp["status"] = "error";
        resp["error"] = "Failed to create transient thermal task";
        return resp;
    }
    
    // 2. 设置时间步参数（可能需要原生命令）
    // 等待 API 支持或通过 cvxCmdSend 发送
    
    // 3. 返回成功
    resp["status"] = "success";
    resp["data"]["taskId"] = idxTask;
    resp["data"]["taskType"] = "transient_thermal";
    resp["data"]["totalTime"] = totalTime;
    resp["data"]["timeStep"] = timeStep;
    
    return resp;
}

// 设置初始热效应
nlohmann::json SetInitialThermalEffect(int nStep) {
    szwsInitThermEffectData initThermData;
    czwsSimStInitialThermalEffectDataInit(&initThermData);
    initThermData.iStep = nStep;
    
    zwsDbId idxInitTherm;
    ZWSimEvxErrors err = czwsSimStCreateInitialThermalEffect(&initThermData, &idxInitTherm);
    
    czwsSimStFreeInitialThermalEffectData(&initThermData);
    
    // 返回结果...
}
```
