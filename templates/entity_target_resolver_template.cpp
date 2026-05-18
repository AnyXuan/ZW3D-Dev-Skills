/*
 * 实体目标解析器模板 - CAE自动化核心
 * 用于在模型重建后稳定定位目标实体 (面/体/边)
 * 
 * 解析优先级: nameTag > feature_child > entityName > 几何规则 > 临时index
 */

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/* ============================================
 * 目标定义数据结构
 * ============================================ */

// 目标解析器类型
namespace ResolverType {
    const std::string NAME_TAG = "nameTag";
    const std::string FEATURE_CHILD = "feature_child";
    const std::string ENTITY_NAME = "entityName";
    const std::string FACE_RULE = "face_rule";
    const std::string BODY_RULE = "body_rule";
}

// 实体类型
namespace EntityType {
    const int FACE = 0;
    const int EDGE = 1;
    const int VERTEX = 2;
    const int SHAPE = 3;  // 体
}

/**
 * @brief 目标实体定义
 */
struct TargetEntity {
    std::string id;              // 唯一标识，如 "heater_face"
    int entityType;              // FACE/EDGE/VERTEX/SHAPE
    std::string resolverType;    // 解析器类型
    
    // nameTag解析器
    std::string nameTagValue;
    
    // feature_child解析器
    std::string featureName;
    int childIndex;
    
    // face_rule解析器
    std::string ownerBody;       // 所属体
    double minArea;
    double maxArea;
    double normalDirection;      // 法向方向
};

/**
 * @brief 解析结果
 */
struct ResolveResult {
    bool success;
    std::vector<long long> entityHandles;  // 实体句柄列表
    std::string debugInfo;
    std::string resolverUsed;
};

/* ============================================
 * 目标解析器接口
 * ============================================ */

/**
 * @brief 解析目标实体 (统一接口)
 * @param target 目标定义
 * @return 解析结果
 */
ResolveResult ResolveTarget(const TargetEntity& target) {
    ResolveResult result;
    result.success = false;
    
    // 根据解析器类型分发
    if (target.resolverType == ResolverType::NAME_TAG) {
        return ResolveByNameTag(target);
    } else if (target.resolverType == ResolverType::FEATURE_CHILD) {
        return ResolveByFeatureChild(target);
    } else if (target.resolverType == ResolverType::ENTITY_NAME) {
        return ResolveByEntityName(target);
    } else if (target.resolverType == ResolverType::FACE_RULE) {
        return ResolveByFaceRule(target);
    } else if (target.resolverType == ResolverType::BODY_RULE) {
        return ResolveByBodyRule(target);
    }
    
    result.debugInfo = "Unknown resolver type";
    return result;
}

/* ============================================
 * 各解析器实现
 * ============================================ */

/**
 * @brief 解析器1: 通过 nameTag 定位
 * 
 * 优点: 最稳定，人可读
 * 适用: 手动命名过的关键面/体
 */
ResolveResult ResolveByNameTag(const TargetEntity& target) {
    ResolveResult result;
    
    // 查询所有带nameTag的实体
    int count = 0;
    void* pTagList = NULL;
    
    // cvxEntityNameTagGetAll 查询所有标签
    // int err = cvxEntityNameTagGetAll(NULL, &count, &pTagList);
    
    // 遍历查找匹配的标签
    for (int i = 0; i < count; i++) {
        // szwNameTagInfo* tagInfo = ((szwNameTagInfo**)pTagList)[i];
        // if (std::string(tagInfo->tagName) == target.nameTagValue) {
        //     result.entityHandles.push_back(tagInfo->entityHandle);
        // }
    }
    
    // 释放
    // cvxMemFree(&pTagList);
    
    result.resolverUsed = ResolverType::NAME_TAG;
    result.success = (result.entityHandles.size() > 0);
    result.debugInfo = "Found " + std::to_string(result.entityHandles.size()) + 
                       " entities with tag '" + target.nameTagValue + "'";
    
    return result;
}

/**
 * @brief 解析器2: 通过 feature + child index 定位
 * 
 * 优点: 适用于明确来自某个特征的面
 * 适用: 拉伸端面、孔壁、阵列子面
 */
ResolveResult ResolveByFeatureChild(const TargetEntity& target) {
    ResolveResult result;
    
    // 1. 查找特征
    int featCount = 0;
    void* pFeatList = NULL;
    // cvxPartInqFeatList(NULL, &featCount, &pFeatList);
    
    long long featureHandle = 0;
    for (int i = 0; i < featCount; i++) {
        // szvFeatureInfo* featInfo = ((szvFeatureInfo**)pFeatList)[i];
        // if (std::string(featInfo->featName) == target.featureName) {
        //     featureHandle = featInfo->id;
        //     break;
        // }
    }
    // cvxMemFree(&pFeatList);
    
    if (featureHandle == 0) {
        result.debugInfo = "Feature not found: " + target.featureName;
        return result;
    }
    
    // 2. 查询特征的子实体
    int childCount = 0;
    void* pChildList = NULL;
    // ZwFeatureChildEntityListGet(featureHandle, NULL, &childCount, &pChildList);
    
    if (target.childIndex >= 0 && target.childIndex < childCount) {
        // szwEntityHandle* childList = (szwEntityHandle*)pChildList;
        // result.entityHandles.push_back(childList[target.childIndex]);
    }
    
    // cvxMemFree(&pChildList);
    
    result.resolverUsed = ResolverType::FEATURE_CHILD;
    result.success = (result.entityHandles.size() > 0);
    result.debugInfo = "Found feature '" + target.featureName + 
                       "' with " + std::to_string(childCount) + " children";
    
    return result;
}

/**
 * @brief 解析器3: 通过实体名称定位
 */
ResolveResult ResolveByEntityName(const TargetEntity& target) {
    ResolveResult result;
    
    // ZwEntityNameGetByName 查找
    // result.entityHandles.push_back(ZwEntityByName(target.entityName));
    
    result.resolverUsed = ResolverType::ENTITY_NAME;
    result.success = (result.entityHandles.size() > 0);
    return result;
}

/**
 * @brief 解析器4: 通过几何规则定位面
 * 
 * 规则: 所属体、面积范围、法向方向、包围盒中心
 */
ResolveResult ResolveByFaceRule(const TargetEntity& target) {
    ResolveResult result;
    
    // 1. 获取所属体的所有面
    int faceCount = 0;
    void* pFaceList = NULL;
    
    // 如果指定了ownerBody，先找到该体
    long long bodyHandle = 0;
    if (!target.ownerBody.empty()) {
        // bodyHandle = FindBodyByName(target.ownerBody);
    }
    
    // 查询体/模型的所有面
    // cvxPartInqShpFace(bodyHandle, &faceCount, &pFaceList);
    
    for (int i = 0; i < faceCount; i++) {
        // szwFaceInfo* faceInfo = ((szwFaceInfo**)pFaceList)[i];
        
        // 面积检查
        if (target.minArea > 0 || target.maxArea > 0) {
            // double area = faceInfo->area;
            // if (area < target.minArea || area > target.maxArea) continue;
        }
        
        // 法向方向检查
        // if (target.normalDirection > 0) {
        //     double nx, ny, nz;
        //     cvxFaceInqNormal(faceInfo->id, &nx, &ny, &nz);
        //     // 检查法向
        // }
        
        // 符合规则
        // result.entityHandles.push_back(faceInfo->id);
    }
    
    // cvxMemFree(&pFaceList);
    
    result.resolverUsed = ResolverType::FACE_RULE;
    result.success = (result.entityHandles.size() > 0);
    result.debugInfo = "Resolved " + std::to_string(result.entityHandles.size()) +
                       " faces matching rules";
    
    return result;
}

/**
 * @brief 解析器5: 通过几何规则定位体
 */
ResolveResult ResolveByBodyRule(const TargetEntity& target) {
    ResolveResult result;
    
    // 查询所有体
    int bodyCount = 0;
    void* pBodyList = NULL;
    // cvxRootListByType(VX_SHAPE, &bodyCount, &pBodyList);
    
    // 遍历筛选
    for (int i = 0; i < bodyCount; i++) {
        // szvShapeInfo* shapeInfo = ((szvShapeInfo**)pBodyList)[i];
        // result.entityHandles.push_back(shapeInfo->id);
    }
    
    // cvxMemFree(&pBodyList);
    
    result.resolverUsed = ResolverType::BODY_RULE;
    result.success = (result.entityHandles.size() > 0);
    return result;
}

/* ============================================
 * JSON 序列化/反序列化
 * ============================================ */

/**
 * @brief TargetEntity → JSON
 */
json TargetEntityToJson(const TargetEntity& target) {
    json j;
    j["id"] = target.id;
    j["entityType"] = target.entityType;
    j["resolver"] = json();
    j["resolver"]["type"] = target.resolverType;
    
    if (target.resolverType == ResolverType::NAME_TAG) {
        j["resolver"]["value"] = target.nameTagValue;
    } else if (target.resolverType == ResolverType::FEATURE_CHILD) {
        j["resolver"]["featureName"] = target.featureName;
        j["resolver"]["childIndex"] = target.childIndex;
    } else if (target.resolverType == ResolverType::FACE_RULE) {
        j["resolver"]["ownerBody"] = target.ownerBody;
        j["resolver"]["minArea"] = target.minArea;
        j["resolver"]["maxArea"] = target.maxArea;
    }
    
    return j;
}

/**
 * @brief JSON → TargetEntity
 */
TargetEntity JsonToTargetEntity(const json& j) {
    TargetEntity target;
    target.id = j.value("id", "");
    target.entityType = j.value("entityType", EntityType::FACE);
    target.resolverType = j.value("resolver", json())["type"];
    
    if (target.resolverType == ResolverType::NAME_TAG) {
        target.nameTagValue = j["resolver"]["value"];
    } else if (target.resolverType == ResolverType::FEATURE_CHILD) {
        target.featureName = j["resolver"]["featureName"];
        target.childIndex = j["resolver"]["childIndex"];
    } else if (target.resolverType == ResolverType::FACE_RULE) {
        target.ownerBody = j["resolver"].value("ownerBody", "");
        target.minArea = j["resolver"].value("minArea", 0.0);
        target.maxArea = j["resolver"].value("maxArea", 0.0);
    }
    
    return target;
}

/**
 * @brief ResolveResult → JSON
 */
json ResolveResultToJson(const ResolveResult& result) {
    json j;
    j["success"] = result.success;
    j["count"] = result.entityHandles.size();
    j["entities"] = json::array();
    for (auto h : result.entityHandles) {
        j["entities"].push_back(h);
    }
    j["debug"] = result.debugInfo;
    j["resolver"] = result.resolverUsed;
    return j;
}

/* ============================================
 * 使用示例
 * ============================================ */

/*
 * 完整工况模板:
 * {
 *     "caseName": "steady_thermal_case_01",
 *     "taskType": "steady_thermal",
 *     "targets": [
 *         {
 *             "id": "heater_face",
 *             "entityType": 0,
 *             "resolver": {
 *                 "type": "nameTag",
 *                 "value": "HEATER_FACE"
 *             }
 *         },
 *         {
 *             "id": "cooling_face",
 *             "entityType": 0,
 *             "resolver": {
 *                 "type": "face_rule",
 *                 "ownerBody": "MainBody",
 *                 "minArea": 150.0,
 *                 "maxArea": 170.0
 *             }
 *         }
 *     ],
 *     "loads": [
 *         {
 *             "type": "temperature",
 *             "target": "heater_face",
 *             "value": 80.0
 *         }
 *     ]
 * }
 */
