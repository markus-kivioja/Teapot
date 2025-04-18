#ifndef _FUNCS_H_
#define _FUNCS_H_

#include <d3dx10math.h>

extern void output(const char* szFormat, ...);

extern float dot(D3DXVECTOR3 v1, D3DXVECTOR3 v2);
extern D3DXVECTOR3 normalize(D3DXVECTOR3 v);
extern D3DXVECTOR4 normalize(D3DXVECTOR4 v);
extern D3DXVECTOR3 cross(D3DXVECTOR3 v1, D3DXVECTOR3 v2);
extern float random(float start, float end);

#endif