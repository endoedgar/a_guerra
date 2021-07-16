#ifndef _NAMED_ENTITY_H
#define _NAMED_ENTITY_H

#pragma once

class NamedEntity {
private:
	Smart::String nome;
	
public:
	NamedEntity()
	{ }

	NamedEntity(const Smart::String &nome)
	{ this->nome = nome; }

	const Smart::String &getNome() const
	{ return this->nome; }

	void setNome(const Smart::String &nome)
	{ this->nome = nome; }
};

#endif