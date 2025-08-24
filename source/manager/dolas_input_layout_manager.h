#ifndef DOLAS_INPUT_LAYOUT_MANAGER_H
#define DOLAS_INPUT_LAYOUT_MANAGER_H

#include <d3d11.h>
#include <map>

#include "render/dolas_mesh.h"

namespace Dolas
{
    class InputLayoutManager
    {
    public:
        InputLayoutManager();
        ~InputLayoutManager();

        bool Initialize();
        bool Clear();

        ID3D11InputLayout* GetInputLayoutByType(const DolasInputLayoutType& type) const;

    private:
        bool CreateLayouts();

        std::map<DolasInputLayoutType, ID3D11InputLayout*> m_input_layouts;
    };
}

#endif // DOLAS_INPUT_LAYOUT_MANAGER_H


