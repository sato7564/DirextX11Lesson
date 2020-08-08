﻿#pragma once

class CameraComponent;
class InputComponent;
class ModelComponent;

struct SphereInfo;
struct RayInfo;

// タグ変数
enum OBJECT_TAG {
	TAG_None		= 0x00000000,	// 属性なし：初期設定用
	TAG_Character	= 0x00000001,	// キャラクター設定
	TAG_Player		= 0x00000002,	// プレイヤー属性
	TAG_StageObject	= 0x00000004,	// 背景オブジェクト属性
	TAG_AttackHit	= 0x00000010,	// 攻撃が当たる属性
};

// thisポインターをシェアードポインターに変更する事に許可する
class GameObject : public std::enable_shared_from_this<GameObject>
{
public:
	GameObject();
	virtual ~GameObject();

	virtual void Deserialize(const json11::Json& jsonObj);	// 初期化：オブジェクト生成用外部データの解放
	virtual void Update();		// 更新
	virtual void Draw();		// 描画

	//半透明物の描画
	virtual void DrawEffect() {}

	inline const KdMatrix& GetMatrix()const { return m_mWorld; }
	inline void SetMatrix(const KdMatrix& rMat) { m_mWorld = rMat; }
	inline bool IsAlive() const { return m_alive; }
	inline void Destroy() { m_alive = false; }

	inline void SetTag(UINT tag) { m_tag = tag; }
	inline UINT GetTag() const { return m_tag; }

	// カメラコンポーネント取得
	std::shared_ptr<CameraComponent>	GetCameraComponent() { return m_spCameraComponent; }
	// 入力コンポーネント取得
	std::shared_ptr<InputComponent>		GetInputComponent() { return m_spInputComponent; }
	// モデルコンポーネント取得
	std::shared_ptr<ModelComponent>		GetModelComponent() { return m_spModelComponent; }

	// 球による当たり判定
	bool HitCheckBySphere(const SphereInfo& rInfo);
	
	// レイによる当たり判定
	bool HitCheckByRay(const RayInfo& rInfo, KdrayResult& rResult);

protected:

	//virtual void Release();		// 解放

	// カメラコンポーネント
	std::shared_ptr<CameraComponent> m_spCameraComponent = std::make_shared<CameraComponent>(*this);
	//インプットコンポーネント
	std::shared_ptr<InputComponent> m_spInputComponent = std::make_shared<InputComponent>(*this);
	//インプットコンポーネント
	std::shared_ptr<ModelComponent> m_spModelComponent = std::make_shared<ModelComponent>(*this);

	KdMatrix	m_mWorld;				// ゲーム内の絶対領域, 飛行機のワールド座標
	bool		m_alive = true;
	UINT		m_tag = OBJECT_TAG::TAG_None;

	std::string m_name = "GameObject";	// 名前
	
	float		m_colRadius = 2.0f;		// このキャラクターの半径

};
// クラス名からGameObjectを作成
std::shared_ptr<GameObject> CreateGameObject(const std::string& name);

// 球判定に使うデータ
struct SphereInfo
{
	kdVec3 m_pos = {};
	float m_radius = 0.0f;

};

struct RayInfo
{
	kdVec3	m_pos;	// レイ(光線)の発射場所
	kdVec3	m_dir;	// レイの発射方向
	float	m_maxRange = 0.0f;	// レイが届く最大距離
};