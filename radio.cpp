#include "stdafx.h"
#include "objeto.h"
#include "GenericFunctions.h"
#include "GameGlobals.h"
#include "radio.h"
#include "3DFunctions.h"
#include "Jogador.h"

int prioridade_atual = 0;
CObjeto **objfalante = NULL;
LPDIRECTSOUNDBUFFER radio_sound_channel = NULL;
CSound *radio_speak = NULL;
unsigned int numerosons[FalasBot::RADIO_MAX];
RADIO_ESTADO esradio = RADIO_SILENCIO;

CSimpleList<SomRadio> *SR = NULL;
CSimpleListAP<Pergunta> *Perguntas = NULL;

void CarregarSonsRadio(const Smart::String &filepath, FalasBot tipo);

void InicializaSomRadios()
{
	ZeroMemory(numerosons, sizeof(unsigned int)*FalasBot::RADIO_MAX);
	SR = new CSimpleList<SomRadio>;
	Perguntas = new CSimpleListAP<Pergunta>;

	CarregarSonsRadio("sons\\bot\\ajuda", RADIO_AJUDA);
	CarregarSonsRadio("sons\\bot\\ajudar", RADIO_AJUDAR);
	CarregarSonsRadio("sons\\bot\\inimigofugindo", RADIO_INIMIGO_FUGIU);
	CarregarSonsRadio("sons\\bot\\lidermandandoatacar", RADIO_LIDER_MANDANDO_ATACAR);
	CarregarSonsRadio("sons\\bot\\lidermandouatacar", RADIO_LIDER_MANDOU_ATACAR);
	CarregarSonsRadio("sons\\bot\\lidermorto", RADIO_LIDER_MORTO);
	CarregarSonsRadio("sons\\bot\\liderpuxandosaco", RADIO_LIDER_PUXANDO_SACO);
	CarregarSonsRadio("sons\\bot\\lidermandouespalhar", RADIO_LIDER_MANDOU_ESPALHAR);
	CarregarSonsRadio("sons\\bot\\matou", RADIO_MATOU_INIMIGO);
	CarregarSonsRadio("sons\\bot\\nao", RADIO_NAO);
	CarregarSonsRadio("sons\\bot\\ninguem", RADIO_NINGUEM_NO_RADAR);
	CarregarSonsRadio("sons\\bot\\preocupadocomolider", RADIO_PREOCUPADO_COM_O_LIDER);
	CarregarSonsRadio("sons\\bot\\puxandosacodolider", RADIO_PUXANDO_SACO_DO_LIDER);
	CarregarSonsRadio("sons\\bot\\sim", RADIO_SIM);
	CarregarSonsRadio("sons\\bot\\nomeucaminho", RADIO_NO_MEU_CAMINHO);
	CarregarSonsRadio("sons\\bot\\obrigado", RADIO_OBRIGADO);
	CarregarSonsRadio("sons\\bot\\dano", RADIO_DANO);
	CarregarSonsRadio("sons\\bot\\morreu", RADIO_MORREU);
	CarregarSonsRadio("sons\\bot\\novolider", RADIO_NOVO_LIDER);
}

void LiberarSomRadios()
{
	SafeDelete(SR);
	SafeDelete(Perguntas);
}

int CarregarSomRadio(const Smart::String &filename, int attrib, void *ptr)
{
	FalasBot *tipo = ((FalasBot*)ptr);
	SomRadio *som = new SomRadio;
	som->tipo = *tipo;
	som->som = som->som->Load(filename);
	if(som->som) {
		++numerosons[*tipo];
		SR->Add(som);
		som->som->Release();
	}
	else
		SafeDelete(som);
	return 0;
}

void CarregarSonsRadio(const Smart::String &filepath, FalasBot tipo)
{
	Smart::String buffer;
	TRACE("Procurando sons na pasta: %s\n", filepath.c_str());
	buffer = filepath;
	buffer += "\\*.wav";
	for_each_file_ex(buffer,  0, FA_DIREC, CarregarSomRadio, &tipo); 
}

void TocarSomRadio(FalasBot tipo, const CObjeto *obj, int prioridade)
{
	if(g_Jogador->IsObjectValid())
		if(obj->temAlgumaRelacaoCom(g_Jogador->GetObject()))
			prioridade *= 2;
	prioridade /= g_Jogador->GetCamera()->getDistanceSq(obj)*0.0001;
	if(dsounddevice) {
		int nrepeat = 0;
		CSimpleNode<SomRadio> *node = SR->GetFirst();
		if(!g_Jogador->team || (obj->team != g_Jogador->team))
			return;
		if(radio_sound_channel) {
			if((SBPlaying(radio_sound_channel) && prioridade <= prioridade_atual))
				return;
			radio_sound_channel->Stop();
		}
		while(node) {
			if(node->GetPtr()->tipo == tipo) {
				nrepeat = (rand()%numerosons[tipo])-1;
				break;
			}
			node = node->GetNext();
		}
		while(nrepeat > 0) {
			--nrepeat;
			node = node->GetNext();
		}

		radio_speak = node->GetPtr()->som;
		prioridade_atual = prioridade;
		objfalante = obj->ptr->GetPtr();
	}
}

void EfeitoRadio()
{
	if(radio_speak) {
		if(esradio == RADIO_SILENCIO) {
				radio_sound_channel = radio_start->Play();
				esradio = RADIO_BIP1;
		} else if(esradio == RADIO_BIP1) {
			if(!SBPlaying(radio_sound_channel)) {
				if(objfalante && *objfalante) {
					radio_sound_channel = radio_speak->Play((*objfalante)->voz);
				} else
					radio_sound_channel = NULL;
				esradio = RADIO_FALANDO;
			}
		} else if(esradio == RADIO_FALANDO) {
			if(!SBPlaying(radio_sound_channel)) {
				radio_sound_channel = radio_end->Play();
				esradio = RADIO_BIP2;
			}
		} else {
			if(!SBPlaying(radio_sound_channel)) {
				radio_sound_channel = NULL;
				radio_speak = NULL;
				esradio = RADIO_SILENCIO;
			}
		}
	}
}

void ObjetoPerguntarPara(const CObjeto *obj, PerguntaBot id, CObjeto *para, unsigned int expira, unsigned int responder)
{
	//Pergunta *p = Perguntas->New()->GetPtr();
	obj->falarPara(para, id, expira, responder);
	//p->expira = GetTime()+expira/1000;
	//p->responder = GetTime()+responder/1000;
	//p->id = id;
	//p->asker = obj->ptr->GetPtr();
	//if(para)
	//	p->para = para->ptr->GetPtr();
	//else
	//	p->para = NULL;
}

void ObjetoLiderMandouAtacar(CObjeto *obj)
{
	if(obj->getSquadNum()) {
		TocarSomRadio(RADIO_LIDER_MANDANDO_ATACAR, obj, 3);
		ObjetoPerguntarPara(obj, PERGUNTA_ATACAR, NULL, 5000);
	}
}

void ObjetoResponderSobreAtaque(CObjeto *obj)
{
	CObjeto *tgt = obj->getLider()->GetTarget();
	if(obj->getLider() && tgt) {
		if(obj->podeAtacar(tgt)) {
			TocarSomRadio(RADIO_LIDER_MANDOU_ATACAR, obj);
			obj->setTarget(obj->getLider()->getTarget());
			obj->ai.state.set(FSM_ATTACK, 1000);
		} else {
			TocarSomRadio(RADIO_NAO, obj, -1);
		}
	}
}

void ObjetoPerguntarSobrePosicaoInimiga(CObjeto *obj)
{
	TocarSomRadio(RADIO_NINGUEM_NO_RADAR, obj, -1);
	ObjetoPerguntarPara(obj, PERGUNTA_SOBRE_INIMIGOS, NULL, 5000);
	obj->pergAjuda = GameTicks + 600;
}

void ObjetoResponderSobrePosicaoInimiga(CObjeto *obj, CObjeto *preocupado)
{
	TocarSomRadio(RADIO_NO_MEU_CAMINHO, obj, 0);
	ObjetoPerguntarPara(obj, PERGUNTA_RESPOSTA_SOBRE_INIMIGOS, preocupado, 5000);
}

void ObjetoAgradecer(CObjeto *obj, CObjeto *Ajudante)
{
	TocarSomRadio(RADIO_OBRIGADO, obj);
}

void PedirAjuda(CObjeto *obj) {
	ObjetoFalarSobrePedirAjuda(obj);
	ObjetoPerguntarPara(obj, PERGUNTA_AJUDANTE, NULL);
	obj->pergAjuda = GameTicks + 600;
}

void ObjetoFalarSobrePedirAjuda(CObjeto *obj) {
	TocarSomRadio(RADIO_AJUDA, obj, 5);
}

void ObjetoResponderPedidoDeAjuda(CObjeto *obj, CObjeto *necessitado) {
	obj->setLider(necessitado->ptr->GetPtr());
	obj->ai.delayTimeout(90000);
	obj->ai.state.set(FSM_FOLLOW, 1000);
	TocarSomRadio(RADIO_AJUDAR, obj);
}

void ObjetoFalarSobrePerdaDeLider(CObjeto *obj)
{
	TocarSomRadio(RADIO_LIDER_MORTO, obj, 2);
}

void ObjetoFalarSobreNovoLider(CObjeto *obj)
{
	int prioridade = 2;
	if(obj->getLider()) {
		CObjeto *l = obj->getLider();
		if(l->j) {
			prioridade *= 2;
			l->j->mostrarInformacao(esquadraoimg);
		}
	} 
	TocarSomRadio(RADIO_NOVO_LIDER, obj, prioridade);
}

void ObjetoFalarSobreIrEmbora(CObjeto *obj)
{
	int prioridade = 2;

	if(obj->esquadrao == obj->getLider()->esquadrao)
		TocarSomRadio(RADIO_NAO, obj, 4);
	else {
		if(obj->getLider()) {
			CObjeto *l = obj->getLider();
			if(l->j) {
				prioridade *= 2;
				l->j->mostrarInformacao(menosesquadraoimg);
			}
		} 
		TocarSomRadio(RADIO_SIM, obj);
		obj->ai.state.set(FSM_IDLE, 100);
		obj->setLider(NULL);
	}
}