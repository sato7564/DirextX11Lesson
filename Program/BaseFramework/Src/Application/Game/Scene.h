#pragma once

// 前方宣言
class EditorCamera;
class GameObject;
class CameraComponent;

class Scene
{
public:
	
	// シングルトンパターン		1つしかインスタンス化インスタンス化しない
	static Scene& GetInstance()
	{
		static Scene instance;		// 中身はstaticのみ
		return instance;
	}

	~Scene();	// デコンストラクタ
	
	void Init();	// 初期化
	void Deserialize();
	void Release();	// 解放
	void Update();	// 更新
	void Draw();	// 描画

	void LoadScene(const std::string& sceneFilename);

	void AddObject(std::shared_ptr<GameObject> spObject);	// 渡されたミサイルを追加

	// GameObjectのリストを返す
	const std::list<std::shared_ptr<GameObject>> GetObjects()const { return m_spObjects; }

	inline void SetTargetCamera(std::shared_ptr<CameraComponent> spCamera) { m_wpTargetCamera = spCamera; }

	void ImGuiUpdate();	// ImGuiの更新

	// デバックライン描画
	void AddDebugLine(const Math::Vector3& p1, const Math::Vector3& p2, const Math::Color& color = { 1,1,1,1 });

	// デバックスフィア描画
	void AddDebugSphereLine(const Math::Vector3& pos,float radius, const Math::Color& color = {1,1,1,1});

	void AddDebugCoordinateAxisLine(const Math::Vector3& pos, float scale = 1.0f);

private:

	Scene();	// コンストラクタ		シングルトンパターン	NEWなど出来なくなる
	
	std::shared_ptr<KdModel>		m_spSky		= nullptr;						// スカイスフィア
	std::shared_ptr<EditorCamera>	m_spCamera	= nullptr;
	
	bool	m_editorCameraEnable = true;

	std::list<std::shared_ptr<GameObject>> m_spObjects;		// 配列の順番がバラバラ <シェアードポインタ>

	// ターゲットのカメラ (弱参照 → 参照先を見るだけのポインタ)
	std::weak_ptr<CameraComponent> m_wpTargetCamera;

	// デバックライン描画用頂点配列
	std::vector<KdEffectShader::Vertex> m_debugLines;


	//消去
	////iいたポリ関数の初期化
	//KdSquarePolygon m_poly;

};