#ifndef MCI_PLAYER_H

class MCIPlayer
{
	Smart::String buffer;
	Smart::String music;
	int nbuffer;
	bool FExiste(const Smart::String &);
	bool m_pOpened, m_pPlaying;
	public:
		bool Abrir(const Smart::String &, const Smart::String &);
		MCIPlayer(const int);
		~MCIPlayer(void);
		int Tocando();
		bool Aberto() const {
			return this->m_pOpened;
		}
		bool TocandoAgora() const {
			return this->m_pPlaying;
		}
		const Smart::String &GetCurrentMusic() const
		{ return this->music; }
		void Comando(const Smart::String &);
		void Tocar();
		void Parar();
		void Aguardar();
		void Fechar();
};

#define MCI_PLAYER_H
#endif