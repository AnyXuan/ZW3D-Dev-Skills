/**
 * ResultExportAPI_template.cpp - ZW3D 后处理与结果导出 API 模板
 * 
 * IPC 命令编号: 500-599 (结果处理号段)
 * - cmd 500: 云图 PNG 导出
 * - cmd 501: 节点/单元数据 CSV 导出
 * - cmd 502: 曲线数据导出 (XY Plot)
 * - cmd 503: 完整仿真报告生成
 * - cmd 504: 结果数据查询 (通用)
 * - cmd 505: 动画/GIF 导出
 * - cmd 506: 特定载荷工况结果查询
 */

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <cstring>
#include <algorithm>

// ============================================================================
// 类型定义
// ============================================================================

// 结果数据类型枚举 (与 IPC 协议一致)
enum ResultDataType {
    RESULT_DISPLACEMENT   = 1,  // 位移
    RESULT_STRESS         = 2,  // 应力
    RESULT_STRAIN         = 3,  // 应变
    RESULT_TEMPERATURE    = 4,  // 温度
    RESULT_FORCE          = 5,  // 反力
    RESULT_HEATFLUX       = 6,  // 热流密度
    RESULT_CUSTOM         = 99  // 自定义
};

// 云图类型枚举
enum ContourType {
    CONTOUR_TOTAL       = 0,  // 总量云图
    CONTOUR_X           = 1,  // X分量
    CONTOUR_Y           = 2,  // Y分量
    CONTOUR_Z           = 3,  // Z分量
    CONTOUR_MX          = 4,  // 最大值
    CONTOUR_MM          = 5   // 最小值
};

// 曲线类型枚举
enum CurveType {
    CURVE_NODE          = 0,  // 节点历史
    CURVE_ELEMENT       = 1,  // 单元历史
    CURVE_SECTION       = 2,  // 截面历史
    CURVE_GLOBAL        = 3   // 全局响应
};

// ============================================================================
// 云图 PNG 导出 (IPC cmd 500)
// ============================================================================

/**
 * @brief 导出当前结果云图为 PNG 图像
 * 
 * IPC 请求格式:
 * {
 *   "cmd": 500,
 *   "data": {
 *     "taskType": "steady_thermal" | "transient_thermal" | "static",
 *     "resultType": 1-6 (见 ResultDataType 枚举),
 *     "contourType": 0-5 (见 ContourType 枚举),
 *     "timeStep": 0 (瞬态热为时间步索引),
 *     "outputPath": "D:/output/thermal_contour.png",
 *     "width": 1920,
 *     "height": 1080,
 *     "colorBar": true,
 *     "legend": true,
 *     "showMesh": false
 *   }
 * }
 * 
 * IPC 响应格式:
 * {
 *   "cmd": 500,
 *   "status": "success" | "error",
 *   "message": "导出成功",
 *   "data": {
 *     "outputPath": "D:/output/thermal_contour.png",
 *     "width": 1920,
 *     "height": 1080
 *   }
 * }
 */

struct ContourExportParams {
    std::string taskType;          // 任务类型
    int resultType;                // 结果类型 (1-6)
    int contourType;               // 云图类型 (0-5)
    int timeStep;                  // 时间步 (稳态为 0)
    std::string outputPath;        // 输出路径
    int width = 1920;              // 图像宽度
    int height = 1080;             // 图像高度
    bool colorBar = true;          // 显示色标
    bool legend = true;            // 显示图例
    bool showMesh = false;         // 显示网格
};

// DLL 侧实现框架
int ZW_EXPORT ZwExportContourPNG(
    const ContourExportParams* params,
    char* errorMsg,
    int errorMsgSize
) {
    // TODO: 实现步骤
    // 1. 验证任务类型和结果类型有效性
    // 2. 设置当前结果工况和载荷步
    // 3. 配置云图显示选项 (颜色映射、范围等)
    // 4. 调用 ZwPostProcess 相关 API 渲染图像
    // 5. 保存 PNG 到指定路径
    // 6. 释放临时资源
    
    // 伪代码示例:
    // int resultTypeCode = MapResultType(params->resultType);
    // int contourCode = MapContourType(params->contourType);
    // ZwSetCurrentResultCase(taskHandle, params->taskType, params->timeStep);
    // ZwSetContourOptions(resultTypeCode, contourCode, ...);
    // ZwRenderContourImage(params->width, params->height, &imageHandle);
    // ZwSaveImagePNG(imageHandle, params->outputPath.c_str());
    
    return 0;  // 成功返回 0
}

// ============================================================================
// CSV 数据导出 (IPC cmd 501)
// ============================================================================

/**
 * @brief 导出节点或单元结果数据为 CSV 文件
 * 
 * IPC 请求格式:
 * {
 *   "cmd": 501,
 *   "data": {
 *     "taskType": "steady_thermal" | ...,
 *     "resultType": 1-6,
 *     "timeStep": 0,
 *     "dataType": "node" | "element",
 *     "entities": ["entity_1", "entity_2"],  // 可选，为空则导出全部
 *     "outputPath": "D:/output/result_data.csv",
 *     "includeHeader": true,
 *     "precision": 6
 *   }
 * }
 * 
 * CSV 输出格式示例 (节点位移):
 * NodeID,X,Y,Z,Total
 * 1001,0.001234,0.002345,0.003456,0.004567
 * 1002,-0.000123,0.001234,-0.002345,0.002678
 */

struct CSVExportParams {
    std::string taskType;
    int resultType;
    int timeStep;
    std::string dataType;          // "node" 或 "element"
    std::vector<std::string> entities;  // 实体选择 (可选)
    std::string outputPath;
    bool includeHeader = true;
    int precision = 6;             // 小数位数
};

// CSV 导出工具类
class CSVExporter {
public:
    static std::string FormatDouble(double value, int precision) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }
    
    // 表头生成
    static std::string GenerateNodeHeader(const std::string& resultTypeLabel) {
        std::ostringstream oss;
        oss << "NodeID,X,Y,Z,Total";
        if (resultTypeLabel == "Displacement") {
            oss << ",Magnitude";
        }
        return oss.str();
    }
    
    // 数据行写入
    static void WriteNodeRow(std::ofstream& file, 
                              int64_t nodeId, 
                              double x, double y, double z,
                              double magnitude,
                              int precision) {
        file << nodeId << ","
             << FormatDouble(x, precision) << ","
             << FormatDouble(y, precision) << ","
             << FormatDouble(z, precision) << ","
             << FormatDouble(magnitude, precision) << "\n";
    }
};

// DLL 侧实现框架
int ZW_EXPORT ZwExportResultCSV(
    const CSVExportParams* params,
    char* errorMsg,
    int errorMsgSize
) {
    // TODO: 实现步骤
    // 1. 获取结果数据 (ZwGetResultNodeData / ZwGetResultElementData)
    // 2. 过滤选定的实体
    // 3. 创建 CSV 文件并写入表头
    // 4. 逐行写入数据 (使用 CSVExporter 工具)
    // 5. 关闭文件
    
    // 伪代码示例:
    // int count = ZwGetResultNodeDataCount(taskHandle, resultTypeCode);
    // std::ofstream out(params->outputPath, std::ios::binary);
    // out << CSVExporter::GenerateNodeHeader(resultTypeLabel) << "\n";
    // for (int i = 0; i < count; i++) {
    //     szwNodeData data;
    //     ZwGetResultNodeData(taskHandle, i, resultTypeCode, &data);
    //     CSVExporter::WriteNodeRow(out, data.nodeId, data.x, data.y, data.z, data.mag, params->precision);
    // }
    
    return 0;
}

// ============================================================================
// 曲线数据导出 (IPC cmd 502)
// ============================================================================

/**
 * @brief 导出历史曲线数据 (如某节点的位移-时间曲线)
 * 
 * IPC 请求格式:
 * {
 *   "cmd": 502,
 *   "data": {
 *     "taskType": "transient_thermal" | "transient_structural",
 *     "curveType": 0-3 (见 CurveType),
 *     "entityId": "entity_1",
 *     "resultType": 1-6,
 *     "outputPath": "D:/output/curve_data.csv",
 *     "steps": [0, 10, 20, 30]  // 可选，默认全部时间步
 *   }
 * }
 * 
 * CSV 输出格式 (瞬态分析曲线):
 * Step,Time,Value
 * 0,0.0,0.000000
 * 1,0.1,0.001234
 * 2,0.2,0.002468
 */

struct CurveExportParams {
    std::string taskType;
    int curveType;               // 0-3
    std::string entityId;        // 目标实体
    int resultType;
    std::string outputPath;
    std::vector<int> steps;      // 指定时间步 (可选)
};

// DLL 侧实现框架
int ZW_EXPORT ZwExportResultCurve(
    const CurveExportParams* params,
    char* errorMsg,
    int errorMsgSize
) {
    // TODO: 实现步骤
    // 1. 根据 curveType 确定数据源 (节点/单元/截面/全局)
    // 2. 解析 entityId 获取实体句柄
    // 3. 获取时间步列表 (ZwGetTimeStepList)
    // 4. 逐时间步查询结果值 (ZwGetResultAtTimeStep)
    // 5. 写入 CSV 文件
    
    return 0;
}

// ============================================================================
// 完整仿真报告生成 (IPC cmd 503)
// ============================================================================

/**
 * @brief 生成包含摘要、云图、关键数据的完整 HTML 报告
 * 
 * IPC 请求格式:
 * {
 *   "cmd": 503,
 *   "data": {
 *     "taskType": "steady_thermal",
 *     "reportTitle": "热仿真分析报告",
 *     "outputPath": "D:/output/report.html",
 *     "sections": {
 *       "summary": true,           // 任务摘要
 *       "contours": true,          // 云图截图
 *       "keyResults": true,        // 极值、平均等统计
 *       "tables": true,            // 数据表格
 *       "conclusions": false       // 结论 (用户自定义)
 *     },
 *     "includeImages": true,       // 嵌入云图图片
 *     "format": "html"             // html | pdf
 *   }
 * }
 */

struct ReportOptions {
    std::string taskType;
    std::string reportTitle;
    std::string outputPath;
    bool summary = true;
    bool contours = true;
    bool keyResults = true;
    bool tables = true;
    bool conclusions = false;
    bool includeImages = true;
    std::string format = "html";
};

// 报告生成器框架
class ReportGenerator {
public:
    struct TaskSummary {
        std::string analysisType;
        std::string solverVersion;
        double startTime;
        double endTime;
        std::string status;  // "converged" | "diverged"
        int iterations;
        std::string notes;
    };
    
    struct KeyStatistics {
        double maxValue;
        double minValue;
        double avgValue;
        double rmsValue;
        std::string maxLocation;
        std::string minLocation;
    };
    
    // 生成报告元数据
    static std::string GenerateReportMetadata(const ReportOptions* opts, 
                                               const TaskSummary* summary) {
        // TODO: 生成 HTML 报告头部
        std::ostringstream html;
        html << "<html><head><title>" << opts->reportTitle << "</title></head>\n";
        html << "<body>\n";
        html << "<h1>" << opts->reportTitle << "</h1>\n";
        html << "<p>生成时间: " << GetCurrentTimestamp() << "</p>\n";
        return html.str();
    }
    
    // 生成摘要部分
    static std::string GenerateSummarySection(const TaskSummary* summary) {
        std::ostringstream html;
        html << "<h2>1. 仿真摘要</h2>\n";
        html << "<table border='1'>\n";
        html << "<tr><th>项目</th><th>值</th></tr>\n";
        html << "<tr><td>分析类型</td><td>" << summary->analysisType << "</td></tr>\n";
        html << "<tr><td>求解器版本</td><td>" << summary->solverVersion << "</td></tr>\n";
        html << "<tr><td>计算状态</td><td>" << summary->status << "</td></tr>\n";
        html << "<tr><td>迭代次数</td><td>" << summary->iterations << "</td></tr>\n";
        html << "</table>\n";
        return html.str();
    }
    
    // 生成关键统计部分
    static std::string GenerateStatisticsSection(const KeyStatistics* stats,
                                                  const std::string& resultTypeLabel) {
        std::ostringstream html;
        html << "<h2>2. " << resultTypeLabel << "统计</h2>\n";
        html << "<table border='1'>\n";
        html << "<tr><th>统计量</th><th>值</th></tr>\n";
        html << "<tr><td>最大值</td><td>" << stats->maxValue << " @ " << stats->maxLocation << "</td></tr>\n";
        html << "<tr><td>最小值</td><td>" << stats->minValue << " @ " << stats->minLocation << "</td></tr>\n";
        html << "<tr><td>平均值</td><td>" << stats->avgValue << "</td></tr>\n";
        html << "<tr><td>RMS</td><td>" << stats->rmsValue << "</td></tr>\n";
        html << "</table>\n";
        return html.str();
    }
    
    // 生成云图部分
    static std::string GenerateContourSection(const std::string& imagePath,
                                               const std::string& description) {
        std::ostringstream html;
        html << "<h2>3. 结果云图</h2>\n";
        html << "<p>" << description << "</p>\n";
        html << "<img src='" << imagePath << "' style='max-width:100%;'/>\n";
        return html.str();
    }
    
    // 生成结论部分
    static std::string GenerateConclusionSection(const std::string& conclusionText) {
        std::ostringstream html;
        html << "<h2>4. 结论</h2>\n";
        html << "<div class='conclusion'>" << conclusionText << "</div>\n";
        return html.str();
    }
    
    // 关闭报告
    static std::string GenerateReportFooter() {
        return "</body></html>\n";
    }
    
private:
    static std::string GetCurrentTimestamp() {
        // TODO: 实现时间戳获取
        return "2026-05-18 10:30:00";
    }
};

// DLL 侧实现框架
int ZW_EXPORT ZwGenerateSimulationReport(
    const ReportOptions* opts,
    char* errorMsg,
    int errorMsgSize
) {
    // TODO: 实现步骤
    // 1. 查询任务摘要信息 (ZwGetTaskInfo)
    // 2. 查询各结果类型的统计量 (ZwGetResultStatistics)
    // 3. 生成云图并保存到临时目录
    // 4. 使用 ReportGenerator 组装 HTML
    // 5. 写入最终报告文件
    
    return 0;
}

// ============================================================================
// 结果数据通用查询 (IPC cmd 504)
// ============================================================================

/**
 * @brief 查询指定位置的精确结果值 (不导出文件)
 * 
 * IPC 请求格式:
 * {
 *   "cmd": 504,
 *   "data": {
 *     "taskType": "steady_thermal",
 *     "resultType": 1,  // 位移
 *     "timeStep": 0,
 *     "queryType": "node" | "element" | "location",
 *     "entityId": "entity_1",    // node/element 类型时必填
 *     "position": {              // location 类型时必填
 *       "x": 10.0, "y": 20.0, "z": 30.0
 *     }
 *   }
 * }
 * 
 * IPC 响应格式:
 * {
 *   "cmd": 504,
 *   "status": "success",
 *   "data": {
 *     "resultType": "displacement",
 *     "values": {
 *       "X": 0.001234,
 *       "Y": 0.002345,
 *       "Z": 0.003456,
 *       "Total": 0.004567
 *     },
 *     "unit": "mm",
 *     "location": { "x": 10.0, "y": 20.0, "z": 30.0 },
 *     "nearbyEntity": "node_1001"
 *   }
 * }
 */

struct ResultQueryParams {
    std::string taskType;
    int resultType;
    int timeStep;
    std::string queryType;         // "node" | "element" | "location"
    std::string entityId;          // 实体ID
    double queryX, queryY, queryZ; // 位置查询时的坐标
};

struct ResultQueryResponse {
    std::string resultTypeLabel;
    double values[4];  // X, Y, Z, Total
    std::string unit;
    double position[3];
    std::string nearbyEntity;
};

// DLL 侧实现框架
int ZW_EXPORT ZwQueryResultData(
    const ResultQueryParams* params,
    ResultQueryResponse* response,
    char* errorMsg,
    int errorMsgSize
) {
    // TODO: 实现步骤
    // 1. 验证参数有效性
    // 2. 根据 queryType 选择查询方式:
    //    - node: ZwGetResultNodeDataAtEntity()
    //    - element: ZwGetResultElementDataAtEntity()
    //    - location: ZwGetResultAtPosition() (插值)
    // 3. 填充响应结构
    // 4. 返回结果
    
    return 0;
}

// ============================================================================
// 动画/GIF 导出 (IPC cmd 505)
// ============================================================================

/**
 * @brief 导出瞬态结果动画为 GIF 或视频
 * 
 * IPC 请求格式:
 * {
 *   "cmd": 505,
 *   "data": {
 *     "taskType": "transient_thermal",
 *     "resultType": 4,  // 温度
 *     "timeSteps": [0, 1, 2, 3, 4, 5],
 *     "outputPath": "D:/output/thermal_animation.gif",
 *     "frameRate": 10,  // fps
 *     "width": 1280,
 *     "height": 720,
 *     "colorBar": true
 *   }
 * }
 */

struct AnimationExportParams {
    std::string taskType;
    int resultType;
    std::vector<int> timeSteps;
    std::string outputPath;
    int frameRate = 10;
    int width = 1280;
    int height = 720;
    bool colorBar = true;
};

// DLL 侧实现框架
int ZW_EXPORT ZwExportResultAnimation(
    const AnimationExportParams* params,
    char* errorMsg,
    int errorMsgSize
) {
    // TODO: 实现步骤
    // 1. 为每个时间步生成云图帧 (复用 ZwExportContourPNG 逻辑)
    // 2. 使用 GIF 编解码库 (如 libgif) 或 FFmpeg 合成动画
    // 3. 写入输出文件
    
    return 0;
}

// ============================================================================
// 特定载荷工况结果查询 (IPC cmd 506)
// ============================================================================

/**
 * @brief 查询多载荷工况下特定实体在各工况的结果
 * 
 * IPC 请求格式:
 * {
 *   "cmd": 506,
 *   "data": {
 *     "taskType": "static",
 *     "resultType": 1,  // 位移
 *     "entities": ["entity_1", "entity_2"],
 *     "loadCases": ["case_A", "case_B", "case_C"]
 *   }
 * }
 * 
 * IPC 响应格式 (矩阵形式):
 * {
 *   "cmd": 506,
 *   "status": "success",
 *   "data": {
 *     "matrix": [
 *       //         case_A, case_B, case_C
 *       [entity_1]: [0.123, 0.234, 0.345],
 *       [entity_2]: [0.456, 0.567, 0.678]
 *     ],
 *     "labels": {
 *       "rows": ["entity_1", "entity_2"],
 *       "cols": ["case_A", "case_B", "case_C"]
 *     }
 *   }
 * }
 */

struct MultiCaseQueryParams {
    std::string taskType;
    int resultType;
    std::vector<std::string> entities;
    std::vector<std::string> loadCases;
};

// DLL 侧实现框架
int ZW_EXPORT ZwQueryMultiCaseResults(
    const MultiCaseQueryParams* params,
    std::vector<std::vector<double>>& resultMatrix,
    char* errorMsg,
    int errorMsgSize
) {
    // TODO: 实现步骤
    // 1. 遍历每个载荷工况
    // 2. 对每个工况查询每个实体的结果值
    // 3. 填充二维结果矩阵
    // 4. 返回
    
    return 0;
}

// ============================================================================
// 实用工具函数
// ============================================================================

// 映射结果类型枚举到 ZW3D API 内部编码
static int MapResultType(int resultType) {
    switch (resultType) {
        case RESULT_DISPLACEMENT: return 1;
        case RESULT_STRESS:       return 2;
        case RESULT_STRAIN:       return 3;
        case RESULT_TEMPERATURE:  return 4;
        case RESULT_FORCE:        return 5;
        case RESULT_HEATFLUX:     return 6;
        default:                  return 99;
    }
}

// 映射云图类型枚举
static int MapContourType(int contourType) {
    return contourType;  // 直接传递
}

// 检查输出路径有效性
static bool ValidateOutputPath(const std::string& path) {
    if (path.empty()) return false;
    
    // 检查目录是否存在
    std::filesystem::path p(path);
    std::filesystem::path dir = p.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        try {
            std::filesystem::create_directories(dir);
        } catch (...) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// IPC 命令分发处理 (DLL 侧)
// ============================================================================

int ZW_EXPORT ZwHandleResultExportCmd(int cmd, const char* jsonInput, char* jsonOutput, int outputSize) {
    int result = 0;
    
    // 解析输入 JSON (使用 rapidjson 等)
    // Document doc; doc.Parse(jsonInput);
    
    switch (cmd) {
        case 500:  // 云图 PNG 导出
        {
            // ContourExportParams params = ParseContourParams(doc["data"]);
            // result = ZwExportContourPNG(&params, errorMsg, ...);
            // 构建响应 JSON
            break;
        }
        case 501:  // CSV 数据导出
        {
            // CSVExportParams params = ParseCSVParams(doc["data"]);
            // result = ZwExportResultCSV(&params, errorMsg, ...);
            break;
        }
        case 502:  // 曲线导出
        {
            // CurveExportParams params = ParseCurveParams(doc["data"]);
            // result = ZwExportResultCurve(&params, errorMsg, ...);
            break;
        }
        case 503:  // 报告生成
        {
            // ReportOptions opts = ParseReportParams(doc["data"]);
            // result = ZwGenerateSimulationReport(&opts, errorMsg, ...);
            break;
        }
        case 504:  // 结果数据查询
        {
            // ResultQueryParams params = ParseQueryParams(doc["data"]);
            // ResultQueryResponse response;
            // result = ZwQueryResultData(&params, &response, errorMsg, ...);
            // 构建响应 JSON
            break;
        }
        case 505:  // 动画导出
        {
            // AnimationExportParams params = ParseAnimationParams(doc["data"]);
            // result = ZwExportResultAnimation(&params, errorMsg, ...);
            break;
        }
        case 506:  // 多工况查询
        {
            // MultiCaseQueryParams params = ParseMultiCaseParams(doc["data"]);
            // std::vector<std::vector<double>> matrix;
            // result = ZwQueryMultiCaseResults(&params, matrix, errorMsg, ...);
            // 构建响应 JSON
            break;
        }
        default:
            result = -1;  // 未知命令
            break;
    }
    
    // 构建响应 JSON
    // sprintf(jsonOutput, "{\"cmd\":%d,\"status\":\"%s\",\"data\":{...}}", cmd, 
    //         result == 0 ? "success" : "error");
    
    return result;
}

// ============================================================================
// 使用说明
// ============================================================================

/**
 * 后处理 API 设计说明:
 * 
 * 1. 命令号段 500-599 专门用于结果导出和查询，与仿真执行命令 (200-299) 分离
 * 
 * 2. 参数约定:
 *    - taskType: 统一使用 "steady_thermal" / "transient_thermal" / "static" / "modal" / "buckling"
 *    - resultType: 使用 1-6 的整数编码 (见 ResultDataType 枚举)
 *    - timeStep: 稳态分析固定为 0，瞬态分析为 0-based 索引
 * 
 * 3. 实体引用:
 *    - 统一使用 "entity_X" 格式 (X 为数字)
 *    - DLL 侧维护 entity 句柄缓存
 * 
 * 4. 错误处理:
 *    - 所有导出 API 都返回 int，0=成功，负数=错误
 *    - 错误信息通过 errorMsg 参数返回
 *    - 响应 JSON 中 status 字段为 "success" 或 "error"
 * 
 * 5. 性能考虑:
 *    - 大数据量导出 (如全模型节点 CSV) 可能耗时较长
 *    - 建议在前端显示进度指示器
 *    - 动画导出建议先渲染单帧预览确认效果
 */
