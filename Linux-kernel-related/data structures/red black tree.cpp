/*
   Red black tree                                                               
                                                                              
   Red black tree is a binary search tree, which is kept in balance with
   help of the nodes's colors. Root node is always black and there cannot       
   be two adjacent red nodes and every path from any node to the leaf node      
   (null node) must have the same amount of black nodes (null node or black     
   colored node). Red black trees are not as balanced as avl trees. They are    
   well suited for applications, which involve a lot of inserting and deletion  
   whereas avl trees a better for applications, where fetching data is the most 
   used operation. A red black tree is used in the Linux 2.6+ (cfs) scheduler
   for storing rb_node entities which are members of the task_struct strutures
   through the sched_entity structures. The coloring information is stored in
   in the rb_node whereas the virtual runtime (affected by the priority)
   of the process is stored in the sched_entity.                                                           
*/

#include <iostream>

using namespace std;

// rbnode definition
struct Rbnode{
	Rbnode(const int &key, Rbnode *parent):
	key(key), parent(parent){}

	void addchild(Rbnode *rbnode){
		if(rbnode->key < key){left = rbnode;}
		else{right = rbnode;}
	}

	int key;
	bool color = true;
	Rbnode *parent, *left = nullptr, *right = nullptr;
};

// Prints the red black tree in ascending order
void printrbtree(const Rbnode *rbnode){
	if(rbnode){
		printrbtree(rbnode->left);
		cout << rbnode->key;
		if(rbnode->color){cout << " red ";}
		else{cout << " black ";}
		if(rbnode->parent){cout << " " << rbnode->parent->key << endl;}
		else{cout << " <-- root node" << endl;}
		printrbtree(rbnode->right);
	}
}

// Erases the red black tree and reallocates the memory
void eraserbtree(Rbnode *&rbnode){
	if(rbnode){
		eraserbtree(rbnode->right);
		eraserbtree(rbnode->left);
		delete rbnode;
		rbnode = nullptr;
	}
}

// Gets the sibling rbnode
Rbnode *getsibling(Rbnode *rbnode){
	if(rbnode->parent){
		if(rbnode->key < rbnode->parent->key){
			return rbnode->parent->right;
		}
		return rbnode->parent->left;
	}
	return nullptr;
}

// Gets the uncle rbnode
Rbnode *getuncle(Rbnode *rbnode){
	if(rbnode->parent && rbnode->parent->parent){
		if(rbnode->parent->key < rbnode->parent->parent->key){
			return rbnode->parent->parent->right;
		}
		return rbnode->parent->parent->left;
	}
	return nullptr;
}

// Rotates rbtree to left
void leftrotate(Rbnode *rbnode, Rbnode *&rbroot){
	Rbnode *old = rbnode;
	rbnode = rbnode->right;
	rbnode->parent = old->parent;
	old->right = rbnode->left;
	if(old->right){old->right->parent = old;}
	rbnode->left = old;
	old->parent = rbnode;
	if(rbnode->parent){
		if(rbnode->key < rbnode->parent->key){
			rbnode->parent->left = rbnode;
		}
		else{
			rbnode->parent->right = rbnode;
		}
	}
	else{
		rbroot = rbnode;
	}
}

// Rotates rbtree to right
void rightrotate(Rbnode *rbnode, Rbnode *&rbroot){
	Rbnode *old = rbnode;
	rbnode = rbnode->left;
	rbnode->parent = old->parent;
	old->left = rbnode->right;
	if(old->left){old->left->parent = old;}
	rbnode->right = old;
	old->parent = rbnode;
	if(rbnode->parent){
		if(rbnode->key < rbnode->parent->key){
			rbnode->parent->left = rbnode;
		}
		else{
			rbnode->parent->right = rbnode;
		}
	}
	else{
		rbroot = rbnode;
	}
}

// Checks whether rbnode is null or black
bool isblack(Rbnode *rbnode){
	if(!rbnode || !rbnode->color){
		return true;
	}
	return false;
}

// Inserts new rbnode to the rbtree and fixes
// occured violations
void insertrbnode(const int &key, Rbnode *&rbroot){
	Rbnode *rbnode = rbroot;
	Rbnode *parent = nullptr;
	while(rbnode){
		parent = rbnode;
		if(key < rbnode->key){
			rbnode = rbnode->left;
		}
		else{
			rbnode = rbnode->right;
		}
	}

	Rbnode *rbnewnode = new Rbnode(key, parent);
	if(parent){
		parent->addchild(rbnewnode);
	}
	
	else{
		rbroot = rbnewnode;
	}

    rbnode = rbnewnode;

    while(rbnode && rbnode->parent){
    	if(rbnode->color && rbnode->parent->color){
    		Rbnode *uncle = getuncle(rbnode);
    		if(!isblack(uncle)){
    			uncle->color = false;
    			rbnode->parent->parent->color = true;
    			rbnode->parent->color = false;
    		}
    		else{
    			if(rbnode->key < rbnode->parent->key && 
    				rbnode->parent->key < rbnode->parent->parent->key){
    				rightrotate(rbnode->parent->parent, rbroot);
    				rbnode->parent->color = false;
    				getsibling(rbnode)->color = true;
    			}
    			else if(rbnode->parent->key < rbnode->key && 
    				rbnode->key < rbnode->parent->parent->key){
    				leftrotate(rbnode->parent, rbroot);
    				rightrotate(rbnode->parent, rbroot);
    				rbnode->color = false;
    				rbnode->right->color = true;
    			}
    			else if(rbnode->parent->parent->key < rbnode->parent->key && 
    				rbnode->parent->key < rbnode->key){
    				leftrotate(rbnode->parent->parent, rbroot);
    				rbnode->parent->color = false;
    				getsibling(rbnode)->color = true;
    			}
    			else{
    				rightrotate(rbnode->parent, rbroot);
    				leftrotate(rbnode->parent, rbroot);
    				rbnode->color = false;
    				rbnode->left->color = true;
    			}
    		}
    	}

    	rbnode = rbnode->parent;
    }

	rbroot->color = false;
}

// Finds the rbnode with help of key
Rbnode *getrbnode(const int &key, Rbnode *rbnode){
	if(rbnode){
		if(key < rbnode->key){
			return getrbnode(key, rbnode->left);
		}
		else if(rbnode->key < key){
			return getrbnode(key, rbnode->right);
		}
	}
	return rbnode;
}

// Gets the right most rbnode (maximum) of subtree
Rbnode *maximumrbnode(Rbnode *rbnode){
	if(rbnode && rbnode->right){
		return maximumrbnode(rbnode->right);
	}
	return rbnode;
}

// Gets the left most rbnode (minimum) of subtree
Rbnode *minimumrbnode(Rbnode *rbnode){
	if(rbnode && rbnode->left){
		return minimumrbnode(rbnode->left);
	}
	return rbnode;
}

// Utility function to get the replacing rbnode key
Rbnode *getreplacer(Rbnode *rbnode){
	if(rbnode && rbnode->right){
		return minimumrbnode(rbnode->right);
	}
	else if(rbnode && rbnode->left){
		return maximumrbnode(rbnode->left);
	}
}

// Removes rbnode from rbtre and fixes occured
// violations
void removerbnode(const int &key, Rbnode *&rbroot){
	Rbnode *rbnode = getrbnode(key, rbroot);
	if(!rbnode){
		return;
	}
	Rbnode *repl = getreplacer(rbnode);
    if(repl){
    	rbnode->key = repl->key;
    	if(rbroot == rbnode){rbroot->key = repl->key;}
    }
    rbnode = repl ? repl : rbnode;
    if(!isblack(rbnode) || !isblack(rbnode->left) || !isblack(rbnode->right)){
    	Rbnode *child = rbnode->left ? rbnode->left : rbnode->right;
    	if(rbnode->parent){
    		if(rbnode->key < rbnode->parent->key){
    			rbnode->parent->left = child;
    		}
    		else{
    			rbnode->parent->right = child;
    		}
    	}
    	if(child){child->color = false;}
    	delete rbnode;
    	rbnode = nullptr;
    	return;
    }

    Rbnode *copy = rbnode;
    while(true){
    	Rbnode *sibling = getsibling(rbnode);
    	if(!rbnode->parent){
    		rbnode->color = false;
    		break;
    	}
		else if(isblack(rbnode->parent) && !isblack(sibling) &&
			isblack(sibling->left) && isblack(sibling->right)){
			if(rbnode->key < rbnode->parent->key){
				leftrotate(rbnode->parent, rbroot);
			}
			else{
				rightrotate(rbnode->parent, rbroot);
			}
			sibling->color = false;
			rbnode->parent->color = true;
		}
		else if(isblack(rbnode->parent) && isblack(sibling) &&
			isblack(sibling->left) && isblack(sibling->right)){
			sibling->color = true;
		    rbnode = rbnode->parent;
		}
		else if(!isblack(rbnode->parent) && isblack(sibling) && 
			isblack(sibling->left) && isblack(sibling->right)){
			rbnode->parent->color = false;
		    sibling->color = true;

		    break;
		}
		else if(isblack(rbnode->parent) && isblack(sibling) &&
			((rbnode->key < rbnode->parent->key) && !isblack(sibling->left) &&
				isblack(sibling->right)) || (rbnode->parent->key < rbnode->key) &&
			isblack(sibling->left) && !isblack(sibling->right)){
			if(rbnode->key < rbnode->parent->key){
				rightrotate(sibling, rbroot);
			    sibling->color = true;
			    sibling->parent->color = false;
			}
			else if(rbnode->parent->key < rbnode->key){
				leftrotate(sibling, rbroot);
			    sibling->color = true;
			    sibling->parent->color = false;
			}
		}
		else if(isblack(sibling)){
			if(rbnode->key < rbnode->parent->key &&
				!isblack(sibling->right)){
				leftrotate(rbnode->parent, rbroot);
                sibling->color = rbnode->parent->color;
                rbnode->parent->color = false;
                sibling->right->color = false;
			}
			else if(rbnode->parent->key < rbnode->key &&
				!isblack(sibling->left)){
				rightrotate(rbnode->parent, rbroot);
                sibling->color = rbnode->parent->color;
                rbnode->parent->color = false;
                sibling->left->color = false;
			}
		}
    }

    Rbnode *child = copy->left ? copy->left : copy->right;
    if(copy->parent){
    	if(copy->key < copy->parent->key){
    		copy->parent->left = child;
        }
        else{
        	copy->parent->right = child;
        }
    }
    if(rbroot == copy){rbroot = nullptr;}
    delete copy;
    copy = nullptr;
}

int main(){

	Rbnode *root = nullptr;

	insertrbnode(10, root);
	insertrbnode(-10, root);
	insertrbnode(40, root);
	insertrbnode(-20, root);
	insertrbnode(-5, root);
	insertrbnode(20, root);
	insertrbnode(60, root);
	insertrbnode(50, root);
	insertrbnode(80, root);

	printrbtree(root);

	eraserbtree(root);

	return 0;
}
