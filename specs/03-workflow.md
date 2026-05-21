# 03 - 标准开发工作流

> **文件**: `docs/03-workflow.md` | **部分**: SKILL.md §5 + AGENT.md §3

---

## 场景 1：开发新 CAE 模块

```
Step 1: 需求澄清
        - 仿真类型 (稳态热/瞬态热/结构/其他)
        - 需要哪些功能 (载荷/网格/求解/后处理)
        - 实体来源 (用户选择/规则解析/全部)

Step 2: 号段确认
        → 查 Common/define.h TaskType 枚举确认 cmd 号段是否冲突
        → 完整命令码表见 zw3d-ipc-comm SKILL.md §七
        → 如果冲突，协商调整

Step 3: 选择模板
        → 从 templates/ 选最接近的模板
        → 明确需要增删哪些接口

Step 4: 模板增补
        → 修改对应 API 模板文件
        → 在 commands/commands_registry.md 中登记新命令

Step 5: DLL 侧实现
        → 在 CaeHandle.cpp 添加处理函数
        → ZwHandleXXXCmd 中增加 dispatch case

Step 6: Qt 侧对接
        → 添加 Widget/UI
        → ZW3DHelper::HandleIPCMsg 中增加响应处理

Step 7: 自检
        → 过 Pre-Commit Checklist（见 04-quality-gate.md）
        → 过代码审查重点（见 04-quality-gate.md §6.2）

Step 8: 经验沉淀
        → 发现新问题？在工程经验卡片加条目
        → 在 07-experience-index.md 中加引用
```

---

## 场景 2：排查 Bug

```
Step 1: 定位类型
        → 编译问题？ → 查 K-050~K-056
        → IPC 通信问题？ → 查 K-001~K-007
        → 实体选择失败？ → 查 K-010~K-012
        → 网格划分失败？ → 查 K-020
        → 热载荷不生效？ → 查 K-030~K-031

Step 2: 收集日志
        → 入参 (cmd/data)
        → 上下文 (activeTaskId/activeTaskType)
        → 实体来源 (cache/simGeom/fallback)
        → 错误码 (error/errorCode/fallbackErrorCode)

Step 3: 对照经验卡片
        → 是否有相同现象？
        → 是否有已知解决方案？
        → 是否需要在卡片中追加新经验？

Step 4: 实施修复
        → 优先参考卡片的"解决方案"
        → 如果卡片无解，新增一条卡片

Step 5: 更新文档
        → 新发现记录到工程经验卡片
        → 通知团队成员
```

---

## 场景 3：代码审查

```
Step 1: 内存管理审查
        → 所有 cvxMemAlloc 是否都有 cvxMemFree？
        → 指针释放后是否置 NULL？
        → 异常退出时是否有资源泄漏？

Step 2: 线程安全审查
        → 是否有在非 WndProc 中调用 ZW3D API 的情况？
        → g_currentTask 访问是否加锁？
        → IPC 线程是否有断线重连泄漏？

Step 3: IPC 协议审查
        → cmd 回包是否一致？
        → data 层次是否统一（外层 vs 内层）？
        → 错误码是否规范（errorCode 和 error 是否都有）？

Step 4: 实体引用审查
        → 是否用 cacheId 而非原始句柄？
        → cacheId 对应的实体是否存在？
        → cacheId 是否及时清理？

Step 5: 报告结论
        → 列出必须修复项（blocker）
        → 列出建议改进项（建议）
        → 关联到对应的工程经验卡片
```

---

*文档版本: v1.3.0 | 维护者: 韩天尊*
