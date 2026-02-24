# `ds4viz` 后台管理REPL  

一个简单的管理`REPL`, 直接对服务器进行读写操作.  

依赖:  

```bash
sudo dnf install lua-luaossl
sudo luarocks install pgmoon
```

启动:  

```bash
lua admin-repl.lua
```

> 安全性: 直接访问此目录可修改配置. 服务器配置`tmux`终端, 使用`ssh`远程连接  

启动时, 留意从脚本同目录启动, 避免日志文件`admin-repl.log`错误存储.  
