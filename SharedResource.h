#pragma once

class SharedResource
{
	unsigned long refs;
	inline bool NeedCleanup()
	{ return !this->refs; }

	unsigned long SubRef()
	{ return --refs; }
protected:
	virtual ~SharedResource(void);
public:

	inline unsigned long getRefs(void) const
	{ return refs; }

	SharedResource(void);

	HRESULT Release();
	unsigned long AddRef();
};
