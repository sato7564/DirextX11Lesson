#include"Aircraft.h"
#include"Missile.h"
#include"../Scene.h"
#include"../../Component/CameraComponent.h"
#include"../../Component/InputComponent.h"
#include"../../Component/ModelComponent.h"
#include "EffectObject.h"

void Aircraft::Deserialize(const json11::Json& jsonObj)
{
	GameObject::Deserialize(jsonObj);

	//if (jsonObj.is_null() == false)
	//{
	//	if (m_spModelComponent)
	//	{
	//		m_spModelComponent->SetModedl(KdResFac.GetModel(jsonObj["ModelFileName"].string_value()));
	//	}
	//}

	if (m_spCameraComponent)
	{
		m_spCameraComponent->OffsetMatrix().CreatTransslation(0.0f, 1.5f, -10.0f);
	}

	if ((GetTag() & OBJECT_TAG::TAG_Player) != 0)
	{
		Scene::GetInstance().SetTargetCamera(m_spCameraComponent);

		// プレイヤー入力
		m_spInputComponent = std::make_shared<PlayerInputComponent>(*this);
	}
	else
	{
		// 敵入力
		m_spInputComponent = std::make_shared<EnemyInputComponent>(*this);
	}

	m_spActionState = std::make_shared<ActionFly>();

	// レーザー情報を取得
	const json11::Json& LaserObj = KdResFac.GetJSON("Data/Scene/Laser.json");
	
	if (LaserObj["Damage"].is_null() == false)
	{
		m_laserAttackDamage = LaserObj["Damage"].number_value();
	}
	//m_spPropeller = std::make_shared<GameObject>();
	//if (m_spPropeller && m_spPropeller->GetModelComponent())
	//{
	//	//プロペラのモデルを読み込む
	//	m_spPropeller->GetModelComponent()->SetModedl(KdResFac.GetModel("Data/Aircraft/Propeller.gltf"));

	//	m_mPropLocal.CreatTransslation(0.0f, 0.0f, 2.85f);

	//	m_propRotSpeed = 0.3f;
	//}
}

void Aircraft::Update()
{
	if (m_spInputComponent)
	{
		m_spInputComponent->Update();
	}

	m_prevPos = m_mWorld.GetTranslation();

	if (m_spActionState)
	{
		m_spActionState->Update(*this);
	}

	if (m_spCameraComponent)
	{
		m_spCameraComponent->SetCameraMatrix(m_mWorld);
	}

	UpdatePropeller();
}

void Aircraft::UpdateMove()
{
	if(m_spInputComponent == nullptr){ return;}

	const Math::Vector2& inputMove = m_spInputComponent->GetAxis(Input::L);

	// 移動ベクトル作成
	kdVec3 move = { inputMove.x,0.0f,inputMove.y };

	move.Normalize();

	// 移動速度補正
	move *= m_speed;

	/*/
	m_mWorld._41 += move.x;
	m_mWorld._42 += move.y;
	m_mWorld._43 += move.z;
	*/

	// 移動行列の作成
	//Math::Matrix moveMat = DirectX::XMMatrixTranslation(move.x, move.y, move.z);

	KdMatrix moveMat;										// 移動行列行列
	moveMat.CreatTransslation(move.x, move.y, move.z);

	// ワールド行列に合成
	//m_mWorld = DirectX::XMMatrixMultiply(moveMat, m_mWorld); 
	m_mWorld = moveMat * m_mWorld;

	// 回転ベクトルの作成
	//Math::Vector3 rotate = { 0.0f,0.0f,0.0f };

	// 飛行機の向きを変更
	const Math::Vector2& inputRotate = m_spInputComponent->GetAxis(Input::R);
	kdVec3 rotate = { -inputRotate.y,0.0f,-inputRotate.x };						// 先生のプロジェクト … { -inputRotate.y,0.0f,-inputRotate.x };	※ リバースしている
	
	// 回転行列作成
	//Math::Matrix rotateMat = DirectX::XMMatrixRotationX(rotate.x * KdToRadians);									// X軸回転行列の作成
	//rotateMat = DirectX::XMMatrixMultiply(rotateMat, DirectX::XMMatrixRotationZ(rotate.z * KdToRadians));			// Z軸の回転行列を作成し、X軸と合成
	KdMatrix rotateMat;
	rotateMat.CreateRotationX(rotate.x * KdToRadians);
	rotateMat.RotateZ(rotate.z * KdToRadians);

	// ワールド行列に合成
	//m_mWorld = DirectX::XMMatrixMultiply(rotateMat, m_mWorld);		// 第1引数が受け取り側(ベース)の行列、第2引数が送り側の行列
	m_mWorld = rotateMat * m_mWorld;

}

void Aircraft::ImGuiUpdate()
{
	// ImGuiの表示内容
	if (ImGui::TreeNodeEx("Aricraft", ImGuiTreeNodeFlags_DefaultOpen))
	{
		kdVec3 pos;
		pos = m_mWorld.GetTranslation();

		//ImGui::Text("Postion[x:%.2f] [y:%.2f] [z:%.2f]", pos.x, pos.y, pos.z);

		if (ImGui::DragFloat3("Postion", &pos.x, 0.0f))
		{
			KdMatrix mTrans;
			mTrans.CreatTransslation(pos.x, pos.y, pos.z);

			m_mWorld = mTrans;
		}

		ImGui::TreePop();
	}
}

void Aircraft::UpdateShoot()
{
	if (m_spInputComponent == nullptr) { return; }

	if (m_spInputComponent->GetButtoon(Input::Buttons::A))
	{
		if(m_canShoot)
		{
			std::shared_ptr<Missile> spMissile = std::make_shared <Missile>();

			if (spMissile)
			{
				spMissile->Deserialize(KdResFac.GetJSON("Data/Scene/Missile.json"));

				KdMatrix mLaunch;
				mLaunch.CreateRotationX((rand() % 120 - 60.0f) * KdToRadians);
				mLaunch.RotateY((rand() % 120 - 60.0f)* KdToRadians);
				mLaunch *= m_mWorld;

				spMissile->SetMatrix(mLaunch);

				spMissile->SetOwner(shared_from_this());

				Scene::GetInstance().AddObject(spMissile);

				// 一番近いオブジェクトとの距離を格納する変数：初期値はfloatでも大きな値を入れておく
				float minDistance = FLT_MAX;
				// 誘導する予定のターゲットGameObject
				std::shared_ptr<GameObject> spTarget = nullptr;

				// 全ゲームオブジェクトのリストからミサイルが当たる対象(敵)を探す
				for (auto object : Scene::GetInstance().GetObjects())
				{
					// 発射した飛行機自身は無視
					if (object.get() == this) { continue; }

					if ((object->GetTag() & TAG_AttackHit))
					{
						// (ターゲットの座標 - 自身の座標)の長さの2乗
						float distance = kdVec3(object->GetMatrix().GetTranslation() - m_mWorld.GetTranslation()).LengthSquared();

						// 一番近いオブジェクトとの距離よりも近ければ
						if (distance < minDistance)
						{
							// 誘導する予定のターゲットを今チェックしたGameObjectに置き換え
							spTarget = object;

							// 一番近いオブジェクトとの距離を今のものに更新
							minDistance = distance;
						}
					}
				}

				// 誘導するターゲットのセット
				spMissile->SetTarget(spTarget);
			}

			m_canShoot = false;
		}
	}
	else
	{
		m_canShoot = true;
	}

	m_laser = (m_spInputComponent->GetButtoon(Input::Buttons::B) != InputComponent::FREE);
}

// 当たり判定の更新
void Aircraft::UpdateCollsion()
{
	// レーザーが有効であれば当たり判定を行う
	if (m_laser)
	{
		// レイの発射情報
		RayInfo rayInfo;
		rayInfo.m_pos = m_prevPos;				// 移動する前の地点から
		rayInfo.m_dir = m_mWorld.GetAxisZ();	// 自分の向いている方向に
		rayInfo.m_dir.Normalize();
		rayInfo.m_maxRange = m_laserRange;		// レーザー射程分判定

		// レイの判定結果
		KdrayResult rayResult;

		// 全ゲームオブジェクトと判定
		for (auto& obj : Scene::GetInstance().GetObjects())
		{
			// 自分自身を無視
			if (obj.get() == this) { continue; }

			// 背景オブジェクトとキャラクターが対象
			if (!(obj->GetTag() & (TAG_StageObject | TAG_Character))) { continue; }

			// 判定実行
			if (obj->HitCheckByRay(rayInfo, rayResult))
			{
				// 当たったのあれば爆発をインスタンス化	
				std::shared_ptr<EffectObject> effectObj = std::make_shared<EffectObject>();

				// 相手の飛行機へダメージ通知
				std::shared_ptr<Aircraft> aircraft = std::dynamic_pointer_cast<Aircraft>(obj);
				if (aircraft)
				{
					aircraft->OnNotify_Damage(m_laserAttackDamage);
				}

				if (effectObj)
				{
					// キャラクターのヒット位置
					Scene::GetInstance().AddObject(effectObj);

					// レーザーのヒット位置 = レイの発射位置 + (レイの発射方向のベクトル * レイが当たった地点までの距離)
					kdVec3 hitPos(rayInfo.m_pos);
					hitPos = hitPos + (rayInfo.m_dir * rayResult.m_distance);

					// 爆発エフェクトの行列を計算
					KdMatrix mMat;
					mMat.CreatTransslation(hitPos.x, hitPos.y, hitPos.z);
					effectObj->SetMatrix(mMat);
				}				
			}
		}

	}

	// 一回の移動量と移動方向を計算
	kdVec3 moveVec = m_mWorld.GetTranslation() - m_prevPos;	// 動く前 → 今の場所のベクトル
	float moveDistans = moveVec.Length();					// 一回の移動量

	// 動いていないなら判定しない
	if (moveDistans == 0.0f) { return; }

	// 球情報の作成
	SphereInfo info;
	info.m_pos = m_mWorld.GetTranslation();
	info.m_radius = m_colRadius;


	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身を無視
		if (obj.get() == this) { continue; }

		// キャラクターと当たり判定をするのでそれ以外は無視
		if (!(obj->GetTag() & TAG_Character)) { continue; }

		// 当たり判定
		if (obj->HitCheckBySphere( info ))
		{
			Scene::GetInstance().AddDebugSphereLine(
				m_mWorld.GetTranslation(), 2.0f, { 1.0f,0.0f,0.0f,10.0f }
			);

			// 移動する前の位置に戻る
			m_mWorld.SetTranslation(m_prevPos);
		}
	}

	// レイによる当たり判定
	// レイ情報の作成
	RayInfo rayInfo;
	rayInfo.m_pos = m_prevPos;			// 一つ前の場所から
	rayInfo.m_dir = moveVec;			// 動いた方向に向かって
	rayInfo.m_dir.Normalize();
	rayInfo.m_maxRange = moveDistans;	// 動いた分だけ判定を行う

	KdrayResult rayResult;

	for (auto& obj : Scene::GetInstance().GetObjects())
	{
		// 自分自身は無視
		if (obj.get() == this) { continue; }
	
		// 背景タグ以外は無視
		if (!(obj->GetTag() & TAG_StageObject)) { continue; }

		// 判定実行
		if (obj->HitCheckByRay(rayInfo, rayResult))
		{
			// 相手の飛行機へのダメージ通知

			// ミサイルやレーザーの攻撃力はJsonに入れておく

			// 移動する前の1フレーム前に戻る
			m_mWorld.SetTranslation(m_prevPos);
		}
	}
}

void Aircraft::UpdatePropeller()
{
	//if (!m_spPropeller) { return; }
	//	m_spPropeller->SetMatrix(m_mPropLocal * m_mWorld);//プロペラを飛行機の位置に合わせる


	//	//プロペラを回転させる
	//	m_mPropLocal.RotateZ(m_propRotSpeed);
	

}

void Aircraft::Draw()
{
	GameObject::Draw();	// 基底クラスのDrawを呼び出す

	////プロペラの描画
	//if (m_spPropeller)
	//{
	//	m_spPropeller->Draw();
	//}

	// レーザー描画
	if (m_laser)
	{
		// レーザーの終点を求める
		kdVec3 laserStart(m_prevPos);
		kdVec3 laserEnd;
		kdVec3 laserDir(m_mWorld.GetAxisZ());

		laserDir.Normalize();	// 拡大が入っていると1以上になるので正規化

		laserDir *= m_laserRange;	//レーザーの射程方向ベクトルを伸ばす

		laserEnd = laserStart + laserDir;	// レーザーの終点は発射ベクトル＋レーザーの長さ分

		Scene::GetInstance().AddDebugLine(m_prevPos, laserEnd, { 0.0f,1.0,1.0f,1.0f });
	}
}

// ダメージ判定処理
void Aircraft::OnNotify_Damage(int damege)
{
	m_hp -= damege;	// 相手からの攻撃力分、HPを減らす

	// HPが0になったら消える
	if (m_hp <= 0)
	{
		m_spActionState = std::make_shared<ActionCrash>();
	}
}

void Aircraft::ActionFly::Update(Aircraft& owner)
{
	owner.UpdateMove();

	owner.UpdateCollsion();

	owner.UpdateShoot();
}

void Aircraft::ActionCrash::Update(Aircraft& owner)
{
	if (!(--m_timer))
	{
		owner.Destroy();
	}

	KdMatrix rotation;
	rotation.CreateRotationX(0.08f);
	rotation.RotateY(0.055f);
	rotation.RotateZ(0.03f);

	owner.m_mWorld = rotation * owner.m_mWorld;

	owner.m_mWorld.Move(kdVec3(0.0f, -0.2f, 0.0f));
}