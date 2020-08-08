#include"Scene.h"
#include"GameObject.h"
#include"../Component/CameraComponent.h"

#include"EditorCamera.h"
#include"Shooting/Aircraft.h" 
#include"Shooting/Missile.h" 
#include"Shooting/AnimationEffect.h"


// コンストラクタ
Scene::Scene()
{

}

// デコンストラクタ
Scene::~Scene()
{

}

// 初期化
void Scene::Init()
{
	// jsonファイルを開く
	std::ifstream ifs("Data/test.json");
	if (ifs.fail()) { assert(0 && "jsonファイルのパスが間違っています"); }

	// 文字列として全読み込み
	std::string strJson((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

	// 文字列のJSONを解析(パース)する
	std::string err;
	json11::Json jsonObj = json11::Json::parse(strJson, err);
	if (err.size() > 0)
	{
		assert(0 && "読み込んだファイルのJson変換に失敗");
	}

	// 値アクセス
	{
		OutputDebugStringA(jsonObj["Name"].string_value().append("\n").c_str());
		// auto name = jsonObj["Name"].string_value(); 本来値を取得するだけならこれだけで良い
		OutputDebugStringA(std::to_string(jsonObj["Hp"].int_value()).append("\n").c_str());		// to_string … 渡された値を文字列に変換
	}

	// 配列アクセス
	{
		// 配列全アクセス
		auto& pos = jsonObj["Position"].array_items();
		for (auto&& p : pos)
		{
			OutputDebugStringA(std::to_string(p.number_value()).append("\n").c_str());
		}
		// 配列添字アクセス
		OutputDebugStringA(std::to_string(pos[0].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(pos[1].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(pos[2].number_value()).append("\n").c_str());
	}

	// Object取得
	{
		auto& object = jsonObj["monster"].object_items();
		OutputDebugStringA(object["name"].string_value().append("\n").c_str());
		OutputDebugStringA(std::to_string(object["hp"].int_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][0].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][1].number_value()).append("\n").c_str());
		OutputDebugStringA(std::to_string(object["pos"][2].number_value()).append("\n").c_str());
	}
	
	// Object配列取得
	{
		auto& objects = jsonObj["techniques"].array_items();
		for (auto&& object : objects)
		{
			// 共通のパラメータはチェック無しでアクセス
			OutputDebugStringA(object["name"].string_value().append("\n").c_str());
			OutputDebugStringA(std::to_string(object["atk"].int_value()).append("\n").c_str());
			OutputDebugStringA(std::to_string(object["hitrate"].number_value()).append("\n").c_str());
		
			// 固有のパラメータはチェックしてからアクセス
			if (object["effect"].is_string())
			{
				OutputDebugStringA(object["effect"].string_value().append("\n").c_str());

			}
		}
	}

	//OutputDebugStringA("初期化関数\n");

	m_spCamera = std::make_shared<EditorCamera>();

	m_spSky = KdResouceFactory::GetInstance().GetModel("Data/Sky/Sky.gltf");


	Deserialize();


	//m_poly.Init(10.0f, 10.0f, { 1,1,1,1 });
	//m_poly.SetTexture(KdResFac.GetTexture("Data/Texture/Explosion00.png"));
	//
	//m_poly.SetAnimationInfo(5, 5);//アニメーション情報の設定
	

}

void Scene::Deserialize()
{
	LoadScene("Data/Scene/ShootingGame.json");
}

// 解放
void Scene::Release()
{
	if (m_spCamera)
	{
		m_spCamera = nullptr;
	}

	m_spObjects.clear();
}

// 更新
void Scene::Update()
{
	//OutputDebugStringA("更新関数\n");

	if (m_editorCameraEnable)
	{
		m_spCamera->Update();
	}

	for (auto pObject : m_spObjects)
	{
		pObject->Update();
	}

	for (auto spObjectItr = m_spObjects.begin(); spObjectItr != m_spObjects.end();)
	{
		// 寿命が尽きていたらリストから除外
		if ((*spObjectItr)->IsAlive() == false)
		{
			spObjectItr = m_spObjects.erase(spObjectItr);
		}
		else
		{
			++spObjectItr;
		}
	}

	//m_poly.Animation(0.5f, true);
}

// 描画
void Scene::Draw()
{
	//OutputDebugStringA("描画関数\n");

	// エディターカメラをシェーダーにセット
	//if (m_spCamera)
	//m_spCamera->SetToShader();

	// カメラ情報(ビュー号列、射影行列)を、各シェーダの定数バックバッファにセット
	// SHADER.m_cb7_Camera.Write();

	if (m_editorCameraEnable)
	{
		m_spCamera->SetToShader();
	}
	else
	{
		// 消されないようにする
		std::shared_ptr<CameraComponent> spCamera = m_wpTargetCamera.lock();
		if (spCamera)
		{
			spCamera->SetToShader();
		}
	}

	// ライトの情報を描画デバイスにセット
	SHADER.m_cb8_Light.Write();

	// エフェクトシェーダを描画デバイスにセット
	SHADER.m_effectShader.SetToDevice();

	Math::Matrix skyScale = DirectX::XMMatrixScaling(100.0f, 100.0f, 100.0f);

	SHADER.m_effectShader.SetWorldMatrix(skyScale);

	// モデルの描画(メッシュ情報とマテリアルの情報を渡す)
	if (m_spSky)
	{
		SHADER.m_effectShader.DrawMesh(m_spSky->GetMesh(0).get(), m_spSky->GetMaterials());
	}

	// 不透明物描画	
	SHADER.m_standardShader.SetToDevice();

	for (auto pObject : m_spObjects)
	{
		pObject->Draw();
	}


	//半透明物描画
	SHADER.m_effectShader.SetToDevice();
	SHADER.m_effectShader.SetTexture(D3D.GetWhiteTex()->GetSRView());

	//ポリゴンの描画
		//Z情報は使うが書き込みOFF
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEnable_ZWriteDisable, 0);
		//裏側書き込み
		D3D.GetDevContext()->RSSetState(SHADER.m_rs_CullNone);
		//消去
		//KdMatrix tempMat;
		//tempMat.SetTranslation({ 0.0f,5.0f,0.0f });

		//SHADER.m_effectShader.SetWorldMatrix(tempMat);
		//SHADER.m_effectShader.WriteToCB();
		//m_poly.Draw(0);


		////斜め上少し後ろ
		//tempMat.SetTranslation({ 5.0f,10.0f,1.0f });
		//SHADER.m_effectShader.SetWorldMatrix(tempMat);
		//SHADER.m_effectShader.WriteToCB();
		//m_poly.Draw(0);

		//追加
		for (auto spObj : m_spObjects)
		{
			spObj->DrawEffect();
		}
		

		//カリングあり
		D3D.GetDevContext()->RSSetState(SHADER.m_rs_CullBack);
		//Z書き込みON
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEisable_ZWriteEnable, 0);
	

	// デバックラインの描画
	SHADER.m_effectShader.SetToDevice();										// 不透明描画 . 描画準備
	SHADER.m_effectShader.SetTexture(D3D.GetWhiteTex()->GetSRView());
	{
		AddDebugLine(Math::Vector3(), Math::Vector3(0.0f, 10.0f, 0.0f));

		AddDebugSphereLine(Math::Vector3(5.0f, 5.0f, 0.0f), 2.0f);

		AddDebugCoordinateAxisLine(Math::Vector3(5.0f, 5.0f, 5.0f), 3.0f);

		// Zバッフ使用OFF・書き込みOFF
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZDisable_ZWriteDisable, 0);

		// 点があれば
		if (m_debugLines.size() >= 1)
		{											// 単位行列
			SHADER.m_effectShader.SetWorldMatrix(Math::Matrix());					// 拡大率以外の行列
												// 描画する点の配列 
			SHADER.m_effectShader.DrawVertices(m_debugLines, D3D_PRIMITIVE_TOPOLOGY_LINELIST);		// 今回は線で描画する

			m_debugLines.clear();		// m_debugLinesは重いので使わないときは消しておく
		}

		// Zバッフ使用ON・書き込みON
		D3D.GetDevContext()->OMSetDepthStencilState(SHADER.m_ds_ZEisable_ZWriteEnable, 0);

		//std::vector<KdEffectShader::Vertex> debugLines;

		//// ラインの開始頂点
		//KdEffectShader::Vertex ver1;
		//ver1.Color = { 1.0f,1.0f,1.0f,1.0f };
		//ver1.UV = { 0.0f,0.0f };
		//ver1.Pos = { 0.0f,0.0f,0.0f };
		//
		//// ラインの終点頂点
		//KdEffectShader::Vertex ver2;
		//ver2.Color = { 1.0f,1.0f,1.0f,1.0f };
		//ver2.UV = { 0.0f,0.0f };
		//ver2.Pos = { 0.0f,10.0f,0.0f };

		//debugLines.push_back(ver1);
		//debugLines.push_back(ver2);

		//SHADER.m_effectShader.SetWorldMatrix(Math::Matrix());

		//SHADER.m_effectShader.DrawVertices(debugLines, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	}
}

void Scene::LoadScene(const std::string& sceneFilename)
{
	// GameObjectリストを空にする
	m_spObjects.clear();

	// JSON 読み込み
	json11::Json json = KdResFac.GetJSON(sceneFilename);
	if (json.is_null())
	{
		assert(0 && "[LoadJson]jsonファイル読み込み失敗");
		return;
	}

	// オブジェクトリスト取得
	auto& objJsonDataList = json["GameObjects"].array_items();

	// オブジェクト生成ループ
	for (auto&& objJsonData : objJsonDataList)
	{
		// オブジェクト作成
		auto newGameObj = CreateGameObject(objJsonData["ClassName"].string_value());

		// プレハブ指定ありの場合は、プレハブ側のものをベースにこのJSONをマージする
		KdMergePrefab(objJsonData);

		// オブジェクトのデシリアライズ
		newGameObj->Deserialize(objJsonData);

		// リストへ追加
		AddObject(newGameObj);
	}
}

void Scene::AddObject(std::shared_ptr<GameObject> spObject)
{
	if (spObject == nullptr) { return; }
	m_spObjects.push_back(spObject);
}

void Scene::ImGuiUpdate()
{
	if (ImGui::Begin("Scene"))
	{
		// ImGui::Text(u8"今日はいい天気だから\n飛行機の座標でも表示しようかな。\n");	// u8 … UTF-8 "" … const cher

		ImGui::Checkbox("EditorCamera", &m_editorCameraEnable);

		//if (m_pAircraft)
		//{
		//	m_pAircraft->ImGuiUpdate();
		//}
	}

	ImGui::End();	// ウィンドウバー終了
}

//デバックライン描画			始点					　終点					　色
void Scene::AddDebugLine(const Math::Vector3& p1, const Math::Vector3& p2, const Math::Color& color)
{
	// ラインの開始頂点
	KdEffectShader::Vertex ver;
	ver.Color = color;
	ver.UV = { 0.0f,0.0f };
	ver.Pos = p1;
	m_debugLines.push_back(ver);

	// ラインの終点頂点
	ver.Pos = p2;
	m_debugLines.push_back(ver);
}

void Scene::AddDebugSphereLine(const Math::Vector3& pos, float radius, const Math::Color& color)
{
	KdEffectShader::Vertex ver;
	ver.Color = color;
	ver.UV = { 0.0f,0.0f };

	static constexpr int kDetail = 16;
	for (UINT i = 0; i < kDetail + 1; ++i)
	{
		// XZ平面
		ver.Pos = pos;
		ver.Pos.x += cos((float)i * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)i * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.x += cos((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		// XY平面
		ver.Pos = pos;
		ver.Pos.x += cos((float)i * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.y += sin((float)i * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.x += cos((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.y += sin((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		// YZ平面
		ver.Pos = pos;
		ver.Pos.y += cos((float)i * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)i * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);

		ver.Pos = pos;
		ver.Pos.y += cos((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		ver.Pos.z += sin((float)(i + 1) * (360 / kDetail) * KdToRadians) * radius;
		m_debugLines.push_back(ver);
	}
}

// XYZ軸を描画
void Scene::AddDebugCoordinateAxisLine(const Math::Vector3& pos, float scale)
{
	// ラインの開始頂点
	KdEffectShader::Vertex ver;

	// 無くても可
	ver.Color = { 1.0f,1.0f,1.0f,1.0f };
	ver.UV = { 0.0f,0.0f };

	// X軸・赤色
	ver.Color = { 1.0f,0.0f,0.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.x += 1.0f * scale;
	m_debugLines.push_back(ver);

	// Y軸・緑色
	ver.Color = { 0.0f,1.0f,0.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.y += 1.0f * scale;
	m_debugLines.push_back(ver);

	// Z軸・青色
	ver.Color = { 0.0f,0.0f,1.0f,1.0f };
	ver.Pos = pos;
	m_debugLines.push_back(ver);

	ver.Pos.z += 1.0f * scale;
	m_debugLines.push_back(ver);
}