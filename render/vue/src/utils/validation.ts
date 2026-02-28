/**
 * 输入验证工具
 *
 * 提供用户名与密码的客户端校验逻辑，
 * 规则与后端 API 约束一致。
 *
 * @file src/utils/validation.ts
 * @author WaterRun
 * @date 2026-02-28
 */

/**
 * 用户名校验结果
 *
 * @interface
 */
export interface UsernameValidation {
    /** 是否通过全部校验 */
    valid: boolean
    /** 长度在 1–64 之间 */
    lengthOk: boolean
    /** 符合标识符格式 */
    formatOk: boolean
}

/**
 * 密码校验结果
 *
 * @interface
 */
export interface PasswordValidation {
    /** 是否通过全部校验 */
    valid: boolean
    /** 长度在 8–32 之间 */
    lengthOk: boolean
    /** 包含大写字母 */
    hasUppercase: boolean
    /** 包含小写字母 */
    hasLowercase: boolean
    /** 包含数字 */
    hasDigit: boolean
    /** 包含特殊字符 */
    hasSpecial: boolean
}

/** 特殊字符正则 */
const SPECIAL_CHAR_REGEX = /[!"#$%&'()*+,\-./:;<=>?@[\\\]^_`{|}~]/

/**
 * 校验用户名
 *
 * 规则：1–64 字符，以字母或下划线开头，仅含字母、数字、下划线（支持 Unicode 字母）。
 *
 * @param username - 用户名
 * @returns 校验结果
 */
export function validateUsername(username: string): UsernameValidation {
    const lengthOk = username.length >= 1 && username.length <= 64
    const formatOk = username.length > 0 && /^[\p{L}_][\p{L}\p{N}_]*$/u.test(username)
    return { valid: lengthOk && formatOk, lengthOk, formatOk }
}

/**
 * 校验密码
 *
 * 规则：8–32 字符，须同时包含大写字母、小写字母、数字、特殊字符各至少一个。
 *
 * @param password - 密码
 * @returns 校验结果
 */
export function validatePassword(password: string): PasswordValidation {
    const lengthOk = password.length >= 8 && password.length <= 32
    const hasUppercase = /[A-Z]/.test(password)
    const hasLowercase = /[a-z]/.test(password)
    const hasDigit = /[0-9]/.test(password)
    const hasSpecial = SPECIAL_CHAR_REGEX.test(password)
    return {
        valid: lengthOk && hasUppercase && hasLowercase && hasDigit && hasSpecial,
        lengthOk,
        hasUppercase,
        hasLowercase,
        hasDigit,
        hasSpecial,
    }
}