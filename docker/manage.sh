#!/usr/bin/env bash
# ds4viz 一键 Docker 管理脚本
# 用法: ./docker/manage.sh [up|down|restart|logs|rebuild|ps|clean]

set -euo pipefail

COMPOSE_FILE="${COMPOSE_FILE:-docker-compose.yml}"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$PROJECT_DIR"

ensure_env() {
    if [ ! -f .env ]; then
        if [ -f docker/.env.example ]; then
            cp docker/.env.example .env
            echo "[manage] 已根据 docker/.env.example 创建 .env"
        else
            echo "[manage] 警告: docker/.env.example 不存在" >&2
        fi
    fi
}

cmd="${1:-help}"
shift || true

case "$cmd" in
    up)
        ensure_env
        echo "[manage] 启动 ds4viz 栈 (postgres + backend + frontend)..."
        docker compose -f "$COMPOSE_FILE" up -d --build
        echo "[manage] 启动完成:"
        echo "  前端: http://localhost:${FRONTEND_PORT:-5173}"
        echo "  后端: http://localhost:${BACKEND_PORT:-10000}"
        echo "  数据库: localhost:${POSTGRES_PORT:-5432}"
        echo "[manage] 查看日志: ./docker/manage.sh logs"
        ;;
    down)
        echo "[manage] 停止并移除容器..."
        docker compose -f "$COMPOSE_FILE" down
        ;;
    restart)
        ensure_env
        echo "[manage] 重启服务..."
        docker compose -f "$COMPOSE_FILE" restart "$@"
        ;;
    logs)
        docker compose -f "$COMPOSE_FILE" logs -f --tail=100 "$@"
        ;;
    rebuild)
        ensure_env
        echo "[manage] 重新构建镜像..."
        docker compose -f "$COMPOSE_FILE" build --no-cache "$@"
        ;;
    ps)
        docker compose -f "$COMPOSE_FILE" ps
        ;;
    clean)
        echo "[manage] 停止并清理容器、卷、网络..."
        docker compose -f "$COMPOSE_FILE" down -v --remove-orphans
        ;;
    *)
        cat <<EOF
ds4viz Docker 管理脚本

用法: $0 <命令>

命令:
  up         启动整个栈 (postgres + backend + frontend), 首次会构建镜像
  down       停止并移除容器 (保留数据卷)
  restart    重启指定服务 (例: ./docker/manage.sh restart backend)
  logs       跟踪日志 (例: ./docker/manage.sh logs backend)
  rebuild    强制重新构建镜像
  ps         查看服务状态
  clean      停止并清理所有容器、数据卷 (会删除数据库内容!)

环境变量可通过 .env 或 docker/.env.example 配置.
EOF
        ;;
esac