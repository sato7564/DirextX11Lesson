#include "KdCollision.h"

using namespace DirectX;

bool KdRayToMesh(
	const XMVECTOR& rRayPos,
	const XMVECTOR& rRayDir,
	float maxDistans,
	const KdMesh& rMesh,
	const KdMatrix& rMatrix,
	KdrayResult& rRsult)
{

	// モデルの逆行列でレイを変換
	XMMATRIX invMat = XMMatrixInverse(0, rMatrix);	// 高速化！！

	// レイの判定開始位置を逆変換
	XMVECTOR rayPos = XMVector3TransformCoord(rRayPos, invMat);	// 高速化！！

	// 発射方向は正規化されていないと正しく判定できないので正規化
	XMVECTOR rayDir = XMVector3TransformNormal(rRayDir,invMat);	// 高速化！！

	// 逆行列が入っていると
	// レイが当たった距離にも拡縮が反映されてしまうので
	// 判定用の最大距離にも拡縮を反映させておく
	float dirLength = XMVector3Length(rayDir).m128_f32[0];
	float rayCheckRange = maxDistans * dirLength;

	rayDir = XMVector3Normalize(rayDir); // 高速化！！

	//------------------------------------------------------------
	// ブロードフェイズ
	//	比較的軽量な AABB vs レイ判定で、
	//	あきらかにヒットしていない場合は、これ以降の処理を中止
	//------------------------------------------------------------
	{
		// AABB vs レイ
		float AABBdist = FLT_MAX;
		if (rMesh.GetBoundingBox().Intersects(rayPos, rayDir, AABBdist) == false) { return false; }

		// 最大距離以降なら範囲外なので中止
		if (AABBdist > rayCheckRange) { return false; }
	}

	//------------------------------------------------------------
	// ナローフェイズ
	//	レイ vs 全ての面
	//------------------------------------------------------------
	bool ret = false;				// 当たったかどうか
	float closestDist = FLT_MAX;		// 当たった面との距離

	// 面情報の取得
	const KdMeshFace* pFaces = &rMesh.GetFaces()[0];	// 面情報の先頭を取得
	UINT faceNum = rMesh.GetFaces().size();				// 面の総数を取得

	// 全ての面(三角形)と当たり判定
	for (UINT faceIdx = 0; faceIdx < faceNum; ++faceIdx)
	{
		// 三角形を構成する3つの頂点のIndex
		const UINT* idx = pFaces[faceIdx].Idx;

		// レイと三角形の当たり判定
		float triDist = FLT_MAX;
		bool bHit = TriangleTests::Intersects(
			rayPos,			// 発射場所
			rayDir,			// 発射方向

			// 判定する3角形の頂点情報
			rMesh.GetVertexPositions()[idx[0]],
			rMesh.GetVertexPositions()[idx[1]],
			rMesh.GetVertexPositions()[idx[2]],

			triDist		// 当たった場合の距離
		);

		// ヒットしていなかったらスキップ
		if (bHit == false) { continue; }

		// 最大距離以内か
		if (triDist <= rayCheckRange)
		{
			ret = true;	// 当たった

			// 当たり判定でとれる距離は拡縮に影響ないので、実際の長さを計算する
			triDist /= dirLength;

			// 近い方を優先して残す
			if (triDist < closestDist)
			{
				closestDist = triDist;	// 距離を更新
			}
		}
	}

	rRsult.m_distance = closestDist;
	rRsult.m_hit = ret;
	return ret;
}