/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include "rpc_authentication.h"
#include <stdio.h>
#include <stdlib.h>
#include <rpc/pmap_clnt.h>
#include <string.h>
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef SIG_PF
#define SIG_PF void(*)(int)
#endif

void
authentication_prog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	union {
		char *request_authorization_1_arg;
		struct access_token_request request_access_token_1_arg;
		struct validate_delegated_action_request validate_delegated_action_1_arg;
		char *aprove_request_token_1_arg;
	} argument;
	char *result;
	xdrproc_t _xdr_argument, _xdr_result;
	char *(*local)(char *, struct svc_req *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
		return;

	case request_authorization:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_request_authorization_response;
		local = (char *(*)(char *, struct svc_req *)) request_authorization_1_svc;
		break;

	case request_access_token:
		_xdr_argument = (xdrproc_t) xdr_access_token_request;
		_xdr_result = (xdrproc_t) xdr_access_token_response;
		local = (char *(*)(char *, struct svc_req *)) request_access_token_1_svc;
		break;

	case validate_delegated_action:
		_xdr_argument = (xdrproc_t) xdr_validate_delegated_action_request;
		_xdr_result = (xdrproc_t) xdr_validate_delegated_action_response;
		local = (char *(*)(char *, struct svc_req *)) validate_delegated_action_1_svc;
		break;

	case aprove_request_token:
		_xdr_argument = (xdrproc_t) xdr_wrapstring;
		_xdr_result = (xdrproc_t) xdr_aprove_request_token_response;
		local = (char *(*)(char *, struct svc_req *)) aprove_request_token_1_svc;
		break;

	default:
		svcerr_noproc (transp);
		return;
	}
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
		return;
	}
	result = (*local)((char *)&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {
		svcerr_systemerr (transp);
	}
	if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		fprintf (stderr, "%s", "unable to free arguments");
		exit (1);
	}
	return;
}
