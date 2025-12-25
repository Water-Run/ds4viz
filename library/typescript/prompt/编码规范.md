# `ds4viz`: TypeScript编码规范

在编写TypeScript代码时，你需要遵循以下编码规范：

**语言级别:** `TypeScript 5.3+`，目标编译为 `ES2022`。

**类型注解:** 任何变量、参数、返回值等都需要显式类型注解。禁用`any`类型，使用`unknown`替代。启用严格模式（`strict: true`）。将TypeScript视为强类型语言进行编码。

**编码规范:** 遵循以下标准：
- 使用ESLint（推荐`@typescript-eslint`）+ Prettier进行代码检查和格式化
- 在VSCode/WebStorm最严格模式下，不能有任何Warning或Error
- 采用现代ES6+写法，包括：
  - 优先使用`const`，需要重新赋值时使用`let`，禁用`var`
  - 使用箭头函数、解构赋值、展开运算符、可选链（`?.`）、空值合并（`??`）
  - 善用`map`、`filter`、`reduce`、`flatMap`等数组方法
  - 使用模板字符串而非字符串拼接
  - 使用`class`关键字定义类，支持私有字段（`#`）
  - 使用`enum`、`interface`、`type`、`namespace`等TypeScript特性
  - 使用装饰器（当TypeScript支持稳定后）
  - 使用`readonly`、`private`、`protected`、`public`等访问修饰符
  - 使用泛型增强代码复用性

**命名规范:**
- 文件名：使用kebab-case（如`data-processor.ts`）
- 类名、接口名、类型别名、枚举名：使用PascalCase（如`DataProcessor`、`IUserConfig`）
- 函数名、变量名、方法名：使用camelCase（如`processData`、`userName`）
- 常量：使用UPPER_SNAKE_CASE（如`MAX_RETRY_COUNT`）
- 私有成员：使用`#`前缀或`private`关键字

**文档字符串（JSDoc）:** 在文件开头、任何函数、类、接口、类型、枚举等都要有遵循以下规范的JSDoc文档注释。代码即文档的编码规范：代码需要尽可能自描述、易懂，减少非JSDoc之外的注释内容。

文件开头：

```typescript
/**
 * 文件功能描述，使用中文
 *
 * @file 文件名，从项目目录开始的相对路径
 * @author WaterRun
 * @date 时间，如2025-12-25。更新为最新时间。
 */
```

函数及方法：

```typescript
/**
 * 函数功能描述，使用中文
 *
 * @param paramName - 参数说明
 * @returns 返回值说明
 * @throws {ErrorType} 异常说明
 *
 * @example
 * ```typescript
 * const result = functionName(arg1, arg2);
 * ```
 */
```

类：

```typescript
/**
 * 类描述，中文
 *
 * @template T - 泛型参数说明（如有）
 */
```

接口：

```typescript
/**
 * 接口描述，中文
 *
 * @interface
 */
```

类型别名：

```typescript
/**
 * 类型描述，中文
 *
 * @typedef
 */
```

枚举：

```typescript
/**
 * 枚举描述，中文
 *
 * @enum
 */
```

属性：

```typescript
/**
 * 属性描述，中文
 */
```

**模块化:**
- 使用ES6模块系统（`import`/`export`）
- 优先使用命名导出，避免默认导出
- 一个文件一个主要类/接口，相关的辅助类型可以在同一文件
- 使用绝对路径导入（配置`tsconfig.json`的`paths`）

**错误处理:**
- 定义自定义错误类继承自`Error`
- 使用`try-catch`进行异步错误处理
- 对于可能失败的操作，返回`Result<T, E>`类型或使用Promise

**tsconfig.json推荐配置:**

```json
{
  "compilerOptions": {
    "target": "ES2022",
    "module": "ESNext",
    "lib": ["ES2022", "DOM"],
    "strict": true,
    "esModuleInterop": true,
    "skipLibCheck": true,
    "forceConsistentCasingInFileNames": true,
    "moduleResolution": "bundler",
    "resolveJsonModule": true,
    "declaration": true,
    "declarationMap": true,
    "sourceMap": true,
    "noUnusedLocals": true,
    "noUnusedParameters": true,
    "noImplicitReturns": true,
    "noFallthroughCasesInSwitch": true,
    "noUncheckedIndexedAccess": true,
    "noPropertyAccessFromIndexSignature": true,
    "allowUnusedLabels": false,
    "allowUnreachableCode": false,
    "exactOptionalPropertyTypes": true
  }
}
```

**代码示例:**

```typescript
/**
 * 数据处理工具类示例
 *
 * @file src/utils/data-processor.ts
 * @author WaterRun
 * @date 2025-12-25
 */

/**
 * 数据项接口
 *
 * @interface
 */
interface IDataItem {
  readonly id: string;
  name: string;
  value: number;
}

/**
 * 处理结果类型
 *
 * @typedef
 */
type ProcessResult = {
  success: boolean;
  data?: IDataItem[];
  error?: string;
};

/**
 * 数据处理器类
 *
 * @template T - 数据类型
 */
class DataProcessor<T extends IDataItem> {
  readonly #data: T[];

  /**
   * 构造函数
   *
   * @param data - 初始数据
   */
  constructor(data: T[]) {
    this.#data = data;
  }

  /**
   * 过滤数据
   *
   * @param predicate - 过滤条件函数
   * @returns 过滤后的数据
   */
  public filter(predicate: (item: T) => boolean): T[] {
    return this.#data.filter(predicate);
  }

  /**
   * 静态工厂方法
   *
   * @param data - 原始数据
   * @returns 处理器实例
   */
  public static create<T extends IDataItem>(data: T[]): DataProcessor<T> {
    return new DataProcessor(data);
  }
}

export { DataProcessor, type IDataItem, type ProcessResult };
```
