#ifndef DOLAS_TICK_MANAGER_H
#define DOLAS_TICK_MANAGER_H

#include "base/dolas_base.h"
namespace Dolas
{
    class TickManager
    {
    public:
        TickManager();
        ~TickManager();

        bool Initialize();
        bool Clear();

        void Tick(Float delta_time);
    protected:
        void TickRender(Float delta_time);
        void TickLogic(Float delta_time);

        unsigned long long m_frame_count = 0;
    };
}// namespace Dolas

#endif // DOLAS_TICK_MANAGER_H