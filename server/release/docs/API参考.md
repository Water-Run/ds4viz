# API 参考文档

## 1. 认证相关 API

### 1.1 用户注册

**POST** `/api/auth/register`

注册新用户账号。

**请求体:**
```json
{
  "username": "string",
  "password": "string"
}
```

**响应 (200):**
```json
{
  "user_id": 1,
  "username": "string",
  "created_at": "2026-01-26T10:00:00Z"
}
```

**错误响应:**
- 400: 用户名已存在
- 400: 用户名或密码格式无效

---

### 1.2 用户登录

**POST** `/api/auth/login`

用户登录并获取JWT令牌。

**请求体:**
```json
{
  "username": "string",
  "password": "string"
}
```

**响应 (200):**
```json
{
  "token": "eyJhbGciOiJIUzI1NiIs...",
  "user": {
    "id": 1,
    "username": "string",
    "status": "Active"
  },
  "expires_at": "2026-01-27T10:00:00Z"
}
```

**错误响应:**
- 401: 用户名或密码错误
- 403: 账号已被封禁或暂停

---

### 1.3 用户登出

**POST** `/api/auth/logout`

登出并销毁当前会话。

**请求头:**
```
Authorization: Bearer <token>
```

**响应 (200):**
```json
{
  "message": "Logged out successfully"
}
```

---

### 1.4 获取当前用户信息

**GET** `/api/auth/me`

获取当前登录用户的信息。

**请求头:**
```
Authorization: Bearer <token>
```

**响应 (200):**
```json
{
  "id": 1,
  "username": "string",
  "avatar_url": "/api/users/1/avatar",
  "status": "Active",
  "created_at": "2026-01-26T10:00:00Z"
}
```

---

## 2. 用户相关 API

### 2.1 获取用户头像

**GET** `/api/users/:id/avatar`

获取指定用户的头像图片。

**响应 (200):**
- Content-Type: image/png 或 image/jpeg
- 二进制图片数据

**错误响应:**
- 404: 用户不存在或未设置头像

---

### 2.2 上传用户头像

**PUT** `/api/users/:id/avatar`

上传或更新用户头像。

**请求头:**
```
Authorization: Bearer <token>
Content-Type: multipart/form-data
```

**请求体:**
```
avatar: <binary file>
```

**响应 (200):**
```json
{
  "message": "Avatar uploaded successfully",
  "avatar_url": "/api/users/1/avatar"
}
```

**错误响应:**
- 401: 未登录
- 403: 只能修改自己的头像
- 400: 文件格式不支持或文件过大

---

### 2.3 修改密码

**PUT** `/api/users/:id/password`

修改用户密码。

**请求头:**
```
Authorization: Bearer <token>
```

**请求体:**
```json
{
  "old_password": "string",
  "new_password": "string"
}
```

**响应 (200):**
```json
{
  "message": "Password updated successfully"
}
```

**错误响应:**
- 401: 未登录
- 403: 只能修改自己的密码
- 400: 原密码错误

---

## 3. 模板相关 API

### 3.1 获取模板列表

**GET** `/api/templates`

获取所有模板列表，支持分类筛选。

**查询参数:**
- `category` (可选): 分类名称
- `page` (可选): 页码，默认1
- `limit` (可选): 每页数量，默认20

**响应 (200):**
```json
{
  "templates": [
    {
      "id": 1,
      "title": "冒泡排序",
      "category": "排序算法",
      "description": "经典的冒泡排序算法...",
      "favorite_count": 42,
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

**GET** `/api/templates/:id`

获取指定模板的详细信息和所有语言实现。

**响应 (200):**
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
    },
    {
      "language": "cpp",
      "code": "void bubbleSort(int arr[], int n) {\n    ...",
      "explanation": "### C++实现\n..."
    }
  ],
  "created_at": "2026-01-20T10:00:00Z"
}
```

**错误响应:**
- 404: 模板不存在

---

### 3.3 获取模板代码

**GET** `/api/templates/:id/code/:language`

获取指定模板在特定语言下的代码实现。

**响应 (200):**
```json
{
  "template_id": 1,
  "language": "python",
  "code": "def bubble_sort(arr):\n    ...",
  "explanation": "### Python实现\n..."
}
```

**错误响应:**
- 404: 模板或语言实现不存在

---

## 4. 收藏相关 API

### 4.1 获取用户收藏列表

**GET** `/api/users/:id/favorites`

获取指定用户的收藏模板列表。

**请求头:**
```
Authorization: Bearer <token>
```

**响应 (200):**
```json
{
  "favorites": [
    {
      "id": 1,
      "title": "冒泡排序",
      "category": "排序算法",
      "favorited_at": "2026-01-25T10:00:00Z"
    }
  ]
}
```

**错误响应:**
- 401: 未登录
- 403: 只能查看自己的收藏

---

### 4.2 收藏模板

**POST** `/api/favorites`

收藏一个模板。

**请求头:**
```
Authorization: Bearer <token>
```

**请求体:**
```json
{
  "template_id": 1
}
```

**响应 (200):**
```json
{
  "message": "Template favorited successfully",
  "favorite_id": 123
}
```

**错误响应:**
- 401: 未登录
- 404: 模板不存在
- 400: 已收藏该模板

---

### 4.3 取消收藏

**DELETE** `/api/favorites/:template_id`

取消收藏指定模板。

**请求头:**
```
Authorization: Bearer <token>
```

**响应 (200):**
```json
{
  "message": "Favorite removed successfully"
}
```

**错误响应:**
- 401: 未登录
- 404: 未收藏该模板

---

## 5. 代码执行相关 API

### 5.1 执行代码

**POST** `/api/execute`

执行用户提交的代码并返回可视化数据。

**请求头:**
```
Authorization: Bearer <token>
```

**请求体:**
```json
{
  "language": "python",
  "code": "def bubble_sort(arr):\n    ..."
}
```

**响应 (200):**
```json
{
  "execution_id": 456,
  "status": "Success",
  "toml_output": "[visualization]\ntype = \"array\"\ndata = [1,2,3]",
  "execution_time": 150,
  "cached": false
}
```

**错误响应:**
- 401: 未登录
- 400: 代码格式错误或语言不支持
- 408: 执行超时
- 500: 执行错误

**执行错误响应 (200):**
```json
{
  "execution_id": 456,
  "status": "Error",
  "error_message": "SyntaxError: invalid syntax (line 5)",
  "execution_time": 50
}
```

---

### 5.2 获取执行历史

**GET** `/api/users/:id/executions`

获取用户的代码执行历史。

**请求头:**
```
Authorization: Bearer <token>
```

**查询参数:**
- `page` (可选): 页码，默认1
- `limit` (可选): 每页数量，默认20
- `status` (可选): 筛选状态 (Success/Error/Timeout)

**响应 (200):**
```json
{
  "executions": [
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

**错误响应:**
- 401: 未登录
- 403: 只能查看自己的执行历史

---

### 5.3 获取执行详情

**GET** `/api/executions/:id`

获取指定执行记录的详细信息。

**请求头:**
```
Authorization: Bearer <token>
```

**响应 (200):**
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

**错误响应:**
- 401: 未登录
- 403: 只能查看自己的执行记录
- 404: 执行记录不存在

---

## 6. 管理员 API

### 6.1 封禁用户

**PUT** `/api/admin/users/:id/ban`

封禁或解封用户账号。

**请求头:**
```
Authorization: Bearer <admin_token>
```

**请求体:**
```json
{
  "status": "Banned"
}
```

**响应 (200):**
```json
{
  "message": "User status updated successfully",
  "user_id": 1,
  "status": "Banned"
}
```

**错误响应:**
- 401: 未登录
- 403: 需要管理员权限

---

### 6.2 创建模板

**POST** `/api/admin/templates`

创建新的算法模板。

**请求头:**
```
Authorization: Bearer <admin_token>
```

**请求体:**
```json
{
  "title": "快速排序",
  "category": "排序算法",
  "description": "## 算法说明\n快速排序...",
  "codes": [
    {
      "language": "python",
      "code": "def quick_sort(arr):\n    ...",
      "explanation": "### Python实现\n..."
    }
  ]
}
```

**响应 (200):**
```json
{
  "template_id": 10,
  "message": "Template created successfully"
}
```

**错误响应:**
- 401: 未登录
- 403: 需要管理员权限

---

### 6.3 更新模板

**PUT** `/api/admin/templates/:id`

更新已有模板信息。

**请求头:**
```
Authorization: Bearer <admin_token>
```

**请求体:**
```json
{
  "title": "快速排序（优化版）",
  "description": "## 更新说明\n..."
}
```

**响应 (200):**
```json
{
  "message": "Template updated successfully"
}
```

---

### 6.4 删除模板

**DELETE** `/api/admin/templates/:id`

删除指定模板。

**请求头:**
```
Authorization: Bearer <admin_token>
```

**响应 (200):**
```json
{
  "message": "Template deleted successfully"
}
```

---

## 通用错误响应

所有API可能返回以下通用错误：

**401 Unauthorized:**
```json
{
  "error": "Unauthorized",
  "message": "Invalid or expired token"
}
```

**403 Forbidden:**
```json
{
  "error": "Forbidden",
  "message": "Insufficient permissions"
}
```

**404 Not Found:**
```json
{
  "error": "Not Found",
  "message": "Resource not found"
}
```

**500 Internal Server Error:**
```json
{
  "error": "Internal Server Error",
  "message": "An unexpected error occurred"
}
```

---

## 认证说明

需要认证的API请求必须在请求头中包含JWT令牌：

```
Authorization: Bearer eyJhbGciOiJIUzI1NiIs...
```

令牌在登录时获取，默认有效期为24小时。
