# 02 - IPC 消息协议规范

> **文件**: `docs/02-ipc-protocol.md` | **部分**: SKILL.md §2.3

---

## 一、协议概述

- **传输层**: Named Pipe — `\\.\pipe\Zw3dPMBridgePipe`
- **数据格式**: JSON + `\n` 行分隔
- **方向**: Qt EXE (主进程) ↔ ZW3D DLL (嵌入插件)

---

## 二、请求格式

```json
// Qt → DLL
{
  "cmd": <int>,
  "data": {
    <key-value pairs>
  }
}
```

**示例**：
```json
{
  "cmd": 250,
  "data": {
    "globalSize": 5.0,
    "minSize": 1.0,
    "meshType": 0
  }
}
```

---

## 三、响应格式

### 成功响应

```json
{
  "cmd": <int>,
  "status": "success",
  "data": { ... }
}
```

### 失败响应

```json
{
  "cmd": <int>,
  "status": "error",
  "error": "可读错误描述",
  "errorCode": <int>,
  "debug": { ... }
}
```

### 字段说明

| 字段 | 类型 | 说明 |
|------|------|------|
| `cmd` | int | 回显请求的命令号，必须一致 |
| `status` | string | 只能是 `"success"` 或 `"error"` |
| `data` | object | 成功时的数据载荷 |
| `error` | string | 失败时的用户可读描述 |
| `errorCode` | int | 程序可判断的错误码（负数系统错误，正数业务错误） |
| `debug` | object | 排查用上下文（可选） |

---

## 四、回包铁律

1. `cmd` 字段必须与请求的 cmd **完全一致**
2. `status` 只能是 `success` 或 `error`，不得有其他值
3. 错误时必须同时提供 `error`（文本）和 `errorCode`（数字）
4. 响应必须换行符 `\n` 结尾，确保行协议正确解析

---

## 五、参考模板

| 模板文件 | 内容 |
|---------|------|
| `templates/ipc_message_template.cpp` | 完整 IPC 通信层实现（DLL 侧 dispatch + EXE 侧发送） |

---

*文档版本: v1.2.0 | 维护者: 韩天尊*
