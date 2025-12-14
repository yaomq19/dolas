{
    "vertex_buffers" :
    {
        pos , float or uint, element count per vertex: ""
        normal: ""
        uv: ""
        uv2:
        color:
    }

    "index_buffer" :
    {
        {0, 1,1,2,2,3,3,1,3,}
    }

    "topology" : ""

    "use_index_buffer" : bool(如果使用 index，再用 topology 区分，如果不使用index_buffer，处理方式也会不同)

    "material"
}

网格本身没有instance的意识
网格不会与 IA 这种 D3D11 API 的底层概念有关系