---
name: zw3d-plugin-base
description: Use when developing ZW3D 2026 C++ DLL plugins, including plugin skeletons, cvx/Zw API usage, command registration, form UI, memory management, encoding conversion, deployment to apilibs, or debugging ZW3D plugin loading.
---

# ZW3D 插件基础 Skill — v1.3.0

> **触发词**: "创建ZW3D插件", "zw3d dll", "cvx API", "命令注册", "表单UI", "内存管理", "插件骨架", "zw3d plugin", "VxApi"
> **适用**: ZW3D 2026 C++ DLL插件基础开发
> **维护者**: 韩天尊
> **依赖**: 无

### 权威来源引用
| 内容 | 引用 |
|------|------|
| C++ API调用规范 | [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md]] |
| 质量门控/Checklist | [[.workbuddy/skills/zw3d-dev-team/docs/04-quality-gate\|04-quality-gate.md]] |
| 开发工作流 | [[.workbuddy/skills/zw3d-dev-team/docs/03-workflow\|03-workflow.md]] |

---

## 一、开发环境

| 项目 | 配置 |
|------|------|
| **ZW3D版本** | 2026 |
| **编译环境** | Visual Studio 2022, x64, Debugx64/Release |
| **环境变量** | `ZW3D_DIR` = ZW3D安装目录 |
| **头文件** | `%ZW3D_DIR%\api\inc\VxApi.h` |
| **部署路径** | `%ZW3D_DIR%\apilibs\` (DLL + 同名.zrc) |
| **资源编译** | `zrc.exe <项目目录> -o <项目目录>\XXX.zrc` |

> `.zrc`文件名必须与`.dll`文件名一致。`apilibs`文件夹需手动创建。

### 构建命令
```powershell
msbuild "Project.vcxproj" /p:Configuration=Debugx64 /p:Platform=x64
```

### 调试方法
- VS→调试→附加到进程→选择`zw3d.exe`
- 项目属性→调试→命令设为`zw3d.exe`路径→环境`_No_Debug_Heap=1`→F5

---

## 二、三条铁律 (违反即Bug)

> 完整规范见 [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md §一]]

1. **ZW3D API必须在主线程调用** — 通过`PostMessage`调度到WndProc，禁止在后台线程直接调用
2. **API返回指针必须配对释放** — `cvxMemAlloc((void**)&ptr, size)` ↔ `cvxMemFree((void**)&ptr)`，禁止`new`/`delete`
3. **禁用C++异常** — 用返回值判断错误(0=成功)，不用try/catch

---

## 三、双版本API架构

### 3.1 传统CAPI (cvx系列)
| 类型 | 前缀 | 示例 |
|------|------|------|
| 函数 | `cvx` | `cvxPartBox()`, `cvxMemAlloc()` |
| 枚举 | `evx` | `evxEntType`, `evxErrors` |
| 结构体 | `svx` | `svxEntPath`, `svxAreaProp` |
| 类型 | `vx` | `vxName`, `vxPath`, `vxLongName` |
| 宏 | `VX_` | `VX_CODE_GENERAL` |

### 3.2 新API (Zw系列)
| 类型 | 前缀 | 示例 |
|------|------|------|
| 函数 | `Zw` | `ZwEntityCreate()`, `ZwFileOpen()` |
| 枚举 | `ezw` | `ezwEntityType`, `ezwHandleType` |
| 结构体 | `szw` | `szwEntityData`, `szwEntityHandle` |

### 3.3 新旧API转换
| 转换方向 | 接口 |
|----------|------|
| ID → Handle | `ZwEntityIdTransfer()` |
| Pick Path → Handle | `ZwEntityPathTransfer()` |
| Handle → ID | `ZwEntityIdGet()` |
| Handle → Pick Path | `ZwEntityPathGet()` |

---

## 四、DLL插件架构

### 4.1 标准结构
```cpp
#include "VxApi.h"
#include <cstring>

#define BUFFER 256

int PluginNameInit(int format, void *data)
{
    cvxCmdFunc("MyCommand", (void*)MyCommand, VX_CODE_GENERAL);

    vxPath ApiPath;
    cvxPathApiLib("PluginName", ApiPath);
    cvxPathAdd(ApiPath);

    return 0;
}

int PluginNameExit(void)
{
    cvxCmdFuncUnload("MyCommand");
    return 0;
}

int MyCommand(int idData)
{
    /* 命令逻辑 */
    return 0;
}
```

### 4.2 .def文件
```def
LIBRARY PluginName.dll
EXPORTS
    PluginNameInit
    PluginNameExit
    MyCommand
```

### 4.3 命令调用方式
| 前缀 | 类型 | 示例 |
|------|------|------|
| `~` | 一般命令 | `~MyCommand` |
| `!` | 交互命令 | `!MyCommand` |
| `$SF=` | 激活对话框 | `$SF=MyCommand` |

---

## 五、核心模式

### 5.1 内存安全查询
> 完整模式见 [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md §二]]

```cpp
int nRoots = 0;
int *pRootIds = NULL;

int result = cvxRootList(&nRoots, &pRootIds);
if (result) {
    cvxMsgDisp("Failed to get root list");
    return result;
}

if (nRoots == 0) {
    cvxMsgDisp("No root objects found");
    return 0;
}

for (int i = 0; i < nRoots; i++) {
    char sBuf[BUFFER];
    sprintf_s(sBuf, BUFFER, "Root[%d]: id=%d", i, pRootIds[i]);
    cvxMsgDisp(sBuf);
}

cvxMemFree((void**)&pRootIds);  /* 必须释放! */
return 0;
```

### 5.2 表单UI
```cpp
int ShowDialog(int idData)
{
    int result = cvxFormCreate("MyDialog", 0);
    if (result) return result;

    cvxItemDel("MyDialog", 1, -1);
    cvxItemAdd("MyDialog", 1, "参数:");
    cvxFormCallback("MyDialog", (void*)DialogCallback);
    cvxFormShow("MyDialog");
    return 0;
}

int DialogCallback(char* formName, int field, int idData)
{
    switch (field) {
        case 1: break;
        default: break;
    }
    return 0;
}
```

### 5.3 编码转换
ZW3D API使用本地编码(GBK/CP_ACP)，Qt使用UTF-8:
```cpp
// DLL → ZW3D: UTF-8 → 本地编码
const char* acpStr = Utf8ToLocalAcp(utf8Str);
// ZW3D → DLL: 本地编码 → UTF-8
std::string utf8Str = LocalAcpToUtf8(acpStr);
```

---

## 六、API功能速查 (20个官方示例)

### 基础查询 (1.BaseInquiry)
| API | 说明 | 命令 |
|-----|------|------|
| `cvxRootList` | 获取根对象列表 | `~InqRoot` |
| `cvxPartInqCompsInfo` | 查询组件信息 | `~InqPartComp` |
| `cvxPartInqVars` | 查询变量 | `~InqPartVar` |

### 拓扑查询 (2.TopoInquiry)
| API | 说明 | 命令 |
|-----|------|------|
| `cvxPartInqShpFace` | 查询面 | `!InqPartShpFace` |
| `cvxPartInqShpEdge` | 查询边 | `!InqPartShpEdge` |
| `cvxFaceInqLoops` | 查询环 | `!InqFaceLoop` |
| `cvxEdgeInqUVCrv` | 查询UV曲线 | `!InqEdgeUVCrv` |

### 视图/图层 (3.ViewTool)
| API | 说明 |
|-----|------|
| `cvxLayerInq/Set/Add/Del` | 图层操作 |
| `cvxEntColorGet/Set` | 实体颜色 |
| `cvxViewGet/Set` | 视图操作 |
| `cvxDispModeSet` | 显示模式 |

### 几何创建 (4-8)
| API | 说明 | 示例 |
|-----|------|------|
| `cvxPartBox` | 创建盒体 | 4.FilletBox |
| `cvxPartFillet` | 倒圆角 | 4.FilletBox |
| `cvxPartPatternLinear` | 线性阵列 | 6.Pattern |
| `cvxPartArc` | 创建圆弧 | 7.CurveCreate |
| `cvxPartFace` | 创建曲面 | 8.NurbsSurface |

### 文件操作 (5,9,10)
| API | 说明 |
|-----|------|
| `cvxFileCreate/Copy` | 文件创建/复制 |
| `cvxFileExportImg/Pdf` | 导出图片/PDF |
| `cvxFileSaveIgs/LoadIgs` | IGES导入导出 |

### 高级功能
| 功能 | 示例 |
|------|------|
| 辅助坐标系/移动 | 11.OptAuxframe |
| DLL注册自启动 | 12.DllRegister |
| 表格控件 | 13.TableSet |
| 变量编辑 | 15.EditVariable |
| 带预览拉伸 | 17.ExTrudewithPreview |
| 倒角增删 | 20.ChamferAddAndDelete |

---

## 七、资源

| 资源 | 路径 |
|------|------|
| 官方示例(20个) | `D:\ZW3D\ZWapi\ApiExample\` |
| 插件模板 | `sample_templates/zw3d_plugin_template.cpp` |
| API参考手册 | [[知识库/01_ZW3D_API/ZW3D_API参考手册\|ZW3D_API参考手册.md]] |
| 技术路线详解 | [[知识库/02_CAE开发/Z3ParametricModeling_技术路线详解\|技术路线详解.md]] |
