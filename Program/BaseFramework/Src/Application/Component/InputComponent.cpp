#include"InputComponent.h"

// コンストラクター：オーナーの設定・ボタンの初期化
InputComponent::InputComponent(GameObject& owner):m_owner(owner)
{
	// 操作軸初期化
	for (auto& axis : m_axes)
	{
		axis = { 0.0f,0.0f };
	}

	m_buttons.fill(FREE);	// 全要素 FREEに
}

void InputComponent::PushButton(Input::Buttons no)
{
	assert(no != Input::Buttons::BIN_MAX);

	// 押している
	if (m_buttons[no] & STAY)
	{
		m_buttons[no] &= ~ENTER;	// ENTER反転とAND = ENTERだけをOFF
	}
	// 押していない
	else
	{
		m_buttons[no] |= ENTER;	// ENTERをOR	= ENTERをON
		m_buttons[no] |= STAY;	// STAYをOR		= STAYをON
	}
}

void InputComponent::ReleaceButton(Input::Buttons no)
{
	assert(no != Input::Buttons::BIN_MAX);

	// 押している
	if (m_buttons[no] & STAY)
	{
		m_buttons[no] &= ~ENTER;	// ENTER反転とAND = ENTERだけをOFF
		m_buttons[no] &= ~STAY;		// STAY反転とAND = STAYだけをOFF
		m_buttons[no] |= EXIT;		// EXUTをOR		=　EXUTをON
	}
	// 押していない
	else
	{
		m_buttons[no] &= ~EXIT;		// EXIT反転とAND = EXITだけをOFF
	}
}

void PlayerInputComponent::Update()
{
	// 操作軸初期化
	for (auto& axis : m_axes)
	{
		axis = { 0.0f,0.0f };
	}

	// [左の軸値] 入力処理
	if (GetAsyncKeyState(VK_UP) & 0x8000)	{ m_axes[Input::Axes::L].y = 1.0f; }
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)	{ m_axes[Input::Axes::L].y = -1.0f; }
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000){ m_axes[Input::Axes::L].x = 1.0f; }
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)	{ m_axes[Input::Axes::L].x = -1.0f; }

	// [右の軸値] 入力処理
	if (GetAsyncKeyState('W') & 0x8000) { m_axes[Input::Axes::R].y = 1.0f; }
	if (GetAsyncKeyState('S') & 0x8000) { m_axes[Input::Axes::R].y = -1.0f; }
	if (GetAsyncKeyState('D') & 0x8000) { m_axes[Input::Axes::R].x = 1.0f; }
	if (GetAsyncKeyState('A') & 0x8000) { m_axes[Input::Axes::R].x = -1.0f; }

	// [ボタン] 入力処理
	if (GetAsyncKeyState('Z')) { PushButton(Input::Buttons::A); }
	else { ReleaceButton(Input::Buttons::A); }

	if (GetAsyncKeyState('X')) { PushButton(Input::Buttons::B); }
	else { ReleaceButton(Input::Buttons::B); }

	if (GetAsyncKeyState('C')) { PushButton(Input::Buttons::X); }
	else { ReleaceButton(Input::Buttons::X); }

	if (GetAsyncKeyState('V')) { PushButton(Input::Buttons::Y); }
	else { ReleaceButton(Input::Buttons::Y); }

	if (GetAsyncKeyState('Q')) { PushButton(Input::Buttons::L1); }
	else { ReleaceButton(Input::Buttons::L1); }

	if (GetAsyncKeyState('E')) { PushButton(Input::Buttons::R1); }
	else { ReleaceButton(Input::Buttons::R1); }
}

void EnemyInputComponent::Update()
{
	//m_axes[Input::Axes::L].y = 1.0f;
	//m_axes[Input::Axes::R].y = 1.0f;

	//rotate += 2.0f/270.0f;	// 2/n° … 最大角度 

	//m_axes[Input::Axes::L].y = 1.0f;
	//m_axes[Input::Axes::R].y = sin(rotate);
	////m_axes[Input::Axes::R].x = sin(rotate);
}