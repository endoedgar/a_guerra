#include "stdafx.h"
#include "GameGlobals.h"
#include "CTeclado.h"
#include "GenericFunctions.h"

LPDIRECTINPUT dinput = NULL;
LPDIRECTINPUTDEVICE key_dinput_device = NULL;
BYTE diKeys[256];
#define DINPUT_BUFFERSIZE 256

HRESULT InicializaDInput()
{
	HRESULT hr;
   DIPROPDWORD property_buf_size =
   {
      /* the header */
      {
         sizeof(DIPROPDWORD),  // diph.dwSize
         sizeof(DIPROPHEADER), // diph.dwHeaderSize
         0,                     // diph.dwObj
         DIPH_DEVICE,           // diph.dwHow
      },

      /* the data */
      DINPUT_BUFFERSIZE,         // dwData
   };

	hr = DirectInput8Create(g_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<VOID**>(&dinput), NULL);
	if(SUCCEEDED(hr)) {
		hr = dinput->CreateDevice(GUID_SysKeyboard, &key_dinput_device, NULL);
		if(SUCCEEDED(hr)) {
			hr = key_dinput_device->SetDataFormat(&c_dfDIKeyboard);
			if(SUCCEEDED(hr)) {
				hr = key_dinput_device->SetProperty(DIPROP_BUFFERSIZE, &property_buf_size.diph);
				if(SUCCEEDED(hr)) {
					hr = key_dinput_device->Acquire();
					if(SUCCEEDED(hr))
						TRACE("Teclado adquirido com sucesso!\n");
				} else
					TRACE("Falha ao mudar propriedade!\n");
			} else {
				TRACE("Falha ao mudar formato de dados do dispositivo do teclado!");
			}
		} else
			TRACE("Falha ao adquirir dispositivo do teclado!\n");
	} else
		TRACE("Falha ao criar objeto DirectInput!\n");
	return hr;
}

void HandleKey(unsigned char scancode, const int pressed)
{
	if(pressed) {
		keys[scancode] = GameTicks;
	} else {
		keys[scancode] = 0;
	}
}

HRESULT PollKeyboard()
{
	static DIDEVICEOBJECTDATA scancode_buffer[DINPUT_BUFFERSIZE];
	DWORD waiting_scancodes;
	HRESULT hr;

	waiting_scancodes = DINPUT_BUFFERSIZE;

	hr = key_dinput_device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), scancode_buffer, &waiting_scancodes, 0);

	if ((hr == DIERR_NOTACQUIRED) || (hr == DIERR_INPUTLOST)) {
		hr = key_dinput_device->Acquire();
	} else if(FAILED(hr)) {
		RuntimeError("Erro inesperado no tratamento de teclas do teclado %d!\n", HRESULT_CODE(hr));
	} else {
		for(register unsigned int i = 0; i < waiting_scancodes; ++i) {
			HandleKey(static_cast<unsigned char>(scancode_buffer[i].dwOfs & 0xFF), scancode_buffer[i].dwData & 0x80);
		}
	}
	return hr;
}

HRESULT FinalizaDInput()
{
	HRESULT hr;
	if(key_dinput_device) {
		key_dinput_device->Unacquire();
		hr = key_dinput_device->Release();
		if(SUCCEEDED(hr)) {
			key_dinput_device = NULL;
		} else
			RuntimeError("Não foi possível liberar o teclado!");
	}
	SafeRelease(dinput);
	return S_OK;
}
