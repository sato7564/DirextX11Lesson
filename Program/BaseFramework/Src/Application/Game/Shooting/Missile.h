#pragma once

#include"../GameObject.h"

class Missile : public GameObject
{
public:

	void Deserialize(const json11::Json& jsonObj) override;	// デシリアライズ(初期化)
	void Update();		// 更新

	inline void SetTarget(const std::shared_ptr<GameObject>& spTarget) { m_wpTarget = spTarget; };

	// 当たり判定を更新
	void UpdateCollision();

	// 撃ったキャラクターを登録する
	inline void SetOwner(const std::shared_ptr<GameObject>& spOwner) { m_wpOwner = spOwner; };

	void Explosion();

	void UpdateTrail();//軌跡の更新
	void DrawEffect()override;//透明物の描画

private:

	kdVec3 m_prevPos;	// 1フレーム前の座標

	std::weak_ptr<GameObject> m_wpOwner;	// 発射したオーナーオブジェクト

	float		m_speed			= 0.5f;
	int			m_lifeSpan		= 0;
	std::weak_ptr<GameObject> m_wpTarget;

	int	m_attacPow = 0;	// 攻撃力

	////煙　軌跡
	////1頂点の形式　
	//struct Vertex
	//{
	//	Math::Vector3 Pos;
	//	Math::Vector2 UV;
	//	Math::Vector4 Color{ 1,1,1,1 };
	//};

	////軌跡の位置などを記憶する行列配列
	//std::deque<Math::Matrix> m_pointList;

	////テクスチャ
	//std::shared_ptr<KdTexture> m_texture;

	//煙　軌跡
	KdTrailPolygon m_trailSmoke;
	float m_trailRotate=0.0f;
};