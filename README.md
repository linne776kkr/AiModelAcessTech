# ChatSDK - AI 聊天 SDK

## 目录

- [项目介绍](#项目介绍)
- [目录结构](#目录结构)
- [功能特性](#功能特性)
- [安装方法](#安装方法)
- [接口说明](#接口说明)
- [示例代码](#示例代码)

## 项目介绍

ChatSDK 是一个跨平台的 AI 聊天软件开发工具包（SDK），支持多种大语言模型（LLM）的接入和管理。该 SDK 提供了统一的接口，可以方便地集成 OpenAI GPT、DeepSeek、Ollama 等多种 AI 模型服务。

### 主要特性

- **多模型支持**：支持 OpenAI GPT、DeepSeek、Ollama 等主流 AI 模型
- **会话管理**：提供完整的会话管理功能，支持会话创建、查询、删除
- **持久化存储**：使用 SQLite 数据库存储会话和消息历史
- **流式输出**：支持流式消息返回，实时展示 AI 回复
- **统一接口**：抽象出 LLMProvider 接口，便于扩展新的模型支持

## 目录结构

```
AiModelAcessTech/
├── sdk/                           # SDK 源代码目录
│   ├── include/                   # 头文件目录
│   │   ├── common.h              # 公共数据结构（消息、会话、配置等）
│   │   ├── ChatSDK.h             # SDK 主类接口
│   │   ├── LLMManager.h          # LLM 管理器接口
│   │   ├── LLMProvider.h         # LLM 提供者抽象基类
│   │   ├── SessionManager.h      # 会话管理器接口
│   │   ├── DataManager.h         # 数据管理器接口
│   │   ├── ChatGPTProvider.h     # OpenAI GPT 提供者接口
│   │   ├── DeepSeekProvider.h    # DeepSeek 提供者接口
│   │   ├── OllamaProvider.h      # Ollama 提供者接口
│   │   └── util/
│   │       └── myLog.h           # 日志工具头文件
│   ├── src/                      # 源文件目录
│   │   ├── ChatSDK.cpp           # SDK 主类实现
│   │   ├── LLMManager.cpp        # LLM 管理器实现
│   │   ├── SessionManager.cpp    # 会话管理器实现
│   │   ├── DataManager.cpp       # 数据管理器实现
│   │   ├── ChatGPTProvider.cpp   # OpenAI GPT 提供者实现
│   │   ├── DeepSeekProvider.cpp  # DeepSeek 提供者实现
│   │   ├── OllamaProvider.cpp    # Ollama 提供者实现
│   │   └── util/
│   │       ├── myLog.cpp         # 日志工具实现
│   │       └── myLog.hpp         # 日志工具实现（模板）
│   └── build/                     # 构建目录
├── test/                         # 测试目录
│   └── LLMtest.cpp               # SDK 使用示例
└── README.md                     # 项目文档
```

## 功能特性

### 1. 模型管理

- 支持多种 AI 模型配置（API 密钥方式、本地 Ollama 方式）
- 模型可用性检测
- 模型信息查询

### 2. 会话管理

- 创建新会话
- 获取会话详情
- 获取所有会话列表
- 删除指定会话
- 会话消息历史记录

### 3. 消息处理

- 发送消息（同步方式）
- 发送消息（流式返回）
- 自动维护消息时间戳和 ID

### 4. 数据持久化

- SQLite 数据库存储
- 会话和消息自动保存
- 支持会话数据导出和清理

## 安装方法

### 环境要求

- C++17 或更高版本
- CMake 3.10+
- SQLite3
- spdlog（日志库）
- libcurl（HTTP 请求库）

### 依赖安装

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install cmake g++ libsqlite3-dev libcurl4-openssl-dev libspdlog-dev
```

**macOS:**

```bash
brew install cmake sqlite3 curl spdlog
```

### 编译步骤

1. 进入项目目录：

```bash
cd AiModelAcessTech
```

2. 创建并进入构建目录：

```bash
mkdir -p sdk/build
cd sdk/build
```

3. 使用 CMake 配置项目：

```bash
cmake ..
```

4. 编译项目：

```bash
make -j$(nproc)
```

5. 编译测试程序（如需要）：

```bash
cd ../../test
mkdir -p build
cd build
cmake ..
make
```

## 接口说明

### 命名空间

所有接口均在 `ai_chat_sdk` 命名空间下。

### 公共数据结构

#### Message - 消息结构

| 成员变量 | 类型 | 说明 |
|---------|------|------|
| `_messageId` | `std::string` | 消息唯一标识 |
| `_role` | `std::string` | 消息角色（user/assistant/system） |
| `_content` | `std::string` | 消息内容 |
| `_timestamp` | `std::time_t` | 消息时间戳 |

#### Config - 模型公共配置

| 成员变量 | 类型 | 说明 |
|---------|------|------|
| `_modelName` | `std::string` | 模型名称 |
| `_temperature` | `double` | 温度参数（控制随机性） |
| `_maxTokens` | `int` | 最大 token 数 |

#### ApiConfig - API 云端模型配置

继承自 `Config`，新增以下成员：

| 成员变量 | 类型 | 说明 |
|---------|------|------|
| `_apiKey` | `std::string` | API 密钥 |

#### OllamaConfig - Ollama 本地模型配置

继承自 `Config`，新增以下成员：

| 成员变量 | 类型 | 说明 |
|---------|------|------|
| `_modelDesc` | `std::string` | 模型描述 |
| `_endpoint` | `std::string` | Ollama 服务地址 |

#### Session - 会话信息

| 成员变量 | 类型 | 说明 |
|---------|------|------|
| `_sessionId` | `std::string` | 会话唯一标识 |
| `_sessionName` | `std::string` | 会话名称 |
| `_modelName` | `std::string` | 关联的模型名称 |
| `_messages` | `std::vector<std::shared_ptr<Message>>` | 消息列表 |
| `_createTime` | `std::time_t` | 创建时间 |
| `_lastUpdateTime` | `std::time_t` | 最后更新时间 |

#### ModelInfo - 模型信息

| 成员变量 | 类型 | 说明 |
|---------|------|------|
| `_modelName` | `std::string` | 模型名称 |
| `_modelDesc` | `std::string` | 模型描述 |
| `_provider` | `std::string` | 提供商名称 |
| `_endpoint` | `std::string` | API 端点 |
| `_isAvailable` | `bool` | 是否可用 |

### ChatSDK 主类接口

#### 初始化

**`bool initModel(const std::vector<std::shared_ptr<Config>>& configs)`**

初始化 SDK，传入模型配置列表。支持同时配置多个模型。

**参数：**
- `configs`：模型配置向量，可以是 `ApiConfig` 或 `OllamaConfig`

**返回值：**
- `true`：初始化成功
- `false`：初始化失败

#### 状态检查

**`bool isInitialized() const`**

检查 SDK 是否已初始化。

**返回值：**
- `true`：已初始化
- `false`：未初始化

#### 会话管理

**`std::string createSession(const std::string &modelName)`**

创建新会话。

**参数：**
- `modelName`：要使用的模型名称

**返回值：**
- 成功：返回会话 ID
- 失败：返回空字符串

**`const std::shared_ptr<Session> getSession(const std::string &sessionId)`**

获取指定会话。

**参数：**
- `sessionId`：会话 ID

**返回值：**
- 找到：返回会话指针
- 未找到：返回 `nullptr`

**`std::vector<std::string> getAllSessionLists()`**

获取所有会话 ID 列表。

**返回值：**
- 会话 ID 向量

**`bool deleteSession(const std::string &sessionId)`**

删除指定会话。

**参数：**
- `sessionId`：会话 ID

**返回值：**
- `true`：删除成功
- `false`：删除失败

#### 模型管理

**`std::vector<std::shared_ptr<ModelInfo>> getAvailableModels()`**

获取所有可用模型信息。

**返回值：**
- 可用模型信息向量

#### 消息发送

**`std::string sendMessage(const std::string &sessionId, const std::string &message)`**

发送消息（同步方式），等待完整响应。

**参数：**
- `sessionId`：会话 ID
- `message`：用户消息

**返回值：**
- AI 完整回复文本

**`std::string sendStreamMessage(const std::string &sessionId, const std::string &message, std::function<void(const std::string&, bool)> callback)`**

发送消息（流式返回），通过回调函数实时获取响应片段。

**参数：**
- `sessionId`：会话 ID
- `message`：用户消息
- `callback`：回调函数
  - 第一个参数：响应的文本片段
  - 第二个参数：`true` 表示响应结束，`false` 表示继续

**返回值：**
- 完整的响应文本（如需可使用）

### LLMProvider 抽象基类

所有具体模型提供者（如 ChatGPTProvider、DeepSeekProvider）都继承自此类，实现以下虚函数：

- `virtual bool initModel(const std::map<std::string, std::string> &modelConfig) = 0`
- `virtual bool isAvailable() const = 0`
- `virtual std::string getModelName() const = 0`
- `virtual std::string getModelDesc() const = 0`
- `virtual std::string sendMessage(...) = 0`
- `virtual std::string sendMessageStream(...) = 0`

## 示例代码

### 基础使用示例

```cpp
#include <iostream>
#include <memory>
#include "ChatSDK.h"

int main() {
    using namespace ai_chat_sdk;

    // 创建 SDK 实例
    ChatSDK sdk;

    // 配置模型
    std::vector<std::shared_ptr<Config>> configs;

    // 配置 ChatGPT
    auto chatgptConfig = std::make_shared<ApiConfig>(
        "gpt-3.5-turbo",   // 模型名称
        0.7,               // 温度参数
        2048,              // 最大 token 数
        "your-api-key-here" // API 密钥
    );
    configs.push_back(chatgptConfig);

    // 配置 DeepSeek
    auto deepseekConfig = std::make_shared<ApiConfig>(
        "deepseek-chat",   // 模型名称
        0.7,               // 温度参数
        2048,              // 最大 token 数
        "your-deepseek-api-key"
    );
    configs.push_back(deepseekConfig);

    // 初始化 SDK
    if (!sdk.initModel(configs)) {
        std::cerr << "初始化失败" << std::endl;
        return 1;
    }

    std::cout << "初始化成功！" << std::endl;

    // 查看可用模型
    auto models = sdk.getAvailableModels();
    std::cout << "可用模型：" << std::endl;
    for (const auto& model : models) {
        std::cout << "  - " << model->_modelName 
                  << " (" << model->_provider << ")" << std::endl;
    }

    // 创建会话
    std::string sessionId = sdk.createSession("gpt-3.5-turbo");
    if (sessionId.empty()) {
        std::cerr << "创建会话失败" << std::endl;
        return 1;
    }
    std::cout << "会话已创建，ID: " << sessionId << std::endl;

    // 发送消息（同步方式）
    std::string response = sdk.sendMessage(sessionId, "你好，请介绍一下你自己");
    std::cout << "AI 回复：" << response << std::endl;

    // 发送消息（流式返回）
    std::cout << "流式回复：";
    sdk.sendStreamMessage(sessionId, "请给我讲一个笑话", 
        [](const std::string& chunk, bool isEnd) {
            std::cout << chunk << std::flush;
            if (isEnd) {
                std::cout << std::endl;
            }
        });

    // 获取会话列表
    auto sessions = sdk.getAllSessionLists();
    std::cout << "当前会话数：" << sessions.size() << std::endl;

    // 获取会话详情
    auto session = sdk.getSession(sessionId);
    if (session) {
        std::cout << "会话名称：" << session->_sessionName << std::endl;
        std::cout << "消息数：" << session->_messages.size() << std::endl;
    }

    // 删除会话
    if (sdk.deleteSession(sessionId)) {
        std::cout << "会话已删除" << std::endl;
    }

    return 0;
}
```

### 使用 Ollama 本地模型

```cpp
#include <iostream>
#include <memory>
#include "ChatSDK.h"

int main() {
    using namespace ai_chat_sdk;

    ChatSDK sdk;

    // 配置 Ollama 本地模型
    std::vector<std::shared_ptr<Config>> configs;
    auto ollamaConfig = std::make_shared<OllamaConfig>(
        "llama2",          // 模型名称
        0.7,               // 温度参数
        2048,              // 最大 token 数
        "Llama 2 本地模型", // 模型描述
        "http://localhost:11434" // Ollama 服务地址
    );
    configs.push_back(ollamaConfig);

    // 初始化
    if (!sdk.initModel(configs)) {
        std::cerr << "初始化失败" << std::endl;
        return 1;
    }

    // 创建会话并发送消息
    std::string sessionId = sdk.createSession("llama2");
    std::string response = sdk.sendMessage(sessionId, "用一句话介绍自己");
    std::cout << "LLama 回复：" << response << std::endl;

    return 0;
}
```

### 批量消息处理

```cpp
#include <iostream>
#include <vector>
#include "ChatSDK.h"

int main() {
    using namespace ai_chat_sdk;

    ChatSDK sdk;

    // 初始化配置...
    // sdk.initModel(configs);

    std::string sessionId = sdk.createSession("gpt-3.5-turbo");

    // 批量发送消息
    std::vector<std::string> questions = {
        "什么是人工智能？",
        "什么是机器学习？",
        "什么是深度学习？"
    };

    for (const auto& question : questions) {
        std::cout << "问：" << question << std::endl;
        std::string answer = sdk.sendMessage(sessionId, question);
        std::cout << "答：" << answer << std::endl;
        std::cout << "---" << std::endl;
    }

    return 0;
}
```

## 注意事项

1. **API 密钥安全**：请妥善保管您的 API 密钥，不要将其硬编码在代码中或提交到版本控制系统
2. **网络连接**：使用云端模型（GPT、DeepSeek）需要稳定的网络连接
3. **Ollama 服务**：使用 Ollama 本地模型需要提前启动 Ollama 服务
4. **Token 限制**：注意设置合理的 `maxTokens` 参数，避免响应被截断
5. **错误处理**：建议对所有 SDK 返回值进行错误检查

## 许可证

本项目仅供学习和研究使用。
