#include "stdafx.h"
#include "SharedResource.h"
#include "GameGlobals.h"
#include "D3DObject.h"

SharedResource::SharedResource(void):refs(0)
{ }

HRESULT SharedResource::Release() {
	HRESULT hr = S_OK;
	if(this->NeedCleanup()) {
		CResource *r = dynamic_cast<CResource*>(this);
		mywrapper->RemoveResource(r);
		delete this;
	} else {
		this->SubRef();
	}
	return hr;
}

unsigned long SharedResource::AddRef()
{
	return ++this->refs;
}

SharedResource::~SharedResource(void)
{
	TRACE("%p::~SharedResource() called!\n", this);
}
