/*
 * Author: Chen Minqiang <ptpt52@gmail.com>
 *  Date : Fri, 11 May 2018 14:20:56 +0800
 */
#ifndef _NATFLOW_COMMON_H_
#define _NATFLOW_COMMON_H_
#include <linux/version.h>
#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/netfilter.h>
#include <net/ip.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <net/icmp.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_core.h>
#include "natflow.h"

extern unsigned int debug;
extern unsigned int disabled;

#define IS_NATFLOW_FIXME() (debug & 0x10)
#define IS_NATFLOW_DEBUG() (debug & 0x8)
#define IS_NATFLOW_INFO() (debug & 0x4)
#define IS_NATFLOW_WARN() (debug & 0x2)
#define IS_NATFLOW_ERROR() (debug & 0x1)

#define NATFLOW_println(fmt, ...) \
	do { \
		printk(KERN_DEFAULT "{" MODULE_NAME "}:%s(): " pr_fmt(fmt) "\n", __FUNCTION__, ##__VA_ARGS__); \
	} while (0)

#define NATFLOW_FIXME(fmt, ...) \
	do { \
		if (IS_NATFLOW_FIXME()) { \
			printk(KERN_ALERT "fixme: " pr_fmt(fmt), ##__VA_ARGS__); \
		} \
	} while (0)

#define NATFLOW_DEBUG(fmt, ...) \
	do { \
		if (IS_NATFLOW_DEBUG()) { \
			printk(KERN_DEBUG "debug: " pr_fmt(fmt), ##__VA_ARGS__); \
		} \
	} while (0)

#define NATFLOW_INFO(fmt, ...) \
	do { \
		if (IS_NATFLOW_INFO()) { \
			printk(KERN_DEFAULT "info: " pr_fmt(fmt), ##__VA_ARGS__); \
		} \
	} while (0)

#define NATFLOW_WARN(fmt, ...) \
	do { \
		if (IS_NATFLOW_WARN()) { \
			printk(KERN_WARNING "warning: " pr_fmt(fmt), ##__VA_ARGS__); \
		} \
	} while (0)

#define NATFLOW_ERROR(fmt, ...) \
	do { \
		if (IS_NATFLOW_ERROR()) { \
			printk(KERN_ERR "error: " pr_fmt(fmt), ##__VA_ARGS__); \
		} \
	} while (0)


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
static inline int nf_register_hooks(struct nf_hook_ops *reg, unsigned int n)
{
	return nf_register_net_hooks(&init_net, reg, n);
}

static inline void nf_unregister_hooks(struct nf_hook_ops *reg, unsigned int n)
{
	nf_unregister_net_hooks(&init_net, reg, n);
}
#endif

extern int natflow_session_init(struct nf_conn *ct, gfp_t gfp);
extern struct natflow_t *natflow_session_get(struct nf_conn *ct);
static inline struct natflow_t *natflow_session_in(struct nf_conn *ct)
{
	if (natflow_session_init(ct, GFP_ATOMIC) != 0) {
		return NULL;
	}
	return natflow_session_get(ct);
}

extern const char *const hooknames[];

#define MAC_HEADER_FMT "%02X:%02X:%02X:%02X:%02X:%02X->%02X:%02X:%02X:%02X:%02X:%02X h_proto=%04X"
#define MAC_HEADER_ARG(eth) \
	((struct ethhdr *)(eth))->h_source[0],((struct ethhdr *)(eth))->h_source[1],((struct ethhdr *)(eth))->h_source[2], \
	((struct ethhdr *)(eth))->h_source[3],((struct ethhdr *)(eth))->h_source[4],((struct ethhdr *)(eth))->h_source[5], \
	((struct ethhdr *)(eth))->h_dest[0],((struct ethhdr *)(eth))->h_dest[1],((struct ethhdr *)(eth))->h_dest[2],\
	((struct ethhdr *)(eth))->h_dest[3],((struct ethhdr *)(eth))->h_dest[4],((struct ethhdr *)(eth))->h_dest[5],\
	((struct ethhdr *)(eth))->h_proto

#define IP_TCPUDP_FMT	"%pI4:%u->%pI4:%u"
#define IP_TCPUDP_ARG(i,t)	&(i)->saddr, ntohs(((struct tcphdr *)(t))->source), &(i)->daddr, ntohs(((struct tcphdr *)(t))->dest)
#define TCP_ST_FMT	"%c%c%c%c%c%c%c%c"
#define TCP_ST_ARG(t) \
	((struct tcphdr *)(t))->cwr ? 'C' : '.', \
	((struct tcphdr *)(t))->ece ? 'E' : '.', \
	((struct tcphdr *)(t))->urg ? 'U' : '.', \
	((struct tcphdr *)(t))->ack ? 'A' : '.', \
	((struct tcphdr *)(t))->psh ? 'P' : '.', \
	((struct tcphdr *)(t))->rst ? 'R' : '.', \
	((struct tcphdr *)(t))->syn ? 'S' : '.', \
	((struct tcphdr *)(t))->fin ? 'F' : '.'
#define UDP_ST_FMT "UL:%u,UC:%04X"
#define UDP_ST_ARG(u) ntohs(((struct udphdr *)(u))->len), ntohs(((struct udphdr *)(u))->check)

#define DEBUG_FMT_PREFIX "(%s:%u)"
#define DEBUG_ARG_PREFIX __FUNCTION__, __LINE__

#define DEBUG_FMT_TCP "[" IP_TCPUDP_FMT "|ID:%04X,IL:%u|" TCP_ST_FMT "]"
#define DEBUG_ARG_TCP(i, t) IP_TCPUDP_ARG(i,t), ntohs(((struct iphdr *)(i))->id), ntohs(((struct iphdr *)(i))->tot_len), TCP_ST_ARG(t)

#define DEBUG_FMT_UDP "[" IP_TCPUDP_FMT "|ID:%04X,IL:%u|" UDP_ST_FMT "]"
#define DEBUG_ARG_UDP(i, u) IP_TCPUDP_ARG(i,u), ntohs((i)->id), ntohs((i)->tot_len), UDP_ST_ARG(u)

#define DEBUG_TCP_FMT "[%s]" DEBUG_FMT_PREFIX DEBUG_FMT_TCP
#define DEBUG_TCP_ARG(i, t) hooknames[hooknum], DEBUG_ARG_PREFIX, DEBUG_ARG_TCP(i, t)

#define DEBUG_UDP_FMT "[%s]" DEBUG_FMT_PREFIX DEBUG_FMT_UDP
#define DEBUG_UDP_ARG(i, u) hooknames[hooknum], DEBUG_ARG_PREFIX, DEBUG_ARG_UDP(i, u)

#define TUPLE_FMT "%pI4:%u-%c"
#define TUPLE_ARG(t) &((struct tuple *)(t))->ip, ntohs(((struct tuple *)(t))->port), ((struct tuple *)(t))->encryption ? 'e' : 'o'

#define ETH(e) ((struct ethhdr *)(e))
#define TCPH(t) ((struct tcphdr *)(t))
#define UDPH(u) ((struct udphdr *)(u))
#define ICMPH(i) ((struct icmphdr *)(i))

#endif /* _NATFLOW_COMMON_H_ */
