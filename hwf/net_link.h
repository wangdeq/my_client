#include "encode.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <net/sock.h>
#include <net/netlink.h>

#define NETLINK_TEST 21

struct sock *netlink_skt = NULL ;

int pid ;
int flag = 0 ;
char tmp[100] ;
char trans_policy ;
char current_policy ;

int get_strlenth ( char * str )
{
	int len = 0 ;
	while ( str [len] != '\0' )
	{
		++ len ;
	}

	return len ;
}

char get_choice ( void ) 
{
	if ( flag == 1 )
	{
		current_policy = trans_policy ;
		printk ( "hwf---current policy is : %c\n" , current_policy ) ;
		flag = 0 ;

		return current_policy ;
	}	
	else
	{
		return current_policy ;
	}
}

void send_net_link ( char * ans )
{
	struct sk_buff *skb ;
	struct nlmsghdr *nlh ;
	int len = NLMSG_SPACE(1200) ;
	int rc ;
	int ans_len = 0 ;

	 /*pid of sending process */
	printk ( "net_link: the sender's pid is %d\n" , pid ) ;

	skb = alloc_skb( len, GFP_ATOMIC ) ;
	if ( !skb )
	{
		printk( KERN_ERR "net_link: allocate failed.\n" ) ;
		return;
	}

	nlh = nlmsg_put( skb , 0 , 0 , 0 , 1200 , 0 ) ;

	NETLINK_CB(skb).creds.pid = 0 ; /* from kernel */
	NETLINK_CB(skb).dst_group = 0; 
	
	ans_len = get_strlenth ( ans ) ;
	memcpy( NLMSG_DATA(nlh) , ans , ans_len ) ;

	printk( "net_link: going to send.\n" ) ;
	rc = netlink_unicast( netlink_skt , skb , pid , MSG_DONTWAIT ) ;
	if (rc < 0) 
	{
		printk( KERN_ERR "net_link: can not unicast skb (%d)\n" , rc ) ;
	}
	printk( "net_link: send is ok.\n" ) ;
}

char * recv_net_link ( struct sk_buff *__skb )
{
	struct sk_buff *skb ;
	struct nlmsghdr *nlh ;

	printk("net_link: data is ready to read.\n");
	skb = skb_get(__skb) ;

	if ( skb->len >= NLMSG_SPACE(0) ) 
	{
		nlh = nlmsg_hdr(skb) ;
		memcpy( tmp , NLMSG_DATA (nlh) , sizeof (tmp) ) ; 
		printk( "net_link: recv alg choice : %s.\n" , tmp ) ;

		pid = nlh->nlmsg_pid ;

		kfree_skb(skb) ;
	}

	if ( trans_policy != tmp[0] )
	{
		trans_policy = tmp[0] ;
		flag = 1 ;
	}

	return tmp ;	
}

void nl_data_ready ( struct sk_buff *__skb )
{
	send_net_link ( recv_net_link ( __skb ) ) ;

	return;
}

static int hwf_netlink_init( void )
{
	struct netlink_kernel_cfg mycfg = { .input=nl_data_ready , } ;

	netlink_skt = netlink_kernel_create ( &init_net , NETLINK_TEST , &mycfg ) ;

	if ( !netlink_skt ) 
	{
		printk(KERN_ERR "net_link: Cannot create netlink socket.\n") ;
		return -EIO ;
	}
	printk( "net_link: create socket ok.\n" ) ;
	return 0 ;
}

MODULE_LICENSE("GPL");
