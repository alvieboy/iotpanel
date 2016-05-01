#ifndef __ERROR_H__
#define __ERROR_H__


#define NOERROR          0
#define EOUTOFBOUNDS     -1
#define EINVALIDARGUMENT -2
#define EINVALIDPROPERTY -3
#define ENOTFOUND        -4
#define EINTERNALERROR   -5
#define EINVALIDMAGIC    -6
#define EINVALIDCRC      -7
#define EUNKNOWN         -8
#define EALREADY         -9
#define ETOOMANY        -10
#define EMALFORMED      -11
#define EINVALIDLEN     -12
#define ETOOBIG         -13
#define ENOMEM          -14
#define ECONNECTIONCLOSED -15
#define EINVALIDCRED -16
const char *getErrorString(int errno);

#endif
