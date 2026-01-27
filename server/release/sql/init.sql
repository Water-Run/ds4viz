-- ============================================
-- 数据库初始化脚本
-- ============================================

-- 1. users (用户表)
CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(32) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    avatar BYTEA,
    status VARCHAR(16) NOT NULL DEFAULT 'Active',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_users_username ON users(username);

-- 2. sessions (会话表)
CREATE TABLE sessions (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    token VARCHAR(512) UNIQUE NOT NULL,
    ip_address VARCHAR(64) NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_sessions_token ON sessions(token);
CREATE INDEX idx_sessions_user_id ON sessions(user_id);

-- 3. templates (模板表)
CREATE TABLE templates (
    id SERIAL PRIMARY KEY,
    title VARCHAR(256) NOT NULL,
    category VARCHAR(64) NOT NULL,
    description TEXT NOT NULL,
    favorite_count INTEGER NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_templates_category ON templates(category);

-- 4. template_codes (模板代码表)
CREATE TABLE template_codes (
    id SERIAL PRIMARY KEY,
    template_id INTEGER NOT NULL REFERENCES templates(id) ON DELETE CASCADE,
    language VARCHAR(32) NOT NULL,
    code TEXT NOT NULL,
    explanation TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(template_id, language)
);

CREATE INDEX idx_template_codes_template_id ON template_codes(template_id);

-- 5. user_favorites (用户收藏表)
CREATE TABLE user_favorites (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    template_id INTEGER NOT NULL REFERENCES templates(id) ON DELETE CASCADE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(user_id, template_id)
);

CREATE INDEX idx_user_favorites_user_id ON user_favorites(user_id);

-- 6. executions (执行历史表)
CREATE TABLE executions (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    code_hash VARCHAR(64) NOT NULL,
    language VARCHAR(32) NOT NULL,
    code TEXT NOT NULL,
    toml_output TEXT,
    status VARCHAR(16) NOT NULL,
    error_message TEXT,
    execution_time INTEGER,
    ip_address VARCHAR(64) NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_executions_user_id ON executions(user_id);
CREATE INDEX idx_executions_created_at ON executions(created_at);

-- 7. execution_cache (执行缓存表)
CREATE TABLE execution_cache (
    id SERIAL PRIMARY KEY,
    code_hash VARCHAR(64) NOT NULL,
    language VARCHAR(32) NOT NULL,
    toml_output TEXT NOT NULL,
    hit_count INTEGER NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    last_hit_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    UNIQUE(code_hash, language)
);

CREATE INDEX idx_execution_cache_lookup ON execution_cache(code_hash, language);

-- 8. logs (日志表)
CREATE TABLE logs (
    id SERIAL PRIMARY KEY,
    level VARCHAR(16) NOT NULL,
    module VARCHAR(64) NOT NULL,
    message TEXT NOT NULL,
    user_id INTEGER REFERENCES users(id) ON DELETE SET NULL,
    ip_address VARCHAR(64),
    request_id VARCHAR(64),
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_logs_level ON logs(level);
CREATE INDEX idx_logs_module ON logs(module);
CREATE INDEX idx_logs_created_at ON logs(created_at);
CREATE INDEX idx_logs_user_id ON logs(user_id);
CREATE INDEX idx_logs_request_id ON logs(request_id);

-- ============================================
-- 触发器函数
-- ============================================

-- 更新收藏计数
CREATE OR REPLACE FUNCTION update_template_favorite_count()
RETURNS TRIGGER AS $$
BEGIN
    IF TG_OP = 'INSERT' THEN
        UPDATE templates SET favorite_count = favorite_count + 1 
        WHERE id = NEW.template_id;
    ELSIF TG_OP = 'DELETE' THEN
        UPDATE templates SET favorite_count = favorite_count - 1 
        WHERE id = OLD.template_id;
    END IF;
    RETURN NULL;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trigger_update_favorite_count
AFTER INSERT OR DELETE ON user_favorites
FOR EACH ROW EXECUTE FUNCTION update_template_favorite_count();

-- 自动更新时间戳
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER update_users_updated_at 
BEFORE UPDATE ON users
FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_templates_updated_at 
BEFORE UPDATE ON templates
FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();