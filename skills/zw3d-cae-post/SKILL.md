---
name: zw3d-cae-post
description: Use when implementing or debugging ZW3D 2026 CAE post-processing workflows, including result type queries, contour/cloud plots, CSV export, report generation, animations, thermal or structural result extraction, and bridge result-query commands.
---

# ZW3D 后处理 Skill — v1.3.0

> **触发词**: "后处理", "结果导出", "云图", "CSV", "报告", "动画", "post processing", "result export", "contour", "结果查询"
> **适用**: ZW3D 2026 ZWMeshWorks 结果查询、云图导出、报告生成
> **维护者**: 韩天尊
> **依赖**: [[zw3d-plugin-base\|plugin-base]] (必需), [[zw3d-cae-thermal\|cae-thermal]] 或 [[zw3d-cae-structural\|cae-structural]] (至少一个)

### 权威来源引用
| 内容 | 引用 |
|------|------|
| C++ API调用规范 | [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md]] |
| 质量门控 | [[.workbuddy/skills/zw3d-dev-team/docs/04-quality-gate\|04-quality-gate.md]] |
| 完整命令码表 | [[zw3d-ipc-comm#七完整命令码表\|ipc-comm §七]] |

---

## 一、结果类型

| 编码 | 类型 | 说明 |
|------|------|------|
| 1 | Displacement | 位移 (Total/X/Y/Z) |
| 2 | Stress | 应力 (Von Mises/Principal/Max/Min) |
| 3 | Strain | 应变 (Equivalent/Principal/Max/Min) |
| 4 | Temperature | 温度 |
| 5 | Reaction Force | 支反力 (X/Y/Z/Total) |
| 6 | Heat Flux | 热流密度 |

---

## 二、结果查询API

> 热分析中的结果查询代码示例见 [[zw3d-cae-thermal#七结果查询\|cae-thermal §七]]。此处补充后处理特有API。

### 2.1 查询结果类型
```cpp
int numTypes = 0;
char** types = NULL;
int err = czwsResultInqTypes(256, &types, &numTypes);  // 256=最大类型数
// 返回: "Temperature", "Heat Flux", "Displacement", "VonMises", "Frequency", "Buckle Factor"
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 2.2 查询子类型
```cpp
int numSubTypes = 0;
char** subTypes = NULL;
int err = czwsResultInqSubTypes(resultType, &numSubTypes, &subTypes);
// 如热通量的子类型: "X", "Y", "Z", "Total"
if (err != ZWSIM_API_NO_ERROR) { /* 处理错误 */ }
```

### 2.3 查询/设置活动结果
```cpp
// 查询当前显示的结果类型
char activeType[256];
int err = czwsResultInqActiveType(activeType, sizeof(activeType));

// 切换显示的结果类型
int err = czwsResultSetActiveType("Temperature");
```

### 2.4 ZWMeshWorks内置报告生成
```cpp
int err = czwsResultReport(taskId, outputPath, format);
// format: "PDF" / "Word"
```

---

## 三、云图PNG导出 (cmd=500) 📐 设计草案

> ⚠️ cmd=500 未在 `Common/define.h` 中注册。以下为设计参考，待实现后验证。

### 请求
```json
{
  "cmd": 500,
  "data": {
    "taskType": "steady_thermal",
    "resultType": 4,
    "contourType": 0,
    "timeStep": 0,
    "outputPath": "D:/output/thermal_contour.png",
    "width": 1920,
    "height": 1080,
    "colorBar": true,
    "legend": true,
    "showMesh": false
  }
}
```

### 云图类型
| 编码 | 类型 | 说明 |
|------|------|------|
| 0 | Total | 总量 |
| 1 | X | X分量 |
| 2 | Y | Y分量 |
| 3 | Z | Z分量 |
| 4 | Max | 最大值 |
| 5 | Min | 最小值 |

---

## 四、CSV数据导出 (cmd=501) 📐 设计草案

> ⚠️ cmd=501 未在 `Common/define.h` 中注册。以下为设计参考，待实现后验证。

### 请求
```json
{
  "cmd": 501,
  "data": {
    "taskType": "steady_thermal",
    "resultType": 4,
    "timeStep": 0,
    "dataType": "node",
    "outputPath": "D:/output/result_data.csv",
    "includeHeader": true,
    "precision": 6
  }
}
```

### CSV输出格式
```csv
NodeID,X,Y,Z,Total
1001,0.001234,0.002345,0.003456,0.004567
1002,-0.000123,0.001234,-0.002345,0.002678
```

---

## 五、曲线数据导出 (cmd=502) 📐 设计草案

> ⚠️ cmd=502 未在 `Common/define.h` 中注册。以下为设计参考，待实现后验证。

### 曲线类型
| 编码 | 类型 | 说明 |
|------|------|------|
| 0 | Node | 节点历史 |
| 1 | Element | 单元历史 |
| 2 | Section | 截面历史 |
| 3 | Global | 全局响应 |

### 请求
```json
{
  "cmd": 502,
  "data": {
    "taskType": "transient_thermal",
    "curveType": 0,
    "entityId": "entity_1",
    "resultType": 4,
    "outputPath": "D:/output/curve_data.csv"
  }
}
```

### CSV输出格式 (瞬态分析曲线)
```csv
Step,Time,Value
0,0.0,0.000000
1,0.1,0.001234
2,0.2,0.002468
```

---

## 六、仿真报告生成 (cmd=503) 📐 设计草案

> ⚠️ cmd=503 未在 `Common/define.h` 中注册。以下为设计参考，待实现后验证。

### 请求
```json
{
  "cmd": 503,
  "data": {
    "taskType": "steady_thermal",
    "reportTitle": "热仿真分析报告",
    "outputPath": "D:/output/report.html",
    "sections": {
      "summary": true,
      "contours": true,
      "keyResults": true,
      "tables": true
    },
    "includeImages": true,
    "format": "html"
  }
}
```

### 报告结构
```
<h1>报告标题</h1>
1. 仿真摘要 (分析类型、求解器版本、收敛状态、迭代次数)
2. 关键统计 (最大值、最小值、平均值、RMS)
3. 结果云图 (嵌入PNG图片)
4. 数据表格
5. 结论 (可选)
```

---

## 七、结果数据查询 (cmd=504) 📐 设计草案

> ⚠️ cmd=504 未在 `Common/define.h` 中注册。以下为设计参考，待实现后验证。

### 请求
```json
{
  "cmd": 504,
  "data": {
    "taskType": "steady_thermal",
    "resultType": 4,
    "timeStep": 0,
    "queryType": "node",
    "entityId": "entity_1"
  }
}
```

### 响应
```json
{
  "cmd": 504,
  "status": "success",
  "data": {
    "resultType": "temperature",
    "values": {"X": 0, "Y": 0, "Z": 0, "Total": 85.3},
    "unit": "°C",
    "nearbyEntity": "node_1001"
  }
}
```

---

## 八、动画导出 (cmd=505) 📐 设计草案

> ⚠️ cmd=505 未在 `Common/define.h` 中注册。以下为设计参考，待实现后验证。

### 请求
```json
{
  "cmd": 505,
  "data": {
    "taskType": "transient_thermal",
    "resultType": 4,
    "timeSteps": [0, 1, 2, 3, 4, 5],
    "outputPath": "D:/output/thermal_animation.gif",
    "frameRate": 10,
    "width": 1280,
    "height": 720,
    "colorBar": true
  }
}
```

---

## 九、多工况查询 (cmd=506) 📐 设计草案

> ⚠️ cmd=506 未在 `Common/define.h` 中注册。以下为设计参考，待实现后验证。

### 请求
```json
{
  "cmd": 506,
  "data": {
    "taskType": "static",
    "resultType": 1,
    "entities": ["entity_1", "entity_2"],
    "loadCases": ["case_A", "case_B", "case_C"]
  }
}
```

### 响应 (矩阵形式)
```json
{
  "cmd": 506,
  "status": "success",
  "data": {
    "matrix": [
      [0.123, 0.234, 0.345],
      [0.456, 0.567, 0.678]
    ],
    "labels": {
      "rows": ["entity_1", "entity_2"],
      "cols": ["case_A", "case_B", "case_C"]
    }
  }
}
```

---

## 十、IPC命令号段 (完整表见 [[zw3d-ipc-comm#七完整命令码表\|ipc-comm §七]])

> ⚠️ 以下命令均为 📐 设计草案，未在 `Common/define.h` 中注册，仅作为设计参考。

| cmd | 名称 | 说明 | 状态 |
|-----|------|------|------|
| 261 | 结果查询 | 已实现 (`QueryResults`) | ✅ 已实现 |
| 262 | 结果类型查询 | 已实现 (`QueryResultTypes`) | ✅ 已实现 |
| 263 | 结果子类型查询 | 已实现 (`QueryResultSubtypes`) | ✅ 已实现 |
| 264 | 结果导出 | 已注册待实现 (`ResultExport`) | 📐 设计草案 |
| 500 | 云图PNG导出 | 结果云图截图 | 📐 设计草案 |
| 501 | CSV数据导出 | 节点/单元数据 | 📐 设计草案 |
| 502 | 曲线导出 | 历史曲线数据 | 📐 设计草案 |
| 503 | 报告生成 | HTML仿真报告 | 📐 设计草案 |
| 504 | 结果查询 | 精确位置查询 | 📐 设计草案 |
| 505 | 动画导出 | GIF/视频 | 📐 设计草案 |
| 506 | 多工况查询 | 载荷工况矩阵 | 📐 设计草案 |

---

## 十一、参数约定

| 参数 | 约定 |
|------|------|
| taskType | `"steady_thermal"` / `"transient_thermal"` / `"static"` / `"frequency"` / `"buckling"` |
| resultType | 1-6整数编码 (见结果类型表) |
| timeStep | 稳态固定为0，瞬态为0-based索引 |
| contourType | 0-5 (见云图类型表) |
| dataType | `"node"` 或 `"element"` |

---

## 十二、瞬态热结果查询扩展

瞬态热结果包含时间维度，查询时需指定时间步索引:
```cpp
czwsResultTypeInqData(taskId, "Temperature", timeStepIndex, &count, &pResults);
```

---

## 十三、资源

| 资源 | 路径 |
|------|------|
| 后处理模板 | `sample_templates/ResultExportAPI_template.cpp` |
| CAE功能清单 | [[知识库/02_CAE开发/ZWMeshWorks_CAE_插件_功能清单\|功能清单.md]] |
| 稳态热开发计划 | [[知识库/02_CAE开发/稳态传热模块开发计划\|稳态传热模块开发计划.md]] |

---

*文档版本: v1.3.0 | 维护者: 韩天尊*
