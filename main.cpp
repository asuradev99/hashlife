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

        mutable long int hash; 

        Node(nodeptr, nodeptr, nodeptr, nodeptr, int);
        Node(cells16, cells16, cells16, cells16);
        Node(Node*, Node*, Node*, Node*, int);

        virtual void eval();
        virtual void display();


        bool operator == (const Node &other) const { 
            if(depth == 2) {
                return (   this->nw.raw == other.nw.raw \
                        && this->ne.raw == other.ne.raw \
                        && this->sw.raw == other.sw.raw \
                        && this->se.raw == other.se.raw );
            } else {
                return (   this->nw.ptr == other.nw.ptr \
                        && this->ne.ptr == other.ne.ptr \
                        && this->sw.ptr == other.sw.ptr \
                        && this->se.ptr == other.se.ptr );
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
      int nw_hash = node.nw.ptr->hash ? node.depth != 2 : node.nw.raw; 
      int ne_hash = node.ne.ptr->hash ? node.depth != 2 : node.ne.raw; 
      int sw_hash = node.sw.ptr->hash ? node.depth != 2 : node.sw.raw; 
      int se_hash = node.se.ptr->hash ? node.depth != 2 : node.se.raw; 

      
      node.hash = (hash<int>()(nw_hash) \
                ^ (((hash<int>()(ne_hash) << 1)) >> 1)\
                ^ (hash<int>()(sw_hash) << 1)\
                ^ (hash<int>()(se_hash) << 2));
      return node.hash;
    }
  };
}

std::unordered_map<Node, Node*> HASHTABLE;

Node::Node(nodeptr nw, nodeptr ne, nodeptr sw, nodeptr se, int depth) {
    this->nw = nw;  
    this->ne = ne; 
    this->sw = sw; 
    this->se = se; 
    this->depth = depth;
}

Node::Node(cells16 nw, cells16 ne, cells16 sw, cells16 se) {
    this->nw.raw = nw;  
    this->ne.raw = ne; 
    this->sw.raw = sw; 
    this->se.raw = se; 
    this->depth = 2;
}
Node::Node(Node* nw, Node* ne, Node* sw, Node* se, int depth) {
    this->nw.ptr = nw;  
    this->ne.ptr = ne; 
    this->sw.ptr = sw; 
    this->se.ptr = se; 
    this->depth = depth;
}

void Node::eval() {

    
    //create temporary squares
    if(this->depth == 3) {
            std::cout << "Evaluating as node!" << std::endl;

        Node* nw_ptr = this->nw.ptr;
        Node* ne_ptr = this->ne.ptr;
        Node* sw_ptr = this->sw.ptr;
        Node* se_ptr = this->se.ptr;

        nw_ptr->eval();
        ne_ptr->eval();
        sw_ptr->eval();
        se_ptr->eval();
        Node nm = Node(nw_ptr->ne, ne_ptr->nw,  nw_ptr->se, ne_ptr->sw, depth - 1);
        nm.eval();

        Node wm = Node(nw_ptr->sw, nw_ptr->se,  sw_ptr->nw, sw_ptr->ne, depth - 1);
        wm.eval();

        Node em = Node(ne_ptr->sw, ne_ptr->se,  se_ptr->nw, se_ptr->ne, depth - 1);
        em.eval();

        Node sm = Node(sw_ptr->ne, se_ptr->nw,  sw_ptr->se, se_ptr->sw, depth - 1);
        sm.eval(); 

        Node cc = Node(nw_ptr->se, ne_ptr->sw,  sw_ptr->ne, se_ptr->nw, depth - 1);
        cc.eval();
        
        //compute inner squiare
        Node nw_inner = Node(nw_ptr->res, nm.res, wm.res, cc.res, depth - 1);
            nw_inner.eval();
        Node ne_inner = Node(nm.res, ne_ptr->res, cc.res, em.res, depth - 1);
            ne_inner.eval();
        Node sw_inner = Node(wm.res, cc.res, sw_ptr->res, sm.res, depth - 1);
            sw_inner.eval();
        Node se_inner = Node(cc.res, em.res, sm.res, se_ptr->res, depth - 1);
            se_inner.eval();

        Node *res = new Node(nw_inner.res, ne_inner.res, sw_inner.res, se_inner.res, depth - 1);
        this->res.ptr = res;
        this->res.ptr->display();

    } else {
    	std::cout << "Evaluating as leaf!" << std::endl;
        std::cout << this->nw.ptr << std::endl; 
        std::cout << this->nw.raw << std::endl; 
		cells16 stream =  (cells16) join_leaf(this->nw.raw, this->ne.raw, this->sw.raw, this->se.raw);   
		std::bitset<16> x(stream);
		std::cout << x << std::endl;

		int _nw = life8(stream & 0b1110111011100000, 10);
		int _ne = life8(stream & 0b0111011101110000, 9);
		int _sw = life8(stream & 0b0000111011101110, 6);
		int _se = life8(stream & 0b0000011101110111, 5); 

		this->res.raw =  ((((((_nw << 1) | _ne) << 1) | _sw) << 1) | _se);
    }
}
void Node::display() {
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


int main() {
    
    Node nw = Node( 0b0000,  0b0000, 0b0000, 0b0010);
    Node ne = Node( 0b0000, 0b0000, 0b1010, 0b0000);
    Node sw = Node( 0b0000,  0b0100, 0b0000, 0b0000);
    Node se = Node(  0b1000, 0b0000, 0b0000, 0b0000);
    Node test_node =  Node(&nw, &ne, &sw, &se, 3);
    Node test_node2 =  Node(&nw, &ne, &sw, &se, 3);


    test_node.eval();

    std::cout << test_node.res.ptr << std::endl;
    HASHTABLE.insert({test_node, test_node.res.ptr});

    std::cout << "asd" << std::endl;
    std::cout << HASHTABLE.at(test_node2) << std::endl;
    Node* res =  HASHTABLE.at(test_node2);

    res->display();
   


}


class hashtable {
    void *hash_start;
    void *hash_end; 
};
