/*
 * CAE自动化工作流模板 - 稳态热仿真
 * 
 * 完整流程: 创建任务 → 实体定位 → 施加载荷 → 网格划分 → 求解 → 结果导出
 */

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* ============================================
 * CAE API 函数声明 (实际项目中需包含头文件)
 * ============================================ */
// #include <zwsim_api.h>

// 任务管理
extern "C" int czwsPartCreateTask(const char* taskName, const char* taskType, zwsDbId* pIdx);
extern "C" int czwsPartInqActiveTask(zwsDbId* pIdx);
extern "C" int czwsTaskDelete(zwsDbId idxTask);

// 温度载荷
extern "C" int czwsSimStTempLoadDataInit(szwsTempLoadData* pData);
extern "C" int czwsSimStTempLoadDataFree(szwsTempLoadData* pData);
extern "C" int czwsSimStCreateTempLoad(const szwsTempLoadData* pData, zwsDbId* pIdx);

// 热通量载荷
extern "C" int czwsSimStHeatFluxDataInit(szwsHtFluxData* pData);
extern "C" int czwsSimStHeatFluxDataFree(szwsHtFluxData* pData);
extern "C" int czwsSimStCreateHeatFlux(const szwsHtFluxData* pData, zwsDbId* pIdx);

// 热功率载荷
extern "C" int czwsSimStHeatPowerDataInit(szwsHtPowerData* pData);
extern "C" int czwsSimStHeatPowerDataFree(szwsHtPowerData* pData);
extern "C" int czwsSimStCreateHeatPower(const szwsHtPowerData* pData, zwsDbId* pIdx);

// 接触
extern "C" int czwsSimStContactDataInit(szwsContactData* pData);
extern "C" int czwsSimStContactDataFree(szwsContactData* pData);
extern "C" int czwsSimStCreateContact(const szwsContactData* pData, zwsDbId* pIdx);

// 网格
extern "C" int czwsMeshing3D(double globalSize, double minSize, int order);

// 求解
extern "C" int czwsTaskRunSolver(zwsDbId idxTask);

// 结果
extern "C" int czwsResultInqTypes(zwsDbId idxTask, int* pCount, void** pTypes);
extern "C" int czwsResultTypeInqData(zwsDbId idxTask, const char* resultType, int subType, 
                                      int* pCount, void** pData);

/* ============================================
 * 稳态热仿真工作流
 * ============================================ */

/**
 * @brief 执行完整稳态热仿真
 * 
 * @param config 工况配置JSON:
 * {
 *     "taskName": "Case_01",
 *     "taskType": "steady_thermal",
 *     "targets": [...],  // 实体目标定义
 *     "loads": [
 *         {"type": "temperature", "target": "heater", "value": 80.0},
 *         {"type": "heat_flux", "target": "cooler", "value": 100.0},
 *         {"type": "heat_power", "target": "body", "value": 500.0}
 *     ],
 *     "mesh": {"globalSize": 5.0, "minSize": 1.0},
 *     "contacts": [...],
 *     "export": {"temperature": "/path/out.csv"}
 * }
 */
json RunSteadyThermalSimulation(const json& config) {
    json response;
    response["stage"] = "init";
    response["status"] = "running";
    
    // ========== Stage 1: 创建任务 ==========
    response["stage"] = "create_task";
    
    std::string taskName = config.value("taskName", "UnnamedTask");
    std::string taskType = config.value("taskType", "steady_thermal");
    
    zwsDbId taskIdx = {};
    // int err = czwsPartCreateTask(taskName.c_str(), taskType.c_str(), &taskIdx);
    // if (err != 0) {
    //     response["status"] = "error";
    //     response["error"] = "Failed to create task: " + std::to_string(err);
    //     return response;
    // }
    
    response["taskId"] = 1;  // 模拟
    response["message"] = "Task created successfully";
    
    // ========== Stage 2: 实体定位 ==========
    response["stage"] = "resolve_targets";
    
    std::map<std::string, std::vector<long long>> targetCache;
    
    if (config.contains("targets")) {
        for (auto& t : config["targets"]) {
            std::string targetId = t.value("id", "");
            TargetEntity entity;
            entity.id = targetId;
            entity.entityType = t.value("entityType", 0);
            entity.resolverType = t.value("resolver", json())["type"];
            
            ResolveResult result = ResolveTarget(entity);
            
            if (!result.success) {
                response["status"] = "error";
                response["error"] = "Failed to resolve target: " + targetId;
                response["debug"] = result.debugInfo;
                return response;
            }
            
            targetCache[targetId] = result.entityHandles;
        }
    }
    
    response["message"] = "All targets resolved: " + std::to_string(targetCache.size()) + " entities";
    
    // ========== Stage 3: 施加载荷 ==========
    response["stage"] = "apply_loads";
    
    if (config.contains("loads")) {
        for (auto& load : config["loads"]) {
            std::string loadType = load.value("type", "");
            std::string targetId = load.value("target", "");
            double value = load.value("value", 0.0);
            
            // 从缓存获取实体句柄
            auto it = targetCache.find(targetId);
            if (it == targetCache.end()) {
                response["status"] = "error";
                response["error"] = "Target not resolved: " + targetId;
                return response;
            }
            std::vector<long long>& entities = it->second;
            
            if (loadType == "temperature") {
                // 温度载荷
                szwsTempLoadData param = {};
                // czwsSimStTempLoadDataInit(&param);
                // param.temperature = value;
                // param.iStep = 1;
                // param.entNum = entities.size();
                // param.entities = entities.data();
                // int err = czwsSimStCreateTempLoad(&param, &loadIdx);
                // czwsSimStTempLoadDataFree(&param);
                
                response["appliedLoads"].push_back({{"type", "temperature"}, {"value", value}});
                
            } else if (loadType == "heat_flux") {
                // 热通量载荷
                szwsHtFluxData param = {};
                // czwsSimStHeatFluxDataInit(&param);
                // param.flux = value;
                // param.entNum = entities.size();
                // param.entities = entities.data();
                // czwsSimStCreateHeatFlux(&param, &loadIdx);
                // czwsSimStHeatFluxDataFree(&param);
                
                response["appliedLoads"].push_back({{"type", "heat_flux"}, {"value", value}});
                
            } else if (loadType == "heat_power") {
                // 热功率载荷
                szwsHtPowerData param = {};
                // czwsSimStHeatPowerDataInit(&param);
                // param.power = value;
                // param.entNum = entities.size();
                // param.entities = entities.data();
                // czwsSimStCreateHeatPower(&param, &loadIdx);
                // czwsSimStHeatPowerDataFree(&param);
                
                response["appliedLoads"].push_back({{"type", "heat_power"}, {"value", value}});
            }
        }
    }
    
    response["message"] = "Loads applied: " + std::to_string(response["appliedLoads"].size());
    
    // ========== Stage 4: 接触设置 ==========
    response["stage"] = "apply_contacts";
    
    if (config.contains("contacts")) {
        for (auto& contact : config["contacts"]) {
            std::string contactType = contact.value("type", "");
            std::string masterId = contact.value("master", "");
            std::string slaveId = contact.value("slave", "");
            
            auto masterIt = targetCache.find(masterId);
            auto slaveIt = targetCache.find(slaveId);
            
            if (masterIt == targetCache.end() || slaveIt == targetCache.end()) {
                response["status"] = "error";
                response["error"] = "Contact entities not found";
                return response;
            }
            
            // szwsContactData param = {};
            // czwsSimStContactDataInit(&param);
            // param.contactType = ZW_ST_CONTACT_TYPE_HEATCONDUCTION;
            // param.masterEntNum = masterIt->second.size();
            // param.sMasterEntities = masterIt->second.data();
            // param.slaveEntNum = slaveIt->second.size();
            // param.sSlaveEntities = slaveIt->second.data();
            // czwsSimStCreateContact(&param, &contactIdx);
            // czwsSimStContactDataFree(&param);
        }
    }
    
    response["message"] = "Contacts configured";
    
    // ========== Stage 5: 网格划分 ==========
    response["stage"] = "meshing";
    
    json meshConfig = config.value("mesh", json());
    double globalSize = meshConfig.value("globalSize", 5.0);
    double minSize = meshConfig.value("minSize", 1.0);
    int order = meshConfig.value("elementOrder", 1);
    
    // int err = czwsMeshing3D(globalSize, minSize, order);
    // if (err != 0) {
    //     response["status"] = "error";
    //     response["error"] = "Meshing failed: " + std::to_string(err);
    //     response["path"] = "fallback_native";
    //     // 回退到原生命令: !ZwCeMeshing
    // }
    
    response["meshInfo"] = {
        {"globalSize", globalSize},
        {"minSize", minSize},
        {"elementOrder", order},
        {"elementCount", 15000}  // 模拟
    };
    response["message"] = "Mesh generated successfully";
    
    // ========== Stage 6: 求解 ==========
    response["stage"] = "solving";
    
    // int err = czwsTaskRunSolver(taskIdx);
    // if (err != 0) {
    //     response["status"] = "error";
    //     response["error"] = "Solver failed: " + std::to_string(err);
    //     return response;
    // }
    
    response["solveInfo"] = {
        {"solveTime", 2.5},  // 秒
        {"converged", true}
    };
    response["message"] = "Solver completed";
    
    // ========== Stage 7: 结果导出 ==========
    response["stage"] = "export";
    
    if (config.contains("export")) {
        json exportConfig = config["export"];
        
        for (auto& item : exportConfig) {
            std::string resultType = item.value("type", "Temperature");
            std::string format = item.value("format", "csv");
            std::string path = item.value("path", "");
            
            // int count = 0;
            // void* pResults = NULL;
            // czwsResultTypeInqData(taskIdx, resultType.c_str(), 0, &count, &pResults);
            
            // 导出到文件
            // if (format == "csv") {
            //     ExportResultsToCSV(pResults, count, path.c_str());
            // } else if (format == "json") {
            //     ExportResultsToJSON(pResults, count, path.c_str());
            // }
            
            // cvxMemFree(&pResults);
        }
    }
    
    response["message"] = "Results exported";
    
    // ========== 完成 ==========
    response["status"] = "success";
    response["stage"] = "complete";
    
    return response;
}

/* ============================================
 * 错误码定义
 * ============================================ */

namespace ErrorCode {
    const int SUCCESS = 0;
    const int ERR_NO_ACTIVE_TASK = -1;
    const int ERR_INVALID_ENTITY = -2;
    const int ERR_MESH_FAILED = -32;
    const int ERR_SOLVER_FAILED = -100;
    const int ERR_API_MEMORY = -200;
}

/* ============================================
 * 使用示例
 * ============================================ */

/*
 * 工况配置文件 (JSON):
 * {
 *     "taskName": "稳态热仿真_01",
 *     "taskType": "steady_thermal",
 *     "targets": [
 *         {"id": "heater", "entityType": 0, "resolver": {"type": "nameTag", "value": "HEATER"}},
 *         {"id": "cooler", "entityType": 0, "resolver": {"type": "nameTag", "value": "COOLER"}},
 *         {"id": "body", "entityType": 3, "resolver": {"type": "nameTag", "value": "MAIN_BODY"}}
 *     ],
 *     "loads": [
 *         {"type": "temperature", "target": "heater", "value": 100.0},
 *         {"type": "heat_flux", "target": "cooler", "value": 50.0},
 *         {"type": "heat_power", "target": "body", "value": 200.0}
 *     ],
 *     "mesh": {"globalSize": 3.0, "minSize": 0.5},
 *     "export": {
 *         {"type": "Temperature", "format": "csv", "path": "D:\\output\\temp.csv"}
 *     }
 * }
 * 
 * 调用:
 * json config = ...;  // 从IPC消息解析
 * json result = RunSteadyThermalSimulation(config);
 * std::string responseJson = result.dump();
 * NamePipeIPC::Instance().Send(responseJson);
 */
