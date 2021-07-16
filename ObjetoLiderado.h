#pragma once

class CObjeto;

class ObjetoLiderado
{
	CObjeto **lider;
	unsigned char squads;
public:
	inline CObjeto **getLiderPtr() const
	{ return this->lider; }

	inline CObjeto *getLider() const
	{ return (this->lider) ? *this->lider : NULL; }

	ObjetoLiderado(void);
	~ObjetoLiderado(void);
	void setLider(CObjeto ** lider);

	inline unsigned char getSquadNum(void) const
	{ return this->squads; }

	inline bool isLider() const
	{ return this->squads > 0; }
};
