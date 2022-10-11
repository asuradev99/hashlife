#include <bitset> 
#include <iostream>
#include <unordered_map>
#include <cmath>
#include <string>
#include <stdexcept>
#include <fstream>
#include <malloc.h>
#include <future>
#include <chrono>


//Variable which stores the pattern size (depth) of the pattern to load
const int PATTERN_SIZE = 10; 


//Custom type definition to describe the amount of memory one 4 by 4 cell (a leaf) uses
using cells16 =  unsigned short int;

//Class declaration of a Macrocell (called a Node)
class Node; 

//This union is used to store either a pointer to a node/macrocell or a raw leaf
//This is because a macrocell can either have pointers to other macrocells or leaves as its children.
typedef union {
    Node *ptr;
    cells16 raw; 
} nodeptr; 

//Subroutine for creating a leaf node (which stores the raw bits representing the alive and dead states of a cell) from four 2-by-2 4-bit numbers
//Each bit represents an invidiual cell. 
#define join_leaf(nw, ne, sw, se) \
    (((nw & 0b1100) | ((ne & 0b1100) >>2)) << 12) | \
    ((((nw & 0b0011) <<2 )| (ne & 0b0011)) << 8) | \
    (((sw & 0b1100) | ((se & 0b1100) >>2)) << 4) | \
    (((sw & 0b0011) <<2 ) | (se & 0b0011))

//This is the manual Life evaluation function, which evaluates a 4 by 4 leaf node one cell at a time using bit-manipulation operations
bool life8(cells16 neighbor_bits, int center_bit) {
    int count = 0;
    int center = (neighbor_bits & (1 << center_bit));
    neighbor_bits &= ~(1 << center_bit);
    while (neighbor_bits)
    {
        count += (neighbor_bits & 1);    
        neighbor_bits >>= 1;
    }
    return center ? (count == 2 || count == 3) : (count == 3);

};

//The Node (MacroCell) class definition
class Node {
    public: 
        int depth; //used to determine whether a Node is a leaf or a regular macro-cell (eg. depth = 2 -> leaf, depth > 2 -> macrocell)
        mutable nodeptr nw, ne, sw, se; //the children nodes of the current macro-cell, either leaves (cells16 raw) or nodes (Node* ptr)
        nodeptr res; //used to store the RESULT cell 

        mutable long int hash; //A variable to store the hash of this node so it can be reused to calculate parent macro-cell's hashes

        //Constructor declarations
        Node(nodeptr, nodeptr, nodeptr, nodeptr, int);
        Node(cells16, cells16, cells16, cells16);
        Node(Node*, Node*, Node*, Node*, int);

        //Hashlife evaluation function declaration
        Node* eval();

        //Utility function declarations (copying, cloning, displaying) used to set up the experiment
        Node* clone_shallow();
        Node* clone_deep();
        std::string display(int);
        void display_all(); 
        void setbit(int, int, int); 
        void separate_stream(cells16 stream);
        void zero_extend(int);
        void load_pattern(const char*);

        //This function defines how to compare two nodes for equality and is used by the hash function below.
        bool operator == (const Node &other) const { 
            if(other.depth != this->depth) {
                return false; //prevent segmentation fault by making sure the macrocell looked up by the hashtable has the same depth as the given one
            }
            if(depth == 2) {
                return (   this->nw.raw == other.nw.raw \
                        && this->ne.raw == other.ne.raw \
                        && this->sw.raw == other.sw.raw \
                        && this->se.raw == other.se.raw );
            } else {
                return (   this->nw.ptr == other.nw.ptr\
                        && this->ne.ptr == other.ne.ptr \
                        && this->sw.ptr == other.sw.ptr\
                        && this->se.ptr == other.se.ptr);
            }
        }
};

//This function defines how to hash the macro-cell into a storeable format. The hash function is not perfect, so nodes with different cell states could
//get the same hash. This is the reason for the equality function above, which checks a node against all the other nodes with the same hash to find 
//if there is an macro-cell that is actually identical to that node.
namespace std {

  template <>
  struct hash<Node>
  {
    std::size_t operator()(const Node& node) const 
    {
      using std::size_t;
      using std::hash;
      using std::string;

      // Compute individual hash values for first,
      // second and third and combine them by addition into one hash

      int nw_hash, ne_hash, sw_hash, se_hash;     
      if(node.depth == 2) {
        nw_hash = node.nw.raw; 
        ne_hash = node.ne.raw; 
        sw_hash = node.sw.raw; 
        se_hash = node.se.raw; 
      } else {

        nw_hash = node.nw.ptr->hash; 
        ne_hash = node.ne.ptr->hash; 
        sw_hash = node.sw.ptr->hash; 
        se_hash = node.se.ptr->hash; 
      }
      
      node.hash = nw_hash + ne_hash + sw_hash + se_hash; 
      return node.hash;
    }
  };
}


//Mutex declaration to limit access to the hashtable
std::mutex hash_lock;

//Global hashtable declaration
std::unordered_map<Node, Node*> HASHTABLE;

//Node constructor function definitions
Node::Node(cells16 nw, cells16 ne, cells16 sw, cells16 se) {
    this->nw.raw = nw;  
    this->ne.raw = ne; 
    this->sw.raw = sw; 
    this->se.raw = se; 
    this->depth = 2;
    this->hash = 0;
}
Node::Node(Node* nw, Node* ne, Node* sw, Node* se, int depth) {
    this->nw.ptr = nw;  
    this->ne.ptr = ne; 
    this->sw.ptr = sw; 
    this->se.ptr = se; 
    this->depth = depth;
    this->hash = 0;
}
Node::Node(nodeptr nw, nodeptr ne, nodeptr sw, nodeptr se, int depth) {
    this->nw = nw;  
    this->ne = ne; 
    this->sw = sw; 
    this->se = se; 
    this->depth = depth;
    this->hash = 0;
}
//Utility functions which clone nodes either by copying their pointers (shallow) or copying their raw contents. 
Node* Node::clone_deep() {
    if(this->depth == 2) {
        return new Node(this->nw, this->ne, this->sw, this->se, this->depth);
    } else {
        return new Node(this->nw.ptr->clone_deep(), this->ne.ptr->clone_deep(), this->sw.ptr->clone_deep(), this->se.ptr->clone_deep(), this->depth);
    }
}


Node* Node::clone_shallow() {
    return new Node(this->nw, this->ne, this->sw, this->se, this->depth);
}

//This is the  recursive HashLife evaluation function as described in my essay. 
//Unlike the regular version, this function is not a part of the node class due to 
//constraints caused by multithreading
Node* eval(Node* node) {
     
    //create temporary squares
    if(node->depth > 2) {
        node->nw.ptr = eval(node->nw.ptr);
        node->ne.ptr = eval(node->ne.ptr);
        
        if(node->depth == PATTERN_SIZE || node->depth == PATTERN_SIZE - 1) {
            //Run half of the top-level node's children and half of it's children's children 
            // on different threads (10 threads total for any pattern)
            auto sw_result = std::async(std::launch::async, &eval, node->sw.ptr);
            auto se_result = std::async(std::launch::async, &eval, node->se.ptr);

            //wait for both threads to finish running
            node->sw.ptr = sw_result.get();
            node->se.ptr = se_result.get();
        } else {
            //Instead, recursively evaluate two of the macrocell node's children
            node->sw.ptr = eval(node->sw.ptr);
            node->se.ptr = eval(node->se.ptr);
        }
    } 
    
    //Look up this node in the hashtable to see if another identical node has already stored its RESULT there
    auto search = HASHTABLE.find(*node);
    
    //If the search was successful, then copy that node into this node, including the RESULT
    if(search  != HASHTABLE.end()) {
        return search->second;
    } else {
        //Search was not successful
        if (node->depth > 2) {

            //This node is not a leaf, so proceed normally
            //This is the rest of HashLife's evaluation function
            Node* nw_ptr = node->nw.ptr;
            Node* ne_ptr = node->ne.ptr;
            Node* sw_ptr = node->sw.ptr;
            Node* se_ptr = node->se.ptr;
            
            //Create five temporary nodes (NM, WM, EM, SM, CC) and evaluate them
            Node *nm = new Node(nw_ptr->ne, ne_ptr->nw,  nw_ptr->se, ne_ptr->sw, node->depth - 1);
            Node *wm = new Node(nw_ptr->sw, nw_ptr->se,  sw_ptr->nw, sw_ptr->ne, node->depth - 1);
            Node *em = new Node(ne_ptr->sw, ne_ptr->se,  se_ptr->nw, se_ptr->ne, node->depth - 1);
            Node *sm = new Node(sw_ptr->ne, se_ptr->nw,  sw_ptr->se, se_ptr->sw, node->depth - 1);
            Node *cc = new Node(nw_ptr->se, ne_ptr->sw,  sw_ptr->ne, se_ptr->nw, node->depth - 1);
            
            nm = eval(nm);
            wm = eval(wm);
            em = eval(em);
            sm = eval(sm);
            cc = eval(cc);
            
            
            //Use the results from these temporary nodes to compute four new RESULTS (NW, NE, SW, SE)
            Node *nw_inner = new Node(nw_ptr->res, nm->res, wm->res, cc->res, node->depth - 1);
            Node *ne_inner = new Node(nm->res, ne_ptr->res, cc->res, em->res, node->depth - 1);
            Node *sw_inner = new Node(wm->res, cc->res, sw_ptr->res, sm->res, node->depth - 1);
            Node *se_inner = new Node(cc->res, em->res, sm->res, se_ptr->res, node->depth - 1);

            
            nw_inner = eval(nw_inner);
            ne_inner = eval(ne_inner);
            sw_inner = eval(sw_inner);
            se_inner = eval(se_inner);

            //Create the RESULT node and copy it into this node
            Node *res_ = new Node(nw_inner->res, ne_inner->res, sw_inner->res, se_inner->res, node->depth - 1);
            node->res.ptr = res_;

        } else {
            //This cell is a leaf, so use the naive way to evaluate GOL
            cells16 stream =  (cells16) join_leaf(node->nw.raw, node->ne.raw, node->sw.raw, node->se.raw);   
            std::hash<Node> hashm = std::hash<Node>();
            hashm(*node);
            
            //Compute the four inner cells that constitute this leaf's RESULT manually
            int _nw = life8(stream & 0b1110111011100000, 10);
            int _ne = life8(stream & 0b0111011101110000, 9);
            int _sw = life8(stream & 0b0000111011101110, 6);
            int _se = life8(stream & 0b0000011101110111, 5); 
            
            //Construct the RESULT, copy it into this leaf's RESULT variable, and insert it into the hashtable
            node->res.raw =  ((((((_nw << 1) | _ne) << 1) | _sw) << 1) | _se);
        }

        //Lock the hashtable so that no other threads can write to it
        //and insert this node (passed in by the parameter "node") into the hashtable
        std::unique_lock lock(hash_lock);            
        HASHTABLE.insert({*node, node});
        return node;
    }
}

//Recursive utility function to display the contents a node on a terminal output
std::string Node::display(int r) {
    if (this->depth == 2) {
        cells16 stream =  (cells16) join_leaf(this->nw.raw, this->ne.raw, this->sw.raw, this->se.raw); 
        std::bitset<16> x(stream);
        std::bitset<4> r1(stream >> 12);
        std::bitset<4> r2((stream << 4) >> 12);
        std::bitset<4> r3((stream << 8) >> 12);
        std::bitset<4> r4((stream << 12) >> 12 );
        if (r == 0) {
            return r1.to_string();
        } else if (r == 1) {
            return r2.to_string();
        } else if (r == 2) {
            return r3.to_string();
        } else if (r == 3) {
           return  r4.to_string();
        } else {
            throw(std::invalid_argument("row is out of range"));
        }
    } else {
        int n = pow(2, this->depth);
        int sr = r % (n / 2);
        if(r >= n / 2) {
            return (this->sw.ptr->display(sr) + this->se.ptr->display(sr));
        } else {
            return (this->nw.ptr->display(sr) + this->ne.ptr->display(sr));
        }
    }
}

//Utility function (opposite of join_leaf)
void Node::separate_stream(cells16 stream) {
    this->nw.raw = ((stream & 0b1100000000000000) >> 12) | ((stream & 0b0000110000000000) >> 10); 
    this->ne.raw = ((stream & 0b0011000000000000) >> 10) | ((stream & 0b0000001100000000) >> 8);
    this->sw.raw = ((stream & 0b0000000011000000) >> 4) | ((stream & 0b0000000000001100) >> 2);
    this->se.raw = ((stream & 0b0000000000110000) >> 2) | ((stream & 0b0000000000000011));
}

//Modifies a bit with the specified row and column coordinates in a Node (used to load pattern configurations)
void Node::setbit(int row, int col, int bit) {
    if(this->depth == 2) {
        cells16 stream =  (cells16) join_leaf(this->nw.raw, this->ne.raw, this->sw.raw, this->se.raw); 
        int index = row * 4 + col; 
        int output = bit << (15 - index); 
        if(bit == 1) {
            stream = stream | (bit << (15 - index));
            this->separate_stream(stream);
        } else if (bit == 0) {
            stream = stream & (bit << (15 - index));
        } else {
            std::cout << "Error: bit to set must be 0 or 1";
        }
    } else {
        int n = pow(2, this->depth);
        int sr = row % (n / 2);
        int sc = col % (n / 2);
        if(row >= n / 2) {
            if(col >= n / 2) {
                this->se.ptr->setbit(sr, sc, bit);
            } else {
                this->sw.ptr->setbit(sr, sc, bit);
            }
        } else {
            if(col >= n / 2) {
                this->ne.ptr->setbit(sr, sc, bit);
            } else {
                this->nw.ptr->setbit(sr, sc, bit);
            }
        }
    }
}

//Util that helps display Nodes
void Node::display_all() {
    for(int i = 0; i < pow(2, this->depth); i++) {
        std::cout << this->display(i) <<  std::endl;
    }
}

//Build an empty node with all dead cells of a given size (depth)
Node* build_zero(int depth) {
    Node* zero_node = new Node(0, 0, 0, 0);
    for (int i = 2; i < depth; i++) {
        zero_node = new Node(zero_node->clone_deep(), zero_node->clone_deep(), zero_node->clone_deep(), zero_node->clone_deep(), i + 1);
    }
    return zero_node;
}

//Extend a Node to be a power of 2 larger by padding it with 0s
Node* zero_extend(Node node) {
    Node* zero_node = build_zero(node.depth - 1);
    std::cout << node.depth << std::endl;
    std::cout << zero_node->depth << std::endl;
    Node* nw = new Node(zero_node->clone_shallow(), zero_node->clone_shallow(), zero_node->clone_shallow(), node.nw.ptr, node.depth);
    Node* ne = new Node(zero_node->clone_shallow(), zero_node->clone_shallow(), node.ne.ptr, zero_node->clone_shallow(), node.depth);
    Node* sw = new Node(zero_node->clone_shallow(), node.sw.ptr, zero_node->clone_shallow(), zero_node->clone_shallow(), node.depth);
    Node* se = new Node(node.se.ptr, zero_node->clone_shallow(), zero_node->clone_shallow(), zero_node->clone_shallow(), node.depth);

    return new Node(nw, ne, sw, se, node.depth + 1);

}

//Utility to load a RLE-encoded GOL pattern into a Node object
void Node::load_pattern(const char *name) {
    std::fstream file_stream;
    file_stream.open(name);
    int n;
    file_stream >> n;

    if(n != 1 << this->depth) {
        std::cout << "Error: wrong size: " << n << " " << (1 << this->depth) << " " << std::endl;
    }
    char c;
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            file_stream >> c;
            if(c == 'b') {
                this->setbit(i, j, 0);
            } else if (c == 'o') {
                this->setbit(i, j, 1);
            } else {
                std::cout << "Error: unknown character, skipping at "<< i << " " << j << std::endl;
            }
        }
    }
    

}


int main() {
    //Construct an empty node of the same size as the pattern to be loaded 
     Node* test_node = build_zero(5);
    
     //Load a pattern from a file (specified by name) into the test node
     test_node->load_pattern("popover.rle.txt");
    

     //include libraries to help with measuring runtime
     using std::chrono::high_resolution_clock;
     using std::chrono::duration_cast;
     using std::chrono::duration;
     using std::chrono::milliseconds;

    //record the time before and after running the evaluation function on the test Node
     auto t1 = high_resolution_clock::now();
     eval(test_node);
     auto t2 = high_resolution_clock::now();
    
    //calculate the time it took for eval() to complete
     duration<double, std::milli> ms_double = t2 - t1;
    
    //print result
     std::cout << ms_double.count() << "ms\n";

}