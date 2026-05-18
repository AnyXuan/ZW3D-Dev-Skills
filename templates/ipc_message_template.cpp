/*
 * IPC 消息处理模板 - 双进程通信
 * 用于 Qt EXE ↔ ZW3D DLL 通信
 * Protocol: Named Pipe + JSON
 */

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* ============================================
 * IPC 消息协议定义
 * ============================================ */

// 命令编号
namespace CmdID {
    const int TASK_CREATE = 200;      // 创建仿真任务
    const int TASK_DELETE = 201;      // 删除任务
    const int TASK_QUERY = 202;       // 查询任务
    const int ENTITY_SELECT = 220;    // 选择实体
    const int TEMP_LOAD = 230;        // 温度载荷
    const int HEAT_FLUX = 231;        // 热通量
    const int HEAT_POWER = 232;       // 热功率
    const int CONTACT_SET = 240;      // 接触
    const int MESH_3D = 250;          // 3D网格
    const int SOLVE_RUN = 260;        // 求解
    const int RESULT_QUERY = 261;     // 查询结果
    const int VAR_QUERY = 300;        // 查询变量
    const int VAR_SET = 301;          // 设置变量
}

// 响应状态
namespace Status {
    const char* SUCCESS = "success";
    const char* ERROR = "error";
}

/* ============================================
 * 消息格式
 * ============================================ */

// 请求消息
// {
//     "cmd": 230,
//     "data": {
//         "taskId": 1,
//         "temperature": 80.0,
//         "entityId": 12345,
//         "iStep": 1
//     }
// }

// 响应消息
// {
//     "cmd": 230,
//     "status": "success",
//     "data": {
//         "loadId": 100
//     },
//     "error": ""
// }

/* ============================================
 * JSON 消息处理函数
 * ============================================ */

/**
 * @brief 解析命令消息
 */
json ParseCommand(const std::string& message) {
    try {
        return json::parse(message);
    } catch (const json::parse_error& e) {
        json resp;
        resp["status"] = "error";
        resp["error"] = "Invalid JSON: " + std::string(e.what());
        return resp;
    }
}

/**
 * @brief 创建成功响应
 */
json CreateSuccessResponse(int cmd, const json& data = json()) {
    json resp;
    resp["cmd"] = cmd;
    resp["status"] = "success";
    resp["data"] = data;
    resp["error"] = "";
    return resp;
}

/**
 * @brief 创建错误响应
 */
json CreateErrorResponse(int cmd, const std::string& error) {
    json resp;
    resp["cmd"] = cmd;
    resp["status"] = "error";
    resp["data"] = json();
    resp["error"] = error;
    return resp;
}

/* ============================================
 * 命令分发器
 * ============================================ */

/**
 * @brief IPC消息处理主入口
 * @param message JSON消息字符串
 * @return 响应JSON字符串
 */
std::string HandleIPCMessage(const std::string& message) {
    // 1. 解析消息
    json req = ParseCommand(message);
    if (req.contains("status") && req["status"] == "error") {
        return req.dump();
    }
    
    int cmd = req["cmd"];
    json data = req.value("data", json());
    
    // 2. 分发到对应处理函数
    json resp;
    switch (cmd) {
        case CmdID::TASK_CREATE:
            resp = HandleCreateTask(data);
            break;
        case CmdID::ENTITY_SELECT:
            resp = HandleSelectEntity(data);
            break;
        case CmdID::TEMP_LOAD:
            resp = HandleTempLoad(data);
            break;
        case CmdID::MESH_3D:
            resp = HandleMesh3D(data);
            break;
        case CmdID::SOLVE_RUN:
            resp = HandleSolveRun(data);
            break;
        case CmdID::VAR_SET:
            resp = HandleVarSet(data);
            break;
        default:
            resp = CreateErrorResponse(cmd, "Unknown command: " + std::to_string(cmd));
    }
    
    // 3. 返回JSON字符串
    return resp.dump();
}

/* ============================================
 * 各命令处理函数示例
 * ============================================ */

/**
 * @brief 创建仿真任务
 */
json HandleCreateTask(const json& data) {
    std::string taskName = data.value("taskName", "UnnamedTask");
    std::string taskType = data.value("taskType", "steady_thermal");
    
    // 调用CAE API创建任务
    // zwsDbId taskIdx = {};
    // int err = czwsPartCreateTask(taskName.c_str(), taskType.c_str(), &taskIdx);
    
    // 模拟返回
    json resp = CreateSuccessResponse(CmdID::TASK_CREATE);
    resp["data"]["taskId"] = 1;
    resp["data"]["taskName"] = taskName;
    resp["data"]["taskType"] = taskType;
    return resp;
}

/**
 * @brief 选择实体
 */
json HandleSelectEntity(const json& data) {
    int entityType = data.value("entityType", 3);  // 0=面, 3=体
    bool allowMulti = data.value("allowMulti", true);
    
    // 调用实体选择API
    // int count = 0;
    // szwEntityHandle* list = nullptr;
    // ZwEntityListGetByPick("请选择实体", entityType, allowMulti, &count, &list);
    
    // 模拟返回
    json resp = CreateSuccessResponse(CmdID::ENTITY_SELECT);
    resp["data"]["count"] = 1;
    resp["data"]["entities"] = json::array({12345});
    return resp;
}

/**
 * @brief 设置温度载荷
 */
json HandleTempLoad(const json& data) {
    int taskId = data.value("taskId", 0);
    double temperature = data.value("temperature", 0.0);
    long long entityId = data.value("entityId", 0);
    int iStep = data.value("iStep", 1);
    
    // 验证参数
    if (taskId <= 0) {
        return CreateErrorResponse(CmdID::TEMP_LOAD, "Invalid taskId");
    }
    if (entityId <= 0) {
        return CreateErrorResponse(CmdID::TEMP_LOAD, "Invalid entityId");
    }
    
    // 调用CAE API
    // szwsTempLoadData param = {};
    // czwsSimStTempLoadDataInit(&param);
    // param.temperature = temperature;
    // param.iStep = iStep;
    // param.entNum = 1;
    // param.entities = &entityId;
    // int err = czwsSimStCreateTempLoad(&param);
    
    json resp = CreateSuccessResponse(CmdID::TEMP_LOAD);
    resp["data"]["loadId"] = 100;
    return resp;
}

/**
 * @brief 3D网格划分
 */
json HandleMesh3D(const json& data) {
    double meshSize = data.value("meshSize", 5.0);
    double minSize = data.value("minSize", 1.0);
    int elementOrder = data.value("elementOrder", 1);
    
    // 验证
    if (meshSize <= 0) {
        return CreateErrorResponse(CmdID::MESH_3D, "Invalid meshSize");
    }
    
    // 调用网格API
    // int err = czwsMeshing3D(meshSize, minSize, elementOrder);
    
    json resp = CreateSuccessResponse(CmdID::MESH_3D);
    resp["data"]["elementCount"] = 1000;
    resp["data"]["meshSize"] = meshSize;
    return resp;
}

/**
 * @brief 运行求解器
 */
json HandleSolveRun(const json& data) {
    int taskId = data.value("taskId", 0);
    
    // 调用求解器API
    // int err = czwsTaskRunSolver(taskId);
    
    json resp = CreateSuccessResponse(CmdID::SOLVE_RUN);
    resp["data"]["solveTime"] = 1.23;  // 秒
    return resp;
}

/**
 * @brief 设置变量 (参数化建模)
 */
json HandleVarSet(const json& data) {
    json variables = data.value("variables", json::array());
    
    // 遍历变量并设置
    for (auto& var : variables) {
        std::string name = var.value("name", "");
        double value = var.value("value", 0.0);
        
        // 调用变量API
        // cvxPartVarSet(name.c_str(), value);
    }
    
    // 触发重建
    // cvxUpdate();
    
    json resp = CreateSuccessResponse(CmdID::VAR_SET);
    resp["data"]["updated"] = variables.size();
    return resp;
}

/* ============================================
 * 线程安全调度 (DLL侧关键设计)
 * ============================================ */

// 使用隐藏窗口 + PostMessage 确保API在主线程调用
// 参考 Z3ParametricModeling 项目的 WorkHandle 实现

static HWND g_hWorkerWnd = NULL;
static std::mutex g_taskMutex;
static std::condition_variable g_taskCondition;
static std::string g_currentTaskMessage;
static bool g_taskFinished = false;

/**
 * @brief 收到IPC消息后调度到主线程执行
 */
void DispatchToMainThread(const std::string& message) {
    // 1. 存储消息
    {
        std::lock_guard<std::mutex> lock(g_taskMutex);
        g_currentTaskMessage = message;
        g_taskFinished = false;
    }
    
    // 2. 发送消息到隐藏窗口
    PostMessage(g_hWorkerWnd, WM_USER + 1, 0, 0);
    
    // 3. 等待执行完成
    {
        std::unique_lock<std::mutex> lock(g_taskMutex);
        g_taskCondition.wait(lock, [] { return g_taskFinished; });
    }
}

/**
 * @brief 隐藏窗口消息处理
 */
LRESULT CALLBACK WorkerWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_USER + 1) {
        // 处理消息
        std::string response;
        {
            std::lock_guard<std::mutex> lock(g_taskMutex);
            response = HandleIPCMessage(g_currentTaskMessage);
            g_taskFinished = true;
        }
        g_taskCondition.notify_one();
        
        // 通过Named Pipe发送响应
        NamePipeIPC::Instance().Send(response);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
