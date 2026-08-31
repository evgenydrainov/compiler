#pragma once

#include "base_types.h"

template <typename TNode>
struct list
{
	struct iterator
	{
		TNode *node;

		iterator &operator++()
		{
			node = static_cast<TNode *>(node->next);
			return *this;
		}

		TNode * operator*()
		{
			return node;
		}

		bool operator==(iterator other)
		{
			return node == other.node;
		}

		bool operator!=(iterator other)
		{
			return node != other.node;
		}
	};

	TNode *head;
	TNode *tail;
	usize count;

	iterator begin() { return {head};    }
	iterator end()   { return {nullptr}; }
};

template <typename TNode>
inline void
list_append(list<TNode> *list,
			typename identity<TNode>::type *node)
{
	if (list->head)
	{
		list->tail->next = node;
		list->tail = node;
	}
	else
	{
		list->head = node;
		list->tail = node;
	}

	list->count++;
}
