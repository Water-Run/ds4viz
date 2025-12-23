# `ds4viz`: 服务  

`ds4viz`服务端, 提供了安全的远程代码执行返回`.toml` IR的服务.  
采用`Python`+`FastAPI`. 数据库使用`PostgreSQL 18`.  

> 服务环境部署于: `Fedora 43`  

## 复现准备  

### Python环境  

Python已自带, 检查版本, 建议使用`3.14`:  

```bash
waterrun@WaterRun:~$ python --version
Python 3.14.2
```

安装库:  

```bash
sudo dnf -y update
sudo dnf -y install git curl python3 python3-pip python3-devel gcc postgresql-libs postgresql-devel
```

```bash
sudo pip3 install -U pip setuptools wheel
sudo pip3 install "psycopg[binary]"
sudo pip3 install asyncpg
sudo pip3 install "SQLAlchemy>=2.0" "psycopg[binary]" alembic
```

### 数据库  

安装`PostgreSQL`:  

```bash
sudo dnf install -y postgresql-server postgresql-contrib
sudo postgresql-setup --initdb --unit postgresql
sudo systemctl enable --now postgresql
```

验证:  

```bash
psql --version
sudo -u postgres psql -c "select version();"
```

执行`./sqls/create_table.sql`建表.  

## 启动服务  

确保`PostgreSQL`服务:  

```bash
```

```bash
python main.py
```
