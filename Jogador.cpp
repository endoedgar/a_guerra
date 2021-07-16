#include "stdafx.h"
#include "Jogador.h"
#include "D3DObject.h"
#include "GameGlobals.h"
#include "3DFunctions.h"
#include "CTerrain.h"
#include "3DParticles.h"
#include "projetil.h"
#include "objeto.h"
#include "GenericFunctions.h"
#include "radio.h"
#include "missao.h"
#include "base.h"
#include "GameHUD.h"
#include "MiniMapa.h"
#include "LineRender.h"

InfoHUD::InfoHUD():ImagemHUD()
{ }

void InfoHUD::atualizar() {
	if(time <= 60) {
		const real dest = static_cast<real>(gc->GetGraphicsHeight()/2);
		this->setPosition(Vector(this->getPosition().x, this->getPosition().y+((dest-this->getPosition().y)/10), 0));
	} else if(time > 120) {
		const real dest = static_cast<real>(gc->GetGraphicsHeight());
		this->setPosition(Vector(this->getPosition().x, this->getPosition().y+((dest-this->getPosition().y)/10), 0));
	}
	++this->time;
}

void InfoHUD::desenhar(GameHUD *gh) {
	if(this->time <= 200)
		ImagemHUD::desenhar(gh);
}

void InfoHUD::mostrar() {
	this->time = 0;
	this->setPosition(Vector(static_cast<real>(gc->GetGraphicsWidth()/2), 0, 0));
}

void CControle::Atualizar()
{
	
	switch(this->GetType())
	{
		case CONTROLE_TIPO_TECLADO:
			v.atualizar();
			if(keys[DIK_V]) {
				const real i = r(1)/r(50);
				this->v.suavizar(i);
			}
			this->eixoX = v.getEixo().getX();
			this->eixoY = v.getEixo().getY();
			this->input[INPUT_ESQUERDA] = keys[DIK_A];
			this->input[INPUT_DIREITA] = keys[DIK_D];
			this->input[INPUT_ACELERAR] = keys[DIK_LCONTROL] || keys[DIK_RCONTROL] || keys[DIK_W];
			this->input[INPUT_DESACELERAR] = keys[DIK_LSHIFT] || keys[DIK_LSHIFT] || keys[DIK_S];
			this->input[INPUT_ATIRAR] = keys[DIK_SPACE] || mouse_click;
			this->input[INPUT_STEALTH] = false;
			break;
		case CONTROLE_TIPO_JOYSTICK:
			break;
		default:
			RuntimeError("Controle %d não implementado!", this->GetType());
	}
}

void CJogador::showDamage(const Vector &w)
{
	this->wdamage = w;
	this->damagetime = GameTicks + 30;
}

CJogador::CJogador()
{
	TRACE("Jogador %p criado!\n", this);
	ZeroMemory(this, sizeof(CJogador));
	this->info = new InfoHUD;
}

CJogador::~CJogador()
{
	this->Limpar();
	TRACE("Jogador %p removido!\n", this);
}

void CJogador::ControleManual()
{
	CObjeto *myobj = *this->obj;
	real minvdistance = -1;
	if(myobj)
	{
		CObjeto **tgt = NULL;
		CObjeto *pObj = g_Unidades->GetFirstActive();
		//myobj->setTarget(NULL);
		while(pObj)
		{
			if(pObj != myobj)
			{
				if(this->cam->SphereVisible(pObj->GetPosition(), pObj->dados->getRadius()))
				{
					if(pObj->team != myobj->team && pObj->ptr && myobj->podeAtacar(pObj->ptr->GetPtr()))
					{
						Vector tmp;
						if(myobj->dados->tipo == TIPO_VEICULO) {
							tmp.y = DeltaPitch(myobj->Turret, pObj->GetPosition(true));
							tmp.x = DeltaYaw(myobj->Turret, pObj->GetPosition(true));
						} else {
							tmp.y = DeltaPitch(myobj, pObj->GetPosition(true));
							tmp.x = DeltaYaw(myobj, pObj->GetPosition(true));
						}
						tmp.z = 0;
						const real lengthsq = tmp.lengthsq();
						if(lengthsq < minvdistance || minvdistance == -1) {
							minvdistance = lengthsq;
							tgt = pObj->ptr->GetPtr();
						}
						/*if(tmp.z <= 1.0) {
							if(myobj->alvo && *myobj->alvo) {
								//if((DistanceF(myobj->GetPosition(), pObj->GetPosition()) < mindistance || mindistance == -1) && ) {
									myobj->alvo = pObj->ptr->GetPtr();
	
								}
							}
							else {
								myobj->alvo = pObj->ptr->GetPtr(); 
								mindistance = 
							}
						}*/
					}
				}
			}
			pObj = g_Unidades->GetNext(pObj);
		}
		if(myobj->getTarget() != tgt)
			myobj->setTarget(tgt);
	}
}

void CJogador::MostrarTexto(const Smart::String &texto, const unsigned int texttime, const D3DCOLOR textcolor) {
	this->textcolor = textcolor;
	this->texto = texto;
	this->texttime = GameTicks+texttime;
}

void CJogador::Atualizar()
{
	this->cam->UpdateShake();
	this->info->atualizar();
	if(keys[DIK_M] == GameTicks)
		ShowHUD = !ShowHUD;
	if(obj)
	{
		if(*obj)
		{
			Vector vec;
			Vector wher;
			//vec = Vector(0, -0.2, 0);
			//wher = Vector(0, 0, 1);
			//(*obj)->Impulse(&wher, &vec);
			if(keys[DIK_V]) {
				if(this->cam->GetScale()->y < 5)
					this->cam->Scale((this->cam->GetScale()->x)*r(1.05), (this->cam->GetScale()->y)*r(1.05), (this->cam->GetScale()->z)*r(1.05));
			} else
				this->cam->Scale(1, 1, 1);

			if(keys[DIK_F1] == GameTicks)
				PedirAjuda(*obj);
			if(keys[DIK_F2] == GameTicks)
				--this->CameraTipo;
			else if(keys[DIK_F3] == GameTicks)
				++this->CameraTipo;

			if(CameraTipo > 4)
				CameraTipo = 4;
			else if(CameraTipo < 0)
				CameraTipo = 0;

			switch(this->CameraTipo) {
				case 1:
					vec.x = 0;
					vec.y = 15;
					vec.z = -50;

					if((*this->obj)->dados->tipo == TIPO_VEICULO) {
						CQuaternion qT;

						TFormPoint(&vec, &vec, (*obj)->Turret, NULL);
						//vec += *(*obj)->Turret->GetPosition();

						qT = *(*this->obj)->Turret->GetQuaternion();
						qT *= *(*this->obj)->GetQuaternion();
						//qT = CQuaternion(0, 0, 0) * qT;
				
						this->cam->Rotate(&qT);
					} else {
						CQuaternion qT;

						TFormPoint(&vec, &vec, *obj, NULL);

						qT = gIdentityQuaternion;
						qT *= *(*this->obj)->GetQuaternion();
						this->cam->Rotate(&qT);
					}
					vec = (vec-*this->cam->GetPosition())*0.2;
					this->cam->Translate(&vec);
					break;
				case 2:
					vec.x = 0;
					vec.y = 15;
					vec.z = 50;

					if((*this->obj)->dados->tipo == TIPO_VEICULO) {
						CQuaternion qT;

						TFormPoint(&vec, &vec, (*obj)->Turret, NULL);
						//vec += *(*obj)->Turret->GetPosition();

						qT = *(*this->obj)->Turret->GetQuaternion();
						qT *= *(*this->obj)->GetQuaternion();
						qT = CQuaternion(0, D3DXToRadian(180), 0) * qT;
				
						this->cam->Rotate(&qT);
					} else {
						CQuaternion qT;

						TFormPoint(&vec, &vec, *obj, NULL);

						qT = CQuaternion(0, D3DXToRadian(180), 0);
						qT *= *(*this->obj)->GetQuaternion();
						this->cam->Rotate(&qT);
					}
					vec = (vec-*this->cam->GetPosition())*0.2;
					this->cam->Translate(&vec);
					break;
				case 3: {
					const real dPitch = DeltaPitch(this->cam, *this->obj), dYaw = DeltaYaw(this->cam, *this->obj);
					this->cam->Turn(dPitch, dYaw, 0);
					this->cam->Rotate(this->cam->pitch(), this->cam->yaw(), 0);
					if(!this->cam->GetQuaternion()->IsSensible())
						RuntimeError("Erro fatal!");
					break;
						}
				case 0:

					if((*this->obj)->dados->tipo == TIPO_VEICULO) {
						CQuaternion qT;

						TFormPoint(&vec, &(*this->obj)->dados->fpcamera, (*obj)->Turret, NULL);
						//vec += *(*obj)->Turret->GetPosition();

						qT = *(*this->obj)->Turret->GetQuaternion();
						qT *= *(*this->obj)->GetQuaternion();
						//qT = CQuaternion(0, 0, 0) * qT;
				
						this->cam->Rotate(&qT);
					} else {
						CQuaternion qT;

						TFormPoint(&vec, &(*this->obj)->dados->fpcamera, *obj, NULL);

						qT = gIdentityQuaternion;
						qT *= *(*this->obj)->GetQuaternion();
						this->cam->Rotate(&qT);
					}
					vec = (vec-*this->cam->GetPosition());
					this->cam->Translate(&vec);
					break;
			}
			if(this->controle)
				this->ControleManual();
		}
	}
	else
	{
		real pitch, yaw;
		const real halfPI = D3DX_PI*static_cast<real>(0.48);

		this->cam->Turn(D3DXToRadian(this->controle->v.getEixo().getY()), D3DXToRadian(this->controle->v.getEixo().getX()), 0);
		pitch = this->cam->pitch();
		yaw = this->cam->yaw();
		if(pitch > halfPI)
			pitch = halfPI;
		else if(pitch < -halfPI)
			pitch = -halfPI;
		this->cam->Rotate(pitch, yaw, 0);
		if((*this->controle)(INPUT_ACELERAR))
			this->cam->Move(0, 0, 3);
		else if((*this->controle)(INPUT_DESACELERAR))
			this->cam->Move(0, 0, -3);
	}
	//this->cam->Turn(D3DXToRadian(1), 0, 0);
	this->cam->UpdatePhysics();
	this->controle->Atualizar();
}

#include "3DPlane.h"
extern CPlane *mar;

void CJogador::Atualizar3D()
{
	HRESULT hr;
	d3dDevice->SetViewport(this->cam->getViewport());
	this->cam->UpdateMatrix();
	this->cam->Project();
	hr = Render(this);
	// Present 
	//g_Jogador->AtualizarHUD();
	if(FAILED(hr))
	{
		if(hr == D3DERR_DEVICELOST)
			mywrapper->Reset();
		else
			RuntimeError("Falha ao apresentar buffer a saída!\nCódigo do Erro: %d", HRESULT_CODE(hr));
	}
}

extern CTexture *martex;

void CJogador::mostrarAlvo() {
	if((*obj)->GetTarget()) {
		Vector vec;
		CObjeto *target = (*obj)->GetTarget();
		this->cam->ProjectVector(&vec, target->GetPosition());
		if(vec.z <= 1.0) {
#ifdef UNICODE
			std::wostringstream temp;
#else
			std::ostringstream temp;
#endif
			
			FontPosition.left = static_cast<LONG>(vec.x-gTxtmetrics.tmAveCharWidth*target->dados->simbolo.GetLength());
			FontPosition.top = static_cast<LONG>(vec.y-gTxtmetrics.tmHeight);
			g_Font->DrawText(fontsprlayer, target->dados->simbolo.c_str(), -1, &FontPosition, 0, D3DCOLOR_XRGB(255, 0, 0));

			temp << static_cast<int>(Distance((*this->obj)->GetPosition(), target->GetPosition()));
			FontPosition.left = static_cast<LONG>(vec.x+gTxtmetrics.tmAveCharWidth);
			FontPosition.top = static_cast<LONG>(vec.y+gTxtmetrics.tmHeight);
			g_Font->DrawText(fontsprlayer, temp.str().c_str(), -1, &FontPosition, 0, D3DCOLOR_XRGB(255, 255, 255));
		}
	}
}

void CJogador::mostrarInformacao(CTexture *tex)
{
	this->info->setTextura(tex);
	this->info->mostrar();
}

VECTOR2D *transform(VECTOR2D *out, const Vector *in, D3DVIEWPORT9 &mport, real mapScale) {
	real wscale = terrain->getTexture()->GetWidth() * mapScale;

	out->x = in->x;
	out->x /= terrain->GetScale()->x;
	out->x *= mapScale * terrain->getTexture()->GetWidth();
	out->x = -wscale + out->x;
	out->y = in->z;
	out->y /= terrain->GetScale()->z;
	out->y *= mapScale * terrain->getTexture()->GetHeight();
	out->y = -out->y;

	return out;
}

void CJogador::renderizarMapa(const unsigned int x, const unsigned int y, const unsigned int w, const unsigned int h) {
	RECT mrect, oldrect;
	D3DVIEWPORT9 mport, oldport;
	const real mapScale = 2;//r(1000)/(*this->obj)->dados->getRadar();
	VECTOR2D campostransformed;
	const unsigned int whalf = w/2, hhalf = h/2;
	static Entity pivot;
	
	Vector front(0, 0, 1);

	d3dDevice->GetViewport(&oldport);

	mport.X = x;
	mport.Y = y;
	mport.Width = w;
	mport.Height = h;
	mport.MaxZ = oldport.MaxZ;
	mport.MinZ = oldport.MinZ;

	d3dDevice->SetViewport(&mport);

	transform(&campostransformed, this->cam->GetPosition(true), mport, mapScale);

	//positionx = this->cam->GetPositionX(true);
	//positionx /= terrain->GetScale()->x;
	//positionx *= mapScale * terrain->getTexture()->GetWidth();

	//positiony = this->cam->GetPositionZ(true);
	//positiony /= terrain->GetScale()->z;
	//positiony *= mapScale * terrain->getTexture()->GetHeight();

	//real wscale = terrain->getTexture()->GetWidth() * mapScale;

	gh->SetDrawMode(GameHUD::F_TEXTURIZED | GameHUD::F_CENTER | GameHUD::F_ALPHABLEND);

	pivot.Position(this->cam->GetPosition(true));
	pivot.Rotate(0, this->cam->yaw(), 0);
	pivot.UpdatePhysics();
	pivot.UpdateMatrix();

	pivot.GetQuaternion()->apply(&front, &front);

	CObjeto *ob = g_Unidades->GetFirstActive();
	while(ob) {
		if((*this->obj)->noRadar(ob)) {
			if(ob->dados->radarimg) {
				VECTOR2D objt;
				Vector tmp;
				D3DCOLOR cor;

				TFormPoint(&tmp, ob->GetPosition(true), NULL, &pivot);
				objt.x = tmp.x;//ob->GetPositionX(true)-this->cam->GetPositionX(true);
				objt.y = tmp.z;//ob->GetPositionZ(true)-this->cam->GetPositionZ(true);
				objt.x /= terrain->GetScale()->x;
				objt.x *= mapScale * terrain->getTexture()->GetWidth();
				objt.y /= terrain->GetScale()->z;
				objt.y *= mapScale * terrain->getTexture()->GetHeight();
				if(ob->team)
					cor = ob->team->getColor();
				else
					cor = 0xFFFFFFFF;
				if(objfalante)
					if(ob == *objfalante && SBPlaying(radio_sound_channel))
						cor = 0xFFFFFFFF;
				gh->RenderQuad(x+whalf+objt.x, y+hhalf-objt.y, static_cast<real>(0.1)*mapScale, ob->dados->radarimg, ((ob->yaw()+D3DX_PI)-(front.yaw()+D3DX_PI)), 0, 0, cor);
			}
		}
		ob = g_Unidades->GetNext(ob);
	}

	for(register size_t i = 0; i < mymission->GetBases().GetCapacity(); ++i) {
		CBase *atual = &mymission->GetBases()[i];
		if(atual->Dona() && atual->Dona()->getFlag()) {
			VECTOR2D objt;
			Vector tmp;
			D3DCOLOR cor;

			TFormPoint(&tmp, atual->GetPosition(true), NULL, &pivot);
			objt.x = tmp.x;//ob->GetPositionX(true)-this->cam->GetPositionX(true);
			objt.y = tmp.z;//ob->GetPositionZ(true)-this->cam->GetPositionZ(true);
			objt.x /= terrain->GetScale()->x;
			objt.x *= mapScale * terrain->getTexture()->GetWidth();
			objt.y /= terrain->GetScale()->z;
			objt.y *= mapScale * terrain->getTexture()->GetHeight();
			gh->RenderQuad(x+whalf+objt.x, y+hhalf-objt.y, static_cast<real>(0.05)*mapScale, atual->Dona()->getFlag(), 0, 0, 0);
		}
	}

	gh->SetDrawMode(GameHUD::F_TEXTURIZED | GameHUD::F_INVU);
	gh->RenderQuad(static_cast<real>(x+whalf), static_cast<real>(y+hhalf), mapScale, terrain->getTexture(), -front.yaw()+D3DX_PI, static_cast<real>(campostransformed.x), static_cast<real>(campostransformed.y));
	
	gh->Render();

	d3dDevice->SetViewport(&oldport);
}

void CJogador::mostrarTarefas(CMissao *m) {
	// TODO: Exibição visual das tarefas
}

void CJogador::AtualizarHUD()
{
	PROFILE_START;
	real t = GetTime();
	static char buffer[256];

	if(this->obj)
		if(*this->obj) {
			this->renderizarMapa(650, 0, 150, 150);
			Vector vec(0, 0, 500);
			if((*this->obj)->dados->tipo == TIPO_VEICULO)
				TFormPoint(&vec, &vec, (*this->obj)->Turret, NULL);
			else
				TFormPoint(&vec, &vec, *this->obj, NULL);
			this->cam->ProjectVector(&vec, &vec);
			CObjeto *myobj = *this->obj;
			const real liferate = myobj->getLife()/myobj->dados->armour;
			gh->SetDrawMode(GameHUD::MODES::F_ALPHABLEND);
			gh->RenderQuad(57, 9, imgHudBar->GetWidth()*(myobj->getLife()/myobj->dados->armour), imgHudBar->GetHeight(), imgHudBar, D3DCOLOR_XRGB(0, 255, 0));
			if(myobj->dados->arma)
				gh->RenderQuad(57, 48, imgHudBar->GetWidth()*(static_cast<real>(myobj->shootdelay)/myobj->dados->arma->shootdelay/myobj->dados->stock), imgHudBar->GetHeight(), imgHudBar, D3DCOLOR_XRGB(255, 255, 0));
			gh->RenderQuad(0, 0, imgHudPanel);
			gh->SetDrawMode(GameHUD::MODES::F_ALPHABLENDADD | GameHUD::MODES::F_CENTER);
			//gh->RenderQuad((600.0/640.0)*gc->GetGraphicsWidth(), (400.0/480.0)*gc->GetGraphicsHeight(), static_cast<real>(0.5), myobj->dados->iconehud, D3DCOLOR_RGBA(static_cast<unsigned char>(255*(1-liferate)), static_cast<unsigned char>(255*liferate), 0, 255));
			if(myobj->dados->arma)
				gh->RenderQuad(static_cast<real>(gc->GetGraphicsWidth()/2), static_cast<real>(gc->GetGraphicsHeight()-(myobj->dados->arma->hudicon->GetHeight()*2)), myobj->dados->arma->hudicon, myobj->shootdelay/static_cast<real>(myobj->dados->arma->shootdelay), 1, 0xFFFFFFFF);
			if(this->damagetime > GameTicks) {
				real alpha = ((this->damagetime-GameTicks)/r(30))*r(255);
				Vector tmp;
				unsigned int cor = (static_cast<unsigned int>(alpha) << 24) | 0xFFFFFF;

				TFormPoint(&tmp, &this->wdamage, NULL, this->GetObjectA());
				TFormVector(&tmp, &tmp, this->GetObjectA(), this->GetCamera());
				tmp.normalize();

				gh->RenderQuad(gc->GetGraphicsWidth()/2, gc->GetGraphicsHeight()/2, 1, imgDamage, tmp.yaw(), 0, -90, cor);
			}
			if(vec.z <= 1.0)
				gh->RenderQuad(vec.x, vec.y, crosshairicon, 1, 1);
			if((*obj)->GetTarget()) {
				Vector vec;
				CObjeto *target = (*obj)->GetTarget();
				this->cam->ProjectVector(&vec, target->GetPosition());
				if(vec.z <= 1.0) {
					const real dist = 1.0-(Distance(this->cam->GetPosition(), target->GetPosition())/10000.0);
					gh->RenderQuad(vec.x, vec.y, dist*targeticon->GetWidth(), dist*targeticon->GetHeight(),targeticon, 0xFFFFFFFF);
					if((*obj)->dados->arma && (*obj)->lockOnTime ) {
						if((*obj)->lockOnTime >= static_cast<CHotSeekingProjectileData*>((*obj)->dados->arma)->lockOnTime)
							gh->RenderQuad(vec.x, vec.y, dist*imgLockOn->GetWidth(), dist*imgLockOn->GetHeight(),imgLockOn, 0xFFFFFFFF);
						else if(GameTicks % 2 == 0)
							gh->RenderQuad(vec.x, vec.y, dist*imgLockingOn->GetWidth(), dist*imgLockingOn->GetHeight(),imgLockingOn, 0xFFFFFFFF);
					}
				}
			}
			gh->SetDrawMode(GameHUD::MODES::F_CENTER | GameHUD::MODES::F_TEXTURIZED | GameHUD::MODES::F_ALPHABLEND);
			this->info->desenhar(gh);
			gh->SetDrawMode(0);
			gh->Render();
			//this->desenharMiniMapa();
		}

	if(SUCCEEDED(fontsprlayer->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_DO_NOT_ADDREF_TEXTURE)))
	{
		if(GameTicks-ObjetivoMudou <= 500 && CurrentObjective >= 0 && CurrentObjective < mymission->GetNObjetivos()) {
			//gh->RenderQuad(0, imgHudPanel->GetHeight(), 200, 200, D3DCOLOR_XRGB(255, 0, 0));
			RECT rect;
			int alpha;
			Smart::String str = (*mymission->GetDados())[CurrentObjective]->getNome().c_str();
			const size_t ticksElapsed = (GameTicks-ObjetivoMudou);
			if(ticksElapsed > 30) {
				alpha = 500-ticksElapsed;
			} else {
				alpha = (ticksElapsed/static_cast<real>(30))*255;
			}
			if(alpha > 255)
				alpha = 255;
			rect.left = 0;
			rect.top = 0;
			rect.right = gc->GetGraphicsWidth();
			rect.bottom = gc->GetGraphicsHeight();
			g_Font->DrawText(fontsprlayer, str.c_str(), -1, &rect, DT_CALCRECT | DT_WORDBREAK, 0xFFFFFFFF);
			rect.top = mywrapper->d3dpp.BackBufferHeight-rect.bottom;
			rect.bottom = gc->GetGraphicsHeight();
			g_Font->DrawText(fontsprlayer, str.c_str(), -1, &rect, DT_WORDBREAK, D3DCOLOR_ARGB(alpha, 255, 255, 255));
		}
		if(this->obj && *this->obj) {
			std::ostringstream tmp;
			Smart::String tmps;
			FontPosition.top = 0;
			FontPosition.left = 0;
			tmp << "Armadura: ";
			tmp << (((*this->obj)->getLife())/(*this->obj)->dados->armour)*100;
			tmp << "%";
			tmp << ' ';
			tmp << "Torque: ";
			tmp << (*this->obj)->torque.x; 
			tmp << ' ';
			tmp << (*this->obj)->torque.y;
			tmp << ' ';
			tmp << (*this->obj)->torque.z;
			tmp << ' ';
			tmp << (*this->obj)->ai.wpt.x;
			tmp << ' ';
			tmp << (*this->obj)->ai.wpt.y;
			tmp << ' ';
			tmp << (*this->obj)->ai.wpt.z;
			//tmp << " AVelocity: " << (*this->obj)->GetAngularMomentum()->x << ' ' << (*this->obj)->GetAngularMomentum()->y << ' ' << (*this->obj)->GetAngularMomentum()->z;
			tmps =  tmp.str().c_str();
			//g_Font->DrawText(fontsprlayer, tmps.c_str(), -1, &FontPosition, 0, 0xFFFFFFFF);
			mostrarAlvo();
		}
		if(game_over) {
			FontPosition.left = 0;
			FontPosition.top = gc->GetGraphicsHeight()/2;
			g_Font->DrawTextA(fontsprlayer, "VOCÊ FALHOU", -1, &FontPosition, DT_CENTER, D3DCOLOR_RGBA(255,0,0,255));
		} else if(mission_complete) {
			FontPosition.left = 0;
			FontPosition.top = gc->GetGraphicsHeight()/2;
			g_Font->DrawTextA(fontsprlayer, "MISSÃO CUMPRIDA", -1, &FontPosition, DT_CENTER, D3DCOLOR_RGBA(0,0,255,255));
		} else if(CurrentObjective < mymission->GetNObjetivos()){
			CObjetivo *objetivo = &mymission->GetDados()->pObjetivos[CurrentObjective];
			if(objetivo->maxSec) {
				static std::string ssa;
				ssa = "Tempo Restante: ";
				ssa += _gcvt(objetivo->maxSec-(GetTime()-objetivoStartSec), 6, buffer);
				FontPosition.left = 0;
				FontPosition.top = 100;
				g_Font->DrawTextA(fontsprlayer, ssa.c_str(), -1, &FontPosition, 0, D3DCOLOR_RGBA(0,0,255,255));
			}
		}
	
		//TextOut(hdc, 120, 120, ios.str().c_str(), ios.str().size());

				Vector vec;
				if(ShowHUD)
				{
					if(this->obj && *this->obj) {
						Vector v[4];
						bool b[4];

						CObjeto *myobj = *this->obj;
						this->cam->ProjectVector(&vec, &myobj->ai.wpt);
						if(vec.z <= 1.0f) {
							FontPosition.left = static_cast<LONG>(vec.x);
							FontPosition.top = static_cast<LONG>(vec.y);
							g_Font->DrawTextA(fontsprlayer, "-----------------", -1, &FontPosition, 0, 0xFF00FFFF);
						}
						TFormPoint(&vec, &myobj->dados->getCOM(), myobj, NULL);
						this->cam->ProjectVector(&vec, &vec);
						if(vec.z <= 1.0f) {
							Vector vFric(0, 0, 1), vFriction;

							const real yaw = myobj->yaw();
							const real cyaw = fabs(cosf(yaw));
							const real syaw = fabs(sinf(yaw));

							vFriction.x = vFric.x*cyaw+vFric.z*syaw;
							vFriction.y = vFric.y;
							vFriction.z = vFric.z*cyaw+vFric.x*syaw;

							//myobj->GetQuaternion()->apply(&vFront, &vFront);
							std::stringstream ssss;
							ssss << vFriction.x << ' ' << vFriction.y << ' ' << vFriction.z;
							FontPosition.left = static_cast<LONG>(vec.x);
							FontPosition.top = static_cast<LONG>(vec.y);
							g_Font->DrawTextA(fontsprlayer, ssss.str().c_str(), -1, &FontPosition, 0, 0xFF00FFFF);
						}
						for(register unsigned int a = 0; a < 8; ++a) {
							Vector v(myobj->dados->meshCol.getVertice(a));

							TFormPoint(&v, &v, myobj, NULL);
							this->cam->ProjectVector(&v, &v);
							if(v.z <= 1.0f) {
								FontPosition.left = static_cast<LONG>(v.x);
								FontPosition.top = static_cast<LONG>(v.y);
#ifdef UNICODE
								std::wostringstream sss;
#else
								std::ostringstream sss;
#endif
								sss << a;
								g_Font->DrawTextA(fontsprlayer, sss.str().c_str(), -1, &FontPosition, 0, 0xFFFFFFFF);
							}
						}
						//TFormPoint(&v[0], myobj->dados->p1, myobj, NULL);
						//TFormPoint(&v[1], myobj->dados->p2, myobj, NULL);
						//TFormPoint(&v[2], myobj->dados->p3, myobj, NULL);
						//TFormPoint(&v[3], myobj->dados->p4, myobj, NULL);
						for(register unsigned int a = 0; a < 4; ++a) {
							v[a] = *myobj->suspensao[a].GetPosition(true);
						}

						for(register size_t a = 0; a < 4; ++a)
							b[a] = v[a].y <= terrain->GetTerrainHeight(&v[a]);

						this->cam->ProjectVector(&v[0], &v[0]);
						this->cam->ProjectVector(&v[1], &v[1]);
						this->cam->ProjectVector(&v[2], &v[2]);
						this->cam->ProjectVector(&v[3], &v[3]);

#define DCOLOR D3DCOLOR_RGBA(255,0,255,255)
						D3DCOLOR cor = DCOLOR;

						if(v[0].z <= 1.0f) {
							FontPosition.left = static_cast<LONG>(v[0].x);
							FontPosition.top = static_cast<LONG>(v[0].y);
							if(b[0])
								cor = D3DCOLOR_XRGB(255, 0, 0);

							g_Font->DrawTextA(fontsprlayer, "F", -1, &FontPosition, 0, cor);
						}

						cor = DCOLOR;

						if(v[1].z <= 1.0f) {
							FontPosition.left = static_cast<LONG>(v[1].x);
							FontPosition.top = static_cast<LONG>(v[1].y);
							if(b[1])
								cor = D3DCOLOR_XRGB(255, 0, 0);

							g_Font->DrawTextA(fontsprlayer, "B", -1, &FontPosition, 0, cor);
						}

						cor = DCOLOR;

						if(v[2].z <= 1.0f) {
							FontPosition.left = static_cast<LONG>(v[2].x);
							FontPosition.top = static_cast<LONG>(v[2].y);
							if(b[2])
								cor = D3DCOLOR_XRGB(255, 0, 0);

							g_Font->DrawTextA(fontsprlayer, "L", -1, &FontPosition, 0, cor);
						}

						cor = DCOLOR;

						if(v[3].z <= 1.0f) {
							FontPosition.left = static_cast<LONG>(v[3].x);
							FontPosition.top = static_cast<LONG>(v[3].y);
							if(b[3])
								cor = D3DCOLOR_XRGB(255, 0, 0);

							g_Font->DrawTextA(fontsprlayer, "R", -1, &FontPosition, 0,cor);
						}
					}
					//if(*this->obj) {
					//	CObjeto *myobj = *this->obj;
						CObjeto *ob = g_Unidades->GetFirstActive();
						while(ob)
						{
							Vector tmp;
							tmp = *ob->dados->midpoint;
							tmp = tmp.Transformed(&ob->mWorld);
							tmp += *ob->GetPosition();
							this->cam->ProjectVector(&vec, &tmp);
		
							if(vec.z <= 1.0f)// && myobj != ob)
							{
								char *msg = "";
								D3DCOLOR cor = D3DCOLOR_RGBA(255,255,255,255);
								static Smart::String a;
								/*if(ob->team != myobj->team)
								{
									if(myobj->alvo)
										if(*myobj->alvo == ob)
										{
											msg = "Alvo";
											cor = D3DCOLOR_RGBA(255,0,0,255);
										}
								}
								else
								{
									msg = "Amigo";
									cor = D3DCOLOR_RGBA(0,0,255,255);
								}*/
								switch(ob->ai.state.get())
								{
									case FSM_IDLE:
										msg = "Idle";
										break;
									case FSM_ATTACK:
										msg = "Attack";
										break;
									case FSM_PATROL:
										msg = "Patrol";
										break;
									case FSM_CAPTURE:
										msg = "Capture";
										break;
									case FSM_GUARD:
										msg = "Guard";
										break;
									case FSM_FOLLOW:
										msg = "Follow";
										break;
									case FSM_FIXPOSITION:
										msg = "FixPosition";
										break;
									case FSM_SEARCH:
										msg = "Search";
										break;
									case FSM_REPAIR:
										msg = "Repair";
										break;
								}
								a = msg;
								a += ' ';
								a += ob->dados->simbolo.c_str();
								a += ' ';
								a += _gcvt(ob->ai.aimvar, 6, buffer);
								a += ' ';
								a += _gcvt(ob->ai.state.timeToFinish(), 6, buffer);
								
								FontPosition.left = static_cast<LONG>(vec.x);
								FontPosition.top = static_cast<LONG>(vec.y);

								g_Font->DrawText(fontsprlayer, a.c_str(), -1, &FontPosition, 0, cor);
								
								//TextOut(hdc, vec.x, vec.y, a.str().c_str(), a.str().size());
							}
							//Vector vertexes[2];
							//vertexes[0] = *ob->GetPosition(true);
							//vertexes[1] = ob->ai.wpt;
							//D3DXMATRIX m = *this->cam->GetMatrixView();
							//m *= *this->cam->GetMatrixProj();
							//lr->DrawTransform((D3DXVECTOR3*)vertexes, 2, &m, ob->team->getColor());
							/*this->cam->ProjectVector(&vertexes[0], &vertexes[0]);
							this->cam->ProjectVector(&vertexes[1], &vertexes[1]);
							D3DXVECTOR2 v2D[2];
							if(vertexes[0].z <= 1 || vertexes[1].z <= 1) {
								v2D[0].x = vertexes[0].x;
								v2D[0].y = vertexes[0].y;
								v2D[1].x = vertexes[1].x;
								v2D[1].y = vertexes[1].y;
								lr->Draw(v2D, 2, ob->team->getColor());
							}*/
							ob = g_Unidades->GetNext(ob);
						}
						//lr->Render();
					//}
			}
				if(SBPlaying(radio_sound_channel))
					if(objfalante && *objfalante) {
						Vector myvec;
						this->cam->ProjectVector(&myvec, (*objfalante)->GetPosition());
						if(myvec.z <= 1.0f)// && myobj != ob)
						{
							static const Smart::String szSpeaking("Falando");
							D3DCOLOR cor = D3DCOLOR_RGBA(255,0,0,255);
							FontPosition.left = static_cast<LONG>(myvec.x);
							FontPosition.top = static_cast<LONG>(myvec.y - 20);
							g_Font->DrawText(fontsprlayer, szSpeaking.c_str(), -1, &FontPosition, 0, cor);
						}
					}
		FontPosition.left = 0;
		FontPosition.top = 0;

#ifdef _ENABLEDEBUGPOINTS
		Vector tmp;
		const real sqrtaaa = r(sqrt((real)NTERRAINPOINTS));
		real x = -sqrtaaa/2, y = -sqrtaaa/2;
		for(register unsigned int i = 0; i < NTERRAINPOINTS; ++i) {
			g_TerrainPoints[i].Position(this->cam->GetPositionX()+x, 0, this->cam->GetPositionZ()+y);
			g_TerrainPoints[i].PositionY(terrain->GetTerrainHeight(g_TerrainPoints[i].GetPosition()));
			g_TerrainPoints[i].temp = terrain->lasttri;
			g_TerrainPoints[i].UpdateMatrix();
			this->cam->ProjectVector(&tmp, g_TerrainPoints[i].GetPosition(true));
			if(tmp.z <= 1.0f) {
				D3DCOLOR cor = (g_TerrainPoints[i].temp) ? D3DCOLOR_XRGB(0, 0, 255) : D3DCOLOR_XRGB(255, 0, 0);
				FontPosition.left = static_cast<LONG>(tmp.x);
				FontPosition.top = static_cast<LONG>(tmp.y);

				g_Font->DrawTextA(fontsprlayer, "X", -1, &FontPosition, 0, cor);
			}
			++x;
			if(x >= sqrtaaa/2) {
				x = -sqrtaaa/2;
				++y;
				if(y >= sqrtaaa/2) 
					y = -sqrtaaa/2;
			}
		}
#endif

		for(register size_t a = 0; a < mymission->GetBases().GetCapacity(); ++a) {
			Vector vec;
			const CBase *atual = &mymission->GetBases()[a];
			vec = atual->GetPosition();
			vec.y += 80;
			this->cam->ProjectVector(&vec, &vec);
			if(vec.z <= 1.0f) {
				D3DCOLOR backcolor;
				Smart::String tmp;
				float alpha;
				char buf[31];
				alpha = Distance(this->cam->GetPosition(),atual->GetPosition());
				alpha /= 3000.0;
				alpha = 1-min(alpha, 1);
				FontPosition.left = static_cast<LONG>(vec.x)-(gTxtmetrics.tmAveCharWidth*atual->Dados()->getNome().GetLength())/2;
				FontPosition.top = static_cast<LONG>(vec.y);
				if(atual->Dona())
					backcolor = atual->Dona()->getColor();
				else
					backcolor = 0xFFFFFFFF;

				backcolor = backcolor & 0x00FFFFFF | static_cast<unsigned int>(alpha*255) << 24;

				g_Font->DrawText(fontsprlayer, atual->Dados()->getNome().c_str(), -1, &FontPosition, 0, backcolor);
			}
		}

		if(GameTicks < this->texttime) {
			FontPosition.left = 0;
			FontPosition.top = 50;

			g_Font->DrawText(fontsprlayer, this->texto.c_str(), -1, &FontPosition, 0, this->textcolor);
		}
		//g_Font->DrawText(fontsprlayer, tmps.c_str(), -1, &FontPosition, 0, 0xFFFFFFFF);
		fontsprlayer->End();
	}

	FontPosition.left = 0;
	FontPosition.top = 0;

	PROFILE_END;
	render_t = GetTime()-t;
}

void CJogador::Limpar()
{
	TRACE("Limpando jogador %p...\n", this);
	SafeDelete(this->info);
	SafeDelete(this->controle);
	SafeDelete(this->cam);
}

void CJogador::Criar(D3DVIEWPORT9 &viewport, ControlTipo tipo)
{
	this->controle = new CControle(tipo);

	this->nome = "Rei da cocada preta";

	this->cam = new CCamera();
	this->cam->SetupPhysics();
	this->cam->Position(0, 0, 0);
	this->CameraTipo = 0;
	this->cam->setViewport(viewport);
	TRACE("Jogador %p batizado com o nome \"%s\".\n", this, this->nome);
}

