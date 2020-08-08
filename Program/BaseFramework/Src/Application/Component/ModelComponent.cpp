#include"ModelComponent.h"

void ModelComponent::Draw()
{
	// 有効じゃない時はスキップ
	if (m_enable == false) { return; }

	// モデルが無い時はスキップ
	if (m_spModel == nullptr) { return; }


	//すべてのノードを一つ一つ描画
	for (UINT i = 0; i < m_spModel->GetOriginalNodes().size(); i++)
	{
		auto& rNode = m_spModel->GetOriginalNodes()[i];
		if (rNode.m_spMesh == nullptr) { continue; }
		SHADER.m_standardShader.SetWorldMatrix(rNode.m_localTransform * m_owner.GetMatrix());
		SHADER.m_standardShader.DrawMesh(rNode.m_spMesh.get(), m_spModel->GetMaterials());
	}
}