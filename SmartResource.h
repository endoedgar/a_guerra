#ifndef _SMARTPTR_H
#define _SMARTPTR_H

#pragma once

#include "assert.h"
#include "SharedResource.h"

namespace Smart
{
	#define CONST const

	template<class T>
	class SimpleRes abstract
	{
		protected:
			SharedResource *ptr;

			SimpleRes(SharedResource* address = NULL):ptr(address)
			{}

			SimpleRes(SharedResource &address):ptr(&address)
			{}
		public:
			T *operator->()
			{
				assert(ptr);
				return ptr;
			}

			T &operator*()
			{
				assert(ptr);
				return *ptr;
			}

			bool operator!()
			{
				return (!ptr) ? true : false;
			}

			void Clear()
			{
				if(this->ptr)
					this->ptr->Release();
				this->ptr = NULL;
			}
	};

	class Res : public SimplePtr<T>
	{
	public:
		Res(SharedResource* address):SimpleRes(address)
		{ }

		Res &operator=(SharedResource* anotherptr)
		{
			if(this->ptr)
				this->ptr->Release();
			this->ptr = anotherptr;

			return *this;
		}

		~Res()
		{
			if(ptr)
				ptr->Release();
		}
	};

	class SharedRes : public SimpleRes
	{
		size_t *refs;
	public:
		SharedRes(SharedResource *address):refs(new size_t(0)), SimpleRes(address)
		{ }

		SharedRes(SharedPtr &ptr):refs(ptr.refs), SimpleRes(ptr.ptr)
		{ (*this->refs)++;}

		~SharedRes()
		{
			if(!(*refs)) {
				delete this->refs;
				if(this->ptr)
					this->ptr->Release();
			} else
				(*refs)--;
		}
	};		

	#undef CONST
}

#endif