#ifndef _RADIO_H
#define _RADIO_H

#pragma once

class CObjeto;

enum FalasBot {
	RADIO_AJUDA,
	RADIO_AJUDAR,
	RADIO_INIMIGO_FUGIU ,
	RADIO_LIDER_MANDANDO_ATACAR,
	RADIO_LIDER_MANDOU_ATACAR,
	RADIO_LIDER_MORTO,
	RADIO_LIDER_PUXANDO_SACO,
	RADIO_LIDER_MANDOU_ESPALHAR,
	RADIO_MATOU_INIMIGO,
	RADIO_NAO,
	RADIO_NINGUEM_NO_RADAR,
	RADIO_PREOCUPADO_COM_O_LIDER,
	RADIO_PUXANDO_SACO_DO_LIDER,
	RADIO_SIM,
	RADIO_NO_MEU_CAMINHO,
	RADIO_OBRIGADO,
	RADIO_DANO,
	RADIO_MORREU,
	RADIO_NOVO_LIDER,
	RADIO_MAX
};

enum RADIO_ESTADO {
	RADIO_SILENCIO,
	RADIO_BIP1,
	RADIO_FALANDO,
	RADIO_BIP2
};

enum PerguntaBot {
	PERGUNTA_AJUDANTE,
	PERGUNTA_STATUS,
	PERGUNTA_ATACAR,
	PERGUNTA_FORMACAO,
	PERGUNTA_SOBRE_INIMIGOS,
	PERGUNTA_RESPOSTA_SOBRE_INIMIGOS,
	PERGUNTA_SEPARAR,
	PERGUNTA_PROVOCACAO,
	PERGUNTA_IMPRESSIONAR_LIDER,
	PERGUNTA_IMPRESSIONAR_GRUPO,
	PERGUNTA_MAX
};

class SomRadio {
public:
	CSound *som;
	FalasBot tipo;

	SomRadio():som(NULL)
	{
	}

	~SomRadio()
	{
		SafeRelease(som);
	}
};

struct Pergunta {
	PerguntaBot id;
	unsigned int expira, responder;
	CObjeto **asker;
};

extern CSimpleListAP<Pergunta> *Perguntas;
extern LPDIRECTSOUNDBUFFER radio_sound_channel;
extern CObjeto **objfalante;

void ObjetoPerguntarPara(const CObjeto *obj, PerguntaBot id, CObjeto *para, unsigned int expira = 10000, unsigned int responder = 3000);
void InicializaSomRadios();
void TocarSomRadio(FalasBot tipo, const CObjeto *obj, int prioridade = 0);
void LiberarSomRadios();
void ObjetoPerguntarSobrePosicaoInimiga(CObjeto *obj);
void ObjetoResponderSobrePosicaoInimiga(CObjeto *obj, CObjeto *preocupado);
void ObjetoAgradecer(CObjeto *obj, CObjeto *Ajudante);
void ObjetoFalarSobrePerdaDeLider(CObjeto *obj);
void ObjetoFalarSobreNovoLider(CObjeto *obj);
void ObjetoFalarSobreIrEmbora(CObjeto *obj);
void ObjetoLiderMandouAtacar(CObjeto *obj);
void ObjetoResponderSobreAtaque(CObjeto *obj);
void EfeitoRadio();
void PedirAjuda(CObjeto *obj);
void ObjetoFalarSobrePedirAjuda(CObjeto *obj);
void ObjetoResponderPedidoDeAjuda(CObjeto *obj, CObjeto *necessitado);

#endif