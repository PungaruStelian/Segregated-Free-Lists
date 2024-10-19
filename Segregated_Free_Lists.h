#ifndef Segregated_Free_Lists_h
#define Segregated_Free_Lists_h
typedef struct node_t
{
	void *addr_node;
	struct node_t *prev, *next;
} node_t;

typedef struct
{
	struct node_t *head;
	int data_size, size, size_in_bytes;
} list_t;

typedef struct
{
	int nr_lists, recons, total_bytes, bytes, nr_frag, nr_malloc, nr_free;
	list_t **lists;
	void *addr_struct;
} mystruct_t;

typedef struct
{
	int n, f, size, size_max;
	void *p;
	char *value;
} free_malloc;

list_t *create(int data_size)
{
	list_t *list = malloc(sizeof(list_t));
	list->data_size = data_size;
	list->head = NULL;
	list->size = 0;
	return list;
}

void add_nth_node(list_t *list, int n, void *addres)
{
	int i;
	node_t *new_node = malloc(sizeof(node_t));
	new_node->addr_node = addres;
	new_node->next = NULL;
	new_node->prev = NULL;
	if (list->size == 0)
	{
		list->head = new_node;
		new_node->next = new_node;
		new_node->prev = new_node;
		list->size++;
		return;
	}
	if (n == 0)
	{
		new_node->next = list->head;
		new_node->prev = list->head->prev;
		list->head->prev->next = new_node;
		list->head->prev = new_node;
		list->head = new_node;
		list->size++;
		return;
	}
	if (n >= list->size)
	{
		node_t *ult = list->head->prev;
		ult->next = new_node;
		new_node->prev = ult;
		new_node->next = list->head;
		list->head->prev = new_node;
		list->size++;
		return;
	}
	node_t *curr = list->head;
	for (i = 0; i < n; i++)
		curr = curr->next;
	new_node->next = curr;
	new_node->prev = curr->prev;
	curr->prev->next = new_node;
	curr->prev = new_node;
	list->size++;
}

node_t *remove_nth_node(list_t *list, int n)
{
	int i;
	if (n >= list->size)
		n = list->size - 1;
	node_t *curr = list->head;
	for (i = 0; i < n; i++)
		curr = curr->next;
	if (n == 0)
		list->head = curr->next;
	curr->prev->next = curr->next;
	curr->next->prev = curr->prev;
	list->size--;
	return curr;
}

// give the position of a new addres in a sorted list
int position(list_t *list, void *addr)
{
	if (list->size == 0)
		return 0;
	int i;
	node_t *x = list->head;
	for (i = 0; i < list->size; i++)
	{
		if (x->addr_node > addr)
			return i;
		x = x->next;
	}
	return list->size;
}

void Free(list_t **pp_list)
{
	int i;
	list_t *list = *pp_list;
	node_t *curr = list->head;
	node_t *urm;
	for (i = 0; i < list->size; i++)
	{
		urm = curr->next;
		free(curr);
		curr = urm;
	}
	free(list);
	*pp_list = NULL;
}

void sort(int dim, free_malloc **v)
{
	int i, j, ok = 1;
	free_malloc aux;
	for (i = 0; i < dim - 1; i++)
		if ((*v)[i].p > (*v)[i + 1].p)
			ok = 0;
	if (ok)
		return;
	for (i = 0; i < dim - 1; i++)
		for (j = i + 1; j < dim; j++)
			if ((*v)[i].p > (*v)[j].p)
			{
				aux = (*v)[i];
				(*v)[i] = (*v)[j];
				(*v)[j] = aux;
			}
}

// deletes an element from a position: k in the vector
void delete(int k, free_malloc **v, int *dim)
{
	int i;
	for (i = k; i < (*dim) - 1; i++)
		(*v)[i] = (*v)[i + 1];
	(*dim)--;
}

// move the lists after k to the right by 1
void add_list(int k, list_t ***list, int n)
{
	int i;
	for (i = n - 1; i > k; i--)
		(*list)[i] = (*list)[i - 1];
}

int is_pow_2(int x)
{
	while (x % 2 == 0 && x != 1)
		x /= 2;
	if (x == 1)
		return 1;
	return 0;
}

int next_pow_2(int x)
{
	while (!is_pow_2(x))
		x++;
	return x;
}

void dump_mem(mystruct_t mystruct, int dim, free_malloc *v)
{
	int i, j, s = 0;
	printf("+++++DUMP+++++\n");
	sort(dim, &v);
	for (i = 0; i < dim; i++)
		if (v[i].f)
			s += v[i].f;
		else
			s += mystruct.lists[v[i].n]->size_in_bytes;
	printf("Total memory: %d bytes\n", mystruct.total_bytes);
	printf("Total allocated memory: %d bytes\n", s);
	printf("Total free memory: %d bytes\n", mystruct.total_bytes - s);
	s = 0;
	for (i = 0; i < mystruct.nr_lists; i++)
		s += mystruct.lists[i]->size;
	printf("Free blocks: %d\n", s);
	printf("Number of allocated blocks: %d\n", mystruct.nr_malloc - mystruct.nr_free);
	printf("Number of malloc calls: %d\n", mystruct.nr_malloc);
	printf("Number of fragmentations: %d\n", mystruct.nr_frag);
	printf("Number of free calls: %d\n", mystruct.nr_free);
	for (i = 0; i < mystruct.nr_lists; i++)
	{
		if (mystruct.lists[i]->size)
		{
			printf("Blocks with %d bytes - %d free block(s) : ", mystruct.lists[i]->size_in_bytes, mystruct.lists[i]->size);
			node_t *node = mystruct.lists[i]->head;
			for (j = 0; j < mystruct.lists[i]->size; j++)
			{
				if (j == mystruct.lists[i]->size - 1)
					printf("%p", node->addr_node);
				else
					printf("%p ", node->addr_node);
				node = node->next;
			}
			printf("\n");
		}
	}
	printf("Allocated blocks :");
	for (i = 0; i < dim; i++)
	{
		if (v[i].p)
		{
			if (v[i].f)
				printf(" (%p - %d)", v[i].p, v[i].f);
			else
				printf(" (%p - %d)", v[i].p, mystruct.lists[v[i].n]->size_in_bytes);
		}
	}
	printf("\n-----DUMP-----\n");
}
#endif