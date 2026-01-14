# `ds4viz`: 服务  

`ds4viz`服务端, 提供了安全的远程代码执行返回`.toml` IR的服务.  
采用`Python`+`FastAPI`. 数据库使用`PostgreSQL 18`. 缓存系统使用`Redis`.  

> 服务环境部署于: `Fedora 43`  

## 技术栈  

- `Rust`  
- `Poem`  
- `PostgreSQL`  
- `Redis`  

## 架构  

## 编译和测试  

## 配置项和说明  

## 启动服务  

## 后台管理  

后台管理系统以部署在服务器本机上的TUI提供.  

### 技术栈  

- `lua`  
- `ltui`  
- `sqlite`  

### 启动  

```bash
lua main.lua
```

### 配置定义和修改  

|配置项|说明|
|---|---|
|`password`|进入口令, 用于在部署在公网上等情况时添加最小的验证|
