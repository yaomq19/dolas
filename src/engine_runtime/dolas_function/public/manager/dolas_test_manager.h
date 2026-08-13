#ifndef DOLAS_TEST_MANAGER_H
#define DOLAS_TEST_MANAGER_H

namespace Dolas
{
    class TestManager
    {
    public:
        TestManager();
        ~TestManager();

        bool Initialize();
        bool Clear();

        void TestTextureManager();
    };
}

#endif // DOLAS_TEST_MANAGER_H