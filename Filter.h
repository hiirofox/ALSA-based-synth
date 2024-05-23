#ifndef _IIR_
#define _IIR_

#include <math.h>

class IIRFilter
{
private:
	float coeffs[6];
	float dx1, dx2, dy1, dy2;

public:
	void AutoZero()
	{
		if (isnanf(dx1))
			dx1 = 0;
		if (isnanf(dx2))
			dx2 = 0;
		if (isnanf(dy1))
			dy1 = 0;
		if (isnanf(dy2))
			dy2 = 0;
	}
	void UpdataLPF(float ctof, float reso)
	{
		ctof *= M_PI * 0.5;
		float ncos = cos(ctof);
		float alpha = sin(ctof) / reso * 0.5;
		coeffs[1] = 1.0 - ncos;
		coeffs[2] = coeffs[0] = coeffs[1] * 0.5;
		coeffs[3] = 1.0 / (1.0 + alpha);
		coeffs[4] = -ncos * 2.0;
		coeffs[5] = 1.0 - alpha;
		AutoZero();
	}
	void UpdataBPF(double ctof, double reso)
	{
		ctof *= M_PI * 0.5;
		float ncos = cos(ctof);
		float alpha = sin(ctof) / reso * 0.5;
		coeffs[0] = alpha;
		coeffs[1] = 0;
		coeffs[2] = -alpha;
		coeffs[3] = 1.0 / (1.0 + alpha);
		coeffs[4] = -ncos * 2.0;
		coeffs[5] = 1.0 - alpha;
		AutoZero();
	}
	void UpdataHPF(double ctof, double reso)
	{
		ctof *= M_PI * 0.5;
		float ncos = cos(ctof);
		float alpha = sin(ctof) / reso * 0.5;
		coeffs[0] = (1.0 + ncos) * 0.5;
		coeffs[1] = -(1.0 + ncos);
		coeffs[2] = (1.0 + ncos) * 0.5;
		coeffs[3] = 1.0 / (1.0 + alpha);
		coeffs[4] = -ncos * 2.0;
		coeffs[5] = 1.0 - alpha;
		AutoZero();
	}
	float proc(float vin)
	{
		float vout = (coeffs[0] * vin + coeffs[1] * dx1 + coeffs[2] * dx2 - coeffs[4] * dy1 - coeffs[5] * dy2) * coeffs[3];
		dy2 = dy1;
		dy1 = vout;
		dx2 = dx1;
		dx1 = vin;
		return vout;
	}
	float proc(float vin, const IIRFilter *param)
	{
		float vout = (param->coeffs[0] * vin + param->coeffs[1] * dx1 + param->coeffs[2] * dx2 - param->coeffs[4] * dy1 - param->coeffs[5] * dy2) * param->coeffs[3];
		dy2 = dy1;
		dy1 = vout;
		dx2 = dx1;
		dx1 = vin;
		return vout;
	}
};

class Filter {  //滤波器
private:
  float tmp1 = 0, tmp2 = 0, out1 = 0, out2 = 0;
public:
  void reset() {
    tmp1 = 0, tmp2 = 0, out1 = 0, out2 = 0;
  }
  void AutoZero() {
    if (isnanf(tmp1)) tmp1 = 0.0;
    if (isnanf(tmp2)) tmp2 = 0.0;
    if (isnanf(out1)) out1 = 0.0;
    if (isnanf(out2)) out2 = 0.0;
  }
  float LPF1(float vin, float ctof, float reso) {  //一阶低通带反馈
    float fb = reso + reso / (1.0 - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    return out1;
  }
  float LPF1_limit(float vin, float ctof, float reso, float limVol = 52000.0, float limK = 0.125) {  //一阶低通带限制反馈
    float fb = reso + reso / (1.0 - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    return out1;
  }
  float LPF1_Oversampling_limit(float vin, float ctof, float reso, float limVol = 52000.0, float limK = 0.5) {  //超采样一阶低通，可以和普通低通一样使用
    ctof *= 0.5;
    float fb = reso + reso / (1.0 - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    return out1;
  }
  float LPF2(float vin, float ctof, float reso) {  //二阶的，同上
    float fb = reso + reso / (1.0 - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    return out2;
  }
  float LPF2_limit(float vin, float ctof, float reso, float limVol = 52000.0, float limK = 0.125) {
    float fb = reso + reso / (1.0 - ctof);
    tmp1 += ctof * (vin - tmp1 + fb * (tmp1 - out1));
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    out2 = out2 > limVol ? ((out2 - limVol) * limK + limVol) : out2;
    out2 = out2 < -limVol ? ((out2 + limVol) * limK - limVol) : out2;
    tmp2 = tmp2 > limVol ? ((tmp2 - limVol) * limK + limVol) : tmp2;
    tmp2 = tmp2 < -limVol ? ((tmp2 + limVol) * limK - limVol) : tmp2;
    return out2;
  }
  float LPF2_Oversampling_ResoLimit_limit(float vin, float ctof, float reso, float limVol = 52000.0, float limK = 0.125) {
    ctof *= 0.5;
    float fb = reso + reso / (1.0 - ctof);
    tmp1 += ctof * (vin - tmp1);
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    tmp1 += ctof * (vin - tmp1);
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    out2 = out2 > limVol ? ((out2 - limVol) * limK + limVol) : out2;
    out2 = out2 < -limVol ? ((out2 + limVol) * limK - limVol) : out2;
    tmp2 = tmp2 > limVol ? ((tmp2 - limVol) * limK + limVol) : tmp2;
    tmp2 = tmp2 < -limVol ? ((tmp2 + limVol) * limK - limVol) : tmp2;
    return out2;
  }
  float LPF2_ResoLimit_limit(float vin, float ctof, float reso, float limVol = 52000.0, float limK = 0.125) {
    float fb = reso + reso / (1.0 - ctof);
    tmp1 += ctof * (vin - tmp1);
    out1 += ctof * (tmp1 - out1);
    tmp2 += ctof * (out1 - tmp2 + fb * (tmp2 - out2));
    out2 += ctof * (tmp2 - out2);
    out1 = out1 > limVol ? ((out1 - limVol) * limK + limVol) : out1;
    out1 = out1 < -limVol ? ((out1 + limVol) * limK - limVol) : out1;
    tmp1 = tmp1 > limVol ? ((tmp1 - limVol) * limK + limVol) : tmp1;
    tmp1 = tmp1 < -limVol ? ((tmp1 + limVol) * limK - limVol) : tmp1;
    out2 = out2 > limVol ? ((out2 - limVol) * limK + limVol) : out2;
    out2 = out2 < -limVol ? ((out2 + limVol) * limK - limVol) : out2;
    tmp2 = tmp2 > limVol ? ((tmp2 - limVol) * limK + limVol) : tmp2;
    tmp2 = tmp2 < -limVol ? ((tmp2 + limVol) * limK - limVol) : tmp2;
    return out2;
  }
  float HPF1(float vin, float ctof) {  //高通
    tmp1 += ctof * (vin - tmp1);
    return vin - tmp1;
  }
  float HPF1_limit(float vin, float ctof, float limVol = 52000.0, float limK = 0.125) {  //高通
    vin = vin > limVol ? ((vin - limVol) * limK + limVol) : vin;
    vin = vin < -limVol ? ((vin + limVol) * limK - limVol) : vin;
    tmp1 += ctof * (vin - tmp1);
    return vin - tmp1;
  }
};

#endif
