#include "stdafx.h"
#include "GenericFunctions.h"
#include "INI Parser.h"
#include "Equipe.h"
#include "missao.h"
#include "3DPlane.h"
#include "Ambiente.h"
#include "Base.h"
#include "loader.h"
#include "ManterDir.h"

using namespace std;

ErrorInfo CMissaoDados::lastError;
const CEquipe *CMissaoDados::EncontrarEquipe(const Smart::String &equipedesejada) const
{
	for(register Smart::LinkedList<const CEquipe*>::LinkedNode *node = this->pEquipesParticipantes.GetFirst(); node; node = node->GetNext())
		if(!(*node).GetData()->getNome().cmpi(equipedesejada))
			return node->GetData();
	return NULL;
}
CMissaoDados::~CMissaoDados()
{
	//SafeRelease(this->terreno);
}

const CEquipe *CMissaoDados::AdicionarEquipe(const Smart::String &equipedesejada)
{
	const CEquipe *novaequipe = this->EncontrarEquipe(equipedesejada);
	if(!novaequipe) {
		for(register unsigned int a = 0; a < eDM->getSize(); ++a)
			if(!(*eDM)[a]->getNome().cmpi(equipedesejada)) {
				novaequipe = (*eDM)[a];
				this->pEquipesParticipantes.RInsert(novaequipe);
				break;
			}
	}
	return novaequipe;
}

const CBaseTipoDados *CMissaoDados::EncontrarBaseTipo(const Smart::String &tbasedesejada) const
{
	for(register Smart::LinkedList<CBaseTipoDados>::LinkedNode *node = this->pBasesTipos.GetFirst(); node; node = node->GetNext())
		if(!(*node)->getNome().cmpi(tbasedesejada))
			return &node->GetData();
	return NULL;
}

const CBaseDados *CMissaoDados::EncontrarBase(const Smart::String &basedesejada) const
{
	for(register Smart::LinkedList<CBaseDados>::LinkedNode *node = this->pBases.GetFirst(); node; node = node->GetNext())
		if(!(*node)->getNome().cmpi(basedesejada))
			return &node->GetData();
	return NULL;
}

bool CMissaoDados::CarregarBases(const INIParser::INIFile &myfile)
{
	const size_t nTBases = myfile.Get<size_t>("Tipos de Bases", "n", 0);
	const Smart::String szBases("Bases");

	for(register size_t a = 0; a < nTBases; ++a) {
		Smart::String Secao;
		missionstream ss;
		ss << "Tipo de Base " << a;
		Secao = ss.str().c_str();
		Smart::String nome = myfile.GetString(Secao, "nome", "");

		if(nome != "") {
			
			Smart::LinkedList<CBaseTipoDados>::LinkedNode *node = this->pBasesTipos.RInsert(new CBaseTipoDados);
			if(!node)
				return false;

			CBaseTipoDados &btipo = node->GetData();

			btipo.setNome(nome);
			btipo.setRaioDeInfluencia(myfile.Get<real>(Secao, "raio", 1000));
			btipo.setVelDeConquista(myfile.Get<real>(Secao, "vconquista", 1));
			btipo.setRepairAmount(myfile.Get<real>(Secao, "rquantia", r(0.1)));
			Smart::String gen = myfile.GetString(Secao, "gen", "0 3 1");
			size_t gentime = 60*GetSub<size_t>(gen, 2);
			gentime = max(gentime, 600);
			btipo.Gen(GetSub<size_t>(gen, 0), GetSub<size_t>(gen, 1), gentime);

			const size_t nObjetos = myfile.Get<size_t>(Secao, "nobjetos", 0);
			for(register size_t i = 0; i < nObjetos; ++i) {
				missionstream ss2;
				Smart::String ob;
				ss2 << i;
				ob = ss2.str().c_str();
				const Smart::String valor = myfile.GetString(Secao, ob);
				const Smart::String objname = GetSubString(valor, 0);
				DadosDaUnidade *obj = oDataManager->find(objname);
				if(obj) {
					btipo.InserirUnidade(obj, GetSub<real>(valor, 1));
				} else {
					missionstream ss;
					ss << "Unidade " << objname.c_str() << " não encontrado!";
					CMissaoDados::lastError.set(ss.str().c_str());
					return false;
				}
			}
			btipo.NormalizarProbabilidades();
		} else {
			missionstream ss;
			ss << "Tipo de base " << a << " não encontrada!";
			CMissaoDados::lastError.set(ss.str().c_str());
			return false;
		}
	}

	const size_t nBases = myfile.Get<size_t>("Bases", "nbases", 0);

	for(register size_t a = 0; a < nBases; ++a) {
		Smart::LinkedList<CBaseDados>::LinkedNode *node = this->pBases.RInsert(new CBaseDados);
		if(!node)
			return false;

		CBaseDados &base = node->GetData();
		Smart::String Item, tmp;
		missionstream ss;
		ss << a;
		Item = ss.str().c_str();

		tmp = myfile.GetString(szBases, Item, "\"Base sem nome\" 0 0 0 Neutral Default");
		
		base.setNome(GetSubString(tmp, 0));
		base.setPosicao(GetSub<real>(tmp, 1), GetSub<real>(tmp, 2), GetSub<real>(tmp, 3));
		Smart::String Equipe = GetSubString(tmp, 4);
		if(Equipe.GetLength() >= 1)
			base.setFaccao(this->AdicionarEquipe(Equipe));
		else
			base.setFaccao(NULL);
		const CBaseTipoDados *btipo = this->EncontrarBaseTipo(GetSubString(tmp, 5));
		if(!btipo) {
			missionstream ss;
			ss << "Não foi possível encontrar o tipo de base " << GetSubString(tmp, 5).c_str() << "!";
			CMissaoDados::lastError.set(ss.str().c_str());
			return false;
		}
		base.setTipo(btipo);
	}
	return true;
}

bool FExist(const Smart::String &str)
{
	FILE *fp;

	fp = fopen(str.c_str(), "r");

	if(fp) {
		fclose(fp);
		return true;
	}

	return false;
}

void CObjetivo::setNome(const Smart::String &nome)
{
	Smart::String tmp;
	for(register size_t i = 0; i < nome.GetLength(); ++i) {
		Smart::String::char_type c = nome[i];
		if(c == '\\') {
			if(nome[i+1]=='n') {
				tmp+='\n';
				++i;
			} else
				tmp+=c;
		} else
			tmp+=c;
	}
	NamedEntity::setNome(tmp);
	
}

bool CMissaoDados::CarregarObjetivos(const INIParser::INIFile &myfile)
{
	CManterDir cmd;

	SetCurrentDirectory(gamedir.c_str());
	if(this->pObjetivos.SetSize(myfile.Get<size_t>("missao", "nObjetivos", 0))) {
		for(register size_t a = 0; a < this->pObjetivos.GetCapacity(); ++a) {
			missionstream ss;
			CObjetivo *objetivo = &this->pObjetivos[a];
			ss << "Objetivo " << a;
			const Smart::String id = ss.str().c_str();

			objetivo->setNome(myfile.GetString(id, "nome", "NONAME"));
			size_t nTarefas = myfile.Get<int>(id, "nTarefas", 0);
			objetivo->maxSec = myfile.Get<int>(id, "tempo", 0);
			Smart::String mstr = myfile.GetString(id, "musica");
			
			if(mstr.GetLength() >= 1) {
				objetivo->szMusic = mstr;
			}
			objetivo->admv = FExist(objetivo->szMusic);
			nTarefas = max(0, nTarefas);
			if(nTarefas <= 0) {
				missionstream ss;
				ss << "Número de tarefas inválido no objetivo " << a << "!";
				CMissaoDados::lastError.set(ss.str().c_str());
				return false;
			}

			objetivo->SetNTask(nTarefas);

			for(register size_t b = 0; b < objetivo->GetSize(); ++b) {
				missionstream ss2;
				CTarefaDados *tarefa = &objetivo->pTarefas[b];
				ss2 << ss.str();
				ss2 << " Tarefa " << b;
				const Smart::String tid = ss2.str().c_str();

				tarefa->setNome(myfile.GetString(tid, "nome", "NONAME"));
				Smart::String str = myfile.GetString(tid, "tipo", "");
				if(str != "") {
					if(str == "TAREFA_DESTRUIR")
						tarefa->SetType(TAREFA_DESTRUIR);
					else if(str == "TAREFA_PROTEGER")
						tarefa->SetType(TAREFA_PROTEGER);
					else if(str == "TAREFA_ESPERAR")
						tarefa->SetType(TAREFA_ESPERAR);
					else if(str == "TAREFA_DOMINACAO")
						tarefa->SetType(TAREFA_DOMINACAO);
					else if(str == "TAREFA_CAPTURAR")
						tarefa->SetType(TAREFA_CAPTURAR);
					else
						tarefa->SetType(TAREFA_DEFENDER_BASE);

					const size_t id = myfile.Get<int>(tid.c_str(), "valor", 0);
					if(tarefa->GetType() != TAREFA_DOMINACAO) {
						if(tarefa->GetType() == TAREFA_CAPTURAR || tarefa->GetType() == TAREFA_DEFENDER_BASE) {
							if(id >= 0 && id < this->pBases.GetCapacity()) {
								tarefa->setBase(id);
							} else {
								missionstream ss;
								ss << "ID Base inválida no objetivo " << a << " tarefa " << b << "!";
								CMissaoDados::lastError.set(ss.str().c_str());
								return false;
							}
						} else {
							if(id >= 0 && id < this->pGObjetos.GetCapacity()) {
								if(this->pGObjetos[id].objetivo <= a) {
									tarefa->SetObjGroup(&this->pGObjetos[id]);
								} else {
									CMissaoDados::lastError.set("GObjetivo do Grupo ID invalido!");
									return false;
								}
							} else {
								missionstream ss;
								ss << "ID Grupo inválido no objetivo " << a << " tarefa " << b << "!";
								CMissaoDados::lastError.set(ss.str().c_str());
								return false;
							}
						}
					}
				} else {
					missionstream ss;
					ss << "Tipo de tarefa inválida no objetivo " << a << " tarefa " << b << "!";
					CMissaoDados::lastError.set(ss.str().c_str());
					return false;
				}
			}
		}
	}
	return true;
}

bool CMissaoDados::CarregarGrupoDeObjetos(const INIParser::INIFile &myfile)
{
	Smart::String tmp;

	if(this->pGObjetos.SetSize(myfile.Get<size_t>("missao", "nGObjetos", 0))) {
		for(register size_t a = 0; a < this->pGObjetos.GetCapacity(); ++a) {
		#ifdef UNICODE
			wstringstream ss;
		#else
			stringstream ss;
		#endif
			GrupoObjetoMissaoDados *gom = &this->pGObjetos[a];

			ss << "GObjetos " << a;
			tmp = ss.str().c_str();
			const Smart::String equipe(myfile.GetString(tmp, "equipe"));

			gom->setNome(myfile.GetString(tmp, "nome", "NONAME"));
			gom->objetivo = myfile.Get<int>(tmp, "objetivo", 0);
			gom->team = this->AdicionarEquipe(equipe);
			gom->index = a;
			if(!gom->team) {
				missionstream ss;
				ss << "Grupo de Unidades " << a << " não tem uma equipe válida (" << equipe.c_str() << ")!";
				CMissaoDados::lastError.set(ss.str().c_str());
				return false;
			}
			gom->nUnidades = myfile.Get<int>(tmp, "nObjetos", 0);
			if(gom->nUnidades > 0) {
				if(gom->unidades.SetSize(gom->nUnidades)) {
					for(register size_t b = 0; b < gom->nUnidades; ++b) {
						UnidadeMissao *om = &gom->unidades[b];
						missionstream ss;
						ss << b;
						om->setNome(myfile.GetString(tmp, ss.str().c_str(), ""));
						if(om->getNome().GetLength() > 0) {
							om->setPosicao(GetSub<real>(om->getNome(), 1), GetSub<real>(om->getNome(), 2), GetSub<real>(om->getNome(), 3));
							om->setLife(GetSub<real>(om->getNome(), 4));
							om->setNome(GetSubString(om->getNome(), 0));

							om->setUD(oDataManager->find(om->getNome()));
							if(!om->getUD()) {
								missionstream ss;
								ss << "Dados da unidade " << om->getNome().c_str() << " não encontrados!";
								CMissaoDados::lastError.set(ss.str().c_str());
								return false;
							}
						} else {
							missionstream ss;
							ss << "Problema ao checar unidade " << b << " do grupo " << a << "!";
							CMissaoDados::lastError.set(ss.str().c_str());
							return false;
						}
					}
				} else {
					CMissaoDados::lastError.set("Memória insuficiente!");
					return false;
				}
			} else {
				missionstream ss;
				ss << "Número de objetos inválido no grupo " << a << "!";
				CMissaoDados::lastError.set(ss.str().c_str());
				return false;
			}
		}
	}
	return true;
}

bool CMissaoDados::CarregarTerreno(const INIParser::INIFile &file)
{
	const Smart::String secaoGeral = "geral", secaoFisico = "fisico", secaoMapas = "mapas";
	Smart::String tmp;
	this->mapaFilename = file.GetString("missao", "mapa", "mapapadrao");
	const Smart::String folder = "mapas\\" + this->mapaFilename + '\\';
	INIParser::INIFile fmap;
	CManterDir cmd;
	tmp = folder;
	tmp += this->mapaFilename;
	tmp += ".ini";

	SetCurrentDirectory(gamedir.c_str());

	if(fmap.open(tmp)) {
		tmp = fmap.GetString(secaoGeral, "ambiente", "AmbienteIndefinido");
		this->ambdados = adm->find(tmp);
		if(this->ambdados) {
			this->terrainFriction = fmap.Get<real>(secaoFisico, "friccao", 0.1);
			tmp = fmap.GetString(secaoFisico, "posicao", "0 0 0"); 
			this->terrainPosition = Vector(GetSub<real>(tmp, 0), GetSub<real>(tmp, 1), GetSub<real>(tmp, 2));
			tmp = fmap.GetString(secaoFisico, "escala", "1 1 1"); 
			this->marPosition = fmap.Get<real>(secaoFisico, "mar", 0)*GetSub<real>(tmp, 1);
			this->terrainScale = Vector(GetSub<real>(tmp, 0), GetSub<real>(tmp, 1), GetSub<real>(tmp, 2));
			this->terrainPath = folder+fmap.GetString(secaoMapas, "h");
			this->terrainDetailTexturePath = folder+fmap.GetString(secaoMapas, "d");
			this->terrainTexturePath = folder+fmap.GetString(secaoMapas, "t");
			return true;

			/*if(SUCCEEDED(this->terreno->LoadTerrain(filepath))) {
				filepath = folder+fmap.GetString(secaoMapas, "t");
				CTexture *tmp = CTexture::Load(filepath);
				if(tmp) {
					this->terreno->setTexture(tmp);
					tmp->Release();
					filepath = folder+fmap.GetString(secaoMapas, "d");
					tmp = CTexture::Load(filepath);
					if(tmp) {
						this->terreno->setDetailTexture(tmp);
						tmp->Release();
						fmap.close();
						return true;
					} else {
						missionstream ss;
						ss << "Textura de detalhe \"" << filepath.c_str() << "\" não encontrada!";
						CMissaoDados::lastError.set(ss.str().c_str());
					}
				} else {
					missionstream ss;
					ss << "Textura \"" << filepath.c_str() << "\" não encontrada!";
					CMissaoDados::lastError.set(ss.str().c_str());
				}
			} else {
				missionstream ss;
				ss << "Heightmap \"" << filepath.c_str() << "\" não encontrado!";
				CMissaoDados::lastError.set(ss.str().c_str());
			}*/
		} else {
			missionstream ss;
			ss << "Ambiente \"" << tmp.c_str() << "\" não encontrado!";
			CMissaoDados::lastError.set(ss.str().c_str());
		}
		fmap.close();
	} else {
		missionstream ss;
		ss << "Mapa \"" << tmp.c_str() << "\" não encontrado!";
		CMissaoDados::lastError.set(ss.str().c_str());
	}
	return false;
}

size_t CMissao::getNBases(const CEquipe *equipe) const {
	size_t nBases =0;
	for(register size_t a = 0; a < bases.GetCapacity(); ++a) {
		if(bases[a].Dona() == equipe)
			++nBases;
	}
	return nBases;
}

void CMissao::Reset()
{
	for(register size_t a = 0; a < cgom.GetCapacity(); ++a)
		cgom[a].Reset();
	for(register size_t a = 0; a < bases.GetCapacity(); ++a)
		bases[a].Reset();
}

void CMissao::Start(const CMissaoDados *cd)
{
	this->cgom.DestroyData();
	this->dados = NULL;


	if(cd) {
		this->dados = cd;
		this->cgom.Resize(this->dados->pGObjetos.GetCapacity());
		this->bases.Resize(this->dados->pBases.GetCapacity());
		Smart::LinkedList<CBaseDados>::LinkedNode *node = this->dados->pBases.GetFirst();
		for(register size_t a = 0; a < bases.GetCapacity(); ++a) {
			bases[a].Setup(&node->GetData(), this->GetDados()->hotspot);
			node = node->GetNext();
		}
		this->Reset();
	}
}

void CMissao::Update(const Matematica::real dt)
{
	for(register size_t a = 0; a < bases.GetCapacity(); ++a) {
		bases[a].Update(dt);
		if(bases[a].Dados()->getTipo()->getNUnidades())
			if(GameTicks % bases[a].Dados()->getTipo()->GenTime() == 0) {
				const size_t generate = this->bases[a].Dados()->getTipo()->MinGen()+random(this->bases[a].Dados()->getTipo()->MaxGen()-this->bases[a].Dados()->getTipo()->MinGen());
				for(register size_t b = 0; b < generate; ++b)
					bases[a].Reinforcements();
			}
	}
}

void CMissao::Render()
{
	for(register size_t a = 0; a < bases.GetCapacity(); ++a)
		bases[a].Render();
}

bool CMissaoDados::Unload() {
	return true;
}

CMissaoDados *MissionDataManager::LoadData(const Smart::String &filename) {
	INIParser::INIFile myfile;
	CManterDir cmd;

	if(myfile.open(filename)) {
		CMissaoDados *mymission = new CMissaoDados(myfile.GetString("missao", "nome", "NONAME"));

		if(mymission) {
			Smart::String tmp;
			tmp = myfile.GetString("missao", "p1", "0 0 0 1");
			mymission->p1pos.x = GetSub<real>(tmp, 0);
			mymission->p1pos.y = GetSub<real>(tmp, 1);
			mymission->p1pos.z = GetSub<real>(tmp, 2);
			mymission->p1life = GetSub<real>(tmp, 3);
			tmp = myfile.GetString("missao", "hotspot", "0 0 0");
			mymission->hotspot.x = GetSub<real>(tmp, 0);
			mymission->hotspot.y = GetSub<real>(tmp, 1);
			mymission->hotspot.z = GetSub<real>(tmp, 2);

			if(mymission->CarregarTerreno(myfile))
				if(mymission->CarregarBases(myfile))
					if(mymission->CarregarGrupoDeObjetos(myfile))
						if(mymission->CarregarObjetivos(myfile)) {
							return mymission;
						}
		}

		SafeDelete(mymission);
	} else
		CMissaoDados::lastError.set("Arquivo não encontrado!");
	return NULL;
}

void MissionDataManager::OnPostLoad() {
}