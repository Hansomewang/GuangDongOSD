#include "rwlpnrapi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/utils_net.h"
#include "GuangDong_OSD.h"

#define Sleep(n)        usleep((n)*1000)


static int OSD_handle(HANDLE handle, int code)
{
	printf("recv event:%d \r\n", code);
	if (code == EVT_ONLINE )
		on_line = 1;
	if (code == EVT_OFFLINE)
		on_line = 0;
	return TRUE;
}

int main( int argc, char const *argv[] ) 
{
    int		fd;
    FILE *fp;
    int serv_sock;
	fd_set set;
    char *str = NULL;
    char *end = NULL;
    int len;
    char val[256] = { 0 };
    int ret;
    char osd_buf[2048] = { 0 };

    char flg[] = { 0x0d, 0x0a };

	if ( argc != 2 )
	{
		fprintf(stderr, "USUAGE: %s <ip addr>\n", argv[0] );
		return -1;
	}
	
	lpnr = LPNR_Init(argv[1]);
	if( lpnr )
	{
		LPNR_SetCallBack( lpnr, OSD_handle );
		printf("LPNR Init [%s] Success..\r\n", argv[1]);
	}else{
		printf("LPNR Init [%s] Failed \r\n", argv[1] );
		return -1;
	}
	
    fd = sock_listen( 9910, NULL, 5 );
    printf("init socket!\n");
    
    //ÅäÖÃserver socket
    while( 1 )
    {
        if( sock_dataready( fd, 0 ) > 0 )
        {
            serv_sock = sock_accept( fd );
            printf("socket accept successful!\n");    
        }
        else
        {
            Sleep(1000);
            continue;
        }

        //TODO 
        printf("ready to read data \n");
        int rc;
		if ( rc = sock_read(serv_sock, osd_buf, sizeof(osd_buf) ) )
		{
            printf("read %d data\n",rc);
			if ( rc<=0 )
			{
				printf("read serv_sock error \n");
				continue;
			}
		}

		if( !(str = strstr( osd_buf, "Value=")) )
		{
		    printf("not find start\n");
            goto exit;
		}
		str += 7; 

		if( NULL == (end = strchr( str, 0x22)))
		{
		    printf("not find end\n");
            goto exit;
		}

		memcpy( val, str, (end-str));
        printf("%s\n", val);

		LPNR_UserOSDOn(lpnr, -7, 100, 0, 40, 0xffffffff, 100, val);

		LPNR_SetOSDTimeStamp( lpnr, TRUE, 0, 0 );

exit:
		memset( osd_buf, 0, sizeof(osd_buf) );
        memset( val, 0, sizeof(val) );
		close(serv_sock);
    } 

	LPNR_Terminate(lpnr);

    return 0;
}

