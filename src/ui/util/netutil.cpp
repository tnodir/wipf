#include "netutil.h"

#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>

struct sock_addr {
  union {
    struct sockaddr addr;
    struct sockaddr_in in;
  } u;
  int addrlen;
};

#define MAX_IPV4_LEN    20
#define SOCK_ADDR_LEN   offsetof(struct sock_addr, addrlen)

#define sock_addr_get_inp(sap) \
    ((void *) &(sap)->u.in.sin_addr)

NetUtil::NetUtil(QObject *parent) :
    QObject(parent)
{
}

quint32 NetUtil::textToIp4(const QString &text, bool *ok)
{
    quint32 ip4 = 0;
    void *p = &ip4;

    const bool res = InetPtonW(AF_INET, (PCWSTR) text.utf16(), p) == 1;

    if (ok) {
        *ok = res;
    }

    return ntohl(*((unsigned long *) p));
}

QString NetUtil::ip4ToText(quint32 ip)
{
    quint32 ip4 = htonl((unsigned long) ip);
    const void *p = &ip4;
    wchar_t buf[MAX_IPV4_LEN];

    if (!InetNtopW(AF_INET, p, buf, MAX_IPV4_LEN))
        return false;

    return QString::fromWCharArray(buf);
}