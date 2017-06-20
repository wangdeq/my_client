#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>

#define ser_addr "2001::3"
#define port 9521
#define MAX_PAYLOAD 1024 /* maximum payload size*/
#define NETLINK_TEST 21
#define samp_itvl 5

struct sockaddr_nl src_addr, dest_addr ;
struct nlmsghdr *nlh = NULL ;
struct iovec iov ;
int sock_fd ;
struct msghdr msg ;
char my_policy ;
char policy_ans = 'n' ;
char tmp [100] ;
int flag = 0 ;
int flag_pipes[2] ;
int msgs_pipes[2] ;

int get_flag () ;
int change_flag ( int num ) ;
char get_policy ( void ) ;
char set_policy ( void ) ;
void net_link_init ( void ) ;
char mysend_msg ( int sock_fd , struct msghdr msg , const char my_policy ) ;
void mysend_ans ( int sock_fd , struct msghdr msg , const char my_policy ) ;
void my_netlink_sender ( void ) ;
int my_server ( void ) ;

int main ( int argc , char ** argv )
{
	pid_t mypid ;
	
	if ( ( pipe ( flag_pipes ) == 0 ) && ( pipe ( msgs_pipes ) == 0 ) )
	{
		mypid = fork () ;

		if ( mypid < 0 )
		{
			printf ( "fork failed !!!\n" ) ;
		}
		else if ( mypid == 0 )
		{
			printf ( "netlink starting !!!\n" ) ;		
			my_netlink_sender () ;
			printf ( "netlink closed !!!\n" ) ;
		}
		else
		{
			printf ( "client starting !!!\n" ) ;
			my_client () ;
			printf ( "client closed !!!\n" ) ;
		}
	}

	exit ( 0 ) ;
}

int get_flag ()
{
	return flag ;
}

int change_flag ( int num )
{
	if ( flag == num )
	{
		return 0 ;
	}
	else 
	{
		flag = num ;
	}

	return 0 ;
}

int my_client ( void )
{
	int sockfd ; 
	int len ; 
	struct sockaddr_in6 address ; 
	int result ; 
	int n_flag ;
	char ch ;
	char ans ;

	sockfd = socket ( AF_INET6 , SOCK_STREAM , 0 ) ;

	address.sin6_family = AF_INET6 ; 
	//address.sin6_addr.s_addr = inet_addr ( ser_addr ) ;
	inet_pton ( AF_INET6 , ser_addr , &address.sin6_addr ) ;	
	address.sin6_port = port ;
	len = sizeof ( address ) ;

	result = connect ( sockfd , ( struct sockaddr * )&address , len ) ;

	if ( result == -1 )
	{
		perror ( "oops : client error !!" ) ;
		exit ( 1 ) ;
	}
	printf ( "connect to the server successful !!!\nwaiting the message ...\n" ) ;

	while ( ch != 'x' )
	{			
		printf ( "------------------------\n\n" ) ;
		
		read ( sockfd , &ch , 1 ) ;
		write ( sockfd , &ch , 1 ) ;
		printf ( "get the alg choice is : %c , waiting for checking  .\n" , ch ) ;
		read ( sockfd , &ans , 1 ) ;
		
		switch (ans)
		{
			case 'y' :
				if ( my_policy != ch )
				{ 			
					if ( ch == 'x' )
					{
						my_policy = '0' ;
					}	
					else
					{
						my_policy = ch ;
					}

					change_flag ( 1 ) ;
					n_flag = get_flag () ;
					write ( flag_pipes[1] , &n_flag , sizeof ( int ) ) ;
					printf ( "flag is : %d\n" , n_flag ) ;
					write ( msgs_pipes[1] , &my_policy , sizeof ( char ) ) ;
					printf ( "alg choice comm correct !\n" ) ;
				}
				else
				{
					change_flag ( 0 ) ;
					n_flag = get_flag () ;
					write ( flag_pipes[1] , &n_flag , sizeof ( int ) ) ;
					printf ( "flag is : %d\n" , n_flag ) ;
					printf ( "alg choice comm correct !\n" ) ;
				}
				break ;
			case 'n' :
				printf ( "alg choice comm wrong !\n" ) ;
				break ;
			default :
				break ;
		}

		printf ( "------------------------\n" ) ;
	}

	close ( sockfd ) ;

	return 0 ;
}


void my_netlink_sender ( void )
{
	char buff [BUFSIZ + 1] ;

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST) ;
	if ( sock_fd == -1 )
	{
		printf ( "error happened in getting socket : %s.\n" , strerror (errno) ) ;
		return ;
	}

	while ( 1 )
	{	
		read ( flag_pipes[0] , &flag , sizeof ( int ) ) ;
		if ( flag == 0 )
		{
			printf ( "not change .\n" ) ; 
			continue ;
		}
		else
		{
			printf ( "ready to read !\n" ) ;
			read ( msgs_pipes[0] , buff , BUFSIZ ) ;

			my_policy = buff[0] ;

			while (1)
			{
				policy_ans = mysend_msg ( sock_fd, msg , my_policy) ;
	
				printf ( "sended message : %c.\n" , my_policy ) ;	
				printf ( "received message : %c.\n" , policy_ans ) ;	

				if ( policy_ans == my_policy )
				{
					printf ( "send message successful !\n" ) ;
					policy_ans = 'y' ;
					break ;
				}
				else 
				{
					printf ( "send message wrong , try again !\n" ) ;
					policy_ans = 'n' ;
					continue ;
				}
			}
		}

		printf ( "------------------------\n" ) ;
	}

	/* Close Netlink Socket */
	close(sock_fd) ;
}

void net_link_init ()
{
	memset(&msg, 0, sizeof(msg)) ;
	memset(&src_addr, 0, sizeof(src_addr)) ;

	src_addr.nl_family = AF_NETLINK ;
	src_addr.nl_pid = getpid() ; /* self pid */
	src_addr.nl_groups = 0 ; /* not in mcast groups */

	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) ;

	memset(&dest_addr, 0, sizeof(dest_addr)) ;
	dest_addr.nl_family = AF_NETLINK ;
	dest_addr.nl_pid = 0 ; /* For Linux Kernel */
	dest_addr.nl_groups = 0 ; /* unicast */

	/* Fill the netlink message header */
	nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD)) ;
	
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD) ;
	nlh->nlmsg_pid = getpid() ; /* self pid */
	nlh->nlmsg_flags = 0 ;

	memset(&msg, 0, sizeof(msg)); 	
	msg.msg_name = (void *)&dest_addr ;
	msg.msg_namelen = sizeof(dest_addr) ;
}

char mysend_msg ( int sock_fd , struct msghdr msg , const char key_word )
{
	int state ;

	net_link_init () ;

	strcpy(NLMSG_DATA(nlh), &key_word) ;
	iov.iov_base = (void *)nlh ;
	iov.iov_len = nlh->nlmsg_len ;
	msg.msg_iov = &iov ;
	msg.msg_iovlen = 1 ;

	printf(" Sending message. ...\n") ;
	state = sendmsg(sock_fd, &msg, 0) ;
	if ( state == -1 )
	{
		printf( "error happen in sending msg : %s\n" , strerror(errno) ) ;
	}

	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD)) ;
	printf(" Waiting message. ...\n") ;

	recvmsg(sock_fd, &msg, 0) ;
	strcpy( tmp , NLMSG_DATA(nlh)) ;

	return tmp[0] ;	
}
