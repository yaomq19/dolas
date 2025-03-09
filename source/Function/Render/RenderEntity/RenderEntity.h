#ifndef RENDERENTITY_H
#define RENDERENTITY_H

namespace Dolas
{
    class RenderEntity
    {
    public:
        RenderEntity();
        ~RenderEntity();
        bool Initialize();
        virtual void Render() const;
    private:

    };
}

#endif // RENDERENTITY_H