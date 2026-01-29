r"""
服务层模块，提供业务逻辑处理

:file: src/service/__init__.py
:author: WaterRun
:time: 2026-01-29
"""

from service.auth_service import (
    create_session,
    verify_session,
    invalidate_session,
    invalidate_all_sessions,
    cleanup_expired_sessions,
)
from service.user_service import (
    create_user,
    authenticate_user,
    get_user_by_id,
    update_avatar,
    get_avatar,
    change_password,
    check_user_status,
    require_active_status,
    add_favorite,
    remove_favorite,
    get_favorites,
    UserSuspendedError,
)
from service.template_service import (
    get_template_by_id,
    list_templates,
    get_categories,
    search_templates,
)
from service.execution_service import (
    execute_code,
    get_execution_by_id,
    list_executions,
    create_sandbox_config_from_app_config,
)
from service.sandbox_service import (
    SandboxResult,
    SandboxConfig,
    run_code,
)

__all__: list[str] = [
    # auth
    "create_session",
    "verify_session",
    "invalidate_session",
    "invalidate_all_sessions",
    "cleanup_expired_sessions",
    # user
    "create_user",
    "authenticate_user",
    "get_user_by_id",
    "update_avatar",
    "get_avatar",
    "change_password",
    "check_user_status",
    "require_active_status",
    "add_favorite",
    "remove_favorite",
    "get_favorites",
    "UserSuspendedError",
    # template
    "get_template_by_id",
    "list_templates",
    "get_categories",
    "search_templates",
    # execution
    "execute_code",
    "get_execution_by_id",
    "list_executions",
    "create_sandbox_config_from_app_config",
    # sandbox
    "SandboxResult",
    "SandboxConfig",
    "run_code",
]
