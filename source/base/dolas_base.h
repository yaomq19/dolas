#ifndef DOLAS_BASE_H
#define DOLAS_BASE_H

// 基础头文件内容将在这里
#define DOLAS_NEW(type, ...) new type(__VA_ARGS__)
#define DOLAS_DELETE(ptr) if (ptr) { delete ptr; ptr = nullptr; }

#endif // DOLAS_BASE_H

