#pragma once

class KdModel
{
public:

	KdModel();	// コンストラクタ
	~KdModel();	// デコンストラクタ

	bool Lood(const std::string& filename);

	// アクセサ
	//const std::shared_ptr<KdMesh> GetMesh() const { return m_spMesh;}
	const std::shared_ptr<KdMesh> GetMesh(UINT index) const
	{
		return index < m_originalNodes.size() ? m_originalNodes[index].m_spMesh : nullptr;
	}
	
	const std::vector<KdMaterial>&GetMaterials()  const { return m_materials;}

	//ノード：モデルウィ形成するメッシュを扱うための最小単位
	struct Node
	{
		std::string m_name;				//ノード名
		KdMatrix m_localTransform;		//変換行列
		std::shared_ptr<KdMesh> m_spMesh;//メッシュ情報
	};


	//ノード配列の取得
	const std::vector<Node>& GetOriginalNodes() const { return m_originalNodes; };

private:

	void Release();
	std::vector<Node> m_originalNodes;

	//std::shared_ptr<KdMesh> m_spMesh = nullptr;

	// マテリアル配列
	std::vector<KdMaterial> m_materials;
};