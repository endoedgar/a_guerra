#ifndef _INLINELIST_H
#define _INLINELIST_H

#include "defines.h"

class CInlineNode {
	private:
		CInlineNode *m_pPrev, *m_pNext;
	public:
		inline CInlineNode *GetNext() const
		{ return this->m_pNext; }

		inline CInlineNode *GetPrevious() const
		{ return this->m_pPrev; }

		inline void SetNext(CInlineNode *node)
		{ this->m_pNext = node; }

		inline void SetPrevious(CInlineNode *node)
		{ this->m_pPrev = node; }
};

class CInlineList {
	private:
		CInlineNode *m_pFirst;
		unsigned int _nnode;

		inline void SetFirst(CInlineNode *node)
		{ this->m_pFirst = node; }

		inline void IncNode()
		{ ++this->_nnode; }

		inline void DecNode()
		{ --this->_nnode; }

	public:
		CInlineList():m_pFirst(NULL),_nnode(0) {
		}

		~CInlineList() {
		}

		inline CInlineNode *GetFirst() const
		{ return this->m_pFirst; }

		inline void Insert(CInlineNode *node)
		{
			if(this->GetFirst()) {
				this->GetFirst()->SetPrevious(node);
			}
			node->SetPrevious(NULL);
			node->SetNext(this->GetFirst());
			this->SetFirst(node);

			this->IncNode();
		}

		inline CInlineNode *Pop()
		{
			CInlineNode *ret = this->GetFirst();
			this->SetFirst(this->GetFirst()->GetNext());
			if(this->GetFirst())
				this->GetFirst()->SetPrevious(NULL);
			this->DecNode();
			return ret;
		}

		inline void DeleteNode(CInlineNode *node)
		{
			CInlineNode *lnext = node->GetNext();
			CInlineNode *lprev = node->GetPrevious();

			if(this->GetFirst() == node)
				this->SetFirst(lnext);
			if(lnext) {
				lnext->SetPrevious(lprev);
				if(lnext->GetPrevious())
					lnext->GetPrevious()->SetNext(lnext);
			}
			if(lprev) {
				lprev->SetNext(lnext);
				if(lprev->GetNext())
					lprev->GetNext()->SetPrevious(lprev);
			}
			this->DecNode();
		}

		inline void MoveNodeTo(CInlineNode *node, CInlineList *dest)
		{
			this->DeleteNode(node);
			dest->Insert(node);
		}

		inline unsigned int GetNodeNum() const
		{ return this->_nnode; }
};

template<class Type>
class CObjectManager {
	CInlineList *m_pActiveList, *m_pFreeList;
public:

	CObjectManager()
	{
		this->m_pActiveList = new CInlineList();
		this->m_pFreeList = new CInlineList();
	}

	inline Type *GetNext(const Type *Node) const
	{ return static_cast<Type*>(Node->GetNext()); }

	inline Type *GetFirstActive() const
	{ return static_cast<Type*>(this->m_pActiveList->GetFirst()); }

	inline Type *GetFirstInactive() const
	{ return static_cast<Type*>(this->m_pFreeList->GetFirst()); }

	void _CleanupManager()
	{
		while(this->m_pActiveList->GetFirst())
		{
			Type *pT = this->GetFirstActive();//static_cast<Type*>(this->m_pActiveList->GetFirst());
			this->m_pActiveList->Pop();
			if(pT)
				delete pT;
			pT = NULL;
		}
		while(this->m_pFreeList->GetFirst())
		{
			Type *pT = this->GetFirstInactive();//static_cast<Type*>(this->m_pFreeList->GetFirst());
			this->m_pFreeList->Pop();
			if(pT)
				delete pT;
			pT = NULL;
		}
	}

	~CObjectManager()
	{
		this->_CleanupManager();
		if(this->m_pActiveList)
			delete this->m_pActiveList;
		this->m_pActiveList = NULL;
		if(this->m_pFreeList)
			delete this->m_pFreeList;
		this->m_pFreeList = NULL;
	}

	void Precache(const unsigned int n)
	{
		Type *pT;
		for(register unsigned int i = 0; i < n; ++i)
		{
			if(NULL == (pT = new Type))
				return;
			this->m_pFreeList->Insert(pT);
		}
	}

	inline Type *New()
	{
		Type *pT;
		if(this->m_pFreeList->GetFirst())
			pT = static_cast<Type*>(this->m_pFreeList->Pop());
		else if(NULL == (pT = new Type))
			return NULL;
		this->m_pActiveList->Insert(pT);
		return pT;
	}

	inline void Delete(Type **Node)
	{
		Type *next = this->GetNext(*Node);
		this->m_pActiveList->MoveNodeTo(*Node, this->m_pFreeList);
		*Node = next;
	}

	inline void DeleteAll()
	{
		while(this->m_pActiveList->GetFirst())
		{
			Type *pT = this->GetFirstActive();//static_cast<Type*>(this->m_pActiveList->GetFirst());
			this->m_pActiveList->MoveNodeTo(pT, this->m_pFreeList);
		}
	}

	inline size_t GetNumActive() const
	{
		return this->m_pActiveList->GetNodeNum();
	}

};

template <class Type>
class CSimpleNode {
private:
	CSimpleNode *m_pNext, *m_pPrev;
	Type *m_pPtr;
public:
	CSimpleNode():m_pPtr(NULL)
	{
		m_pPtr = new Type;
	}

	~CSimpleNode()
	{
		SafeDelete(m_pPtr);
	}
	void SetPrev(CSimpleNode *prev)
	{ this->m_pPrev = prev; }

	void SetNext(CSimpleNode *nxt)
	{ this->m_pNext = nxt; }

	void SetPtr(Type *ptr)
	{ SafeDelete(this->m_pPtr);
	  this->m_pPtr = ptr; }

	void SetPtr(Type ptr)
	{
		/*if(!m_pPtr)
		{
			Type *nptr = new Type;
			*nptr = ptr;
		}
		this->m_pPtr = nptr;*/
		*this->m_pPtr = ptr;
	}

	CSimpleNode *GetPrev() const
	{ return this->m_pPrev; }

	CSimpleNode *GetNext() const
	{ return this->m_pNext; }

	Type *GetPtr() const
	{ return this->m_pPtr; }
};

template <class Type>
class CSimpleList {
private:
	CSimpleNode<Type> *m_pFirst;
	unsigned int nnode;
public:
	CSimpleList():m_pFirst(NULL), nnode(0)
	{
	}

	~CSimpleList()
	{
		CSimpleNode<Type> *n = this->GetFirst();
		while(n)
		{
			CSimpleNode<Type> *next = n->GetNext();
			SafeDelete(n);
			n = next;
			this->nnode--;
		}
	}

	void SetFirst(CSimpleNode<Type> *node)
	{ this->m_pFirst = node; }

	CSimpleNode<Type> *GetFirst() const
	{ return this->m_pFirst; }

	void Add(CSimpleNode<Type> *node)
	{
		node->SetPrev(NULL);
		node->SetNext(this->m_pFirst);
		if(this->m_pFirst)
			this->m_pFirst->SetPrev(node);

		this->SetFirst(node);
		
		++this->nnode;
	}

	void Add(Type *node)
	{
		CSimpleNode<Type> *novo = new CSimpleNode<Type>;
		novo->SetPtr(node);
		this->Add(novo);
	}

	void Delete(CSimpleNode<Type> *node)
	{
		CSimpleNode<Type> *lnext = node->GetNext();
		CSimpleNode<Type> *lprev = node->GetPrev();

		if(this->GetFirst() == node)
			this->SetFirst(lnext);
		if(lnext) {
			lnext->SetPrev(lprev);
			if(lnext->GetPrev())
				lnext->GetPrev()->SetNext(lnext);
		}
		if(lprev) {
			lprev->SetNext(lnext);
			if(lprev->GetNext())
				lprev->GetNext()->SetPrev(lprev);
		}

		this->nnode--;
	}

	inline CSimpleNode<Type> *Pop()
	{
		CInlineNode<Type> *ret = this->GetFirst();
		this->SetFirst(this->GetFirst()->GetNext());
		if(this->GetFirst())
			this->GetFirst()->SetPrevious(NULL);
		this->nnode--;
		return ret;
	}

	inline void MoveNodeTo(CSimpleNode<Type> *node, CSimpleList<Type> *dest)
	{
		this->Delete(node);
		dest->Add(node);
	}

	inline unsigned int GetNodeNum()
	{ return this->nnode; }
};

template <class Type>
class CSimpleListAP {
private:
	CSimpleList<Type> *m_pActiveList, *m_pFreeList;

public:
	CSimpleListAP():m_pActiveList(NULL), m_pFreeList(NULL)
	{
		this->m_pActiveList = new CSimpleList<Type>;
		this->m_pFreeList = new CSimpleList<Type>;
	}

	~CSimpleListAP()
	{
		SafeDelete(this->m_pActiveList);
		SafeDelete(this->m_pFreeList);
	}

	inline size_t GetNumActive() const
	{ return this->m_pActiveList->GetNodeNum(); }

	inline CSimpleNode<Type> *New()
	{
		CSimpleNode<Type> *n;
		if(this->m_pFreeList->GetFirst()) {
			n = this->m_pFreeList->GetFirst();
			this->m_pFreeList->MoveNodeTo(n, this->m_pActiveList);
		}
		else {
			if(NULL == (n = new CSimpleNode<Type>))
				return NULL;
			this->m_pActiveList->Add(n);
		}
		return n;
	}

	inline void Delete(CSimpleNode<Type> **Node)
	{
		CSimpleNode<Type> *next = (*Node)->GetNext();
		this->m_pActiveList->MoveNodeTo(*Node, this->m_pFreeList);
		*Node = next;
	}

	CSimpleNode<Type> *GetFirstActive()
	{
		return this->m_pActiveList->GetFirst();
	}
};

template <class Type>
class CInlineStack {
	class CStackItem {
		Type *ptr;
		CStackItem *nxt;

		public:
		CStackItem():ptr(NULL)
		{ }

		Type *GetItem()
		{ return ptr; }

		void SetItem(Type *p)
		{ this->ptr = p; }

		CStackItem *GetNext()
		{ return this->nxt; }

		void SetNext(CStackItem *n)
		{ this->nxt = n; }
	};
	CStackItem *lst, *rlist;
	unsigned int level;

public:
	CInlineStack():lst(NULL), rlist(NULL), level(0)
	{ }
	~CInlineStack()
	{
		while(this->lst)
			this->Pop();
		while(this->rlist) {
			CStackItem *nxt = this->rlist->GetNext();
			SafeDelete(this->rlist);
			this->rlist = nxt;
		}
	}

	void Push(Type *data)
	{
		CStackItem *item;
		if(rlist) {
			item = rlist;
			rlist = rlist->GetNext();
		}
		else {
			item = new CStackItem();
		}
		item->SetItem(data);
		item->SetNext(this->lst);
		this->lst = item;
		++this->level;
	}

	void Pop()
	{
		ASSERT(this->level)

		CStackItem *nxt = this->lst->GetNext();
		this->lst->SetNext(this->rlist);
		this->rlist = this->lst;
		this->lst = nxt;
		--this->level;
	}

	Type *Get()
	{
		ASSERT(this->lst);
		return this->lst->GetItem();
	}
};

#endif