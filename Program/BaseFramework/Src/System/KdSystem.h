#pragma once

//====================================================
//
// システム系のヘッダーファイル
//
//====================================================

// 便利機能
#include "Utility/KdUtility.h"

// 算術
#include "Math/kdMath.h"

// バッファ
#include "Direct3D/KdBuffer.h"
// Direct3D
#include "Direct3D/KdDirect3D.h"


// テクスチャ
#include "Direct3D/KdTexture.h"
// メッシュ
#include "Direct3D/KdMesh.h"
// モデル
#include"Direct3D/KdModel.h"

//板ポリゴン
#include"Direct3D/KdSquarePolygon.h"

//軌跡ポリゴン
#include "Direct3D/KdTrailPolygon.h"

// 判定系
#include "Math//KdCollision.h"

// シェーダ
#include "Shader/KdShaderManager.h"

// ウィンドウ
#include "Window/KdWindow.h"

// リソース管理
#include"Utility/KdResouceFactory.h"