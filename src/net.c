/******************************************************************************
*   TinTin++                                                                  *
*   Copyright (C) 2004 (See CREDITS file)                                     *
*                                                                             *
*   This program is protected under the GNU GPL (See COPYING)                 *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 2 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with this program; if not, write to the Free Software               *
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
*******************************************************************************/

/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                         coded by Peter Unold 1992                           *
******************************************************************************/


#include "tintin.h"

#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

/*
	IPv6 compatible connect code, doesn't work on several platforms.
*/

#ifdef HAVE_GETADDRINFO

int connect_mud(struct session *ses, char *host, char *port)
{
	int sock, error;
	struct addrinfo *address;
	static struct addrinfo hints;
	char ip[100];

	if (!is_number(port))
	{
		tintin_puts(ses, "#THE PORT SHOULD BE A NUMBER.");
		return -1;
	}

	hints.ai_family   = AF_UNSPEC;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;

	error = getaddrinfo(host, port, &hints, &address);

	switch (error)
	{
		case 0:
			break;

		case -2:
			tintin_printf(ses, "#SESSION ERROR - UNKNOWN HOST.");
			return -1;

		default:
			tintin_printf(ses, "#SESSION ERROR - CANNOT CONNECT.");
			return -1;
	}

	sock = socket(address->ai_family, address->ai_socktype, address->ai_protocol);

	if (sock < 0)
	{
		syserr("socket");
	}

	ses->connect_error = connect(sock, address->ai_addr, address->ai_addrlen);

	if (ses->connect_error)
	{
		close(sock);
		freeaddrinfo(address);

		return 0;
	}

	if (fcntl(sock, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
	{
		perror("connect_mud: fcntl O_NDELAY|O_NONBLOCK");
	}

	getnameinfo(address->ai_addr, address->ai_addrlen, ip, 100, NULL, 0, NI_NUMERICHOST);

	RESTRING(ses->ip, ip);

	freeaddrinfo(address);

	return sock;
}

#else

int connect_mud(struct session *ses, char *host, char *port)
{
	int sock, d;
	struct sockaddr_in sockaddr;

	if (sscanf(host, "%d.%d.%d.%d", &d, &d, &d, &d) == 4)
	{
		sockaddr.sin_addr.s_addr = inet_addr(host);
	}
	else
	{
		struct hostent *hp;

		if (!(hp = gethostbyname(host)))
		{
			tintin_puts2(ses, "#ERROR - UNKNOWN HOST.");
			return -1;
		}
		memcpy((char *)&sockaddr.sin_addr, hp->h_addr, sizeof(sockaddr.sin_addr));
	}

	if (is_number(port))
	{
		sockaddr.sin_port = htons(atoi(port));
	}
	else
	{
		tintin_puts(ses, "#THE PORT SHOULD BE A NUMBER.");
		return -1;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		syserr("socket");
	}

	sockaddr.sin_family = AF_INET;

	ses->connect_error = connect(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr));

	if (ses->connect_error)
	{
		close(sock);

		return 0;
	}

	if (fcntl(sock, F_SETFL, O_NDELAY|O_NONBLOCK) == -1)
	{
		perror("connect_mud: fcntl O_NDELAY|O_NONBLOCK");
	}

	RESTRING(ses->ip, inet_ntoa(sockaddr.sin_addr));

	return sock;
}

#endif

void write_line_mud(struct session *ses, char *line, int size)
{
	push_call("write_line_mud(%p,%p)",line,ses);

	if (ses == gts)
	{
		tintin_printf2(ses, "#NO SESSION ACTIVE. USE: %csession {name} {host} {port} TO START ONE.", gtd->tintin_char);

		pop_call();
		return;
	}

	if (!HAS_BIT(ses->flags, SES_FLAG_CONNECTED))
	{
		tintin_printf2(ses, "#THIS SESSION IS NOT CONNECTED.");

		pop_call();
		return;
	}

	if (write(ses->socket, line, size) == -1)
	{
		syserr("write in write_line_mud");
	}
	pop_call();
	return;
}


int read_buffer_mud(struct session *ses)
{
	char buffer[BUFFER_SIZE];
	int size;

	push_call("read_buffer_mud(%p)",ses);

	size = read(ses->socket, buffer, BUFFER_SIZE - 1);

	if (size <= 0)
	{
		pop_call();
		return FALSE;
	}

	ses->read_len = translate_telopts(ses, buffer, size);

	pop_call();
	return TRUE;
}


void readmud(struct session *ses)
{
	char *line, *next_line;
	char linebuf[STRING_SIZE];

	push_call("readmud(%p)", ses);

	gtd->mud_output_len = 0;

	/* separate into lines and print away */

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_SPLIT))
	{
		save_pos(gtd->ses);
		goto_rowcol(gtd->ses, gtd->ses->bot_row, 1);
	}

	SET_BIT(gtd->ses->flags, SES_FLAG_READMUD);

	for (line = gtd->mud_output_buf ; line && *line ; line = next_line)
	{
		next_line = strchr(line, '\n');

		if (next_line)
		{
			*next_line = 0;
			next_line++;
		}
		else if (*line == 0)
		{
			break;
		}

		if (next_line == NULL && strlen(ses->more_output) < BUFFER_SIZE / 2)
		{
			if (!HAS_BIT(gtd->ses->telopts, TELOPT_FLAG_PROMPT))
			{
				if (gts->check_output)
				{
					strcat(ses->more_output, line);
					ses->check_output = utime() + gts->check_output;
					break;
				}
			}
		}

		if (ses->more_output[0])
		{
			if (ses->check_output)
			{
				sprintf(linebuf, "%s%s", ses->more_output, line);

				ses->more_output[0] = 0;
			}
			else if (*line && HAS_BIT(ses->flags, SES_FLAG_SPLIT))
			{
				sprintf(linebuf, "%s%s", ses->more_output, line);

				ses->more_output[0] = 0;
			}
			else
			{
				strcpy(linebuf, line);
			}
		}
		else
		{
			strcpy(linebuf, line);
		}

		process_mud_output(ses, linebuf, next_line == NULL);
	}
	DEL_BIT(gtd->ses->flags, SES_FLAG_READMUD);

	if (HAS_BIT(gtd->ses->flags, SES_FLAG_SPLIT))
	{
		restore_pos(gtd->ses);
	}

	pop_call();
	return;
}


void process_mud_output(struct session *ses, char *linebuf, int prompt)
{
	char line[STRING_SIZE];

	ses->check_output = 0;

	if (HAS_BIT(ses->flags, SES_FLAG_COLORPATCH))
	{
		get_color_codes(ses->color, linebuf, ses->color);

		sprintf(line, "%s%s", ses->color, linebuf);

		linebuf = line;
	}

	do_one_line(linebuf, ses);   /* changes linebuf */

	/*
		Take care of gags, vt102 support still goes
	*/

	if (HAS_BIT(ses->flags, SES_FLAG_GAG))
	{
		strip_non_vt102_codes(linebuf, ses->more_output);

		printf("%s", ses->more_output);

		ses->more_output[0] = 0;

		DEL_BIT(ses->flags, SES_FLAG_GAG);

		return;
	}

	add_line_buffer(ses, linebuf, prompt);

	if (ses == gtd->ses)
	{
		printline(ses, linebuf, prompt);
	}
	else if (HAS_BIT(ses->flags, SES_FLAG_SNOOP))
	{
		strip_vt102_codes_non_graph(linebuf, linebuf);

		tintin_printf2(gtd->ses, "[%s] %s", ses->name, linebuf);
	}
}

