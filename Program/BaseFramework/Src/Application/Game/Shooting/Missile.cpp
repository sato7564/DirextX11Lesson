#include"Missile.h"
#include"Application/main.h"
#include"../../Component/ModelComponent.h"
#include "../Scene.h"
#include "Aircraft.h"
#include "EffectObject.h"
#include"AnimationEffect.h"

void Missile::Deserialize(const json11::Json& jsonObj)
{
	m_lifeSpan = APP.m_maxFps * 10;	// 秒数
	
	if (jsonObj.is_null()) { return; }

	GameObject::Deserialize(jsonObj);

	if (jsonObj["Speed"].is_null() == false)
	{
		m_speed = jsonObj["Speed"].number_value();
	}

	if (jsonObj["Damage"].is_null() == false)
	{
		m_attacPow = jsonObj["Damage"].number_value();
	}

	//煙テクスチャ
	m_trailSmoke.SetTexture(KdResFac.GetTexture("Data/Texture/smokeline2.png"));

	//if (m_spModelComponent)
	//{
	//	m_spModelComponent->SetModedl(KdResFac.GetModel("Data/Missile/Missile.gltf"));
	//}
}

void Missile::Update()
{
	if (m_alive == false) { return; }

	if(--m_lifeSpan <= 0)
	{
		Destroy();
	}

	// ターゲットを shared_ptr 化
	auto target = m_wpTarget.lock();

	if (target)
	{
		// 自分からタ―ゲットへのベクトル
		kdVec3 vTarget = target->GetMatrix().GetTranslation() - m_mWorld.GetTranslation();
		//kdVec3 vTarget = kdVec3( 0.0f - m_mWorld._41, 0.0f - m_mWorld._42, 0.0f - m_mWorld._43 );

		// 単位ベクトル化：自身からターゲットへ向かう長さ１のベクトル
		vTarget.Normalize();

		// 自身のZ軸方向(前方向)
		kdVec3 vZ = m_mWorld.GetAxisZ();

		//拡大率が入っていると計算がおかしくなるため単位ベクトル化
		vZ.Normalize();

		// ※※※※※ 回転軸作成 (この軸で回転する) ※※※※※
		kdVec3 vRotAxis = kdVec3::Cross(vZ, vTarget);

		// 0ベクトルなら回転しない
		if (vRotAxis.LengthSquared() != 0)
		{
			// 自分のZ軸方向ベクトルと自分のターゲットへ向かうベクトルの内積
			float d = kdVec3::Dot(vZ, vTarget);

			// 誤差で -1～1 以外になる可能性大なので、クランプする。(std::clampでも可)
			if (d > 1.0f)d = 1.0f;
			else if (d < -1.0f)d = -1.0f;

			// 自分の前方向ベクトルと自身からTargetへ向かうベクトル間の角度(radian)
			float radian = acos(d);

			// 角度制限 １フレームにつき最大で１度以上回転しない
			if (radian > 1.0f * KdToRadians)
			{
				radian = 1.0f * KdToRadians;
			}

			// ※※※※※ radian (ここまでで回転角度が求まった) ※※※※※

			KdMatrix mRot;
			mRot.CreateRotationAxis(vRotAxis, radian);
			auto pos = m_mWorld.GetTranslation();
			m_mWorld.SetTranslation({ 0,0,0 });
			m_mWorld *= mRot;
			m_mWorld.SetTranslation(pos);

			//m_mWorld = mRot * m_mWorld;
		}
	}

	kdVec3 move = m_mWorld.GetAxisZ();		// 前方向に向く
	move.Normalize();						// 向きの長さを1に

	move *= m_speed;

	m_mWorld.Move(move);

	// 寿命処理
	m_prevPos = m_mWorld.GetTranslation();

	// 当たり判定
	UpdateCollision();

	//軌跡の更新
	UpdateTrail();
}

void Missile::UpdateCollision()
{
	// [ オンライン授業ver ]
	kdVec3 moveVec = m_prevPos;
	float moveDistance = moveVec.Length();

	// 動いてないなら判定しない
	if (moveDistance == 0.0f) { return; }

	// 球情報を作成
	SphereInfo info;
	info.m_pos = m_mWorld.GetTranslation();
	info.m_radius = m_colRadius;

	// 発射した主人のShared_ptr取得
	auto spOwner = m_wpOwner.lock();

	// 全ての物体と判定を試みる
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身は無視
		if (obj.get() == this) { continue; }

		// 発射した主人も無視
		if (obj.get() == spOwner.get()) { continue;	}

		// キャラクターが対象
		if (!(obj->GetTag() & TAG_Character)) { continue; }

		if (obj->HitCheckBySphere(info))
		{
			// std::dynamic_pointer_cast = 基底クラス型をダウンキャストする時に使う。失敗するとnullptrが返る
			// 重たい、多発する場合は設計がミスっている
			// 改善したい人は先生まで相談(自分で1度は考えてみる)
			std::shared_ptr<Aircraft> aircraft = std::dynamic_pointer_cast<Aircraft>(obj);
			if (aircraft)
			{
				aircraft->OnNotify_Damage(m_attacPow);

				// 爆発をインスタンス化	
				std::shared_ptr<EffectObject> effectObj = std::make_shared<EffectObject>();

				if (effectObj)
				{
					// キャラクターのヒット位置
					Scene::GetInstance().AddObject(effectObj);

					// ヒット位置
					kdVec3 hitPos(obj->GetMatrix().GetTranslation());

					// 爆発エフェクトの行列を計算
					KdMatrix mMat;
					mMat.CreatTransslation(hitPos.x, hitPos.y, hitPos.z);
					effectObj->SetMatrix(mMat);
				}
			}
			Explosion();
			Destroy();
		}
	}

	// レイ情報の作成
	RayInfo rayInfo;
	rayInfo.m_pos = m_prevPos;
	rayInfo.m_dir = moveVec;
	rayInfo.m_maxRange = moveDistance;
	rayInfo.m_dir.Normalize();

	KdrayResult rayResult;

	// 全ての物体と判定を試みる
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身は無視
		if (obj.get() == this) { continue; }

		// 背景オブジェクトが対象
		if (!(obj->GetTag() & TAG_StageObject)) { continue; }

		// 判定実行
		if (obj->HitCheckByRay(rayInfo, rayResult))
		{
			Destroy();
		}
	}

	// [ 授業動画(長浜先生ver) ( 弾が非表示だった ) ]
	/*
	// 発射した主人のShared_ptr取得
	auto spOwner = m_wpOwner.lock();

	// 球情報の作成
	SphereInfo sInfo;
	sInfo.m_pos = m_mWorld.GetTranslation();
	sInfo.m_radius = 2.0f;

	// レイ情報の作成
	RayInfo rInfo;
	rInfo.m_pos = m_prevPos;
	rInfo.m_dir = m_mWorld.GetTranslation() - m_prevPos;
	rInfo.m_maxRange = rInfo.m_dir.Length();
	rInfo.m_dir.Normalize();

	// 全ての物体と判定を試みる
	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身は無視
		if (obj.get() == this) { continue; }

		// 発射した主人も無視
		if (obj.get() == spOwner.get()) { continue;	}
	
		bool hit = false;

		// TAG_Character とは 球判定を行う
		if (obj->GetTag() & TAG_Character)
		{
			hit = obj->HitCheckBySphere(sInfo);
		}

		// TAG_StageObject とはレイ判定を行う
		if (obj->GetTag() & TAG_StageObject)
		{
			hit = obj->HitCheckByRay(rInfo);
		}

		// 当たったら
		if(hit)	Destroy();
	}
	*/
}


void Missile::Explosion()
{
	//アニメーションエフェクトをインスタンス化
	std::shared_ptr<AnimationEffect>effect = std::make_shared<AnimationEffect>();

	//爆発テクスチャとアニメーション情報を渡す
	effect->SetAnimationInfo(
		KdResFac.GetTexture("Data/Texture/Explosion00.png"), 10.0f, 5, 5, rand() % 360);

	//場所をミサイルの位置に合わせる
	effect->SetMatrix(m_mWorld);
	//リストに追加
	Scene::GetInstance().AddObject(effect);
}

void Missile::UpdateTrail()
{
	//m_trailRotate += 1.0f;

	//軌跡の座標を先頭に追加
	//KdMatrix mTrail;
	//mTrail.RotateZ(m_trailRotate);
	//mTrail *= m_mWorld;

	m_trailSmoke.AddPoint(m_mWorld);


	//軌跡の数制限（100以前の軌跡を消去する）
	if (m_trailSmoke.GetNumPoints() > 100)
	{
		m_trailSmoke.DlPoint_Back();
	}
}

void Missile::DrawEffect()
{
	if (!m_alive) { return; }

	SHADER.m_effectShader.SetWorldMatrix(KdMatrix());
	SHADER.m_effectShader.WriteToCB();

	m_trailSmoke.DrawBillboard(0.5f);

	
}
