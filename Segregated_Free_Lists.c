#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Segregated_Free_Lists.h"

int main()
{
	int ok1 = 0, ok2 = 0, i, j, k, dim = 0, max_size = 1;
	mystruct_t mystruct;
	free_malloc *v;
	v = (free_malloc *)malloc(sizeof(free_malloc));
	// in 'v' I keep all the blocks allocated
	// v[i].f is the number of bytes when fragmented
	// v[i].size_max is the total length at write
	v[0].f = 0;
	v[0].n = 0;
	v[0].p = NULL;
	v[0].size = 0;
	v[0].size_max = 0;
	char command[100], *word;
	fgets(command, sizeof(command), stdin);
	command[strlen(command) - 1] = '\0';
	word = strtok(command, " ");
	while (strcmp(word, "DESTROY_HEAP"))
	{
		int ok = 0;
		if (!strcmp(word, "INIT_HEAP"))
		{
			ok = 1;
			word = strtok(NULL, " ");
			if (!word || word[0] != '0' || word[1] != 'x')
			{
				ok = 0;
				goto invalid;
			}
			// convert a string into an unsigned and cast the void *
			unsigned long adr = strtoul(word, NULL, 16);
			void *addr = (void *)adr;
			int lists, bytes, recon;
			word = strtok(NULL, " ");
			if (!word)
			{
				ok = 0;
				goto invalid;
			}
			lists = atoi(word);
			word = strtok(NULL, " ");
			if (!word)
			{
				ok = 0;
				goto invalid;
			}
			bytes = atoi(word);
			word = strtok(NULL, " ");
			if (!word)
			{
				ok = 0;
				goto invalid;
			}
			recon = atoi(word);
			mystruct.addr_struct = addr;
			mystruct.lists = malloc(lists * sizeof(void *));
			mystruct.nr_lists = lists;
			mystruct.recons = recon;
			mystruct.total_bytes = bytes * lists;
			mystruct.bytes = 8;
			mystruct.nr_frag = 0;
			mystruct.nr_free = 0;
			mystruct.nr_malloc = 0;
			// each node in the list has the ascending address
			// each list has ascending size (power of 2)
			for (i = 0; i < lists; i++)
			{
				mystruct.lists[i] = create(1);
				mystruct.lists[i]->data_size = 1;
				mystruct.lists[i]->size_in_bytes = mystruct.bytes * pow(2, i);
				for (j = 0; j < bytes / mystruct.lists[i]->size_in_bytes; j++)
				{
					add_nth_node(mystruct.lists[i], j, addr);
					addr += mystruct.lists[i]->size_in_bytes;
				}
			}
		}
		if (!strcmp(word, "MALLOC"))
		{
			ok = 1;
			int nr_bytes;
			word = strtok(NULL, " ");
			if (!word)
			{
				ok = 0;
				goto invalid;
			}
			nr_bytes = atoi(word);
			ok1 = 0;
			ok2 = 0;
			mystruct.nr_malloc++;
			for (i = mystruct.nr_lists - 1; i >= 0; i--)
				if (mystruct.lists[i]->size)
				{
					if (mystruct.lists[i]->size_in_bytes < nr_bytes)
						goto nothing;
					else
						break;
				}
			for (i = 0; i < mystruct.nr_lists; i++)
			{
				if (nr_bytes == mystruct.lists[i]->size_in_bytes && mystruct.lists[i]->size)
				{
					// we found the free address and we are removing it
					node_t *node = remove_nth_node(mystruct.lists[i], 0);
					v[dim].size_max = 0;
					v[dim].size = 0;
					v[dim].f = 0;
					v[dim].n = i;
					v[dim].p = node->addr_node;
					dim++;
					if (dim == max_size)
					{
						v = (free_malloc *)realloc(v, (2 * dim) * sizeof(free_malloc));
						max_size = 2 * dim;
					}
					free(node);
					ok1 = 1;
					ok2 = 1;
					break;
				}
			}
			if (ok1 == 0)
			{
				for (i = 0; i < mystruct.nr_lists; i++)
				{
					if (nr_bytes < mystruct.lists[i]->size_in_bytes && mystruct.lists[i]->size)
					{
						// there is not already a list of that size then fragmentation occurs
						mystruct.nr_frag++;
						int np2, Okay = 0;
						np2 = nr_bytes;
						while (!Okay)
						{
							for (j = 0; j < mystruct.nr_lists; j++)
								if (np2 == mystruct.lists[j]->size_in_bytes && mystruct.lists[j]->size)
									Okay = 1;
							if (!Okay)
								np2++;
						}
						Okay = 0;
						for (j = 0; j < mystruct.nr_lists; j++)
							if (mystruct.lists[j]->size_in_bytes == np2 - nr_bytes)
								Okay = 1;
						// I update the index of the list of elements larger than the current one
						for (j = 0; j < dim; j++)
							if (mystruct.lists[v[j].n]->size_in_bytes > np2 - nr_bytes && Okay == 0)
								v[j].n++;
						node_t *node = remove_nth_node(mystruct.lists[i], 0);
						v[dim].size_max = 0;
						v[dim].size = 0;
						v[dim].f = nr_bytes;
						v[dim].p = node->addr_node;
						dim++;
						if (dim == max_size)
						{
							v = (free_malloc *)realloc(v, (2 * dim) * sizeof(free_malloc));
							max_size = 2 * dim;
						}
						int this;
						for (j = 0; j < mystruct.nr_lists; j++)
						{
							if (mystruct.lists[i]->size_in_bytes - nr_bytes < mystruct.lists[j]->size_in_bytes)
							{
								// I need the new list with the new size
								this = j;
								ok2 = 1;
								v[dim - 1].n = j;
								break;
							}
							if (mystruct.lists[i]->size_in_bytes - nr_bytes == mystruct.lists[j]->size_in_bytes)
							{
								// I put in the already existing list
								if (position(mystruct.lists[j], node->addr_node + nr_bytes) >= 0)
									add_nth_node(mystruct.lists[j], position(mystruct.lists[j], node->addr_node + nr_bytes), node->addr_node + nr_bytes);
								ok2 = 1;
								v[dim - 1].n = j;
								goto no_extra_list;
							}
						}
						if (ok2)
						{
							mystruct.nr_lists++;
							mystruct.lists = realloc(mystruct.lists, mystruct.nr_lists * sizeof(list_t));
							add_list(this, &mystruct.lists, mystruct.nr_lists);
							mystruct.lists[this] = create(1);
							mystruct.lists[this]->size_in_bytes = mystruct.lists[i + 1]->size_in_bytes - nr_bytes;
							add_nth_node(mystruct.lists[this], 0, node->addr_node + nr_bytes);
						no_extra_list:
							free(node);
							break;
						}
					}
				}
			}
			if (!ok2)
			{
			nothing:
				printf("Out of memory\n");
				mystruct.nr_malloc--;
			}
		}
		if (!strcmp(word, "FREE"))
		{
			ok = 1;
			word = strtok(NULL, " ");
			if (!word || word[0] != '0' || word[1] != 'x')
			{
				ok = 0;
				goto invalid;
			}
			// FREE 0x0 does nothing
			if (strcmp(word, "0x0"))
			{
				unsigned long adr = strtoul(word, NULL, 16);
				void *addr = (void *)adr;
				mystruct.nr_free++;
				// I didn't do the bonus
				if (mystruct.recons == 0)
				{
					ok1 = 0;
					for (i = 0; i < dim; i++)
					{
						if (v[i].p == addr)
						{
							ok1 = 1;
							if (!v[i].f)
							{
								// I put the read address in the list of free addresses
								add_nth_node(mystruct.lists[v[i].n], position(mystruct.lists[v[i].n], v[i].p), v[i].p);
								if (v[i].size)
									free(v[i].value);
								delete (i, &v, &dim);
							}
							else
							{
								// fragmented block
								for (j = 0; j < mystruct.nr_lists; j++)
								{
									if (v[i].f == mystruct.lists[j]->size_in_bytes && mystruct.lists[j]->size)
									{
										// there is already a list with the right size
										add_nth_node(mystruct.lists[j], position(mystruct.lists[j], v[i].p), v[i].p);
										if (v[i].size)
											free(v[i].value);
										delete(i, &v, &dim);
										break;
									}
									if (v[i].f < mystruct.lists[j]->size_in_bytes)
									{
										// I add a list in ascending order
										mystruct.nr_lists++;
										mystruct.lists = realloc(mystruct.lists, mystruct.nr_lists * sizeof(void *));
										add_list(j, &mystruct.lists, mystruct.nr_lists);
										mystruct.lists[j] = create(1);
										mystruct.lists[j]->size_in_bytes = v[i].f;
										add_nth_node(mystruct.lists[j], 0, v[i].p);
										if (v[i].size)
											free(v[i].value);
										delete (i, &v, &dim);
										break;
									}
								}
							}
							break;
						}
					}
					if (!ok1)
					{
						printf("Invalid free\n");
						mystruct.nr_free--;
					}
				}
			}
		}
		if (!strcmp(word, "READ"))
		{
			ok = 1;
			word = strtok(NULL, " ");
			if (!word || word[0] != '0' || word[1] != 'x')
			{
				ok = 0;
				goto invalid;
			}
			unsigned long adr = strtoul(word, NULL, 16);
			void *addr = (void *)adr;
			int nr_bytes;
			word = strtok(NULL, " ");
			if (!word)
			{
				ok = 0;
				goto invalid;
			}
			nr_bytes = atoi(word);
			ok1 = 0;
			for (i = 0; i < dim; i++)
			{
				if (v[i].p == addr)
				{
					ok1 = 1;
					for (j = i; j < dim - 1; j++)
					{
						// if a byte is not allocated or consecutive
						if (nr_bytes > v[j].f && v[j].f && v[j].p + v[j].f != v[j + 1].p)
						{
							printf("Segmentation fault (core dumped)\n");
							dump_mem(mystruct, dim, v);
							goto skip;
						}
						if (nr_bytes > mystruct.lists[v[j].n]->size_in_bytes && !v[j].f && v[j].p + mystruct.lists[v[j].n]->size_in_bytes != v[j + 1].p)
						{
							printf("Segmentation fault (core dumped)\n");
							dump_mem(mystruct, dim, v);
							goto skip;
						}
					}
					if (nr_bytes > v[i].size_max && v[i].size_max)
						nr_bytes = v[i].size_max;
					for (j = i; j < dim; j++)
					{
						if (nr_bytes < v[j].size)
						{
							for (k = 0; k < nr_bytes; k++)
								printf("%c", v[j].value[k]);
							printf("\n");
							break;
						}
						printf("%s", v[j].value);
						nr_bytes -= v[j].size;
						if (nr_bytes == 0)
						{
							printf("\n");
							break;
						}
					}
					break;
				}
			}
			if (!ok1)
			{
				printf("Segmentation fault (core dumped)\n");
				dump_mem(mystruct, dim, v);
				goto skip;
			}
		}
		if (!strcmp(word, "WRITE"))
		{
			ok = 1;
			int nr_bytes;
			word = strtok(NULL, " ");
			unsigned long adr = strtoul(word, NULL, 16);
			void *addr = (void *)adr;
			char string[100];
			word = strtok(NULL, " ");
			if (!word)
			{
				ok = 0;
				goto invalid;
			}
			// remove the quotation marks
			strcpy(string, word + 1);
			if (word[strlen(word) - 1] == '"')
				string[strlen(string) - 1] = '\0';
			else
			{
				while (word[strlen(word) - 1] != '"')
				{
					word = strtok(NULL, " ");
					strcat(string, " ");
					strcat(string, word);
				}
				string[strlen(string) - 1] = '\0';
			}
			word = strtok(NULL, " ");
			if (!word)
			{
				ok = 0;
				goto invalid;
			}
			nr_bytes = atoi(word);
			int offset = 0, SIZE = 0;
			// by offset I go through the string
			// SIZE is the number of bytes per allocated block
			if (nr_bytes > (int)strlen(string))
				nr_bytes = (int)strlen(string);
			sort(dim, &v);
			ok1 = 0;
			for (i = 0; i < dim; i++)
			{
				if (addr == v[i].p)
				{
					ok1 = 1;
					for (j = i; j < dim - 1; j++)
					{
						// if a byte is not allocated or consecutive
						if (v[j].f && v[j].p + v[j].f != v[j + 1].p)
						{
							printf("Segmentation fault (core dumped)\n");
							dump_mem(mystruct, dim, v);
							goto skip;
						}
						if (!v[j].f && v[j].p + mystruct.lists[v[j].n]->size_in_bytes != v[j + 1].p)
						{
							printf("Segmentation fault (core dumped)\n");
							dump_mem(mystruct, dim, v);
							goto skip;
						}
					}
					if (v[i].size_max < nr_bytes)
						v[i].size_max = nr_bytes;
					for (j = i; j < dim; j++)
					{
						if (v[j].f)
							SIZE = v[j].f;
						else
							SIZE = mystruct.lists[v[j].n]->size_in_bytes;
						if (v[j].size)
						{
							// replace the first characters
							if (nr_bytes < v[j].size)
							{
								for (i = 0; i < nr_bytes; i++)
									v[j].value[i] = (string + offset)[i];
								offset += nr_bytes;
								nr_bytes = 0;
								break;
							}
							else
							{
								if (nr_bytes >= SIZE)
								{
									for (i = 0; i < SIZE; i++)
										v[j].value[i] = (string + offset)[i];
									v[j].size = SIZE;
									nr_bytes -= SIZE;
									offset += SIZE;
								}
								else
								{
									for (i = 0; i < nr_bytes; i++)
										v[j].value[i] = (string + offset)[i];
									offset += nr_bytes;
									nr_bytes = 0;
									break;
								}
							}
						}
						else
						{
							// I add to the memory the given text divided by nodes
							if (nr_bytes < v[j].size)
							{
								v[j].value = (char *)malloc((nr_bytes + 1) * sizeof(char));
								strncpy(v[j].value, string + offset, nr_bytes);
								v[j].value[nr_bytes] = '\0';
								v[j].size = nr_bytes;
								offset += nr_bytes;
								nr_bytes = 0;
								break;
							}
							else
							{
								if (nr_bytes >= SIZE)
								{
									v[j].value = (char *)malloc((SIZE + 1) * sizeof(char));
									strncpy(v[j].value, string + offset, SIZE);
									v[j].value[SIZE] = '\0';
									v[j].size = SIZE;
									nr_bytes -= SIZE;
									offset += SIZE;
								}
								else
								{
									v[j].value = (char *)malloc((nr_bytes + 1) * sizeof(char));
									strncpy(v[j].value, string + offset, nr_bytes);
									v[j].value[nr_bytes] = '\0';
									v[j].size = nr_bytes;
									offset += nr_bytes;
									nr_bytes = 0;
									break;
								}
							}
							if (nr_bytes == 0)
							{
								break;
							}
						}
					}
					break;
				}
			}
			if (!ok1)
			{
				printf("Segmentation fault (core dumped)\n");
				dump_mem(mystruct, dim, v);
				goto skip;
			}
		}
		if (!strcmp(word, "DUMP_MEMORY"))
		{
			ok = 1;
			dump_mem(mystruct, dim, v);
		}
		// not all command parameters were read
		invalid:
		if (ok == 0)
			printf("INVALID_COMMAND\n");
		fgets(command, sizeof(command), stdin);
		command[strlen(command) - 1] = '\0';
		word = strtok(command, " ");
	}
	// on segmentation fault close the program
	skip:
	for (i = 0; i < mystruct.nr_lists; i++)
		Free(&mystruct.lists[i]);
	free(mystruct.lists);
	for (i = 0; i < dim; i++)
		if (v[i].size)
			free(v[i].value);
	free(v);
	return 0;
}