#pragma once

#include <QtCore/qglobal.h>

#if defined(JSONIPC_LIBRARY)
#  define JSONIPC_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define JSONIPC_SHARED_EXPORT Q_DECL_IMPORT
#endif
