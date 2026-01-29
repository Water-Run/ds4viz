# ds4viz API 参考

## 0. 通用说明

### 0.1 认证方式（JWT + Session）
- 登录成功后获得 JWT（`token`）
- 需要认证的接口必须携带请求头：

```
Authorization: Bearer <token>
```

- 令牌默认有效期：由配置项 `security.access_token_expire_hours` 决定

---

### 0.2 统一错误响应格式

当前后端自定义异常的响应格式为：

```json
{
  "error": "错误信息"
}
```

示例：

```json
{
  "error": "用户名已存在"
}
```

> 说明：错误信息为中文，来源于异常类的 `message` 字段。

---

### 0.3 时间格式

所有时间字段为 ISO 8601 格式（UTC）：

```
2026-01-26T10:00:00Z
```

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

**响应 (200)：**
```json
{
  "id": 1,
  "username": "string",
  "avatar_url": null,
  "status": "Active",
  "created_at": "2026-01-26T10:00:00Z"
}
```

**可能错误：**
- 409 `{"error": "用户名已存在"}`

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
    "created_at": "2026-01-26T10:00:00Z"
  },
  "expires_at": "2026-01-27T10:00:00Z"
}
```

**可能错误：**
- 401 `{"error": "用户名或密码错误"}`
- 403 `{"error": "用户已被封禁"}`

> 注：Suspended 用户允许登录，但部分操作会被限制。

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

**可能错误：**
- 401 `{"error": "认证失败"}`

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
  "created_at": "2026-01-26T10:00:00Z"
}
```

**可能错误：**
- 401 `{"error": "认证失败"}`

---

## 2. 用户相关 API

### 2.1 获取用户头像

**GET** `/api/users/{id}/avatar`

**响应 (200)：**
- Content-Type: `application/octet-stream`
- 二进制数据（原样返回数据库中的 BYTEA）

**可能错误：**
- 404 `{"error": "用户不存在"}`
- 404 `{"error": "头像不存在"}`（若 API 层选择区分）

> 说明：当前代码未存储图片类型，API 层可选择返回 `application/octet-stream` 或通过内容嗅探设置为 `image/*`。

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

**响应 (200)：**
```json
{
  "message": "Avatar uploaded successfully",
  "avatar_url": "/api/users/1/avatar"
}
```

**可能错误：**
- 401 `{"error": "认证失败"}`
- 403 `{"error": "权限不足"}`（只能修改自己的头像）
- 404 `{"error": "用户不存在"}`

> 说明：当前服务层不校验格式/大小，API 层如需限制请同步更新文档。

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

**响应 (200)：**
```json
{
  "message": "Password updated successfully"
}
```

**可能错误：**
- 401 `{"error": "认证失败"}`
- 403 `{"error": "权限不足"}`（只能修改自己的密码）
- 400 `{"error": "原密码错误"}`
- 404 `{"error": "用户不存在"}`

---

## 3. 模板相关 API

### 3.1 获取模板列表

**GET** `/api/templates`

**查询参数：**
- `category`（可选）: 分类名称
- `page`（可选）: 页码，默认 1
- `limit`（可选）: 每页数量，默认 20

**可选认证：**
- 若携带 token，可返回 `is_favorited`
- 未登录时 `is_favorited` 恒为 false

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
      "created_at": "2026-01-20T10:00:00Z"
    }
  ],
  "total": 100,
  "page": 1,
  "limit": 20
}
```

---

### 3.2 获取模板详情

**GET** `/api/templates/{id}`

**可选认证：**
- 若携带 token，可返回 `is_favorited`
- 未登录时 `is_favorited` 恒为 false

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
  "created_at": "2026-01-20T10:00:00Z",
  "updated_at": "2026-01-21T08:00:00Z"
}
```

**可能错误：**
- 404 `{"error": "模板不存在"}`

---

### 3.3 获取模板代码（指定语言）

**GET** `/api/templates/{id}/code/{language}`

**响应 (200)：**
```json
{
  "language": "python",
  "code": "def bubble_sort(arr):\n    ...",
  "explanation": "### Python实现\n..."
}
```

**可能错误：**
- 404 `{"error": "模板不存在"}`
- 404 `{"error": "模板语言实现不存在"}`

> 说明：当前服务层未提供专用方法，API 层可通过模板详情过滤得到。

---

### 3.4 获取模板分类列表

**GET** `/api/templates/categories`

**响应 (200)：**
```json
{
  "items": ["排序算法", "图算法", "数据结构"]
}
```

---

### 3.5 搜索模板

**GET** `/api/templates/search`

**查询参数：**
- `keyword`（必填）: 搜索关键词
- `page`（可选）: 页码，默认 1
- `limit`（可选）: 每页数量，默认 20

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
      "created_at": "2026-01-20T10:00:00Z"
    }
  ],
  "total": 10,
  "page": 1,
  "limit": 20
}
```

---

## 4. 收藏相关 API

### 4.1 获取用户收藏列表

**GET** `/api/users/{id}/favorites`

**请求头：**
```
Authorization: Bearer <token>
```

**查询参数：**
- `page`（可选）: 页码，默认 1
- `limit`（可选）: 每页数量，默认 20

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
      "favorited_at": "2026-01-25T10:00:00Z"
    }
  ],
  "total": 5,
  "page": 1,
  "limit": 20
}
```

**可能错误：**
- 401 `{"error": "认证失败"}`
- 403 `{"error": "权限不足"}`（只能查看自己的收藏）
- 404 `{"error": "用户不存在"}`

---

### 4.2 收藏模板

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

**可能错误：**
- 401 `{"error": "认证失败"}`
- 403 `{"error": "用户已被封禁"}`
- 403 `{"error": "用户已被暂停，无法执行该操作"}`
- 404 `{"error": "模板不存在"}`
- 409 `{"error": "已收藏该模板"}`

---

### 4.3 取消收藏

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

**可能错误：**
- 401 `{"error": "认证失败"}`
- 404 `{"error": "未收藏该模板"}`
- 403 `{"error": "权限不足"}`（若 API 层限制只能操作自己）

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

**language 取值：**
- `python`
- `lua`
- `rust`

**响应 (200)：**
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

**执行失败时仍返回 200**（由执行状态区分）：

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

超时示例：

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

**可能错误：**
- 401 `{"error": "认证失败"}`
- 403 `{"error": "用户已被封禁"}`
- 403 `{"error": "用户已被暂停，无法执行该操作"}`

---

### 5.2 获取执行历史

**GET** `/api/users/{id}/executions`

**请求头：**
```
Authorization: Bearer <token>
```

**查询参数：**
- `page`（可选）: 页码，默认 1
- `limit`（可选）: 每页数量，默认 20

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
      "created_at": "2026-01-26T10:00:00Z"
    }
  ],
  "total": 50,
  "page": 1,
  "limit": 20
}
```

**可能错误：**
- 401 `{"error": "认证失败"}`
- 403 `{"error": "权限不足"}`（只能查看自己的执行历史）

---

### 5.3 获取执行详情

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
  "created_at": "2026-01-26T10:00:00Z"
}
```

**可能错误：**
- 401 `{"error": "认证失败"}`
- 403 `{"error": "无权访问该执行记录"}`
- 404 `{"error": "执行记录不存在"}`
