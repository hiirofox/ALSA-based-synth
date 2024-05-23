#ifndef _VCO_
#define _VCO_

#ifndef SAMPLE_RATE
#define SAMPLE_RATE (48000)
#endif

struct StereoSignal
{
    float l, r;
};

#define MaxUnisonN (320) // 最多振荡器数
class VCO
{
private:
    double v1[MaxUnisonN];
    short v2[MaxUnisonN];
    short v3[MaxUnisonN];

public:
    VCO()
    {
        for (int i = 0; i < MaxUnisonN; ++i)
        {
            v2[i] = rand() * rand() * rand(); // 随机相位
            v3[i] = rand() * rand() * rand(); // 随机相位
        }
    }
    StereoSignal Saw(float SawFreq, int UniN, float delta)
    {
        SawFreq *= 65536.0 / SAMPLE_RATE;
        float tmp1 = 0, tmp2 = 0, tmp3 = 0;
        for (int i = 0; i < UniN; ++i)
            tmp1 += (v2[i] += (SawFreq + (v1[i] = tmp3))), tmp3 += delta; // 等差数列
        for (int i = 1; i < UniN; ++i)
            tmp2 += (v3[i] += (SawFreq - v1[i]));
        return (StereoSignal){
            tmp1 + tmp2, tmp1 - tmp2};
    }
};

#endif