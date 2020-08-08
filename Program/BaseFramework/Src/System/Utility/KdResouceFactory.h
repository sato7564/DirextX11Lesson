#pragma once

// ==================================
// リソース管理クラス
// ・デザインパターンのFlyweightパターンを採用
// モデルを1度しか読み込まないようにする
// ※ 射出するたびモデルを読み込み停止するのを防ぐため 
// ==================================

class KdResouceFactory
{
public:
	// モデルデータ取得
	std::shared_ptr<KdModel> GetModel(const std::string& filename);

	//テクスチャーデータ取得
	std::shared_ptr<KdTexture>GetTexture(const std::string& filename);

	// JSON取得
	json11::Json GetJSON(const std::string& filename);


	// 管理を破棄する
	void Clear()
	{
		m_modelMap.clear();
		m_texMap.clear();
		m_jsonMap.clear();
	}

private:

	// JSON読み込み
	json11::Json LoadJSON(const std::string& filename);

	// モデルデータ管理マップ	連想配列
	std::unordered_map<std::string, std::shared_ptr<KdModel>> m_modelMap;

	//テクスチャー管理マップ
	std::unordered_map < std::string, std::shared_ptr < KdTexture >>m_texMap;

	// JSON管理マップ
	std::unordered_map<std::string, json11::Json>  m_jsonMap;

	// ==================================
	// シングルトン
	// ==================================
private:
	KdResouceFactory() {}
public:
	static KdResouceFactory& GetInstance()
	{
		static KdResouceFactory instance;
		return instance;
	}
};

#define KdResFac KdResouceFactory::GetInstance()