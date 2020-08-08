//全部追加

#include "KdSquarePolygon.h"

void KdSquarePolygon::Init(float w, float h, const Math::Vector4& _color)
{
	m_vertex[0].pos = { -w / 2,-h / 2,0 };
	m_vertex[1].pos = { -w / 2,h / 2,0 };
	m_vertex[2].pos = { w / 2,-h / 2,0 };
	m_vertex[3].pos = { w / 2,h / 2,0 };

	m_vertex[0].color = _color;
	m_vertex[1].color = _color;
	m_vertex[2].color = _color;
	m_vertex[3].color = _color;

	m_vertex[0].UV = { 0,1 };
	m_vertex[1].UV = { 0,0 };
	m_vertex[2].UV = { 1,1 };
	m_vertex[3].UV = { 1,0 };
}

void KdSquarePolygon::Draw(int setTextureNo)
{
	//テクスチャが設定されていたらDirectX側に教える
	if (m_texture) {
		D3D.GetDevContext()->PSSetShaderResources(setTextureNo, 1, m_texture->GetSRViewAddress());
	}
	else {
		//無ければ白いテクスチャを張り付ける
		D3D.GetDevContext()->PSSetShaderResources(setTextureNo, 1, D3D.GetWhiteTex()->GetSRViewAddress());
	}
	D3D.DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, &m_vertex, sizeof(Vertex));
}

void KdSquarePolygon::SetAnimationPos(float no)
{
	int x = (int)no % m_animSplitX;
	int y = (int)no / m_animSplitX;

	float w = 1.0f / m_animSplitX;
	float h = 1.0f / m_animSplitY;

	m_vertex[0].UV = { x * w,(y + 1) * h };
	m_vertex[1].UV = { x * w,y*h };
	m_vertex[2].UV = { (x+1) * w,(y + 1) * h };
	m_vertex[3].UV = { (x + 1) * w,y * h };

	m_animPos = no;
}

void KdSquarePolygon::Animation(float speed, bool loop)
{
	m_animPos += speed;
	if (m_animPos >= (m_animSplitX * m_animSplitY))
	{
		if (loop)
		{
			m_animPos = 0;
		}
		else
		{
			//最後のマスで止まる
			m_animPos = (float)(m_animSplitX * m_animSplitY) - 1;
		}
	}
	SetAnimationPos(m_animPos);

}

bool KdSquarePolygon::IsAnimationEnd()
{
	if (m_animPos >= (m_animSplitX * m_animSplitY) - 1)
	{
		return true;
	}

	return false;
}



