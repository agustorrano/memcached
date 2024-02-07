#include "common.h"

int valid_rq(int code)
{
	return code == PUT || code == GET || code == DEL || code == STATS;
}

const char *code_str(enum code e)
{
	switch (e)
	{
	case PUT:
		return "PUT";
	case GET:
		return "GET";
	case DEL:
		return "DEL";

	case STATS:
		return "STATS";

	case OK:
		return "OK";
	case EINVALID:
		return "EINVALID";
	case ENOTFOUND:
		return "ENOTFOUND";
	case EBINARY:
		return "EBINARY";
	case EBIG:
		return "EBIG";
	case EUNK:
		return "EUNK";
	case EOOM:
		return "EOOM";
	default:
		assert(0);
		return "";
	}
}