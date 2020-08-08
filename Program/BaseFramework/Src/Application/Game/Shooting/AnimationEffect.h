//全部追加
#pragma once
#include"../GameObject.h"

class AnimationEffect :public GameObject
{
public:
	AnimationEffect();
	~AnimationEffect();

	//アニメーション情報の設定
	void SetAnimationInfo(const std::shared_ptr<KdTexture>& rTex, float size, int splitX, int splitY, float angle);

	
	//アニメーションの更新
	virtual void Update() override;
	
	//半透明物の描画
	virtual void DrawEffect() override;

private:
	//四角形のポリゴン
	KdSquarePolygon m_poly;

	//Z軸の回転
	float m_angleZ=0;

};