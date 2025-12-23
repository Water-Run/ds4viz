# `ds4viz`  

`WaterRun` `2025-12-22`  

毕业论文项目. 一个可扩展的, 数据结构可视化教学软件.  

## 架构  

```plaintext
调用 [各语言的ds4viz库实现] 编写代码 -> 运行生成统一中间文件(.toml) -> 渲染器显示交互
```

## `.toml`IR生成: 对应语言支持, 安装及文档  

库设计遵循原则: 代码即文档, 统一且简易上手.  

|语言|安装|文档|
|---|---|---|
|`Python`|`pip install ds4viz`|[ds4viz-Python](./library/python/README.md)|  

## 渲染器  

|名称|下载|文档|
|---|---|---|
|`vue`: 网站, SPA|||
|`cli`: 命令行|||
|`winui3`: Windows桌面端|||
|`dart&flutter`: Android应用|||

## 远程服务(Vue Web)  
