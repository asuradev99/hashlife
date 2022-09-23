#include <bitset> 
#include <iostream>
#include <unordered_map>

using cells16 =  unsigned short int;
class Node; 


typedef union {
    Node *ptr;
    cells16 raw; 
} nodeptr; 


#define join_leaf(nw, ne, sw, se) \
    (((nw & 0b1100) | ((ne & 0b1100) >>2)) << 12) | \
    ((((nw & 0b0011) <<2 )| (ne & 0b0011)) << 8) | \
    (((sw & 0b1100) | ((se & 0b1100) >>2)) << 4) | \
    (((sw & 0b0011) <<2 ) | (se & 0b0011))


//bruh asudhashdkajhskdlhlkasjdh
bool life8(cells16 neighbor_bits, int center_bit) {
    int count = 0;
    int center = (neighbor_bits & (1 << center_bit));
    neighbor_bits &= ~(1 << center_bit);
    while (neighbor_bits)
    {
        count += (neighbor_bits & 1);    // check last bit
        neighbor_bits >>= 1;
    }
    return center ? (count == 2 || count == 3) : (count == 3);

};


class Node {
    public: 
        int depth;
        nodeptr nw, ne, sw, se; 
        nodeptr res; 

        Node* parent; 

        mutable long int hash; 

        Node(nodeptr, nodeptr, nodeptr, nodeptr, int);
        Node(cells16, cells16, cells16, cells16);
        Node(Node*, Node*, Node*, Node*, int);

        virtual void eval();
        virtual void display();


        bool operator == (const Node &other) const { 
            if(other.depth != this->depth) {
                 std::cout << "yay you prevented a segfault good job" << std::endl;

                return false; //prevent segmentation fault
            }
            if(depth == 2) {
                return (   this->nw.raw == other.nw.raw \
                        && this->ne.raw == other.ne.raw \
                        && this->sw.raw == other.sw.raw \
                        && this->se.raw == other.se.raw );
            } else if(depth == 3){
                std::cout << "Foudn potential match of depth 3" << std::endl;
                //could be a reason for incorrect results if gol has more than one parent gen per child gen
                return (   this->nw.ptr->res.raw == other.nw.ptr->res.raw\
                        && this->ne.ptr->res.raw == other.ne.ptr->res.raw \
                        && this->sw.ptr->res.raw == other.sw.ptr->res.raw\
                        && this->se.ptr->res.raw == other.se.ptr->res.raw);
            } else {
                std::cout << "Foudn potential match eeeeeeeeeeeeeeeeeeeeeeeeeee" << std::endl;
                //could be a reason for incorrect results if gol has more than one parent gen per child gen
                return (   this->nw.ptr->res.ptr == other.nw.ptr->res.ptr \
                        && this->ne.ptr->res.ptr == other.ne.ptr->res.ptr \
                        && this->sw.ptr->res.ptr == other.sw.ptr->res.ptr\
                        && this->se.ptr->res.ptr == other.se.ptr->res.ptr);
            }
        }
};

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
      // second and third and combine them using XOR
      // and bit shifting:

      int nw_hash, ne_hash, sw_hash, se_hash;     
      if(node.depth == 2) {
        nw_hash = node.nw.raw; 
        ne_hash = node.ne.raw; 
        sw_hash = node.sw.raw; 
        se_hash = node.se.raw; 
           // node.hash = 4;
      } else {

        nw_hash = node.nw.ptr->hash; 
        ne_hash = node.ne.ptr->hash; 
        sw_hash = node.sw.ptr->hash; 
        se_hash = node.se.ptr->hash; 

               // node.hash = 48;

      }
      node.hash = (hash<int>()(nw_hash) \
                ^ (((hash<int>()(ne_hash) << 1)))\
                ^ (hash<int>()(sw_hash) << 2)\
                ^ (hash<int>()(se_hash) << 3));
        // node.hash = nw_hash + ne_hash + sw_hash + se_hash; 
        // std::cout << "hash computation: " << nw_hash << " + " << ne_hash << " + " << sw_hash<< " + " << se_hash << " = " << node.hash << std::endl;

      return node.hash;
    }
  };
}

std::unordered_map<Node, nodeptr> HASHTABLE;

// Node::Node(nodeptr nw, nodeptr ne, nodeptr sw, nodeptr se, int depth) {
//     this->nw = nw;  
//     this->ne = ne; 
//     this->sw = sw; 
//     this->se = se; 
//     this->depth = depth;
//     this->hash = 0;
// }

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

void Node::eval() {

    
    //create temporary squares
    if(this->depth == 3) {
        std::cout << "Evaluating as node!" << std::endl;
        
        Node* nw_ptr = this->nw.ptr;
        Node* ne_ptr = this->ne.ptr;
        Node* sw_ptr = this->sw.ptr;
        Node* se_ptr = this->se.ptr;

        std::cout << "Main node test before children hash:" << this->hash << std::endl; 

        nw_ptr->eval();
        ne_ptr->eval();
        sw_ptr->eval();
        se_ptr->eval();

        try {
            this->res = HASHTABLE.at(*this);
            std::cout << "Found matching node value! " << std::endl;

        } catch (const std::out_of_range& oor) {
            std::cout << std::endl << "Main node test after children hash: " << this->hash << std::endl; 

            std::cout << "Failed to find hash (as node)" << std::endl;
           int n = HASHTABLE.bucket_count();
            std::cout << "umap has " << n << " buckets.\n\n";
 
         // Count no. of elements in each bucket using
            // bucket_size(position)
            for (int i = 0; i < n; i++) {
                std::cout << "Bucket " << i << " has "
                    << HASHTABLE.bucket_size(i) << " elements.\n";
            }
            Node nm = Node(nw_ptr->ne.ptr, ne_ptr->nw.ptr,  nw_ptr->se.ptr, ne_ptr->sw.ptr, depth - 1);
            nm.eval();

            Node wm = Node(nw_ptr->sw.ptr, nw_ptr->se.ptr,  sw_ptr->nw.ptr, sw_ptr->ne.ptr, depth - 1);
            wm.eval();

            Node em = Node(ne_ptr->sw.ptr, ne_ptr->se.ptr,  se_ptr->nw.ptr, se_ptr->ne.ptr, depth - 1);
            em.eval();

            Node sm = Node(sw_ptr->ne.ptr, se_ptr->nw.ptr,  sw_ptr->se.ptr, se_ptr->sw.ptr, depth - 1);
            sm.eval(); 

            Node cc = Node(nw_ptr->se.ptr, ne_ptr->sw.ptr,  sw_ptr->ne.ptr, se_ptr->nw.ptr, depth - 1);
            cc.eval();
            
            //compute inner squiare
            Node nw_inner = Node(nw_ptr->res.ptr, nm.res.ptr, wm.res.ptr, cc.res.ptr, depth - 1);
                nw_inner.eval();
            Node ne_inner = Node(nm.res.ptr, ne_ptr->res.ptr, cc.res.ptr, em.res.ptr, depth - 1);
                ne_inner.eval();
            Node sw_inner = Node(wm.res.ptr, cc.res.ptr, sw_ptr->res.ptr, sm.res.ptr, depth - 1);
                sw_inner.eval();
            Node se_inner = Node(cc.res.ptr, em.res.ptr, sm.res.ptr, se_ptr->res.ptr, depth - 1);
                se_inner.eval();

            Node *res_ = new Node(nw_inner.res.ptr, ne_inner.res.ptr, sw_inner.res.ptr, se_inner.res.ptr, depth - 1);

            this->res.ptr = res_;
            HASHTABLE.insert({*this, this->res});
            std::cout << "insertion completed successfully (node)" <<  std::endl << std::endl << std::endl;
        }
        this->res.ptr->display();

    } else {
            cells16 stream =  (cells16) join_leaf(this->nw.raw, this->ne.raw, this->sw.raw, this->se.raw);   

             std::bitset<16> x(stream);
            std::cout << "Leaf contents: " << x << std::endl;
        try {
            std::cout << "Evaluating as leaf!" << std::endl;

            //std::hash<Node> hashm = std::hash<Node>();
            std::cout << "Leaf node test before  hash:" << this->hash << std::endl; 
            this->res = HASHTABLE.at(*this);
            std::cout << "Leaf node test after  hash:" << this->hash << std::endl; 

            std::cout << "Found matching has, value: " << this->res.raw << std::endl;

        } catch (const std::out_of_range& oor) {
            std::cout << "Failed to find hash (as leaf)" << std::endl;
            std::cout << "after hash (leaf)" << this->hash << std::endl;

            std::hash<Node> hashm = std::hash<Node>();
            std::cout << "force hash: " << hashm(*this) << std::endl;
            int _nw = life8(stream & 0b1110111011100000, 10);
            int _ne = life8(stream & 0b0111011101110000, 9);
            int _sw = life8(stream & 0b0000111011101110, 6);
            int _se = life8(stream & 0b0000011101110111, 5); 

            this->res.raw =  ((((((_nw << 1) | _ne) << 1) | _sw) << 1) | _se);
        
            HASHTABLE.insert({*this, this->res});
            std::cout << "insertion completed successfullly (leaf) with hash " << this->hash << std::endl ;
        }
        std::cout << "Final hash (leaf)" << this->hash << std::endl;

    }
       std::cout << "Bucket count: " << HASHTABLE.bucket_count() << std::endl << std::endl;

}
void Node::display() {
    if (this->depth != 2) {
        std::cout << "bruh ur trying to print from a node of depth 3 or bigger" << std::endl;
        return;
    }
    cells16 stream =  (cells16) join_leaf(nw.raw, ne.raw, sw.raw, se.raw);   
    std::bitset<4> r1(stream >> 12);
    std::bitset<4> r2((stream << 4) >> 12);
    std::bitset<4> r3((stream << 8) >> 12);
    std::bitset<4> r4((stream << 12) >> 12 );

    std::cout << std::endl << r1 <<std::endl << r2 << std::endl << r3 << std::endl << r4 << std::endl;
}

// void Node::eval() {

// }
/*
11  11
00  00

11  11
00  00
*/


#include <execinfo.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
int main() {
    // Node nw = Node( 0b0000,  0b0000, 0b0000, 0b0010);
    // Node ne = Node( 0b0000, 0b0000, 0b1010, 0b0000);
    // Node sw = Node( 0b0000,  0b0100, 0b0000, 0b0000);
    // Node se = Node(  0b1000, 0b0000, 0b0000, 0b0000);
    // Node test_node =  Node(&nw, &ne, &sw, &se, 3);


    // Node nw2 = Node( 0b0000,  0b0000, 0b0000, 0b0010);
    // Node ne2 = Node( 0b0000, 0b0000, 0b1010, 0b0000);
    // Node sw2 = Node( 0b0000,  0b0100, 0b0000, 0b0000);
    // Node se2 = Node(  0b1000, 0b0000, 0b0000, 0b0000);
    // Node test_node2 =  Node(&nw2, &ne2, &sw2, &se2, 3);

     Node nw = Node( 0b0000,  0b0000, 0b0000, 0b0010);
    Node ne = Node( 0b0000, 0b0000, 0b1010, 0b0000);
    Node sw = Node( 0b0000,  0b0100, 0b0000, 0b0000);
    Node se = Node(  0b1000, 0b0000, 0b0000, 0b0000);
    Node test_node =  Node(&nw, &ne, &sw, &se, 3);


    Node nw2 = Node( 0b0000,  0b0000, 0b0000, 0b0010);
    Node ne2 = Node( 0b0000, 0b0000, 0b1010, 0b0000);
    Node sw2 = Node( 0b0000,  0b0100, 0b0000, 0b0000);
    Node se2 = Node(  0b1000, 0b0000, 0b0000, 0b0000);
    Node test_node2 =  Node(&nw2, &ne2, &sw2, &se2, 3);


    test_node.eval();
    
    std::cout << "2nd test node:" << std::endl;

    test_node2.eval();

     std::cout << test_node.res.ptr <<  " " << test_node.ne.ptr << " " << test_node.sw.ptr << " " << test_node.se.ptr << " " << std::endl;
    std::cout << test_node2.res.ptr <<  " " << test_node2.ne.ptr << " " << test_node2.sw.ptr << " " << test_node2.se.ptr << " " << std::endl;

//    nw.res.ptr = &nw;
//     std::cout << "added " << &nw << std::endl;
//     HASHTABLE.insert({nw, nw.res});




}


class hashtable {
    void *hash_start;
    void *hash_end; 
};
