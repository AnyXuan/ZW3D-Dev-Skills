# ZW3D 二次开发 Skills — 导航入口

> **维护者**: 韩天尊 | **版本**: v1.3.0 | **最后更新**: 2026-05-21
> **适用**: ZW3D 2026 C++ DLL插件开发、CAE自动化、参数化建模

---

## 文档架构说明

本项目有三套文档系统，各有定位，**不可互相替代**：

| 系统 | 路径 | 定位 | 读者 |
|------|------|------|------|
| **Skills** | `.agent/skills/` | AI触发式领域知识 — 写代码时自动加载 | AI + 开发者 |
| **团队规范** | `.workbuddy/skills/zw3d-dev-team/` | **权威来源** — 编码规范、IPC协议、质量门控、工作流 | 所有团队成员 |
| **知识库** | `知识库/` | 原始资料 — API文档、工程经验卡片、架构文档、开发计划 | 所有团队成员 |

### 权威来源原则

**规范类内容**（铁律、命名法、IPC协议、Pre-Commit Checklist、工作流）以 `.workbuddy/skills/zw3d-dev-team/docs/` 为唯一真相源。Skills 中只保留领域特定知识，通过 wiki 链接引用规范。

**号段表**以 `zw3d-ipc-comm/SKILL.md §七` 为唯一真相源。各 domain skill 中的号段表只列出本域相关条目，并链接回主表。

**经验卡片**以 `知识库/02_CAE开发/工程经验_结构化.md` 为唯一真相源。Skills 中只保留与本域相关的卡片摘要，并链接回原文。

---

## 新人入门路线

```
第1步 → zw3d-plugin-base    基础规范、DLL架构、API速查（必读）
第2步 → zw3d-ipc-comm       双进程通信、JSON协议、cacheId机制（必读）
第3步 → 按模块选一个        thermal / structural / mesh / optimization / post
第4步 → 遇到问题查经验卡片   各skill末尾的经验卡片 + 知识库/工程经验_结构化.md
第5步 → 深入看知识库/       API参考、架构文档、开发计划
```

---

## Skills 索引

### 基础层

| Skill | 触发词 | 维护者 | 说明 |
|-------|--------|--------|------|
| [[zw3d-plugin-base\|插件基础]] | "创建ZW3D插件", "zw3d dll", "cvx API", "命令注册", "表单UI" | 韩天尊 | DLL架构、API速查(20个示例)、内存管理、编码转换 |
| [[zw3d-ipc-comm\|双进程通信]] | "IPC", "Named Pipe", "JSON协议", "cacheId", "实体解析" | 韩天尊 | 通信架构、28个命令码表、实体选择、经验卡片K-001~K-011 |

### CAE领域层

| Skill | 触发词 | 维护者 | 依赖 |
|-------|--------|--------|------|
| [[zw3d-cae-thermal\|热分析]] | "稳态热", "瞬态热", "温度载荷", "热通量" | 韩天尊 | plugin-base, ipc-comm, cae-mesh |
| [[zw3d-cae-structural\|结构分析]] | "结构分析", "静力学", "模态", "屈曲", "力载荷" | 韩天尊 | plugin-base, ipc-comm, cae-mesh |
| [[zw3d-cae-mesh\|几何清理与网格]] | "几何清理", "网格划分", "mesh", "中面抽取" | 韩天尊 | plugin-base |
| [[zw3d-cae-optimization\|参数优化]] | "参数优化", "MMFD", "SLP", "SQP", "设计变量" | 韩天尊 | plugin-base, ipc-comm, thermal或structural |
| [[zw3d-cae-post\|后处理]] | "后处理", "结果导出", "云图", "CSV", "报告" | 韩天尊 | plugin-base, thermal或structural |

---

## 团队规范 (权威来源)

> 以下内容以 `.workbuddy/skills/zw3d-dev-team/docs/` 为准，Skills 中不重复。

| 规范 | 路径 | 内容 |
|------|------|------|
| **C++ API调用规范** | [[.workbuddy/skills/zw3d-dev-team/docs/01-spec-cpp\|01-spec-cpp.md]] | 铁律、安全查询模式、匈牙利命名法 |
| **IPC消息协议** | [[.workbuddy/skills/zw3d-dev-team/docs/02-ipc-protocol\|02-ipc-protocol.md]] | 请求/响应格式、回包铁律 |
| **标准开发工作流** | [[.workbuddy/skills/zw3d-dev-team/docs/03-workflow\|03-workflow.md]] | 开发新模块/排查Bug/代码审查 3大场景 |
| **质量门控** | [[.workbuddy/skills/zw3d-dev-team/docs/04-quality-gate\|04-quality-gate.md]] | Pre-Commit Checklist、审查重点、错误码速查 |

---

## 知识库导航

| 分类 | 路径 | 说明 |
|------|------|------|
| **API参考** | [[知识库/01_ZW3D_API\|01_ZW3D_API/]] | ZW3D_API参考手册、ZWMeshWorks API文档 |
| **CAE开发** | [[知识库/02_CAE开发\|02_CAE开发/]] | 工程经验卡片、代码架构、功能清单、开发计划 |
| **项目文档** | [[知识库/03_项目文档\|03_项目文档/]] | 编译说明、UI优化、多工况功能 |
| **总索引** | [[知识库/00_INDEX\|00_INDEX.md]] | 知识库全文档导航 |

### 高频文档

| 文档 | 路径 | 何时看 |
|------|------|--------|
| 工程经验卡片 | [[知识库/02_CAE开发/工程经验_结构化\|工程经验_结构化.md]] | 排查Bug时优先查 |
| 代码架构 | [[知识库/02_CAE开发/代码架构\|代码架构.md]] | 理解项目整体结构 |
| 技术路线详解 | [[知识库/02_CAE开发/Z3ParametricModeling_技术路线详解\|技术路线详解.md]] | 窗口嵌入、IPC、线程安全 |
| CAE功能清单 | [[知识库/02_CAE开发/ZWMeshWorks_CAE_插件_功能清单\|功能清单.md]] | 84+功能总表 |
| CAE接口设计 | [[知识库/02_CAE开发/CAE自动化接口设计_一期\|接口设计_一期.md]] | cmd=300~313变量层/目标解析层 |
| 稳态热开发计划 | [[知识库/02_CAE开发/稳态传热模块开发计划\|稳态传热模块开发计划.md]] | Phase 1-7详细方案 |

---

## 外部资源

| 资源 | 路径 | 说明 |
|------|------|------|
| 官方示例(20个) | `D:\ZW3D\ZWapi\ApiExample\` | ZW3D官方API示例工程 |
| 开发模板(9个) | `sample_templates/` | 可运行的模板代码 |
| 参数化项目 | `Z3ParametricModeling-main/` | 完整项目代码 |

---

## 版本变更日志

| 版本 | 日期 | 变更内容 |
|------|------|---------|
| v1.3.0 | 2026-05-21 | 全面校准：修正所有phantom cmd ID并标记📐设计草案、修正热分析API名与字段、补充热-结构耦合文档(cmd=295/296)、修正网格fallback模式、统一版本号 |
| v1.2.0 | 2026-05-18 | 文档架构重构：明确三套系统权威来源、消除规范类重叠、添加wiki链接、新增新人导航 |
| v1.1.0 | 2026-05-18 | 知识库深度整合：补充工程经验卡片、完整命令码表、API调用示例 |
| v1.0.0 | 2026-05-18 | 初始版本：7个模块化skill拆分 |
