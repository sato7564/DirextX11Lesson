#include "CameraComponent.h"

#include "../Component/CameraComponent.h"



// コンストラクター
CameraComponent::CameraComponent(GameObject& owner) : m_owner(owner)
{
	m_mProj.CreateProjectionPerspectiveFov(60 * KdToRadians,
		D3D.GetZBuffer()->GetAspectRatio(), 0.01f, 5000.0f);
}

// デコンストラクター
CameraComponent::~CameraComponent()
{
}

// カメラ行列・ビュー行列設定（ 行列 m と 行列 Offset が合成され、最終てきなカメラ行列になる）
void CameraComponent::SetCameraMatrix(const KdMatrix& m)
{
	// カメラ行列をセット
	m_mCam = m_mOffset * m;

	// カメラ行列からビュー行列を算出
	m_mView = m_mCam;
	m_mView.Inverse();
}

// カメラ情報（ビュー・射影行列など）をシェーダーにセット
void CameraComponent::SetToShader()
{
	// 追従カメラ座標をシェーダーにセット
	SHADER.m_cb7_Camera.Work().CamPos = m_mCam.GetTranslation();

	// 追従カメラビュー行列をシェーダーにセット
	SHADER.m_cb7_Camera.Work().mV = m_mView;

	// 追従カメラの射影行列をシェーダーにセット
	SHADER.m_cb7_Camera.Work().mP = m_mProj;

	// カメラ情報（ ビュー行列、射影行列 ）を、シェーダーの定数バッファへセット
	SHADER.m_cb7_Camera.Write();
}