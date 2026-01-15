# ds4viz API 接口文档 v1.0

## 基础信息

**Base URL:** `https://api.ds4viz.com/api/v1`

**认证方式:** JWT Token (HttpOnly Cookie)

**日期格式:** ISO 8601 (RFC 3339)

**字符编码:** UTF-8

---

## 通用规范

### 请求头

```
Content-Type: application/json
Accept: application/json
```

### 响应格式

#### 成功响应
```json
{
  "data": {}, // 响应数据
  "meta": {}  // 元数据(可选,用于分页等)
}
```

#### 错误响应
```json
{
  "error": {
    "code": "ERROR_CODE",
    "message": "人类可读的错误描述",
    "details": {}  // 可选,额外的错误详情
  }
}
```

### 错误码

| HTTP状态码 | 错误码 | 说明 |
|-----------|--------|------|
| 400 | INVALID_REQUEST | 请求参数错误 |
| 401 | UNAUTHORIZED | 未认证或认证过期 |
| 403 | FORBIDDEN | 无权限访问 |
| 404 | NOT_FOUND | 资源不存在 |
| 409 | CONFLICT | 资源冲突(如用户名已存在) |
| 429 | RATE_LIMIT_EXCEEDED | 请求频率超限 |
| 500 | INTERNAL_ERROR | 服务器内部错误 |

### 分页参数

使用游标分页 (Cursor-based Pagination):

**请求参数:**
```
cursor: string (可选) - 分页游标,首次请求不传
limit: integer (可选) - 每页数量,默认20,最大100
```

**响应格式:**
```json
{
  "data": [...],
  "meta": {
    "next_cursor": "eyJpZCI6IjEyMyJ9",
    "has_more": true,
    "total": 156  // 可选,某些接口提供总数
  }
}
```

---

## 1. 认证模块

### 1.1 用户注册

**端点:** `POST /auth/register`

**请求体:**
```json
{
  "username": "string",     // 必需,4-64字符,字母数字下划线
  "email": "string",        // 必需,有效的邮箱地址
  "password": "string"      // 必需,8-128字符,需包含字母和数字
}
```

**成功响应:** `201 Created`
```json
{
  "data": {
    "user": {
      "id": "uuid",
      "username": "string",
      "email": "string",
      "level": "standard",
      "status": "active",
      "created_at": "2026-01-15T10:00:00Z"
    },
    "token": {
      "access_token": "string",
      "expires_in": 3600
    }
  }
}
```

**错误响应:**
- `409 CONFLICT` - `USERNAME_EXISTS` / `EMAIL_EXISTS`
- `400 INVALID_REQUEST` - `INVALID_USERNAME` / `INVALID_EMAIL` / `WEAK_PASSWORD`

**备注:**
- 注册成功后自动登录,设置HttpOnly Cookie
- 初始用户等级为 `standard`

---

### 1.2 用户登录

**端点:** `POST /auth/login`

**请求体:**
```json
{
  "username": "string",     // 必需,用户名或邮箱
  "password": "string"      // 必需
}
```

**成功响应:** `200 OK`
```json
{
  "data": {
    "user": {
      "id": "uuid",
      "username": "string",
      "email": "string",
      "avatar_url": "string",  // 可为null
      "bio": "string",         // 可为null
      "level": "standard",
      "status": "active",
      "created_at": "2026-01-15T10:00:00Z"
    },
    "token": {
      "access_token": "string",
      "expires_in": 3600
    }
  }
}
```

**错误响应:**
- `401 UNAUTHORIZED` - `INVALID_CREDENTIALS`
- `403 FORBIDDEN` - `ACCOUNT_BANNED` / `ACCOUNT_SUSPENDED`

**备注:**
- 如果账户被封禁,返回封禁原因和截止时间:
```json
{
  "error": {
    "code": "ACCOUNT_BANNED",
    "message": "账户已被封禁",
    "details": {
      "reason": "违反社区规则",
      "banned_until": "2026-02-01T00:00:00Z"  // null表示永久
    }
  }
}
```

---

### 1.3 用户登出

**端点:** `POST /auth/logout`

**认证:** 必需

**成功响应:** `204 No Content`

**备注:** 清除HttpOnly Cookie

---

### 1.4 刷新Token

**端点:** `POST /auth/refresh`

**请求体:**
```json
{
  "refresh_token": "string"  // 可选,如使用Cookie则不需要
}
```

**成功响应:** `200 OK`
```json
{
  "data": {
    "access_token": "string",
    "expires_in": 3600
  }
}
```

**错误响应:**
- `401 UNAUTHORIZED` - `INVALID_TOKEN` / `TOKEN_EXPIRED`

---

### 1.5 获取当前用户

**端点:** `GET /auth/me`

**认证:** 必需

**成功响应:** `200 OK`
```json
{
  "data": {
    "id": "uuid",
    "username": "string",
    "email": "string",
    "avatar_url": "string",
    "bio": "string",
    "level": "standard",
    "status": "active",
    "created_at": "2026-01-15T10:00:00Z",
    "updated_at": "2026-01-15T10:00:00Z"
  }
}
```

---

## 2. 用户管理

### 2.1 获取用户公开信息

**端点:** `GET /users/{user_id}`

**路径参数:**
- `user_id`: UUID - 用户ID

**成功响应:** `200 OK`
```json
{
  "data": {
    "id": "uuid",
    "username": "string",
    "avatar_url": "string",
    "bio": "string",
    "level": "standard",
    "created_at": "2026-01-15T10:00:00Z",
    "stats": {
      "total_submissions": 156,
      "total_templates_liked": 23,
      "total_comments": 12
    }
  }
}
```

**错误响应:**
- `404 NOT_FOUND` - `USER_NOT_FOUND`

**备注:** 不返回email等敏感信息

---

### 2.2 搜索用户

**端点:** `GET /users/search`

**查询参数:**
```
q: string (必需) - 搜索关键词,最少2个字符
limit: integer (可选) - 返回数量,默认10,最大50
```

**成功响应:** `200 OK`
```json
{
  "data": [
    {
      "id": "uuid",
      "username": "string",
      "avatar_url": "string",
      "bio": "string",
      "level": "standard"
    }
  ],
  "meta": {
    "total": 3
  }
}
```

**备注:** 使用PostgreSQL pg_trgm进行模糊搜索

---

### 2.3 更新当前用户信息

**端点:** `PUT /users/me`

**认证:** 必需

**请求体:**
```json
{
  "bio": "string",          // 可选,最大5000字符
  "email": "string"         // 可选,需验证格式
}
```

**成功响应:** `200 OK`
```json
{
  "data": {
    "id": "uuid",
    "username": "string",
    "email": "string",
    "avatar_url": "string",
    "bio": "string",
    "level": "standard",
    "status": "active",
    "updated_at": "2026-01-15T12:00:00Z"
  }
}
```

**错误响应:**
- `409 CONFLICT` - `EMAIL_EXISTS`

**备注:** username不可修改

---

### 2.4 上传头像

**端点:** `PUT /users/me/avatar`

**认证:** 必需

**Content-Type:** `multipart/form-data`

**请求体:**
```
avatar: file - 图片文件,支持 jpeg/png/webp,最大2MB
```

**成功响应:** `200 OK`
```json
{
  "data": {
    "avatar_url": "/api/v1/users/{user_id}/avatar?v={timestamp}"
  }
}
```

**错误响应:**
- `400 INVALID_REQUEST` - `INVALID_FILE_TYPE` / `FILE_TOO_LARGE`

---

### 2.5 获取用户头像

**端点:** `GET /users/{user_id}/avatar`

**查询参数:**
```
v: integer (可选) - 版本号/时间戳,用于缓存控制
```

**成功响应:** `200 OK`

**Content-Type:** `image/jpeg` / `image/png` / `image/webp`

**Response Body:** 二进制图片数据

**错误响应:**
- `404 NOT_FOUND` - 用户无头像

**备注:**
- 返回原始二进制数据
- 设置合适的Cache-Control头

---

### 2.6 获取用户代码历史

**端点:** `GET /users/{user_id}/submissions`

**路径参数:**
- `user_id`: UUID - 用户ID

**查询参数:**
```
cursor: string (可选) - 分页游标
limit: integer (可选) - 每页数量,默认20,最大100
language_id: string (可选) - 语言过滤
status: string (可选) - 状态过滤: success/failed/timeout
```

**成功响应:** `200 OK`
```json
{
  "data": [
    {
      "id": "uuid",
      "language": {
        "id": "python",
        "display_name": "Python",
        "version": "3.12"
      },
      "status": "success",
      "cache_hit": false,
      "cpu_time_ms": 234,
      "created_at": "2026-01-15T10:30:00Z",
      "preview": "print('Hello World')..."  // 代码前100字符
    }
  ],
  "meta": {
    "next_cursor": "eyJpZCI6IjEyMyJ9",
    "has_more": true
  }
}
```

**错误响应:**
- `404 NOT_FOUND` - `USER_NOT_FOUND`

**备注:** 只能查看其他用户成功执行的代码,自己可查看全部

---

## 3. 等级升级

### 3.1 申请升级

**端点:** `POST /level-upgrades`

**认证:** 必需

**请求体:**
```json
{
  "reason": "string"  // 必需,申请理由,10-2000字符
}
```

**成功响应:** `201 Created`
```json
{
  "data": {
    "id": "uuid",
    "user_id": "uuid",
    "status": "pending",
    "reason": "string",
    "created_at": "2026-01-15T10:00:00Z"
  }
}
```

**错误响应:**
- `409 CONFLICT` - `PENDING_REQUEST_EXISTS` (已有待审核申请)
- `403 FORBIDDEN` - `ALREADY_ENHANCED` (已是Enhanced级别)

---

### 3.2 查看申请状态

**端点:** `GET /level-upgrades/me`

**认证:** 必需

**查询参数:**
```
cursor: string (可选)
limit: integer (可选) - 默认10
```

**成功响应:** `200 OK`
```json
{
  "data": [
    {
      "id": "uuid",
      "status": "approved",
      "reason": "string",
      "reviewer_note": "string",
      "created_at": "2026-01-15T10:00:00Z",
      "reviewed_at": "2026-01-16T09:00:00Z"
    }
  ],
  "meta": {
    "next_cursor": null,
    "has_more": false
  }
}
```

---

## 4. 代码执行

### 4.1 提交代码执行

**端点:** `POST /executions`

**认证:** 必需

**请求体:**
```json
{
  "language_id": "python",  // 必需,必须是启用的语言
  "code": "string"          // 必需,最大16KB (Standard用户)
}
```

**成功响应:** `202 Accepted` (需要执行) 或 `200 OK` (缓存命中)

**缓存命中:**
```json
{
  "data": {
    "id": "uuid",
    "status": "success",
    "cache_hit": true,
    "language": {
      "id": "python",
      "display_name": "Python",
      "version": "3.12"
    },
    "result": {
      "stdout": "string",
      "stderr": "string",
      "exit_code": 0,
      "cpu_time_ms": 234
    },
    "created_at": "2026-01-15T10:00:00Z"
  }
}
```

**需要执行:**
```json
{
  "data": {
    "id": "uuid",
    "status": "pending",
    "cache_hit": false,
    "language": {
      "id": "python",
      "display_name": "Python",
      "version": "3.12"
    },
    "created_at": "2026-01-15T10:00:00Z"
  }
}
```

**错误响应:**
- `400 INVALID_REQUEST` - `INVALID_LANGUAGE` / `CODE_TOO_LARGE`
- `403 FORBIDDEN` - `QUOTA_EXCEEDED` / `LANGUAGE_NOT_ALLOWED`
- `429 RATE_LIMIT_EXCEEDED` - 提交频率过高

**配额超限错误详情:**
```json
{
  "error": {
    "code": "QUOTA_EXCEEDED",
    "message": "CPU时间配额已用完",
    "details": {
      "period_start": "2026-01-15T10:00:00Z",
      "period_end": "2026-01-15T11:00:00Z",
      "quota_limit_ms": 10000,
      "quota_used_ms": 10234,
      "reset_at": "2026-01-15T11:00:00Z"
    }
  }
}
```

**备注:**
- Standard用户只能使用解释型语言
- Standard用户代码最大16KB
- 计算SHA256(language_id + code)检查缓存

---

### 4.2 获取执行结果

**端点:** `GET /executions/{execution_id}`

**认证:** 必需

**路径参数:**
- `execution_id`: UUID - 执行ID

**成功响应:** `200 OK`
```json
{
  "data": {
    "id": "uuid",
    "status": "success",  // pending/running/success/failed/timeout
    "cache_hit": false,
    "language": {
      "id": "python",
      "display_name": "Python",
      "version": "3.12"
    },
    "code": "print('Hello World')",
    "result": {
      "stdout": "Hello World\n",
      "stderr": "",
      "exit_code": 0,
      "cpu_time_ms": 234
    },
    "created_at": "2026-01-15T10:00:00Z"
  }
}
```

**执行中响应:**
```json
{
  "data": {
    "id": "uuid",
    "status": "running",
    "cache_hit": false,
    "language": {...},
    "created_at": "2026-01-15T10:00:00Z"
  }
}
```

**错误响应:**
- `404 NOT_FOUND` - `EXECUTION_NOT_FOUND`
- `403 FORBIDDEN` - 不能查看他人的执行

**备注:**
- result字段在执行完成后才返回
- 前端可轮询此接口获取结果(建议间隔1-2秒)

---

### 4.3 获取执行历史

**端点:** `GET /executions/me`

**认证:** 必需

**查询参数:**
```
cursor: string (可选)
limit: integer (可选) - 默认20,最大100
language_id: string (可选)
status: string (可选)
```

**成功响应:** `200 OK`
```json
{
  "data": [
    {
      "id": "uuid",
      "language": {
        "id": "python",
        "display_name": "Python"
      },
      "status": "success",
      "cache_hit": true,
      "cpu_time_ms": 234,
      "created_at": "2026-01-15T10:30:00Z",
      "code_preview": "print('Hello')..."
    }
  ],
  "meta": {
    "next_cursor": "eyJpZCI6IjEyMyJ9",
    "has_more": true
  }
}
```

---

## 5. 模板系统

### 5.1 获取模板列表

**端点:** `GET /templates`

**查询参数:**
```
cursor: string (可选)
limit: integer (可选) - 默认20,最大50
category: string (可选) - 分类过滤
sort: string (可选) - 排序方式: latest/popular,默认latest
```

**成功响应:** `200 OK`
```json
{
  "data": [
    {
      "id": "uuid",
      "slug": "hello-world",
      "title": "Hello World示例",
      "description": "最简单的入门示例...",  // Markdown,最多200字符预览
      "category": "basics",
      "like_count": 42,
      "comment_count": 8,
      "languages": ["python", "lua", "rust"],  // 支持的语言列表
      "is_liked": false,      // 当前用户是否点赞
      "is_favorited": false,  // 当前用户是否收藏
      "created_at": "2026-01-10T10:00:00Z",
      "updated_at": "2026-01-12T10:00:00Z"
    }
  ],
  "meta": {
    "next_cursor": "eyJpZCI6IjEyMyJ9",
    "has_more": true,
    "total": 156
  }
}
```

**备注:**
- `is_liked`和`is_favorited`需要认证才有值,未认证时为false

---

### 5.2 获取模板详情

**端点:** `GET /templates/{slug}`

**路径参数:**
- `slug`: string - 模板URL标识符

**查询参数:**
```
language: string (可选) - 指定返回的语言版本,不传则返回所有
```

**成功响应:** `200 OK`
```json
{
  "data": {
    "id": "uuid",
    "slug": "hello-world",
    "title": "Hello World示例",
    "description": "# 简介\n这是最简单的入门示例...",  // 完整Markdown
    "category": "basics",
    "like_count": 42,
    "comment_count": 8,
    "is_liked": false,
    "is_favorited": false,
    "codes": [
      {
        "language": {
          "id": "python",
          "display_name": "Python",
          "version": "3.12",
          "file_extension": ".py"
        },
        "code": "print('Hello, World!')"
      },
      {
        "language": {
          "id": "lua",
          "display_name": "Lua",
          "version": "5.4",
          "file_extension": ".lua"
        },
        "code": "print('Hello, World!')"
      }
    ],
    "created_at": "2026-01-10T10:00:00Z",
    "updated_at": "2026-01-12T10:00:00Z"
  }
}
```

**如果指定language参数:**
```json
{
  "data": {
    ...,
    "codes": [
      {
        "language": {...},
        "code": "print('Hello, World!')"
      }
    ]
  }
}
```

**错误响应:**
- `404 NOT_FOUND` - `TEMPLATE_NOT_FOUND` / `LANGUAGE_NOT_AVAILABLE`

---

### 5.3 点赞模板

**端点:** `POST /templates/{template_id}/likes`

**认证:** 必需

**路径参数:**
- `template_id`: UUID - 模板ID

**成功响应:** `201 Created`
```json
{
  "data": {
    "liked": true,
    "like_count": 43
  }
}
```

**错误响应:**
- `404 NOT_FOUND` - `TEMPLATE_NOT_FOUND`
- `409 CONFLICT` - `ALREADY_LIKED`

---

### 5.4 取消点赞

**端点:** `DELETE /templates/{template_id}/likes`

**认证:** 必需

**成功响应:** `200 OK`
```json
{
  "data": {
    "liked": false,
    "like_count": 42
  }
}
```

**错误响应:**
- `404 NOT_FOUND` - `NOT_LIKED`

---

### 5.5 评论模板

**端点:** `POST /templates/{template_id}/comments`

**认证:** 必需

**请求体:**
```json
{
  "content": "string"  // 必需,1-2000字符
}
```

**成功响应:** `201 Created`
```json
{
  "data": {
    "id": "uuid",
    "user": {
      "id": "uuid",
      "username": "string",
      "avatar_url": "string"
    },
    "content": "string",
    "created_at": "2026-01-15T10:00:00Z"
  }
}
```

---

### 5.6 获取模板评论

**端点:** `GET /templates/{template_id}/comments`

**查询参数:**
```
cursor: string (可选)
limit: integer (可选) - 默认20,最大100
```

**成功响应:** `200 OK`
```json
{
  "data": [
    {
      "id": "uuid",
      "user": {
        "id": "uuid",
        "username": "string",
        "avatar_url": "string",
        "level": "enhanced"
      },
      "content": "string",
      "created_at": "2026-01-15T10:00:00Z",
      "updated_at": "2026-01-15T10:30:00Z"  // 如果编辑过
    }
  ],
  "meta": {
    "next_cursor": "eyJpZCI6IjEyMyJ9",
    "has_more": true
  }
}
```

---

### 5.7 收藏模板

**端点:** `POST /templates/{template_id}/favorites`

**认证:** 必需

**成功响应:** `201 Created`
```json
{
  "data": {
    "favorited": true
  }
}
```

**错误响应:**
- `409 CONFLICT` - `ALREADY_FAVORITED`

---

### 5.8 取消收藏

**端点:** `DELETE /templates/{template_id}/favorites`

**认证:** 必需

**成功响应:** `200 OK`
```json
{
  "data": {
    "favorited": false
  }
}
```

---

### 5.9 获取我的收藏

**端点:** `GET /favorites`

**认证:** 必需

**查询参数:**
```
cursor: string (可选)
limit: integer (可选) - 默认20,最大50
```

**成功响应:** `200 OK`
```json
{
  "data": [
    {
      "template": {
        "id": "uuid",
        "slug": "string",
        "title": "string",
        "description": "string",
        "category": "string",
        "like_count": 42
      },
      "favorited_at": "2026-01-15T10:00:00Z"
    }
  ],
  "meta": {
    "next_cursor": "eyJpZCI6IjEyMyJ9",
    "has_more": true
  }
}
```

---

## 6. 语言配置

### 6.1 获取语言列表

**端点:** `GET /languages`

**查询参数:**
```
enabled_only: boolean (可选) - 只返回启用的语言,默认true
```

**成功响应:** `200 OK`
```json
{
  "data": [
    {
      "id": "python",
      "display_name": "Python",
      "version": "3.12",
      "is_interpreted": true,
      "file_extension": ".py",
      "enabled": true,
      "timeout_ms": 30000
    },
    {
      "id": "rust",
      "display_name": "Rust",
      "version": "1.75",
      "is_interpreted": false,
      "file_extension": ".rs",
      "enabled": true,
      "timeout_ms": 60000
    }
  ]
}
```

**备注:**
- 按sort_order排序
- Standard用户只能使用is_interpreted=true的语言

---

## 7. 配额查询

### 7.1 查询当前配额

**端点:** `GET /quota/me`

**认证:** 必需

**成功响应:** `200 OK`

**Standard用户:**
```json
{
  "data": {
    "level": "standard",
    "period": {
      "start": "2026-01-15T10:00:00Z",
      "end": "2026-01-15T11:00:00Z",
      "duration_minutes": 60
    },
    "cpu_time": {
      "limit_ms": 10000,
      "used_ms": 3456,
      "remaining_ms": 6544,
      "usage_percentage": 34.56
    },
    "code_size": {
      "max_bytes": 16384
    },
    "reset_at": "2026-01-15T11:00:00Z"
  }
}
```

**Enhanced用户:**
```json
{
  "data": {
    "level": "enhanced",
    "unlimited": true
  }
}
```

---

## 附录

### A. 数据类型定义

#### User Level
```
standard  - 标准用户
enhanced  - 增强用户
```

#### User Status
```
active     - 正常
banned     - 封禁
suspended  - 暂停
```

#### Execution Status
```
pending  - 等待执行
running  - 执行中
success  - 成功
failed   - 失败
timeout  - 超时
```

#### Upgrade Request Status
```
pending   - 待审核
approved  - 已批准
rejected  - 已拒绝
```

### B. 限流规则

| 端点类别 | 速率限制 |
|---------|---------|
| 认证相关 | 5次/分钟 |
| 代码执行 | Standard: 10次/分钟, Enhanced: 60次/分钟 |
| 模板评论/点赞 | 30次/分钟 |
| 其他API | 100次/分钟 |

### C. 服务器配置项

以下配置项可在服务器端调整:

- `quota_period_minutes`: 配额刷新周期(分钟)
- `quota_cpu_time_ms`: Standard用户CPU时间限制(毫秒)
- `execution_timeout_ms`: 各语言默认超时时间
- `max_code_size_bytes`: Standard用户代码大小限制
