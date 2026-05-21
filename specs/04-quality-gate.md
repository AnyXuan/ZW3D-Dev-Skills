# 04 - 质量门控

> **文件**: `docs/04-quality-gate.md` | **部分**: SKILL.md §6

---

## 一、Pre-Commit Checklist

提交前逐项确认：

- [ ] 所有 `cvxMemAlloc` 都有配对 `cvxMemFree`
- [ ] 所有 API 返回值都检查了（result != 0 走错误分支）
- [ ] 没有使用 `new`/`delete`
- [ ] 没有使用 C++ 异常
- [ ] 命名遵循匈牙利约定
- [ ] 错误信息通过 `cvxMsgDisp()` 输出
- [ ] IPC 命令号段在 `commands/commands_registry.md` 中登记
- [ ] IPC 回包格式统一为 `{"cmd": X, "status": "...", "data": {...}}`
- [ ] 实体引用使用 `cacheId` 而非原始句柄
- [ ] ZW3D API 调用通过 `PostMessage` 调度到主线程
- [ ] 新增 CAE API 调用与 `CaeApiWrapper.h` 中的 inline 函数签名一致

---

## 二、代码审查重点

| 审查项 | 检查点 | 对应经验卡片 |
|--------|--------|-------------|
| **内存** | Alloc/Free 是否配对，指针释放后是否置 NULL | K-005 |
| **线程** | 是否在 WndProc 中调用 API，g_currentTask 访问是否加锁 | K-003, K-004 |
| **IPC** | cmd 回包是否一致，data 层次是否统一 | K-002, K-005 |
| **实体** | 是否用 cacheId，cacheId 是否及时清理 | K-001 |
| **命令** | cvxCmdSend 是否混用 vxSend 前缀 | K-006 |
| **日志** | 关键调用是否回传了入参、上下文、错误码 | K-040 |

---

## 三、常见错误码速查

| 错误码 | 含义 | 优先检查 |
|--------|------|---------|
| **-10000** | 拾取 API 无命令上下文 | K-011 — 检查 cvxCmdFuncLoad 是否启用 |
| **-32** | 网格 API vendor 特定错误 | K-020 — fallback 到 ZwCeMeshing |
| **-3001** | 变量不存在 | AUTO_QUERY_VARIABLES / AUTO_SET_VARIABLES |
| **-3101** | 目标规则无匹配 | AUTO_RESOLVE_TARGET — 检查 nameTag/feature 是否准确 |
| **-3102** | 目标规则匹配到多个实体 | AUTO_RESOLVE_TARGET — 规则过于宽松 |
| **-3103** | 解析器类型不支持 | AUTO_RESOLVE_TARGET — 检查 resolver.type |

---

*文档版本: v1.3.0 | 维护者: 韩天尊*
