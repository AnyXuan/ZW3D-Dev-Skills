# 01 - C++ API 调用规范

> **文件**: `docs/01-spec-cpp.md` | **部分**: SKILL.md §2.1~§2.2

---

## 一、API 调用铁律

**违反即视为严重 bug**：

1. **`cvxMemAlloc` 必须配对 `cvxMemFree`** — 禁止 `new`/`delete` 管理 ZW3D API 返回的指针
2. **所有 API 返回值必须检查** — `result != 0` 立即走错误分支，不得忽略
3. **ZW3D API 必须在主线程调用** — 通过 `PostMessage` 调度到 WndProc，禁止在 IPC 线程直接调用
4. **禁用 C++ 异常** (`try`/`catch`)，用返回值判断错误
5. **API 返回列表必须遍历释放** — `for` 循环处理后，最后调用 `cvxMemFree` 一次

---

## 二、典型模式

```cpp
// 安全查询模式
int nItems = 0;
void *pItems = NULL;

int result = cvxQueryFunc(NULL, NULL, &nItems, &pItems);
if (result != 0) {
    cvxMsgDisp("API call failed");
    return result;
}

if (nItems == 0) {
    cvxMsgDisp("No items found");
    return 0;
}

// 使用数据
for (int i = 0; i < nItems; i++) {
    // process pItems[i]
}

// 必须释放！
cvxMemFree((void**)&pItems);
return 0;
```

---

## 三、匈牙利命名法

| 前缀 | 类型 | 示例 |
|------|------|------|
| `n` | int 计数/索引 | `nCmpCount`, `nStepIdx` |
| `d` | double 值 | `dTempValue`, `dMeshSize` |
| `id` | 实体 ID | `idBody01` |
| `p` | 指针 | `pEntityList` |
| `s` | 字符串 | `sTaskName` |
| `z` | 结构体 | `zThermalParams` |
| `h` | 句柄 | `hTaskHandle` |
| `cvx` | ZW3D API 前缀 | `cvxPartBox`, `cvxMemAlloc` |
| `cvv` | ZW3D API 结构体 | `cvvPartInfo` |

---

## 四、参考模板

| 场景 | 模板文件 |
|------|---------|
| 插件骨架 | `templates/zw3d_plugin_template.cpp` |
| IPC 通信 | `templates/ipc_message_template.cpp` |
| 实体解析 | `templates/entity_target_resolver_template.cpp` |

---

## 五、CaeApiWrapper.h 统一封装

项目使用 `Zw3dPMBridge/CaeApiWrapper.h` 作为所有 CAE API 的统一封装层。新增 CAE 功能时，优先使用 Wrapper 中的 inline 函数而非直接调用底层 `czws*` API。

**设计原则**:
- Wrapper 内联函数自动处理 Init → 设置字段 → Create → Free 的完整生命周期
- 统一错误检查和日志输出格式
- 结构体字段命名遵循 `eEntityType / iEntNum / sEntities / dValue` 等匈牙利变体

**参考**: `Zw3dPMBridge/CaeApiWrapper.h` (656行), 各 `Cae*Ops.cpp` 实现文件

---

*文档版本: v1.3.0 | 维护者: 韩天尊*
