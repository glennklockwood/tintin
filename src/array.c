/******************************************************************************
*   TinTin++                                                                  *
*   Copyright (C) 2005 (See CREDITS file)                                     *
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
*   file: class.c - funtions related to the scroll back buffer                *
*                                                                             *
*               (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                  *
*                                                                             *
*                     coded by Igor van den Hoven 2004                        *
******************************************************************************/


#include "tintin.h"


DO_COMMAND(do_list)
{
	char temp[BUFFER_SIZE], left[BUFFER_SIZE], right[BUFFER_SIZE];
	struct listroot *root;
	struct listnode *node;
	int cnt;

	root = ses->list[LIST_VARIABLE];

	arg = get_arg_in_braces(arg, left, FALSE);
	arg = get_arg_in_braces(arg, right, FALSE);

	substitute(ses, left, left, SUB_VAR|SUB_FUN);
	substitute(ses,  arg, temp, SUB_VAR|SUB_FUN);

	arg = temp;

	if (*left == 0 || *right == 0)
	{
		show_message(ses, LIST_VARIABLE, "#SYNTAX: #LIST {listname} {CLR|DEL|FND|GET|INS|LEN|SET|SRT} {argument}.");
	}
	else
	{
		for (cnt = 0 ; *array_table[cnt].name ; cnt++)
		{
			if (is_abbrev(right, array_table[cnt].name))
			{
				break;
			}
		}

		if (*array_table[cnt].name == 0)
		{
			show_message(ses, LIST_VARIABLE, "#SYNTAX: #LIST {%s} {CLR|DEL|FND|INS|GET|SET|LEN} {argument}.", left);
		}
		else
		{
			if ((node = search_node_with_wild(ses, left, LIST_VARIABLE)) == NULL)
			{
				updatenode_list(ses, left, "", "0", LIST_VARIABLE);

				node = search_node_with_wild(ses, left, LIST_VARIABLE);
			}
			arg = get_arg_in_braces(arg, left,  array_table[cnt].lval);
			arg = get_arg_in_braces(arg, right, array_table[cnt].rval);

			array_table[cnt].array(ses, node, left, right);
		}
	}
	return ses;
}

int get_list_length(struct listnode *node)
{
	char *arg, temp[BUFFER_SIZE];
	int cnt = 0;

	for (arg = node->right ; *arg ; cnt++)
	{
		arg = get_arg_in_braces(arg, temp, FALSE);
	}
	return cnt;
}

int get_list_index(struct session *ses, struct listnode *node, char *arg)
{
	int index, size;

	index = get_number(ses, arg);
	size  = get_list_length(node);

	if (size == 0 && (index == -1 || index == 1))
	{
		return 0;
	}

	if (index < 0)
	{
		index = size + 1 + index;
	}

	if (size == 0 || index < 1 || index > size)
	{
		return -1;
	}
	return index;
}

void get_list_item(struct session *ses, struct listnode *node, char *left)
{
	char *arg;
	int cnt, index;

	index = get_list_index(ses, node, left);

	if (index <= 0)
	{
		strcpy(left, "");
	}

	arg = node->right;

	for (cnt = 1 ; cnt <= index ; cnt++)
	{
		arg = get_arg_in_braces(arg, left, FALSE);
	}
}

void set_list_item(struct session *ses, struct listnode *node, char *left, char *right)
{
	char temp[BUFFER_SIZE], buf[STRING_SIZE];
	int cnt, index, length;

	index  = (int) get_number(ses, left);
	length = get_list_length(node);

	if (index > length)
	{
		strcpy(buf, node->right);

		while (index > length++)
		{
			strcat(buf, "{}");
		}
		updatenode_list(ses, node->left, buf, "0", LIST_VARIABLE);
	}

	index = get_list_index(ses, node, left);

	if (index <= 0)
	{
		return;
	}

	left = node->right;

	buf[0] = 0;

	for (cnt = 1 ; cnt < index ; cnt++)
	{
		left = get_arg_in_braces(left, temp, FALSE);

		cat_sprintf(buf, "{%s}", temp);
	}

	left = get_arg_in_braces(left, temp, FALSE);

	cat_sprintf(buf, "{%s}", right);

	while (*left)
	{
		left = get_arg_in_braces(left, temp, FALSE);

		cat_sprintf(buf, "{%s}", temp);
	}

	updatenode_list(ses, node->left, buf, "0", LIST_VARIABLE);
}

DO_ARRAY(array_clr)
{
	internal_variable(ses, "{%s}", list->left);

	return ses;
}

DO_ARRAY(array_del)
{
	char buf[STRING_SIZE], *arg;
	int cnt, index;

	index = get_list_index(ses, list, left);

	if (index <= 0)
	{
		show_message(ses, LIST_VARIABLE, "#LIST DEL: Invalid index: %s", left);

		return ses;
	}

	buf[0] = 0;

	arg = list->right;

	for (cnt = 1 ; cnt < index ; cnt++)
	{
		arg = get_arg_in_braces(arg, right, FALSE);

		cat_sprintf(buf, "{%s}", right);
	}

	arg = get_arg_in_braces(arg, right, FALSE);

	while (*arg)
	{
		arg = get_arg_in_braces(arg, right, FALSE);

		cat_sprintf(buf, "{%s}", right);
	}

	updatenode_list(ses, list->left, buf, "0", LIST_VARIABLE);

	return ses;
}

DO_ARRAY(array_fnd)
{
	char temp[BUFFER_SIZE], *arg;
	int cnt, index, result;

	index = get_list_length(list);

	arg = list->right;

	result = 0;

	for (cnt = 1 ; cnt <= index ; cnt++)
	{
		arg = get_arg_in_braces(arg, temp, FALSE);

		if (match(ses, temp, left))
		{
			result = cnt;
			break;
		}
	}
	internal_variable(ses, "{%s} {%d}", right, result);

	return ses;
}

DO_ARRAY(array_get)
{
	get_list_item(ses, list, left);

	internal_variable(ses, "{%s} {%s}", right, left);

	return ses;
}

DO_ARRAY(array_ins)
{
	char temp[BUFFER_SIZE], buf[STRING_SIZE], *arg;
	int cnt, index;

	index = get_list_index(ses, list, left);
 
	if (index == -1)
  	{
 		show_message(ses, LIST_VARIABLE, "#LIST INS: Invalid index: %s", left);
 
		return ses;
 	}
 
	if (index == 0)
	{
		sprintf(buf, "{%s}", right);
		updatenode_list(ses, list->left, buf, "0", LIST_VARIABLE);

		return ses;
	}

	if (atoi(left) < 0)
	{
		index++;
	}

	buf[0] = 0;

	arg = list->right;

	for (cnt = 1 ; cnt < index ; cnt++)
	{
		arg = get_arg_in_braces(arg, temp, FALSE);

		cat_sprintf(buf, "{%s}", temp);
	}

	cat_sprintf(buf, "{%s}", right);

	while (*arg)
	{
		arg = get_arg_in_braces(arg, temp, FALSE);

		cat_sprintf(buf, "{%s}", temp);
	}

	updatenode_list(ses, list->left, buf, "0", LIST_VARIABLE);

	return ses;
}

DO_ARRAY(array_len)
{
	internal_variable(ses, "{%s} {%d}", left, get_list_length(list));

	return ses;
}

DO_ARRAY(array_set)
{
	set_list_item(ses, list, left, right);

	return ses;
}

DO_ARRAY(array_srt)
{
	char temp[BUFFER_SIZE], buf[STRING_SIZE], *arg;

	buf[0] = 0;

	arg = list->right;

	if (*arg == 0)
	{
		sprintf(buf, "{%s}", left);

		updatenode_list(ses, list->left, buf, "0", LIST_VARIABLE);

		return ses;
	}

	while (*arg)
	{
		arg = get_arg_in_braces(arg, temp, FALSE);

		if (strcmp(left, temp) <= 0)
		{
			break;
		}
		cat_sprintf(buf, "{%s}", temp);
	}

	cat_sprintf(buf, "{%s}", left);

	if (strcmp(left, temp) <= 0)
	{
		cat_sprintf(buf, "{%s}", temp);
	}

	while (*arg)
	{
		arg = get_arg_in_braces(arg, temp, FALSE);

		cat_sprintf(buf, "{%s}", temp);
	}

	updatenode_list(ses, list->left, buf, "0", LIST_VARIABLE);

	return ses;
}
