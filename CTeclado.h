#ifndef _CTECLADO_H
#define _CTECLADO_H

#pragma once

HRESULT InicializaDInput();
HRESULT PollKeyboard();
HRESULT FinalizaDInput();

#endif