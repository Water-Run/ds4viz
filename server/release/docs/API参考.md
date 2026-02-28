# ds4viz API 参考

## 0. 通用说明

### 0.1 认证方式（JWT + Session）

登录成功后获得 JWT（`token`），需要认证的接口必须携带请求头：

```
Authorization: Bearer <token>
```

令牌默认有效期由配置项 `security.access_token_expire_hours` 决定（生产环境默认24小时，测试环境默认1小时）。服务器会定期清理过期会话（默认每小时一次）。

### 0.2 统一错误响应格式

所有自定义异常的响应格式为：

```json
{
  "error": "错误信息"
}
```

错误信息为中文，来源于异常类的 `message` 字段。

### 0.3 时间格式

所有时间字段为 ISO 8601 格式（UTC），数据库使用 UTC 时区存储：

```
2026-01-26T10:00:00
```

### 0.4 分页参数

支持分页的接口统一使用以下查询参数：

| 参数 | 类型 | 默认值 | 范围 | 说明 |
|------|------|--------|------|------|
| page | int | 1 | ≥1 | 页码 |
| limit | int | 20 | 1-100 | 每页数量 |

### 0.5 用户状态说明

| 状态 | 可登录 | 可执行代码 | 可收藏模板 |
|------|--------|------------|------------|
| Active | ✓ | ✓ | ✓ |
| Suspended | ✓ | ✗ | ✗ |
| Banned | ✗ | ✗ | ✗ |

### 0.6 支持的编程语言

| 语言标识 | 说明 | 执行方式 |
|----------|------|----------|
| python | Python 3 | 直接执行 |
| lua | Lua | 直接执行 |
| rust | Rust | 编译后执行 |

### 0.7 头像URL说明

`avatar_url` 字段：
- 用户有头像时返回 `/api/users/{id}/avatar`
- 用户无头像时返回 `null`

---

## 1. 认证相关 API

### 1.1 用户注册

**POST** `/api/auth/register`

**请求体：**
```json
{
  "username": "string",
  "password": "string"
}
```

**字段约束：**
| 字段     | 约束      | 说明                                                                            |
|----------|-----------|---------------------------------------------------------------------------------|
| username | 1-64 字符 | C# 标识符规则：以字母或下划线开头，仅含字母、数字、下划线；支持 Unicode 字母（如中文） |
| password | 8-32 字符 | 须同时包含大写字母、小写字母、数字、特殊字符各至少一个                             |

特殊字符范围：`!"#$%&'()*+,-./:;<=>?@[\]^_\`{|}~`

**可能错误：**
| 状态码 | 错误信息     | 触发条件                                  |
|--------|--------------|-------------------------------------------|
| 409    | 用户名已存在 | username 已被占用                         |
| 422    | 参数校验失败 | 用户名格式不合法、密码长度不符或复杂度不足 |

**响应 (200)：**
```json
{
  "id": 1,
  "username": "string",
  "avatar_url": null,
  "status": "Active",
  "created_at": "2026-01-26T10:00:00"
}
```

新注册用户默认状态为 `Active`，无头像。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 409 | 用户名已存在 | username 已被占用 |
| 422 | 参数校验失败 | 字段长度不符、密码UTF-8字节超64 |

---

### 1.2 用户登录

**POST** `/api/auth/login`

**请求体：**
```json
{
  "username": "string",
  "password": "string"
}
```

**响应 (200)：**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIs...",
  "user": {
    "id": 1,
    "username": "string",
    "avatar_url": "/api/users/1/avatar",
    "status": "Active",
    "created_at": "2026-01-26T10:00:00"
  },
  "expires_at": "2026-01-27T10:00:00"
}
```

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 用户名或密码错误 | 用户不存在或密码错误 |
| 403 | 用户已被封禁 | 用户状态为 Banned |

Suspended 状态用户允许登录，响应中 `status` 字段为 `"Suspended"`，但后续执行代码和收藏操作会被拦截。

---

### 1.3 用户登出

**POST** `/api/auth/logout`

**请求头：**
```
Authorization: Bearer <token>
```

**响应 (200)：**
```json
{
  "message": "Logged out successfully"
}
```

登出后，当前会话从数据库中删除，令牌立即失效。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 认证失败 | 令牌无效、过期或不存在 |

---

### 1.4 获取当前用户信息

**GET** `/api/auth/me`

**请求头：**
```
Authorization: Bearer <token>
```

**响应 (200)：**
```json
{
  "id": 1,
  "username": "string",
  "avatar_url": "/api/users/1/avatar",
  "status": "Active",
  "created_at": "2026-01-26T10:00:00"
}
```

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 认证失败 | 令牌无效、过期或不存在 |

---

## 2. 用户相关 API

### 2.1 获取用户头像

**GET** `/api/users/{id}/avatar`

无需认证。

**响应 (200)：**
- Content-Type: `application/octet-stream`
- Body: 二进制数据（原样返回数据库中的 BYTEA）

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 404 | 用户不存在 | 用户ID不存在 |
| 404 | 头像不存在 | 用户存在但未设置头像 |

---

### 2.2 上传/更新用户头像

**PUT** `/api/users/{id}/avatar`

**请求头：**
```
Authorization: Bearer <token>
Content-Type: multipart/form-data
```

**请求体：**
```
avatar: <binary file>
```

表单字段名必须为 `avatar`。

**响应 (200)：**
```json
{
  "message": "Avatar uploaded successfully",
  "avatar_url": "/api/users/1/avatar"
}
```

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 认证失败 | 令牌无效或不存在 |
| 403 | 权限不足 | 尝试修改他人头像（user_id != 当前用户ID） |
| 404 | 用户不存在 | 用户ID不存在 |

---

### 2.3 修改密码

**PUT** `/api/users/{id}/password`

**请求头：**
```
Authorization: Bearer <token>
```

**请求体：**
```json
{
  "old_password": "string",
  "new_password": "string"
}
```

**字段约束：**
| 字段         | 约束      | 说明                                                |
|--------------|-----------|-----------------------------------------------------|
| new_password | 8-32 字符 | 须同时包含大写字母、小写字母、数字、特殊字符各至少一个 |

**响应 (200)：**
```json
{
  "message": "Password updated successfully"
}
```

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 400 | 原密码错误 | old_password 与当前密码不匹配 |
| 401 | 认证失败 | 令牌无效或不存在 |
| 403 | 权限不足 | 尝试修改他人密码（user_id != 当前用户ID） |
| 404 | 用户不存在 | 用户ID不存在 |

---

### 2.4 获取用户收藏列表

**GET** `/api/users/{id}/favorites`

**请求头：**
```
Authorization: Bearer <token>
```

**查询参数：**
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| page | int | 1 | 页码（≥1） |
| limit | int | 20 | 每页数量（1-100） |

**响应 (200)：**
```json
{
  "items": [
    {
      "template_id": 1,
      "title": "冒泡排序",
      "category": "排序算法",
      "description": "经典的冒泡排序算法...",
      "favorite_count": 42,
      "favorited_at": "2026-01-25T10:00:00"
    }
  ],
  "total": 5,
  "page": 1,
  "limit": 20
}
```

收藏列表按收藏时间倒序排列。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 认证失败 | 令牌无效或不存在 |
| 403 | 权限不足 | 尝试查看他人收藏（user_id != 当前用户ID） |
| 404 | 用户不存在 | 用户ID不存在 |

---

### 2.5 获取用户执行历史

**GET** `/api/users/{id}/executions`

**请求头：**
```
Authorization: Bearer <token>
```

**查询参数：**
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| page | int | 1 | 页码（≥1） |
| limit | int | 20 | 每页数量（1-100） |

**响应 (200)：**
```json
{
  "items": [
    {
      "id": 456,
      "language": "python",
      "code": "def bubble_sort(arr):\n    ...",
      "status": "Success",
      "execution_time": 150,
      "created_at": "2026-01-26T10:00:00"
    }
  ],
  "total": 50,
  "page": 1,
  "limit": 20
}
```

执行历史按创建时间倒序排列。`execution_time` 可能为 `null`。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 认证失败 | 令牌无效或不存在 |
| 403 | 权限不足 | 尝试查看他人执行历史（user_id != 当前用户ID） |

---

## 3. 模板相关 API

### 3.1 获取模板列表

**GET** `/api/templates`

**查询参数：**
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| category | string | - | 分类筛选（可选，精确匹配） |
| page | int | 1 | 页码（≥1） |
| limit | int | 20 | 每页数量（1-100） |

**可选认证：** 携带 token 时返回当前用户的 `is_favorited` 状态，未登录时恒为 `false`。

**响应 (200)：**
```json
{
  "items": [
    {
      "id": 1,
      "title": "冒泡排序",
      "category": "排序算法",
      "description": "经典的冒泡排序算法...",
      "favorite_count": 42,
      "is_favorited": true,
      "created_at": "2026-01-20T10:00:00"
    }
  ],
  "total": 100,
  "page": 1,
  "limit": 20
}
```

模板列表按创建时间倒序排列。

---

### 3.2 获取模板详情

**GET** `/api/templates/{id}`

**可选认证：** 携带 token 时返回当前用户的 `is_favorited` 状态，未登录时恒为 `false`。

**响应 (200)：**
```json
{
  "id": 1,
  "title": "冒泡排序",
  "category": "排序算法",
  "description": "## 算法说明\n冒泡排序...",
  "favorite_count": 42,
  "codes": [
    {
      "language": "python",
      "code": "def bubble_sort(arr):\n    ...",
      "explanation": "### Python实现\n..."
    }
  ],
  "is_favorited": false,
  "created_at": "2026-01-20T10:00:00",
  "updated_at": "2026-01-21T08:00:00"
}
```

`codes` 包含该模板所有语言的实现，`explanation` 字段可能为 `null`。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 404 | 模板不存在 | 模板ID不存在 |

---

### 3.3 获取模板代码（指定语言）

**GET** `/api/templates/{id}/code/{language}`

无需认证。

**响应 (200)：**
```json
{
  "language": "python",
  "code": "def bubble_sort(arr):\n    ...",
  "explanation": "### Python实现\n..."
}
```

`explanation` 字段可能为 `null`。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 404 | 模板不存在 | 模板ID不存在 |
| 404 | 模板语言实现不存在 | 模板存在但无该语言实现 |

---

### 3.4 获取模板分类列表

**GET** `/api/templates/categories`

无需认证。

**响应 (200)：**
```json
{
  "items": ["排序算法", "图算法", "数据结构"]
}
```

分类列表已去重并按字母顺序排列（SQL `ORDER BY category`）。

---

### 3.5 搜索模板

**GET** `/api/templates/search`

**查询参数：**
| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| keyword | string | **必填** | 搜索关键词 |
| page | int | 1 | 页码（≥1） |
| limit | int | 20 | 每页数量（1-100） |

**可选认证：** 携带 token 时返回当前用户的 `is_favorited` 状态，未登录时恒为 `false`。

**响应 (200)：**
```json
{
  "items": [
    {
      "id": 1,
      "title": "冒泡排序",
      "category": "排序算法",
      "description": "经典的冒泡排序算法...",
      "favorite_count": 42,
      "is_favorited": false,
      "created_at": "2026-01-20T10:00:00"
    }
  ],
  "total": 10,
  "page": 1,
  "limit": 20
}
```

搜索范围包括模板标题（title）和描述（description），使用 PostgreSQL `ILIKE` 不区分大小写匹配。搜索结果按收藏数降序、创建时间降序排列。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 422 | 参数校验失败 | 缺少 keyword 参数 |

---

## 4. 收藏相关 API

### 4.1 收藏模板

**POST** `/api/favorites`

**请求头：**
```
Authorization: Bearer <token>
```

**请求体：**
```json
{
  "template_id": 1
}
```

**响应 (200)：**
```json
{
  "message": "Template favorited successfully"
}
```

收藏成功后，对应模板的 `favorite_count` 自动加 1（由数据库触发器 `trigger_update_favorite_count` 维护）。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 认证失败 | 令牌无效或不存在 |
| 403 | 用户已被封禁 | 用户状态为 Banned |
| 403 | 用户已被暂停，无法执行该操作 | 用户状态为 Suspended |
| 404 | 模板不存在 | template_id 不存在 |
| 409 | 已收藏该模板 | 用户已收藏该模板 |

---

### 4.2 取消收藏

**DELETE** `/api/favorites/{template_id}`

**请求头：**
```
Authorization: Bearer <token>
```

**响应 (200)：**
```json
{
  "message": "Favorite removed successfully"
}
```

取消收藏后，对应模板的 `favorite_count` 自动减 1（由数据库触发器维护）。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 认证失败 | 令牌无效或不存在 |
| 404 | 未收藏该模板 | 用户未收藏该模板 |

注意：取消收藏不检查用户状态（Suspended/Banned 用户也可取消收藏）。

---

## 5. 代码执行相关 API

### 5.1 执行代码

**POST** `/api/execute`

**请求头：**
```
Authorization: Bearer <token>
```

**请求体：**
```json
{
  "language": "python",
  "code": "def bubble_sort(arr):\n    ..."
}
```

**language 取值：** `python`、`lua`、`rust`

**响应 (200)：**

执行成功：
```json
{
  "execution_id": 456,
  "status": "Success",
  "toml_output": "[visualization]\ntype = \"array\"\ndata = [1,2,3]",
  "error_message": null,
  "execution_time": 150,
  "cached": false
}
```

执行失败：
```json
{
  "execution_id": 457,
  "status": "Error",
  "toml_output": null,
  "error_message": "SyntaxError: invalid syntax (line 5)",
  "execution_time": 50,
  "cached": false
}
```

执行超时：
```json
{
  "execution_id": 458,
  "status": "Timeout",
  "toml_output": null,
  "error_message": "执行超时",
  "execution_time": 10000,
  "cached": false
}
```

缓存命中：
```json
{
  "execution_id": 459,
  "status": "Success",
  "toml_output": "[visualization]\ntype = \"array\"",
  "error_message": null,
  "execution_time": 0,
  "cached": true
}
```

**执行逻辑说明：**

1. 首先检查用户状态，必须为 Active
2. 计算代码的 SHA256 哈希，检查执行缓存表（`execution_cache`）
3. 缓存命中时直接返回，`execution_time` 为 0，`cached` 为 true
4. 缓存未命中时在 systemd-run 瞬态沙箱中执行代码
5. 执行成功后扫描工作目录中新增的 `.toml` 文件作为输出
6. 成功执行（有 TOML 输出）时将结果存入缓存
7. 无论成功失败，均保存执行记录到 `executions` 表

**沙箱配置（由配置文件定义）：**
- 超时时间：`sandbox.timeout_seconds`（生产默认30秒，测试默认10秒）
- 内存限制：`sandbox.max_memory_mb`（生产默认512MB，测试默认256MB）
- 网络隔离、私有临时目录、只读系统目录

**status 取值：**
| 状态 | 说明 |
|------|------|
| Success | 执行成功且生成了 TOML 文件 |
| Error | 执行失败（语法错误、运行时错误、未生成 TOML 等） |
| Timeout | 执行超时 |

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 认证失败 | 令牌无效或不存在 |
| 403 | 用户已被封禁 | 用户状态为 Banned |
| 403 | 用户已被暂停，无法执行该操作 | 用户状态为 Suspended |
| 422 | 参数校验失败 | language 不在支持列表中 |

---

### 5.2 获取执行详情

**GET** `/api/executions/{id}`

**请求头：**
```
Authorization: Bearer <token>
```

**响应 (200)：**
```json
{
  "id": 456,
  "language": "python",
  "code": "def bubble_sort(arr):\n    ...",
  "toml_output": "[visualization]\ntype = \"array\"\ndata = [1,2,3]",
  "status": "Success",
  "error_message": null,
  "execution_time": 150,
  "created_at": "2026-01-26T10:00:00"
}
```

`toml_output`、`error_message`、`execution_time` 字段可能为 `null`。

**可能错误：**
| 状态码 | 错误信息 | 触发条件 |
|--------|----------|----------|
| 401 | 认证失败 | 令牌无效或不存在 |
| 403 | 无权访问该执行记录 | 执行记录不属于当前用户 |
| 404 | 执行记录不存在 | 执行记录ID不存在 |

---

## 6. 健康检查

### 6.1 健康检查接口

**GET** `/health`

无需认证。

**响应 (200)：**
```json
{
  "status": "ok"
}
```