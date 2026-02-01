# 第二次死锁问题修复

## 问题描述

在编译shader文件时出现死锁错误：
```
[1/4] Compiling: deferred_shading_ps.hlsl...
Error: Exception occurred during compilation: resource deadlock would occur: resource deadlock would occur
```

## 问题分析

虽然之前修复了`Logger::Initialize()`中的死锁问题，但在编译过程中又发现了另一个死锁点：

### 调用流程

1. `ShaderCompiler::CompileShader()` 编译shader失败
2. 调用 `Logger::Instance().LogCompilationError(filename, error_message)`
3. `LogCompilationError()` 获取 `m_mutex` 锁
4. 在方法末尾调用 `Log(LogLevel::ERROR, ...)` 
5. `Log()` 方法也尝试获取同一个 `m_mutex` 锁
6. 同一线程重复获取非递归锁 → 死锁

### 问题代码

```cpp
void Logger::LogCompilationError(const std::string& filename, const std::string& error) {
    std::lock_guard<std::mutex> lock(m_mutex);  // 获取锁
    
    // ... 写入错误文件 ...
    
    // 问题：在持有锁的情况下调用Log方法
    Log(LogLevel::ERROR, "Compilation error details written to error log file: " + filename);
}  // 锁在这里才释放
```

## 解决方案

使用作用域控制，确保在调用`Log()`方法前释放锁：

```cpp
void Logger::LogCompilationError(const std::string& filename, const std::string& error) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // 获取锁
        
        // ... 写入错误文件 ...
        
    } // 锁在这里释放
    
    // 现在在锁外面安全地调用Log方法
    Log(LogLevel::ERROR, "Compilation error details written to error log file: " + filename);
}
```

## 修复要点

1. **作用域管理**: 使用`{}`块限制锁的作用域
2. **延迟调用**: 将可能导致死锁的方法调用移到锁外
3. **保持功能**: 确保错误文件写入和主日志记录功能不变

## 类似问题预防

为了避免将来出现类似的死锁问题，需要注意：

### 1. 识别潜在死锁点
- 任何在持有锁时调用其他可能获取同一锁的方法
- 特别注意日志系统中的嵌套调用

### 2. 设计原则
- **最小锁作用域**: 锁的持有时间应该尽可能短
- **避免嵌套调用**: 在持有锁时避免调用可能获取同一锁的方法
- **锁分离**: 考虑使用不同的锁保护不同的资源

### 3. 代码审查要点
- 检查所有`std::lock_guard`的作用域
- 验证锁保护的代码块中没有可能导致死锁的调用
- 确保锁的获取和释放配对正确

## 潜在的其他问题点

需要检查的其他可能死锁点：
- `LogCompilationSuccess()` - 目前是安全的，直接调用Log()
- 其他可能在锁内调用Log()的方法

## 验证修复

修复后重新编译运行，应该看到正常的编译过程：
```
[1/4] Compiling: deferred_shading_ps.hlsl... yes (XX.XXms)
[2/4] Compiling: deferred_shading_vs.hlsl... yes (XX.XXms)
[3/4] Compiling: solid_ps.hlsl... yes (XX.XXms)
[4/4] Compiling: solid_vs.hlsl... yes (XX.XXms)
```

## 相关文件

- `shader_compiler/src/logger.cpp` - 修复的LogCompilationError方法
- `shader_compiler/src/shader_compiler.cpp` - 调用LogCompilationError的地方
