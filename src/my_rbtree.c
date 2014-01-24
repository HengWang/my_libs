/*
 * Copyright (c) 2013, Heng Wang personal. All rights reserved.
 * 
 * Global exports header file.
 *
 * @Author:  Heng.Wang
 * @Date  :  12/24/2013
 * @Email :  wangheng.king@gmail.com
 *           king_wangheng@163.com
 * @Github:  https://github.com/HengWang/
 * @Blog  :  http://hengwang.blog.chinaunix.net
 * */

/*
  Code for handling red-black (balanced) binary trees.
  key in tree is allocated accrding to following:

  1) If size < 0 then tree will not allocate keys and only a pointer to
     each key is saved in tree.
     compare and search functions uses and returns key-pointer

  2) If size == 0 then there are two options:
       - key_size != 0 to rbtree_insert: The key will be stored in the tree.
       - key_size == 0 to rbtree_insert:  A pointer to the key is stored.
     compare and search functions uses and returns key-pointer.

  3) if key_size is given to rbtree_init then each node will continue the
     key and calls to insert_key may increase length of key.
     if key_size > sizeof(pointer) and key_size is a multiple of 8 (double
     allign) then key will be put on a 8 alligned adress. Else
     the key will be on adress (element+1). This is transparent for user
     compare and search functions uses a pointer to given key-argument.

  - If you use a free function for tree-elements and you are freeing
    the element itself, you should use key_size = 0 to rbtree_init and
    rbtree_search

  The actual key in rbtree_element is saved as a pointer or after the
  rbtree_element struct.
  If one uses only pointers in tree one can use tree_set_pointer() to
  change address of context.

  Implemented by monty.
*/

/*
  NOTE:
  tree->compare function should be ALWAYS called as
    (*tree->compare)(context, ELEMENT_KEY(tree,element), key)
  and not other way around, as
    (*tree->compare)(context, key, ELEMENT_KEY(tree,element))

  ft_boolean_search.c (at least) relies on that.
*/

#include "my_rbtree.h"


#define BLACK		1
#define RED		    0
#define DEFAULT_ALLOC_SIZE 8192
#define DEFAULT_ALIGN_SIZE 8192

static void rbtree_uninit_element(rbtree* ,rbtree_element* );
static int rbtree_walk_left_root_right(rbtree* ,rbtree_element* , rbtree_walk_action, void* );
static int rbtree_walk_right_root_left(rbtree* ,rbtree_element* , rbtree_walk_action, void* );
static void rbtree_left_rotate(rbtree_element** parent, rbtree_element* leaf);
static void rbtree_right_rotate(rbtree_element** parent, rbtree_element* leaf);
static void rb_insert(rbtree* tree, rbtree_element*** parent, rbtree_element* leaf);
static void rb_delete_fixup(rbtree* tree,rbtree_element*** parent);



MY_GLOBAL_API int rbtree_init(rbtree* tree, unsigned long default_alloc_size, unsigned long memory_limit,
                        int size, qsort_cmp2 compare, bool is_delete,
	                    rbtree_element_free free_element, const void* context)
{
    if (default_alloc_size < DEFAULT_ALLOC_SIZE)
        default_alloc_size = DEFAULT_ALLOC_SIZE;
    default_alloc_size = MY_ALIGN(default_alloc_size, DEFAULT_ALIGN_SIZE);
    memset(&tree->null_element, 0, sizeof(tree->null_element));
    tree->root = &tree->null_element;
    tree->compare = compare;
    tree->size = size > 0 ? (unsigned int) size : 0;
    tree->memory_limit = memory_limit;
    tree->free = free_element;
    tree->allocated = 0;
    tree->elements = 0;
    tree->context = context;
    tree->null_element.colour = BLACK;
    tree->null_element.left = tree->null_element.right = NULL;
    tree->flag = 0;
    if (!free_element && size >= 0 && ((unsigned int) size <= sizeof(void*) || ((unsigned int)size & (sizeof(void*) - 1))))
    {
        /*
          We know that the context doesn't have to be aligned (like if the key
          contains a double), so we can store the context combined with the
          rbtree_element.
        */
        tree->offset = sizeof(rbtree_element); /* Put key after element */
        /* Fix allocation size so that we don't lose any memory */
        default_alloc_size /= (sizeof(rbtree_element) + size);
        if (!default_alloc_size)
            default_alloc_size = 1;
        default_alloc_size *= (sizeof(rbtree_element) + size);
    }
    else
    {
        tree->offset = 0;		/* use key through pointer */
        tree->size += sizeof(void*);
    }
    if (!(tree->is_delete = is_delete))
    {
        init_alloc_root(key_memory_rbtree, &tree->mem_root, (unsigned int)default_alloc_size, 0);
        tree->mem_root.min_malloc = (sizeof(rbtree_element) + tree->size);
    }
    return 0;
}

static void rbtree_free(rbtree* tree)
{
    if (tree->root)				/* If initialized */
    {
        if (tree->is_delete)
            rbtree_uninit_element(tree, tree->root);
        else
        {
            if (tree->free)
            {
                if (tree->memory_limit)
                    (*tree->free)(NULL, FREE_INIT, tree->context);
	            rbtree_uninit_element(tree, tree->root);
                if (tree->memory_limit)
                    (*tree->free)(NULL, FREE_UNINIT, tree->context);
            }
            free_root(&tree->mem_root);
        }
    }
    tree->root = &tree->null_element;
    tree->elements = 0;
    tree->allocated = 0;
}

MY_GLOBAL_API void rbtree_uninit(rbtree* tree)
{
    rbtree_free(tree); /* my_free() mem_root if applicable */
}

MY_GLOBAL_API void rbtree_reset(rbtree* tree)
{
    /* do not free mem_root, just mark blocks as free */
    rbtree_free(tree);
}


static void rbtree_uninit_element(rbtree* tree, rbtree_element* element)
{
    if (element != &tree->null_element)
    {
        rbtree_uninit_element(tree, element->left);
        if (tree->free)
            (*tree->free)(ELEMENT_KEY(tree, element), FREE_FREE, tree->context);
        rbtree_uninit_element(tree, element->right);
        if (tree->is_delete)
            my_free(element);
    }
}


/*
  insert, search and delete of elements

  The following should be true:
    parent[0] = & parent[-1][0]->left ||
    parent[0] = & parent[-1][0]->right
*/

MY_GLOBAL_API rbtree_element* rbtree_insert(rbtree* tree, void* key, unsigned int key_size, 
                          const void* context)
{
  int cmp;
  rbtree_element* element,***parent;

  parent= tree->parents;
  *parent = &tree->root; element= tree->root;
  for (;;)
  {
    if (element == &tree->null_element ||
	(cmp = (*tree->compare)(context, ELEMENT_KEY(tree,element),
                                key)) == 0)
      break;
    if (cmp < 0)
    {
      *++parent= &element->right; element= element->right;
    }
    else
    {
      *++parent = &element->left; element= element->left;
    }
  }
  if (element == &tree->null_element)
  {
    unsigned int alloc_size=sizeof(rbtree_element)+key_size+tree->size;
    tree->allocated+=alloc_size;

    if (tree->memory_limit && tree->elements
                           && tree->allocated > tree->memory_limit)
    {
      rbtree_reset(tree);
      return rbtree_insert(tree, key, key_size, context);
    }

    key_size+=tree->size;
    if (tree->is_delete)
      element=(rbtree_element* ) my_malloc(key_memory_rbtree,
                                         alloc_size, MYF(MY_WME));
    else
      element=(rbtree_element* ) alloc_root(&tree->mem_root,alloc_size);
    if (!element)
      return(NULL);
    **parent=element;
    element->left=element->right= &tree->null_element;
    if (!tree->offset)
    {
      if (key_size == sizeof(void*))		 /* no length, save pointer */
	*((void**) (element+1))=key;
      else
      {
	*((void**) (element+1))= (void*) ((void* *) (element+1)+1);
	memcpy((uchar*) *((void* *) (element+1)),key,
	       (size_t) (key_size-sizeof(void*)));
      }
    }
    else
      memcpy((uchar*) element+tree->offset,key,(size_t) key_size);
    element->count=1;			/* May give warning in purify */
    tree->elements++;
    rb_insert(tree,parent,element);	/* rebalance tree */
  }
  else
  {
    if (tree->flag & rbtree_NO_DUPLICATES)
      return(NULL);
    element->count++;
    /* Avoid a wrap over of the count. */
    if (! element->count)
      element->count--;
  }
  DBUG_EXECUTE("check_tree", test_rb_tree(tree->root););
  return element;
}

MY_GLOBAL_API int rbtree_delete(rbtree* tree, void* key, unsigned int key_size, const void* context)
{
  int cmp,remove_colour;
  rbtree_element* element,***parent, ***org_parent, *nod;
  if (!tree->is_delete)
    return 1;					/* not allowed */

  parent= tree->parents;
  *parent= &tree->root; element= tree->root;
  for (;;)
  {
    if (element == &tree->null_element)
      return 1;				/* Was not in tree */
    if ((cmp = (*tree->compare)(context, ELEMENT_KEY(tree,element),
                                key)) == 0)
      break;
    if (cmp < 0)
    {
      *++parent= &element->right; element= element->right;
    }
    else
    {
      *++parent = &element->left; element= element->left;
    }
  }
  if (element->left == &tree->null_element)
  {
    (**parent)=element->right;
    remove_colour= element->colour;
  }
  else if (element->right == &tree->null_element)
  {
    (**parent)=element->left;
    remove_colour= element->colour;
  }
  else
  {
    org_parent= parent;
    *++parent= &element->right; nod= element->right;
    while (nod->left != &tree->null_element)
    {
      *++parent= &nod->left; nod= nod->left;
    }
    (**parent)=nod->right;		/* unlink nod from tree */
    remove_colour= nod->colour;
    org_parent[0][0]=nod;		/* put y in place of element */
    org_parent[1]= &nod->right;
    nod->left=element->left;
    nod->right=element->right;
    nod->colour=element->colour;
  }
  if (remove_colour == BLACK)
    rb_delete_fixup(tree,parent);
  if (tree->free)
    (*tree->free)(ELEMENT_KEY(tree,element), FREE_FREE, tree->context);
  tree->allocated-= sizeof(rbtree_element) + tree->size + key_size;
  my_free(element);
  tree->elements--;
  return 0;
}


MY_GLOBAL_API void* rbtree_search(rbtree* tree, void* key, const void* context)
{
  int cmp;
  rbtree_element* element=tree->root;

  for (;;)
  {
    if (element == &tree->null_element)
      return (void*) 0;
    if ((cmp = (*tree->compare)(context, ELEMENT_KEY(tree,element),
                                key)) == 0)
      return ELEMENT_KEY(tree,element);
    if (cmp < 0)
      element=element->right;
    else
      element=element->left;
  }
}

MY_GLOBAL_API void* rbtree_search_key(rbtree* tree, const void* key, 
                      rbtree_element** parents, rbtree_element*** last_pos,
                      ha_read_func flag, const void* context)
{
  int cmp;
  rbtree_element* element= tree->root;
  rbtree_element** last_left_step_parent= NULL, **last_right_step_parent= NULL;
  rbtree_element** last_equal_element= NULL;

/* 
  TODO: support for HA_READ_KEY_OR_PREV, HA_READ_PREFIX flags if needed.
*/

  *parents = &tree->null_element;
  while (element != &tree->null_element)
  {
    *++parents= element;
    if ((cmp= (*tree->compare)(context, ELEMENT_KEY(tree, element), 
			       key)) == 0)
    {
      switch (flag) {
      case HA_READ_KEY_EXACT:
      case HA_READ_KEY_OR_NEXT:
      case HA_READ_BEFORE_KEY:
	last_equal_element= parents;
	cmp= 1;
	break;
      case HA_READ_AFTER_KEY:
	cmp= -1;
	break;
      case HA_READ_PREFIX_LAST:
      case HA_READ_PREFIX_LAST_OR_PREV:
	last_equal_element= parents;
	cmp= -1;
	break;
      default:
	return NULL;
      }
    }
    if (cmp < 0) /* element < key */
    {
      last_right_step_parent= parents;
      element= element->right;
    }
    else
    {
      last_left_step_parent= parents;
      element= element->left;
    }
  }
  switch (flag) {
  case HA_READ_KEY_EXACT:
  case HA_READ_PREFIX_LAST:
    *last_pos= last_equal_element;
    break;
  case HA_READ_KEY_OR_NEXT:
    *last_pos= last_equal_element ? last_equal_element : last_left_step_parent;
    break;
  case HA_READ_AFTER_KEY:
    *last_pos= last_left_step_parent;
    break;
  case HA_READ_PREFIX_LAST_OR_PREV:
    *last_pos= last_equal_element ? last_equal_element : last_right_step_parent;
    break;
  case HA_READ_BEFORE_KEY:
    *last_pos= last_right_step_parent;
    break;
  default:
    return NULL;
  }
  return *last_pos ? ELEMENT_KEY(tree, **last_pos) : NULL;
}

/* 
  Search first (the most left) or last (the most right) tree element 
*/
MY_GLOBAL_API void* rbtree_search_edge(rbtree* tree, rbtree_element** parents, 
		       rbtree_element*** last_pos, int child_offs)
{
  rbtree_element* element= tree->root;
  
  *parents= &tree->null_element;
  while (element != &tree->null_element)
  {
    *++parents= element;
    element= ELEMENT_CHILD(element, child_offs);
  }
  *last_pos= parents;
  return **last_pos != &tree->null_element ? 
    ELEMENT_KEY(tree, **last_pos) : NULL;
}

MY_GLOBAL_API void* rbtree_search_next(rbtree* tree, rbtree_element*** last_pos, int l_offs, 
                       int r_offs)
{
  rbtree_element* x= **last_pos;
  
  if (ELEMENT_CHILD(x, r_offs) != &tree->null_element)
  {
    x= ELEMENT_CHILD(x, r_offs);
    *++*last_pos= x;
    while (ELEMENT_CHILD(x, l_offs) != &tree->null_element)
    {
      x= ELEMENT_CHILD(x, l_offs);
      *++*last_pos= x;
    }
    return ELEMENT_KEY(tree, x);
  }
  else
  {
    rbtree_element* y= *--*last_pos;
    while (y != &tree->null_element && x == ELEMENT_CHILD(y, r_offs))
    {
      x= y;
      y= *--*last_pos;
    }
    return y == &tree->null_element ? NULL : ELEMENT_KEY(tree, y);
  }
}

/*
  Expected that tree is fully balanced
  (each path from root to leaf has the same length)
*/
MY_GLOBAL_API unsigned long long rbtree_record_pos(rbtree* tree, const void* key, 
			ha_read_func flag, const void* context)
{
  int cmp;
  rbtree_element* element= tree->root;
  double left= 1;
  double right= tree->elements;

  while (element != &tree->null_element)
  {
    if ((cmp= (*tree->compare)(context, ELEMENT_KEY(tree, element), 
			       key)) == 0)
    {
      switch (flag) {
      case HA_READ_KEY_EXACT:
      case HA_READ_BEFORE_KEY:
        cmp= 1;
        break;
      case HA_READ_AFTER_KEY:
        cmp= -1;
        break;
      default:
        return HA_POS_ERROR;
      }
    }
    if (cmp < 0) /* element < key */
    {
      element= element->right;
      left= (left + right) / 2;
    }
    else
    {
      element= element->left;
      right= (left + right) / 2;
    }
  }
  switch (flag) {
  case HA_READ_KEY_EXACT:
  case HA_READ_BEFORE_KEY:
    return (unsigned long long) right;
  case HA_READ_AFTER_KEY:
    return (unsigned long long) left;
  default:
    return HA_POS_ERROR;
  }
}

MY_GLOBAL_API int tree_walk(rbtree* tree, rbtree_walk_action action, void* argument, rbtree_walk_type visit)
{
  switch (visit) {
  case LEFT_ROOT_RIGHT:
    return rbtree_walk_left_root_right(tree,tree->root,action,argument);
  case RIGHT_ROOT_LEFT:
    return rbtree_walk_right_root_left(tree,tree->root,action,argument);
  }
  return 0;			/* Keep gcc happy */
}

static int rbtree_walk_left_root_right(rbtree* tree, rbtree_element* element, rbtree_walk_action action, void* argument)
{
  int error;
  if (element->left)				/* Not null_element */
  {
    if ((error=rbtree_walk_left_root_right(tree,element->left,action,
					  argument)) == 0 &&
	(error=(*action)(ELEMENT_KEY(tree,element),
			  (unsigned long) element->count,
			  argument)) == 0)
      error=rbtree_walk_left_root_right(tree,element->right,action,argument);
    return error;
  }
  return 0;
}

static int rbtree_walk_right_root_left(rbtree* tree, rbtree_element* element, rbtree_walk_action action, void* argument)
{
  int error;
  if (element->right)				/* Not null_element */
  {
    if ((error=rbtree_walk_right_root_left(tree,element->right,action,
					  argument)) == 0 &&
	(error=(*action)(ELEMENT_KEY(tree,element),
			  (unsigned long) element->count,
			  argument)) == 0)
     error=rbtree_walk_right_root_left(tree,element->left,action,argument);
    return error;
  }
  return 0;
}


	/* Functions to fix up the tree after insert and delete */

static void rbtree_left_rotate(rbtree_element** parent, rbtree_element* leaf)
{
  rbtree_element* y;

  y=leaf->right;
  leaf->right=y->left;
  parent[0]=y;
  y->left=leaf;
}

static void rbtree_right_rotate(rbtree_element** parent, rbtree_element* leaf)
{
  rbtree_element* x;

  x=leaf->left;
  leaf->left=x->right;
  parent[0]=x;
  x->right=leaf;
}

static void rb_insert(rbtree* tree, rbtree_element*** parent, rbtree_element* leaf)
{
  rbtree_element* y,*par,*par2;

  leaf->colour=RED;
  while (leaf != tree->root && (par=parent[-1][0])->colour == RED)
  {
    if (par == (par2=parent[-2][0])->left)
    {
      y= par2->right;
      if (y->colour == RED)
      {
	par->colour=BLACK;
	y->colour=BLACK;
	leaf=par2;
	parent-=2;
	leaf->colour=RED;		/* And the loop continues */
      }
      else
      {
	if (leaf == par->right)
	{
	  rbtree_left_rotate(parent[-1],par);
	  par=leaf;			/* leaf is now parent to old leaf */
	}
	par->colour=BLACK;
	par2->colour=RED;
	rbtree_right_rotate(parent[-2],par2);
	break;
      }
    }
    else
    {
      y= par2->left;
      if (y->colour == RED)
      {
	par->colour=BLACK;
	y->colour=BLACK;
	leaf=par2;
	parent-=2;
	leaf->colour=RED;		/* And the loop continues */
      }
      else
      {
	if (leaf == par->left)
	{
	  rbtree_right_rotate(parent[-1],par);
	  par=leaf;
	}
	par->colour=BLACK;
	par2->colour=RED;
	rbtree_left_rotate(parent[-2],par2);
	break;
      }
    }
  }
  tree->root->colour=BLACK;
}

static void rb_delete_fixup(rbtree* tree, rbtree_element*** parent)
{
  rbtree_element* x,*w,*par;

  x= **parent;
  while (x != tree->root && x->colour == BLACK)
  {
    if (x == (par=parent[-1][0])->left)
    {
      w=par->right;
      if (w->colour == RED)
      {
	w->colour=BLACK;
	par->colour=RED;
	rbtree_left_rotate(parent[-1],par);
	parent[0]= &w->left;
	*++parent= &par->left;
	w=par->right;
      }
      if (w->left->colour == BLACK && w->right->colour == BLACK)
      {
	w->colour=RED;
	x=par;
	parent--;
      }
      else
      {
	if (w->right->colour == BLACK)
	{
	  w->left->colour=BLACK;
	  w->colour=RED;
	  rbtree_right_rotate(&par->right,w);
	  w=par->right;
	}
	w->colour=par->colour;
	par->colour=BLACK;
	w->right->colour=BLACK;
	rbtree_left_rotate(parent[-1],par);
	x=tree->root;
	break;
      }
    }
    else
    {
      w=par->left;
      if (w->colour == RED)
      {
	w->colour=BLACK;
	par->colour=RED;
	rbtree_right_rotate(parent[-1],par);
	parent[0]= &w->right;
	*++parent= &par->right;
	w=par->left;
      }
      if (w->right->colour == BLACK && w->left->colour == BLACK)
      {
	w->colour=RED;
	x=par;
	parent--;
      }
      else
      {
	if (w->left->colour == BLACK)
	{
	  w->right->colour=BLACK;
	  w->colour=RED;
	  rbtree_left_rotate(&par->left,w);
	  w=par->left;
	}
	w->colour=par->colour;
	par->colour=BLACK;
	w->left->colour=BLACK;
	rbtree_right_rotate(parent[-1],par);
	x=tree->root;
	break;
      }
    }
  }
  x->colour=BLACK;
}

#ifndef DBUG_OFF

	/* Test that the proporties for a red-black tree holds */

static int test_rb_tree(rbtree_element* element)
{
  int count_l,count_r;

  if (!element->left)
    return 0;				/* Found end of tree */
  if (element->colour == RED &&
      (element->left->colour == RED || element->right->colour == RED))
  {
    printf("Wrong tree: Found two red in a row\n");
    return -1;
  }
  count_l=test_rb_tree(element->left);
  count_r=test_rb_tree(element->right);
  if (count_l >= 0 && count_r >= 0)
  {
    if (count_l == count_r)
      return count_l+(element->colour == BLACK);
    printf("Wrong tree: Incorrect black-count: %d - %d\n",count_l,count_r);
  }
  return -1;
}
#endif
