# ZW3D 二次开发 Skills & 团队规范

> ZW3D 2026 C++ DLL插件开发 + CAE自动化 知识体系
>
> **版本**: v1.2.0 | **维护者**: 韩天尊 | **日期**: 2026-05-18

---

## 快速开始

### 给同事使用

1. 解压此包到项目根目录
2. 阅读 `skills/README.md` — 新人5步入门路线
3. 按模块选择对应 skill 查看 API 和工作流
4. 遇到规范问题查 `specs/` 目录

### 安装到项目

```bash
# 方式1: 直接复制到项目
cp -r skills/ <your-project>/.agent/skills/
cp -r specs/ <your-project>/.workbuddy/skills/zw3d-dev-team/docs/
cp -r templates/ <your-project>/sample_templates/

# 方式2: 作为 Git submodule
git submodule add <repo-url> .agent/skills/zw3d-dev-kit
```

---

## 目录结构

```
ZW3D-Dev-Skills-Package/
├── README.md                          ← 你在这里
├── skills/                            ← AI触发式领域技能 (7个)
│   ├── README.md                      ← 导航入口 + 新人路线
│   ├── zw3d-plugin-base/              ← 插件基础 (DLL架构/API速查)
│   ├── zw3d-ipc-comm/                 ← 双进程通信 (IPC/JSON/cacheId)
│   ├── zw3d-cae-thermal/              ← 热分析 (稳态/瞬态)
│   ├── zw3d-cae-structural/           ← 结构分析 (静力/模态/屈曲)
│   ├── zw3d-cae-mesh/                 ← 几何清理与网格
│   ├── zw3d-cae-optimization/         ← 参数优化 (MMFD/SLP/SQP)
│   └── zw3d-cae-post/                 ← 后处理 (云图/CSV/报告)
├── specs/                             ← 团队规范 (权威来源)
│   ├── 01-spec-cpp.md                 ← C++ API调用规范
│   ├── 02-ipc-protocol.md             ← IPC消息协议
│   ├── 03-workflow.md                 ← 标准开发工作流
│   └── 04-quality-gate.md             ← 质量门控
└── templates/                         ← 开发模板 (9个.cpp)
    ├── zw3d_plugin_template.cpp
    ├── ipc_message_template.cpp
    ├── entity_target_resolver_template.cpp
    ├── caeworkflow_template.cpp
    ├── SteadyThermalAPI_wrapper.cpp
    ├── TransientThermalAPI_wrapper.cpp
    ├── StructuralAnalysisAPI_template.cpp
    ├── ParameterOptimizationAPI_template.cpp
    └── ResultExportAPI_template.cpp
```

---

## 文档架构

| 系统 | 路径 | 定位 |
|------|------|------|
| **Skills** | `skills/` | AI触发式领域知识 — 写代码时自动加载 |
| **团队规范** | `specs/` | **权威来源** — 编码规范、IPC协议、质量门控 |
| **模板代码** | `templates/` | 9个可运行的.cpp模板 |

**权威来源原则**: 规范类内容以 `specs/` 为唯一真相源。Skills 中通过 wiki 链接引用，不重复维护。

---

## 新人路线

```
第1步 → skills/zw3d-plugin-base    基础规范、DLL架构、API速查（必读）
第2步 → skills/zw3d-ipc-comm       双进程通信、JSON协议、cacheId机制（必读）
第3步 → 按模块选一个               thermal / structural / mesh / optimization / post
第4步 → 遇到问题查经验卡片          各skill末尾的经验卡片
第5步 → 参考specs/和templates/     规范文档 + 模板代码
```

---

## 版本变更

| 版本 | 日期 | 变更内容 |
|------|------|---------|
| v1.2.0 | 2026-05-18 | 文档架构重构：明确权威来源、消除规范重叠、wiki链接体系、新人导航 |
| v1.1.0 | 2026-05-18 | 知识库深度整合：工程经验卡片、完整命令码表、API调用示例 |
| v1.0.0 | 2026-05-18 | 初始版本：7个模块化skill拆分 |
