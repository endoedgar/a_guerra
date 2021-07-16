#ifndef _GLOBAL_HEADER_H
#define _GLOBAL_HEADER_H

#pragma once

#if defined(DEBUG) | defined(_DEBUG)
	#define new new(_NORMAL_BLOCK,__FILE__, __LINE__)
#endif

#endif