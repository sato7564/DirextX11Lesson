#pragma once

#include "../Game/GameObject.h"

// ======================================
// カメラコンポーネントクラス
// ======================================

class CameraComponent
{
public:
	// コンストラクター　オーナーの設定と射影行列
	CameraComponent(GameObject& owner);
	~CameraComponent();

	// オフセット行列取得
	inline KdMatrix& OffsetMatrix() { return m_mOffset; }

	// カメラ行列取得
	inline const KdMatrix& GetCameratMatrix() { return m_mCam; }

	// ビュー行列取得
	inline const KdMatrix& GetViewtMatrix() { return m_mView; }

	// カメラ行列・ビュー行列設定（ 行列 m と 行列 Offset が合成され、最終てきなカメラ行列になる）
	void SetCameraMatrix(const KdMatrix& m);

	// カメラ情報（ビュー・射影行列など）をシェーダーにセット
	void SetToShader();

protected:

	// オフセット行列
	KdMatrix	m_mOffset;
	// カメラ行列
	KdMatrix	m_mCam;
	// ビュー行列
	KdMatrix	m_mView;

	// 射影行列
	KdMatrix	m_mProj;

	GameObject& m_owner;
};