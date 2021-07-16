#pragma once

class D3DDevice
{
	LPDIRECT3DDEVICE9 d3dDevice;
public:
	D3DDevice(void);
	void OnCreateDevice();
	void OnResetDevice();
	void OnLostDevice();
	void OnDestroyDevice();
	~D3DDevice(void);
};
