#include <d3dx10math.h>
#include <stdio.h>

D3DXVECTOR3 cross(D3DXVECTOR3 v1, D3DXVECTOR3 v2)
{
	return D3DXVECTOR3(v1.y*v2.z - v2.y*v1.z,
					   v2.x*v1.z - v1.x*v2.z, 
					   v1.x*v2.y - v1.y*v2.x);
}

float dot(D3DXVECTOR3 v1, D3DXVECTOR3 v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

D3DXVECTOR3 normalize(D3DXVECTOR3 v)
{
	return v / sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

D3DXVECTOR4 normalize(D3DXVECTOR4 v)
{
	return v / sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

 void output(const char* szFormat, ...)
{
    char szBuff[4096];
    va_list arg;
    va_start(arg, szFormat);
    _vsnprintf_s(szBuff, sizeof(szBuff), szFormat, arg);
    va_end(arg);

    OutputDebugString(szBuff);
}

float random(float start, float end)
{
	 return start + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX / (end - start)));
}