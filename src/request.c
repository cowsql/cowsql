#include "request.h"

#define REQUEST__IMPLEMENT(LOWER, UPPER, _) \
	SERIALIZE__IMPLEMENT(request_##LOWER, REQUEST_##UPPER);

REQUEST__TYPES(REQUEST__IMPLEMENT, );

SERIALIZE__IMPLEMENT(request_connect, REQUEST_CONNECT);
SERIALIZE__IMPLEMENT(request_assign, REQUEST_ASSIGN);
