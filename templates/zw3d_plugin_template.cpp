/*
 * ZW3D 插件模板 - 基础版本
 * ZW3D Version: 2026
 * Compile: Visual Studio 2022, x64, Debug/Release
 */

#include "VxApi.h"
#include <cstring>
#include <cstdlib>

#define BUFFER_SIZE 256

/* ============================================
 * 全局变量
 * ============================================ */
static bool g_bInitialized = false;

/* ============================================
 * 函数声明
 * ============================================ */
int PluginInit(int format, void *data);
int PluginExit(void);
int MyCommand(int idData);
int MyCallback(char* formName, int field, int idData);

/* ============================================
 * DLL入口点 (必需)
 * ============================================ */

/**
 * @brief 插件初始化 - ZW3D启动时自动调用
 * @param format 格式标识
 * @param data 扩展数据
 * @return 0=成功, 非0=失败
 */
int MyPluginInit(int format, void *data)
{
    // 1. 注册命令
    cvxCmdFunc("MyCommand", (void*)MyCommand, VX_CODE_GENERAL);
    
    // 2. 设置API库路径 (用于查找依赖的DLL)
    vxPath ApiPath;
    cvxPathApiLib("MyPlugin", ApiPath);
    cvxPathAdd(ApiPath);
    
    // 3. 初始化标志
    g_bInitialized = true;
    
    cvxMsgDisp("MyPlugin initialized successfully");
    return 0;
}

/**
 * @brief 插件退出 - ZW3D关闭时自动调用
 * @return 0=成功
 */
int MyPluginExit(void)
{
    // 1. 注销命令
    cvxCmdFuncUnload("MyCommand");
    
    // 2. 释放资源
    g_bInitialized = false;
    
    cvxMsgDisp("MyPlugin exited");
    return 0;
}

/* ============================================
 * 命令实现
 * ============================================ */

/**
 * @brief 示例命令 - 查询根对象信息
 * @param idData 命令参数ID
 * @return 0=成功
 */
int MyCommand(int idData)
{
    // 检查是否已初始化
    if (!g_bInitialized) {
        cvxMsgDisp("Plugin not initialized!");
        return -1;
    }
    
    // 示例: 查询根对象列表
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
    
    // 处理根对象
    char sBuf[BUFFER_SIZE];
    sprintf_s(sBuf, BUFFER_SIZE, "Found %d root objects", nRoots);
    cvxMsgDisp(sBuf);
    
    for (int i = 0; i < nRoots; i++) {
        sprintf_s(sBuf, BUFFER_SIZE, "Root[%d]: id=%d", i, pRootIds[i]);
        cvxMsgDisp(sBuf);
    }
    
    // 必须释放API返回的内存!
    cvxMemFree((void**)&pRootIds);
    
    return 0;
}

/**
 * @brief 表单回调函数
 * @param formName 表单名称
 * @param field 字段ID
 * @param idData 数据ID
 * @return 0=成功
 */
int MyCallback(char* formName, int field, int idData)
{
    char sBuf[BUFFER_SIZE];
    sprintf_s(sBuf, BUFFER_SIZE, "Callback: form=%s, field=%d", formName, field);
    cvxMsgDisp(sBuf);
    
    switch (field) {
        case 1:
            // 处理字段1
            break;
        case 2:
            // 处理字段2
            break;
        default:
            break;
    }
    
    return 0;
}

/* ============================================
 * 工具函数
 * ============================================ */

/**
 * @brief 安全释放内存 (辅助函数)
 */
void SafeFree(void** pPtr)
{
    if (pPtr && *pPtr) {
        cvxMemFree((void**)pPtr);
        *pPtr = NULL;
    }
}

/**
 * @brief 错误处理辅助
 */
int HandleError(int errorCode, const char* errorMsg)
{
    char sBuf[BUFFER_SIZE];
    sprintf_s(sBuf, BUFFER_SIZE, "Error %d: %s", errorCode, errorMsg);
    cvxMsgDisp(sBuf);
    return errorCode;
}
