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
        void TickRenderThread(Float delta_time);
        void TickLogicThread(Float delta_time);

        void TickPreRender(Float delta_time);
        void TickRender(Float delta_time);
        void TickPostRender(Float delta_time);

        void TickPreLogic(Float delta_time);
        void TickLogic(Float delta_time);
        void TickPostLogic(Float delta_time);
    };
}// namespace Dolas

#endif // DOLAS_TICK_MANAGER_H