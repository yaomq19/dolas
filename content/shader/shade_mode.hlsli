#ifndef SHADE_MODE_HLSLI
#define SHADE_MODE_HLSLI

#define SHADE_MODE_OPAQUE 0
#define SHADE_MODE_BLINN_PHONG 1
#define SHADE_MODE_COUNT 2

int DecodeShadeMode(float shade_mode_float)
{
    // 将 [0, 1] 的 float 映射到 [0, SHADE_MODE_COUNT - 1] 的整数索引
    // 逻辑：把区间 [0,1] 均分成 SHADE_MODE_COUNT 份，每一份对应一个 shade mode
    float t = saturate(shade_mode_float);

    // 乘以 SHADE_MODE_COUNT 得到区间索引，再向下取整
    int index = (int)floor(t * SHADE_MODE_COUNT);

    // 当 t == 1.0 时，上式结果会等于 SHADE_MODE_COUNT，需要手动收回到最大合法值
    index = min(index, SHADE_MODE_COUNT - 1);
    index = max(index, 0);

    return index;
}

float EncodeShadeMode(int shade_mode)
{
    // 保证传入的 shade_mode 在合法范围内
    int clamped = clamp(shade_mode, 0, SHADE_MODE_COUNT - 1);

    // 把整数索引映射回 [0, 1]，取每个区间中点，保证 Encode 之后再 Decode 仍然得到原始索引
    // 区间划分：[0,1] / SHADE_MODE_COUNT => 每段宽度为 1 / SHADE_MODE_COUNT
    // 第 i 段的中点为 (i + 0.5) / SHADE_MODE_COUNT
    return (clamped + 0.5f) / SHADE_MODE_COUNT;
}
#endif